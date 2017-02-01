/*
 * Copyright (c) 2011, NVIDIA CORPORATION, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of NVIDIA CORPORATION nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __OV9740_H__
#define __OV9740_H__

#include <linux/ioctl.h>  /* For IOCTL macros */

#define OV9740_IOCTL_SET_MODE           _IOW('o', 1, struct ov9740_mode)
#define OV9740_IOCTL_GET_STATUS         _IOR('o', 2, int)
#define OV9740_IOCTL_SET_WHITE_BALANCE  _IOW('o', 3, int)
#define OV9740_IOCTL_SET_COLOR_EFFECT   _IOW('o', 4, int)
#define OV9740_IOCTL_SET_EXPOSURE       _IOW('o', 5, int)
#define OV9740_IOCTL_GET_EXPOSURE_TIME  _IOR('o', 6, struct ov9740_rational)

struct ov9740_mode {
	int xres;
	int yres;
};

struct ov9740_rational {
	unsigned long numerator;
	unsigned long denominator;
};

// must match with YUVCustomInfoEnum in nvomxcamerasettingsparser.h
enum {
	// start from 1 in case atoi() encounters a string
	// with no numerical sequence and returns 0
	OV9740_FlashMode = 1,
	OV9740_WhiteBalance,
	OV9740_ColorEffect,
	OV9740_Exposure,
};

// must match with NvOmxCameraUserWhitebalanceEnum in nvomxcamerasettingsparser.h
enum {
	OV9740_WhiteBalance_Auto           = 1,
	OV9740_WhiteBalance_Incandescent   = 2,
	OV9740_WhiteBalance_Fluorescent    = 3,
	OV9740_WhiteBalance_Daylight       = 5,
	OV9740_WhiteBalance_CloudyDaylight = 6,
};

// must match with NvOmxCameraUserColorEffect in nvomxcamerasettingsparser.h
enum {
	OV9740_ColorEffect_Aqua     = 1,
	OV9740_ColorEffect_Mono     = 3,
	OV9740_ColorEffect_Negative = 4,
	OV9740_ColorEffect_None     = 5,
	OV9740_ColorEffect_Sepia    = 7,
};

// must match with the exposure value in programExposureYUV() in nvomxcamerasettings.cpp
enum {
	OV9740_Exposure_Minus_Two,
	OV9740_Exposure_Minus_One,
	OV9740_Exposure_Zero,
	OV9740_Exposure_Plus_One,
	OV9740_Exposure_Plus_Two,
};

#ifdef __KERNEL__
struct ov9740_platform_data {
	int (*power_on)(void);
	int (*power_off)(void);

};
#endif /* __KERNEL__ */

#endif /* __OV9740_H__ */
