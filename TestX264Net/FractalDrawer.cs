using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;

namespace TestX264Net
{
	static class Fractal
	{
		const int mandelbrot_max_iteration = 1000;
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
			for (int Py = 0; Py < height; Py++)
			{
				double y0 = ((Py + 1 - yStart) / yRange) * 2.0 - 1.0;
				for (int Px = 0; Px < width; Px++)
				{
					double x0 = ((Px + 1 - xStart) / xRange) * 3.5 - 2.5;
					double x = 0;
					double y = 0;
					int iteration = 0;
					while (x * x + y * y < 4 && iteration < mandelbrot_max_iteration)
					{
						double xtemp = x * x - y * y + x0;
						y = 2 * x * y + y0;
						x = xtemp;
						iteration++;
					}
					SetPixelColor(rgb_data, width, height, Px, Py, iteration, Math.Sqrt(x * x + y * y));
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
					SetPixelColor(rgb_data, width, height, Px, Py, iteration, x * x + y * y);
				}
			}
			return rgb_data;
		}

		private static void SetPixelColor(byte[] rgb_data, int width, int height, int x, int y, int n, double zn)
		{
			int idx = (x * 3) + (y * width * 3);

			byte[] color;
			if (n < mandelbrot_max_iteration)
			{
				float hue = 0.95f + 20 * (float)(n + 1 - Math.Log(Math.Log(zn)) / Math.Log(2));
				color = HsvToRgb((hue % 360), 0.8f, 1f);
			}
			else
				color = new byte[3];
			rgb_data[idx] = color[0];
			rgb_data[idx + 1] = color[1];
			rgb_data[idx + 2] = color[2];
		}
		//private static Color[] MandelbrotColorTable = GenerateMandelbrotColorTable();
		//private static Color[] GenerateMandelbrotColorTable()
		//{
		//	Color[] table = new Color[mandelbrot_max_iteration + 1];
		//	table[0] = Color.Black;
		//	for (int i = 0; i < mandelbrot_max_iteration; i++)
		//	{
		//		byte[] color = new byte[3];
		//		color[0] = Clamp(i, 0, 255); // R
		//		color[1] = Clamp(i - 255, 0, 255); // G
		//		color[2] = Clamp(i - 510, 0, 255); // B
		//		table[i+1] = color;
		//	}
		//	return table;
		//}
		//private static byte Clamp(int i, int min, int max)
		//{
		//	if (i < min)
		//		return (byte)min;
		//	else if (i > max)
		//		return (byte)max;
		//	else
		//		return (byte)i;
		//}
		private static byte[] HsvToRgb(double h, double S, double V)
		{
			byte[] color = new byte[3];
			double H = h;
			while (H < 0) { H += 360; };
			while (H >= 360) { H -= 360; };
			double R, G, B;
			if (V <= 0)
			{ R = G = B = 0; }
			else if (S <= 0)
			{
				R = G = B = V;
			}
			else
			{
				double hf = H / 60.0;
				int i = (int)Math.Floor(hf);
				double f = hf - i;
				double pv = V * (1 - S);
				double qv = V * (1 - S * f);
				double tv = V * (1 - S * (1 - f));
				switch (i)
				{

					// Red is the dominant color

					case 0:
						R = V;
						G = tv;
						B = pv;
						break;

					// Green is the dominant color

					case 1:
						R = qv;
						G = V;
						B = pv;
						break;
					case 2:
						R = pv;
						G = V;
						B = tv;
						break;

					// Blue is the dominant color

					case 3:
						R = pv;
						G = qv;
						B = V;
						break;
					case 4:
						R = tv;
						G = pv;
						B = V;
						break;

					// Red is the dominant color

					case 5:
						R = V;
						G = pv;
						B = qv;
						break;

					// Just in case we overshoot on our math by a little, we put these here. Since its a switch it won't slow us down at all to put these here.

					case 6:
						R = V;
						G = tv;
						B = pv;
						break;
					case -1:
						R = V;
						G = pv;
						B = qv;
						break;

					// The color is not defined, we should throw an error.

					default:
						//LFATAL("i Value error in Pixel conversion, Value is %d", i);
						R = G = B = V; // Just pretend its black/white
						break;
				}
			}
			color[0] = ClampToByte((int)(R * 255.0));
			color[1] = ClampToByte((int)(G * 255.0));
			color[2] = ClampToByte((int)(B * 255.0));
			return color;
		}
		/// <summary>
		/// Clamp an int to 0-255
		/// </summary>
		private static byte ClampToByte(int i)
		{
			if (i < 0)
				return 0;
			else if (i > 255)
				return 255;
			else
				return (byte)i;
		}
	}
}
