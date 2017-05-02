# x264net
.NET wrapper for x264, written in C++/CLI and usable from C# and VB.NET

## License

x264net is distributed under the GPL 2.0 license

https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html

## Introduction

x264net is a very simple .NET wrapper for the x264 video encoder.  For convenience, pre-built binaries for libx264 are included in the solution.  I followed this guide to build x264 in Visual Studio: http://siliconandlithium.blogspot.com/2014/03/building-x264-on-windows-with-visual.html

## Requirements

Windows OS.  I don't know how to make this cross-platform.

Use Visual Studio 2017 to open and build the solution.  The free version is fine.

Since this wrapper is written in C++/CLI using Visual Studio 2017, you probably need the Visual C++ 2017 Redistributable package installed on any machine that is going to use this wrapper.

32 bit: https://go.microsoft.com/fwlink/?LinkId=746571  
64 bit: https://go.microsoft.com/fwlink/?LinkId=746572

## Usage

When you encode a frame with H.264, the result is one or more "NAL units" (https://en.wikipedia.org/wiki/Network_Abstraction_Layer)

X264Net provides two methods for encoding frames.  One method returns each NAL unit as a separate byte array.  The other returns all NAL units that make up the encoded frame together in one array (in case you don't care about the NAL unit boundaries in your application).

The following C# code demonstrates basic usage of the wrapper.  A complete example project is included in the solution.

```cs
// You should choose image dimensions that are divisible by 16, if you can.
const int width = 1280;
const int height = 720;

// Make sure you call Dispose() on the encoder when finished, or use a "using" block like this
using (x264net.X264Net encoder = new x264net.X264Net(width, height))
{
	// X264Net needs to be fed raw RGB data in the form of a byte array.
	byte[] rgb_data = new byte[width * height * 3];
	
	// ... you could do something here to fill the byte array with image data ...
	
	// Choose one of the encode methods based on your needs:
	
	// If you want to handle each NAL unit separately
	byte[][] NALUnits_Separate = encoder.EncodeFrame(rgb_data);
	
	// If you want just 1 byte array containing 1 or more H.264 NAL units
	byte[] NALUnits_AllTogether = encoder.EncodeFrameAsWholeArray(rgb_data);
}
```
## Advanced Usage

At this time, X264Net is a very simplistic wrapper which provides no encoding options.  If (when) you need more control over the encoding parameters, you can easily modify the wrapper to behave differently.
