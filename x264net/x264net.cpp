// This is the main DLL file.
#include "x264net.h"
#include "RGB_To_YUV420.h"
#include "stringconvert.h"
#include <exception>
namespace x264net
{
	/// <summary>
	/// <para>Create an X264Net compressor instance that accepts RGB data frames of a particular size.  Uses 1 thread for encoding unless otherwise specified.</para>
	/// <para>This instance must be disposed when you are finished with it (Call the Dispose() method, or use a C# "using" block).</para>
	/// <para>For best results, use dimensions that are divisible by 16. Otherwise, the encoder may scale your content to a different output resolution, and you may experience padding issues.</para>
	/// </summary>
	/// <param name="widthPx">The width, in pixels, of the input images.</param>
	/// <param name="heightPx">The height, in pixels, of the input images.</param>
	X264Net::X264Net(int widthPx, int heightPx) : Width(widthPx), Height(heightPx), Threads(1)
	{
		Initialize();
	}
	/// <summary>
	/// <para>Create an X264Net compressor instance that accepts RGB data frames of a particular size.</para>
	/// <para>This instance must be disposed when you are finished with it (Call the Dispose() method, or use a C# "using" block).</para>
	/// </summary>
	/// <param name="widthPx">The width, in pixels, of the input images.</param>
	/// <param name="heightPx">The height, in pixels, of the input images.</param>
	/// <param name="threads">The number of threads to use for encoding (default: 1).</param>
	X264Net::X264Net(int widthPx, int heightPx, int threads) : Width(widthPx), Height(heightPx), Threads(threads)
	{
		Initialize();
	}
	void X264Net::Initialize()
	{
		isDisposed = false;
		try
		{
			if(Width % 2 != 0 || Height % 2 != 0)
				throw gcnew Exception("Each dimension must be an even number. Provided dimensions: " + Width + " x " + Height);
			frame = 0;
			// int stride = Width * 3;
			int fps = 1;
			int colorSpace = X264_CSP_I420;

			pic_in = new x264_picture_t();
			int success = x264_picture_alloc(pic_in, colorSpace, Width, Height);
			if (success != 0)
				throw gcnew Exception("x264_picture_alloc failed with code " + success);

			pic_out = new x264_picture_t();

			param = new x264_param_t();

			// Use fastest preset, tuned for minimal latency
			x264_param_default_preset(param, "ultrafast", "zerolatency");

			param->i_csp = colorSpace;
			param->i_threads = Threads;
			param->i_width = Width;
			param->i_height = Height;
			param->i_fps_num = fps; // Frame rate has some effect on image quality ...
			param->i_fps_den = 10;

			// Intra refresh:
			param->i_keyint_max = fps;
			param->b_intra_refresh = 1;

			//Rate control:
			//param.rc.i_bitrate = 4000;
			param->rc.i_rc_method = X264_RC_CRF;
			param->rc.f_rf_constant = 25;
			param->rc.f_rf_constant_max = 35;

			//For streaming:
			param->b_repeat_headers = 1;
			param->b_annexb = 1;

			// Enforce baseline profile
			x264_param_apply_profile(param, "baseline");

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
		return (array<array<Byte>^>^)EncodeFrame_Internal(rgb_data, false);
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
		if (rgb_data->Length != Width * Height * 3)
			throw gcnew ArgumentException("Input image data has size " + rgb_data->Length + " but the expected size is " + (Width * Height * 3) + " (" + Width + " * " + Height + " * 3)", "rgb_data");

		// increment presentation timestamp; just because.
		pic_in->i_pts = frame++;

		// Convert RGB in pinned_rgb_data to YUV420P (a.k.a. YUV420 / I420) in pic_in
		{
			// When pinned_rgb_data goes out of scope, the managed array is unpinned.
			pin_ptr<Byte> pinned_rgb_data = &rgb_data[0];
			Bitmap2Yuv420p_calc2(pic_in->img.plane[0], pinned_rgb_data, Width, Height);
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