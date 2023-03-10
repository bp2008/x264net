#pragma once
namespace x264net
{
	public enum class X264Preset : __int32 { ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow, placebo };
	public enum class X264Tune : __int32 { film, animation, grain, stillimage, psnr, ssim, fastdecode, zerolatency };
	public enum class X264Profile : __int32 { baseline, main, high, high10, high422, high444 };
	//public enum class X264Colorspace : __int32 { I420, I422, I444 };

	public ref class X264Options
	{
	public:
		/// <summary>
		/// <para>The X264 preset to use for encoding.</para>
		/// </summary>
		X264Preset Preset = X264Preset::superfast;
		/// <summary>
		/// <para>The X264 tune option to use for encoding.</para>
		/// </summary>
		X264Tune Tune = X264Tune::zerolatency;
		/// <summary>
		/// <para>The X264 Profile to use for encoding.</para>
		/// </summary>
		X264Profile Profile = X264Profile::high;
		/// <summary>
		/// <para>The Colorspace to encode to.  Affects color quality.  Currently unsupported.</para>
		/// </summary>
		//X264Colorspace Colorspace = X264Colorspace::I420;

		/// <summary>
		/// <para>The width of the video, in pixels.</para>
		/// </summary>
		int Width = 0;
		/// <summary>
		/// <para>The height of the video, in pixels.</para>
		/// </summary>
		int Height = 0;
		/// <summary>
		/// <para>The number of threads to use for encoding (default: 1)</para>
		/// </summary>
		int Threads = 1;

		/// <summary>
		/// <para>The maximum bitrate to use.  Can set to 0 or below to be ignored if using variable bit rate encoding.  Must be > 0 if using CBR encoding.</para>
		/// </summary>
		int MaxBitRate = -1;

		/// <summary>
		/// <para>Smooth out bit rate changes over this many seconds.</para>
		/// </summary>
		double BitRateSmoothOverSeconds = 1;

		/// <summary>
		/// <para>If true, the encoding will use a constant bit rate.  If false, the encoding uses variable bit rate targeting a CRF (quality) value. (default: false)</para>
		/// </summary>
		bool ConstantBitRate = false;

		/// <summary>
		/// <para>(f_rf_constant) A CRF quality value to target when encoding in Variable Bit Rate mode.  Lower is better quality.  17 is extremely good quality.  23 is still quite good.  Our default is 25.</para>
		/// </summary>
		float Quality = 25;

		/// <summary>
		/// <para>(f_rf_constant_max) A 'worst-case' CRF quality value to target when encoding in Variable Bit Rate mode.  Lower is better quality, but higher makes it easier for the encoder to maintain your bit rate limit.  I think.  Set to -1 for this value to be ignored.  Default: 35</para>
		/// </summary>
		float QualityMinimum = 35;

		/// <summary>
		/// <para>The targeted FPS, important for the encoder to optimize bit rate allocation.  Fractional values are not currently supported.</para>
		/// </summary>
		int FPS = 10;

		/// <summary>
		/// <para>The number of frames between iframes.  Default: 300</para>
		/// </summary>
		int IframeInterval = 300;

		/// <summary>
		/// <para>If true, iframes are split among multiple frames, preventing the existence of a single huge iframe and smoothing out the stream, supposedly.</para>
		/// </summary>
		bool IntraRefresh = true;

		/// <summary>
		/// <para>Create an X264Options instance with default values and no Width or Height assigned.</para>
		/// </summary>
		X264Options()
		{
		}
		/// <summary>
		/// <para>Create an X264Options instance with default values and no Width or Height assigned.</para>
		/// </summary>
		/// <param name="width">The width of the video, in pixels.</param>
		/// <param name="height">The height of the video, in pixels.</param>
		X264Options(int width, int height) :Width(width), Height(height)
		{
		}
		/// <summary>
		/// <para>Create an X264Options instance with default values and no Width or Height assigned.</para>
		/// </summary>
		/// <param name="width">The width of the video, in pixels.</param>
		/// <param name="height">The height of the video, in pixels.</param>
		/// <param name="height">The number of threads to use for encoding (default: 1).</param>
		X264Options(int width, int height, int threads) :Width(width), Height(height), Threads(threads)
		{
		}
	};
}

