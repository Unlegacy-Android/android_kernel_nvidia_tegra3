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

#ifndef __OV5640_H__
#define __OV5640_H__

#include <linux/ioctl.h>  /* For IOCTL macros */

#define OV5640_IOCTL_SET_MODE           _IOW('o',  1, struct ov5640_mode)
#define OV5640_IOCTL_GET_STATUS         _IOR('o',  2, int)
#define OV5640_IOCTL_AF_TRIGGER         _IOW('o',  3, int)
#define OV5640_IOCTL_GET_AF_STATUS      _IOR('o',  4, int)
#define OV5640_IOCTL_SET_FLASH_MODE     _IOW('o',  5, int)
#define OV5640_IOCTL_GET_FLASH_STATUS   _IOR('o',  6, int)
#define OV5640_IOCTL_SET_WHITE_BALANCE  _IOW('o',  7, int)
#define OV5640_IOCTL_SET_COLOR_EFFECT   _IOW('o',  8, int)
#define OV5640_IOCTL_SET_EXPOSURE       _IOW('o',  9, int)
#define OV5640_IOCTL_CAPTURE_CMD        _IOW('o', 10, int)
#define OV5640_IOCTL_GET_EXPOSURE_TIME  _IOR('o', 11, struct ov5640_rational)

struct ov5640_mode {
	int xres;
	int yres;
};

struct ov5640_rational {
	unsigned long numerator;
	unsigned long denominator;
};

enum {
	OV5640_AF_ABORT,
	OV5640_AF_TRIGGER,
};

// must match with YUVCustomInfoEnum in nvomxcamerasettingsparser.h
enum {
	// start from 1 in case atoi() encounters a string
	// with no numerical sequence and returns 0
	OV5640_FlashMode = 1,
	OV5640_WhiteBalance,
	OV5640_ColorEffect,
	OV5640_Exposure,
};

// must match with NvOmxCameraUserFlashMode in nvomxcamerasettingsparser.h
enum {
	OV5640_FlashMode_Auto  = 1,
	OV5640_FlashMode_On    = 2,
	OV5640_FlashMode_Off   = 3,
	OV5640_FlashMode_Torch = 4,
};

// must match with NvOmxCameraUserWhitebalanceEnum in nvomxcamerasettingsparser.h
enum {
	OV5640_WhiteBalance_Auto           = 1,
	OV5640_WhiteBalance_Incandescent   = 2,
	OV5640_WhiteBalance_Fluorescent    = 3,
	OV5640_WhiteBalance_Daylight       = 5,
	OV5640_WhiteBalance_CloudyDaylight = 6,
};

// must match with NvOmxCameraUserColorEffect in nvomxcamerasettingsparser.h
enum {
	OV5640_ColorEffect_Aqua     = 1,
	OV5640_ColorEffect_Mono     = 3,
	OV5640_ColorEffect_Negative = 4,
	OV5640_ColorEffect_None     = 5,
	OV5640_ColorEffect_Sepia    = 7,
	OV5640_ColorEffect_Solarize = 8,
};

// must match with the exposure value in programExposureYUV() in nvomxcamerasettings.cpp
enum {
	OV5640_Exposure_Minus_Two,
	OV5640_Exposure_Minus_One,
	OV5640_Exposure_Zero,
	OV5640_Exposure_Plus_One,
	OV5640_Exposure_Plus_Two,
};

#ifdef __KERNEL__
struct ov5640_platform_data {
	int (*power_on)(void);
	int (*power_off)(void);

};
#endif /* __KERNEL__ */

#endif /* __OV5640_H__ */
