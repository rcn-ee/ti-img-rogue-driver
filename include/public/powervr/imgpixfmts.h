/*************************************************************************/ /*!
@File
@Title          Pixel formats
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@License        MIT

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/ /**************************************************************************/

/****************************************************************************
 **
 ** WARNING: This file is autogenerated - DO NOT EDIT.
 **
 ** See fmts_systable.txt to add new formats.
 ****************************************************************************/

#if !defined(_IMGPIXFMTS_H_)
#define _IMGPIXFMTS_H_

typedef enum _IMG_PIXFMT_
{
	IMG_PIXFMT_UNKNOWN = 0,
	IMG_PIXFMT_R10G10B10A2_UNORM = 25,
	IMG_PIXFMT_R8G8B8A8_UNORM = 32,
	IMG_PIXFMT_R8G8B8X8_UNORM = 37,
	IMG_PIXFMT_D32_FLOAT = 51,
	IMG_PIXFMT_D24_UNORM_X8_TYPELESS = 58,
	IMG_PIXFMT_R8G8_UNORM = 62,
	IMG_PIXFMT_D16_UNORM = 69,
	IMG_PIXFMT_R8_UNORM = 76,
	IMG_PIXFMT_S8_UINT = 81,
	IMG_PIXFMT_B5G6R5_UNORM = 85,
	IMG_PIXFMT_R5G6B5_UNORM = 86,
	IMG_PIXFMT_B5G5R5A1_UNORM = 87,
	IMG_PIXFMT_B5G5R5X1_UNORM = 88,
	IMG_PIXFMT_B8G8R8A8_UNORM = 89,
	IMG_PIXFMT_B8G8R8X8_UNORM = 90,
	IMG_PIXFMT_L8_UNORM = 136,
	IMG_PIXFMT_L8A8_UNORM = 138,
	IMG_PIXFMT_B4G4R4A4_UNORM = 145,
	IMG_PIXFMT_R8G8B8_UNORM = 160,
	IMG_PIXFMT_UYVY = 171,
	IMG_PIXFMT_VYUY = 172,
	IMG_PIXFMT_YUYV = 173,
	IMG_PIXFMT_YVYU = 174,
	IMG_PIXFMT_YVU420_2PLANE = 175,
	IMG_PIXFMT_YUV420_2PLANE = 176,
	IMG_PIXFMT_YVU420_2PLANE_MACRO_BLOCK = 177,
	IMG_PIXFMT_YUV420_3PLANE = 178,
	IMG_PIXFMT_YVU420_3PLANE = 179,
	IMG_PIXFMT_V8U8Y8A8 = 184,
	IMG_PIXFMT_YVU8_422_2PLANE_PACK8 = 201,
	IMG_PIXFMT_YVU10_444_1PLANE_PACK10 = 203,
	IMG_PIXFMT_YUV8_422_2PLANE_PACK8 = 207,
	IMG_PIXFMT_YUV8_444_3PLANE_PACK8 = 208,
	IMG_PIXFMT_YUV10_420_2PLANE_PACK10 = 211,
	IMG_PIXFMT_YUV10_422_2PLANE_PACK10 = 213,
	IMG_PIXFMT_YVU8_420_2PLANE_PACK8_P = 245,
	IMG_PIXFMT_YUV8_420_2PLANE_PACK8_P = 249,
	IMG_PIXFMT_UYVY10_422_1PLANE_PACK10_CUST1 = 252,
} IMG_PIXFMT;



#endif /* _IMGPIXFMTS_H_ */
