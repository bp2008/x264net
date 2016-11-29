using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;

namespace TestX264Net
{
	static class Fractal
	{
		/// <summary>
		/// Draw the Mandelbrot set.
		/// Based on: https://en.wikipedia.org/wiki/Mandelbrot_set
		/// </summary>
		public static byte[] MandelBrot(int width, int height, double zoomFactor = 1, double zoomOnX = 0.380000165, double zoomOnY = 0.375)
		{
			if (width <= 0)
				throw new ArgumentException("width " + width + " is not > 0", "width");
			if (height <= 0)
				throw new ArgumentException("height " + height + " is not > 0", "height");

			byte[] rgb_data = new byte[width * height * 3];
			
			double xRange = width * zoomFactor;
			double yRange = height * zoomFactor;
			int xStart = (int)Math.Round((width - xRange) * zoomOnX);
			int yStart = (int)Math.Round((height - yRange) * zoomOnY);
			int max_iteration = 1000;
			for (int Py = 0; Py < height; Py++)
			{
				for (int Px = 0; Px < width; Px++)
				{
					double x0 = ((Px + 1 - xStart) / xRange) * 3.5 - 2.5;
					double y0 = ((Py + 1 - yStart) / yRange) * 2d - 1d;
					double x = 0;
					double y = 0;
					int iteration = 0;
					while (x * x + y * y < 4 && iteration < max_iteration)
					{
						double xtemp = x * x - y * y + x0;
						y = 2 * x * y + y0;
						x = xtemp;
						iteration++;
					}
					SetPixelColor(rgb_data, width, height, Px, Py, iteration);
				}
			}
			return rgb_data;
		}
		/// <summary>
		/// Based on: https://en.wikipedia.org/wiki/Multibrot_set
		/// This "MultiBrot" implementation is really slow, but basic usage is here:
		/// byte[] rgb_data = Fractal.MultiBrot(width, height, (float)((frame / 10.0))); 
		/// </summary>
		/// <param name="width"></param>
		/// <param name="height"></param>
		/// <param name="d"></param>
		/// <returns></returns>
		public static byte[] MultiBrot(int width, int height, float d)
		{
			if (d == 0)
				d = 0.0001f;
			if (width <= 0)
				throw new ArgumentException("width " + width + " is not > 0", "width");
			if (height <= 0)
				throw new ArgumentException("height " + height + " is not > 0", "height");

			byte[] rgb_data = new byte[width * height * 3];
			int max_iteration = 1000;
			for (int Py = 0; Py < height; Py++)
			{
				for (int Px = 0; Px < width; Px++)
				{
					float x0 = ((Px + 1) / (float)width) * 3.5f - 2.5f;
					float y0 = ((Py + 1) / (float)height) * 2f - 1f;
					float x = 0;
					float y = 0;
					int iteration = 0;
					while (x * x + y * y < d * d && iteration < max_iteration)
					{
						double tmp1 = Math.Pow((x * x) + (y * y), d / 2);
						double tmp2 = d * Math.Atan2(y, x);
						double xtmp = (tmp1 * Math.Cos(tmp2)) + x0;
						y = (float)(tmp1 * Math.Sin(tmp2) + y0);
						x = (float)xtmp;
						iteration++;
					}
					SetPixelColor(rgb_data, width, height, Px, Py, iteration);
				}
			}
			return rgb_data;
		}

		private static void SetPixelColor(byte[] rgb_data, int width, int height, int x, int y, int v)
		{
			int idx = (x * 3) + (y * width * 3);
			rgb_data[idx] = rgb_data[idx + 1] = rgb_data[idx + 2] = (byte)Math.Min(255, v);
		}
	}
}
