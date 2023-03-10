// This is the main DLL file.
#include "x264net.h"
#include "RGB_To_YUV420.h"
#include "stringconvert.h"
#include <exception>
namespace x264net
{
	/// <summary>
	/// <para>Create an X264Net compressor instance that accepts RGB data frames of a particular size.</para>
	/// <para>This instance must be disposed when you are finished with it (Call the Dispose() method, or use a C# "using" block).</para>
	/// <para>For best results, use dimensions that are divisible by 16. Otherwise, the encoder may scale your content to a different output resolution, and you may experience padding issues.</para>
	/// </summary>
	/// <param name="options">The encoding options to use.</param>
	X264Net::X264Net(X264Options^ options) : Options(options)
	{
		Initialize();
	}
	void X264Net::Initialize()
	{
		isDisposed = false;
		try
		{
			if (Options->Width % 2 != 0 || Options->Height % 2 != 0)
				throw gcnew Exception("Each dimension must be an even number. Provided dimensions: " + Options->Width + " x " + Options->Height);
			if (Options->Threads < 1)
				Options->Threads = 1;
			if (Options->Threads > System::Environment::ProcessorCount * 2)
				Options->Threads = System::Environment::ProcessorCount * 2;
			frame = 0;
			// int stride = Width * 3;
			int fps = 1;

			int colorSpace = X264_CSP_I420;
			//if (Options->Colorspace == X264Colorspace::I420)
			//	colorSpace = X264_CSP_I420;
			//else if (Options->Colorspace == X264Colorspace::I422)
			//	colorSpace = X264_CSP_I422;
			//else if (Options->Colorspace == X264Colorspace::I444)
			//	colorSpace = X264_CSP_I444;

			pic_in = new x264_picture_t();
			int success = x264_picture_alloc(pic_in, colorSpace, Options->Width, Options->Height);
			if (success != 0)
				throw gcnew Exception("x264_picture_alloc failed with code " + success);

			pic_out = new x264_picture_t();

			param = new x264_param_t();

			x264_param_default_preset(param, getStdString(Options->Preset.ToString()).c_str(), getStdString(Options->Tune.ToString()).c_str());

			param->i_csp = colorSpace;
			param->i_threads = Options->Threads;
			param->i_width = Options->Width;
			param->i_height = Options->Height;
			param->i_fps_num = Options->FPS; // Frame rate has some effect on image quality ...
			param->i_fps_den = 1;

			// Intra refresh:
			param->i_keyint_max = Options->IframeInterval;
			param->b_intra_refresh = Options->IntraRefresh ? 1 : 0;

			//Rate control:
			if (Options->MaxBitRate > 0)
				param->rc.i_vbv_max_bitrate = Options->MaxBitRate;
			if (Options->BitRateSmoothOverSeconds < 0.001)
				Options->BitRateSmoothOverSeconds = 0.001;
			if (Options->BitRateSmoothOverSeconds > 10)
				Options->BitRateSmoothOverSeconds = 10;
			param->rc.i_vbv_buffer_size = (int)(Options->MaxBitRate * Options->BitRateSmoothOverSeconds);
			if (Options->ConstantBitRate)
			{
				param->rc.i_rc_method = X264_RC_ABR;
				if (Options->MaxBitRate > 0)
					param->rc.i_bitrate = Options->MaxBitRate;
				else
					throw gcnew Exception("No MaxBitRate value was specified when using ConstantBitRate encoding");
			}
			else
			{
				param->rc.i_rc_method = X264_RC_CRF;
				param->rc.f_rf_constant = Options->Quality;
				if (Options->QualityMinimum > -1)
					param->rc.f_rf_constant_max = Options->QualityMinimum;
			}

			//For streaming:
			param->b_repeat_headers = 1;
			param->b_annexb = 1;

			// Enforce baseline profile
			x264_param_apply_profile(param, getStdString(Options->Profile.ToString()).c_str());

			// Open Encoder
			encoder = x264_encoder_open(param);
		}
		catch (Exception^)
		{
			throw;
		}
		catch (std::exception const & e)
		{
			throw gcnew Exception(getSystemString(e.what()));
		}
		catch (...)
		{
			throw gcnew Exception("Unknown exception caught");
		}
	}
	X264Net::~X264Net()
	{
		// This method appears as "Dispose()" in C#.
		// We would dispose of managed data here, if we had any that needed disposing.
		this->!X264Net();
	}
	X264Net::!X264Net()
	{
		if (isDisposed)
			return;

		// This is the Finalizer, for disposing of unmanaged data.  Managed data should not be disposed here, because managed classes may have already been garbage collected by the time this runs.
		try
		{
			x264_encoder_close(encoder);
		}
		catch (...)
		{
		}
		try
		{
			x264_picture_clean(pic_in);
		}
		catch (...)
		{
		}
		delete param;
		// delete encoder; // Apparently we shouldn't try to delete this pointer because we didn't use "new"
		delete pic_in;
		delete pic_out;

		isDisposed = true;
	}
	/// <summary>
	/// <para>Encodes a frame, returning an array of H.264 NAL units which are the encoded form of the frame.</para>
	/// </summary>
	/// <param name="rgb_data">A byte array containing raw RGB data (3 bytes / 24 bits per pixel).  This array's length must be equal to Width * Height * 3.</param>
	array<array<Byte>^>^ X264Net::EncodeFrame(array<Byte>^ rgb_data)
	{
		return (array<array<Byte>^>^)EncodeFrame_Internal(rgb_data, true);
	}
	/// <summary>
	/// <para>Encodes a frame, returning a single byte array containing one or more H.264 NAL units which are the encoded form of the frame.</para>
	/// </summary>
	/// <param name="rgb_data">A byte array containing raw RGB data (3 bytes / 24 bits per pixel).  This array's length must be equal to Width * Height * 3.</param>
	array<Byte>^ X264Net::EncodeFrameAsWholeArray(array<Byte>^ rgb_data)
	{
		return (array<Byte>^)EncodeFrame_Internal(rgb_data, false);
	}
	Object^ X264Net::EncodeFrame_Internal(array<Byte>^ rgb_data, bool eachNalGetsOwnArray)
	{
		if (rgb_data->Length != Options->Width * Options->Height * 3)
			throw gcnew ArgumentException("Input image data has size " + rgb_data->Length + " but the expected size is " + (Options->Width * Options->Height * 3) + " (" + Options->Width + " * " + Options->Height + " * 3)", "rgb_data");

		// increment presentation timestamp; just because.
		pic_in->i_pts = frame++;

		// Convert RGB in pinned_rgb_data to YUV420P (a.k.a. YUV420 / I420) in pic_in
		{
			// When pinned_rgb_data goes out of scope, the managed array is unpinned.
			pin_ptr<Byte> pinned_rgb_data = &rgb_data[0];
			Bitmap2Yuv420p_calc2(pic_in->img.plane[0], pinned_rgb_data, Options->Width, Options->Height);
		}

		// Encode frame
		x264_nal_t* nals;
		int i_nals;
		int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, pic_in, pic_out);
		if (frame_size >= 0)
		{
			// Copy encoded frame into managed array(s)
			if (eachNalGetsOwnArray)
			{
				array<array<Byte>^>^ managed_NAL_array = gcnew array<array<Byte>^>(i_nals);
				for (int i = 0; i < i_nals; i++)
				{
					array<Byte>^ managed_NAL = gcnew array<Byte>(nals[i].i_payload);
					System::Runtime::InteropServices::Marshal::Copy((IntPtr)nals[i].p_payload, managed_NAL, 0, nals[i].i_payload);
					managed_NAL_array[i] = managed_NAL;
				}
				return managed_NAL_array;
			}
			else
			{
				int totalDataSize = 0;
				for (int i = 0; i < i_nals; i++)
					totalDataSize += nals[i].i_payload;
				array<Byte>^ managed_NAL_array = gcnew array<Byte>(totalDataSize);
				int copiedSoFar = 0;
				for (int i = 0; i < i_nals; i++)
				{
					System::Runtime::InteropServices::Marshal::Copy((IntPtr)nals[i].p_payload, managed_NAL_array, copiedSoFar, nals[i].i_payload);
					copiedSoFar += nals[i].i_payload;
				}
				return managed_NAL_array;
			}
		}
		else
			throw gcnew Exception("x264_encoder_encode failed with return value " + frame_size);
	}
}