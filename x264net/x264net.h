// x264net.h

#pragma once
#include "stdint.h"
#include "lib/x264/include/x264.h"
#include "X264Options.h"

using namespace System;

namespace x264net {

	/// <summary>
	/// X264Net, a .NET wrapper for x264.  Each instance must be disposed when you are finished with it.
	/// </summary>
	public ref class X264Net
	{
	private:
		x264_param_t* param;
		x264_t* encoder;
		x264_picture_t* pic_in;
		x264_picture_t* pic_out;
		int64_t frame;

		bool isDisposed;
		!X264Net();
		void Initialize();
		Object^ EncodeFrame_Internal(array<Byte>^ rgb_data, bool eachNalGetsOwnArray);
	public:
		X264Options^ Options;

		X264Net(X264Options^ options);
		~X264Net();
		array<array<Byte>^>^ EncodeFrame(array<Byte>^ rgb_data);
		array<Byte>^ EncodeFrameAsWholeArray(array<Byte>^ rgb_data);
	};
}
