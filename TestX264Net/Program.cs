using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace TestX264Net
{
	/// <summary>
	/// A demonstration program for x264net, written in C#.  This program generates a short video of a mandelbrot set, zooming in each frame to create change in the video.
	/// 
	/// A video is saved in raw H.264 format to the file "out.h264" in the current working directory.  It can be opened in VLC Media Player, and VLC will 
	/// </summary>
	class Program
	{
		static void Main(string[] args)
		{
			// Due to the complexity of generating high resolution mandelbrot sets, it is suggested to use a low resolution.
			// For best results, the width and height should both be divisible by 16. (important note: 1080 is not divisible by 16)
			const int width = 320;
			const int height = 192;

			// The number of frames to generate and encode
			const int framesToGenerate = 100;

			// This affects the speed and zoom distance of the mandelbrot graphic animation.
			const double maxZoomPower = 24;

			using (FileStream fsOut = new FileStream("out.h264", FileMode.Create, FileAccess.Write, FileShare.Read))
			{
				// Create an X264Net instance. Be sure to dispose it when finished, either by calling Dispose() on it, or by creating it in a using block.
				using (x264net.X264Net encoder = new x264net.X264Net(width, height))
				{
					for (int frame = 0; frame < framesToGenerate; frame++)
					{
						if (frame % 5 == 0)
							Console.WriteLine("Building frame " + frame + " / " + framesToGenerate);
						
						// Get a video frame in raw RGB format
						double zoomDepthThisFrame = Math.Pow(2, ((frame / (double)framesToGenerate) * maxZoomPower) + 1) - 1;
						byte[] rgb_data = Fractal.MandelBrot(width, height, zoomDepthThisFrame);

						// STEP 2) Encode a frame by calling EncodeFrame or EncodeFrameAsWholeArray, depending on if you care to have NAL units separated for you.
						byte[] buf = encoder.EncodeFrameAsWholeArray(rgb_data);

						// Write frame to file
						fsOut.Write(buf, 0, buf.Length);
					}
				}
			}
		}
	}
}
