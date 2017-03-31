/*
 * s5k5ca_regs.h
 *
 * Register definitions for the S5K5CA Sensor.
 *
 * Leverage MT9P012.h
 *
 * Copyright (C) 2008 Hewlett Packard.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#ifndef S5K5CAG_REGS_H
#define S5K5CAG_REGS_H

#include <media/s5k5cag.h>
#include <media/yuv_sensor_cl2n.h>

struct s5k5cag_reg {
	u16 addr;
	u16 val;
};

/* Effect Settings */
static struct s5k5cag_reg colorfx_none[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x021E},
	{0x0F12, 0x0000},
	{SENSOR_TABLE_END, 0x00}
};
//static struct s5k5cag_reg colorfx_bw[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x021E},
//	{0x0F12, 0x0001},
//	{SENSOR_TABLE_END, 0x00}
//};
static struct s5k5cag_reg colorfx_sepia[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x021E},
	{0x0F12, 0x0004},
	{SENSOR_TABLE_END, 0x00}
};
static struct s5k5cag_reg colorfx_negative[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x021E},
	{0x0F12, 0x0003},
	{SENSOR_TABLE_END, 0x00}
};
//static struct s5k5cag_reg colorfx_aqua[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x021E},
//	{0x0F12, 0x0005},
//	{SENSOR_TABLE_END, 0x00}
//};

//static struct s5k5cag_reg colorfx_sketch[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x021E},
//	{0x0F12, 0x0006},
//	{SENSOR_TABLE_END, 0x00}
//};
//
//static struct s5k5cag_reg colorfx_emboss[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x021E},
//	{0x0F12, 0x0008},
//	{SENSOR_TABLE_END, 0x00}
//};
/* S5K5CA no support. */
/*
static struct s5k5cag_reg colorfx_solarize[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x021E},
	{0x0F12, 0x0005},
	{SENSOR_TABLE_END, 0x00}
};*/


static struct s5k5cag_reg wb_auto_init[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x246E},
	{0x0F12, 0x0001},
	{SENSOR_TABLE_END, 0x00}
};

/* White Balance settings */
static struct s5k5cag_reg wb_auto[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x246E},
	{0x0F12, 0x0001},
	{0x002A, 0x04D2}, // &*&*&*CJ_20120521 for white balance settings
	{0x0F12, 0x067F}, // MWB disable
	{SENSOR_TABLE_END, 0x00}
};
static struct s5k5cag_reg wb_incandescent[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x246E},
	{0x0F12, 0x0000},
	{0x002A, 0x04D2}, // &*&*&*CJ_20120521 for white balance settings
	{0x0F12, 0x0677}, // MWB enable
	{0x002A, 0x04A0},
	{0x0F12, 0x0400},
	{0x0F12, 0x0001},
	{0x0F12, 0x0400},
	{0x0F12, 0x0001},
	{0x0F12, 0x0940},
	{0x0F12, 0x0001},
	{SENSOR_TABLE_END, 0x00}
};
static struct s5k5cag_reg wb_fluorescent[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x246E},
	{0x0F12, 0x0000},
	{0x002A, 0x04D2}, // &*&*&*CJ_20120521 for white balance settings
	{0x0F12, 0x0677}, // MWB enable
	{0x002A, 0x04A0},
	{0x0F12, 0x0575},
	{0x0F12, 0x0001},
	{0x0F12, 0x0400},
	{0x0F12, 0x0001},
	{0x0F12, 0x0800},
	{0x0F12, 0x0001},
	{SENSOR_TABLE_END, 0x00}
};
static struct s5k5cag_reg wb_daylight[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x246E},
	{0x0F12, 0x0000},
	{0x002A, 0x04D2}, // &*&*&*CJ_20120521 for white balance settings
	{0x0F12, 0x0677}, // MWB enable
	{0x002A, 0x04A0},
	{0x0F12, 0x05E0},
	{0x0F12, 0x0001},
	{0x0F12, 0x0400},
	{0x0F12, 0x0001},
	{0x0F12, 0x0530},
	{0x0F12, 0x0001},
	{SENSOR_TABLE_END, 0x00}
};
//static struct s5k5cag_reg wb_shade[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x246E},
//	{0x0F12, 0x0000},
//	{0x002A, 0x04D2}, // &*&*&*CJ_20120521 for white balance settings
//	{0x0F12, 0x0677}, // MWB enable
//	{0x002A, 0x04A0},
//	{0x0F12, 0x0696},
//	{0x0F12, 0x0001},
//	{0x0F12, 0x0400},
//	{0x0F12, 0x0001},
//	{0x0F12, 0x044E},
//	{0x0F12, 0x0001},
//	{SENSOR_TABLE_END, 0x00}
//};
static struct s5k5cag_reg wb_cloudy[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x246E},
	{0x0F12, 0x0000},
	{0x002A, 0x04D2}, // &*&*&*CJ_20120521 for white balance settings
	{0x0F12, 0x0677}, // MWB enable
	{0x002A, 0x04A0},
	{0x0F12, 0x0740},
	{0x0F12, 0x0001},
	{0x0F12, 0x0400},
	{0x0F12, 0x0001},
	{0x0F12, 0x0460},
	{0x0F12, 0x0001},
	{SENSOR_TABLE_END, 0x00}
};
//static struct s5k5cag_reg wb_horizon[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x246E},
//	{0x0F12, 0x0000},
//	{0x002A, 0x04D2}, // &*&*&*CJ_20120521 for white balance settings
//	{0x0F12, 0x0677}, // MWB enable
//	{0x002A, 0x04A0},
//	{0x0F12, 0x066E},
//	{0x0F12, 0x0001},
//	{0x0F12, 0x0400},
//	{0x0F12, 0x0001},
//	{0x0F12, 0x0476},
//	{0x0F12, 0x0001},
//	{SENSOR_TABLE_END, 0x00}
//};
//static struct s5k5cag_reg wb_tungsten[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x246E},
//	{0x0F12, 0x0000},
//	{0x002A, 0x04D2}, // &*&*&*CJ_20120521 for white balance settings
//	{0x0F12, 0x0677}, // MWB enable
//	{0x002A, 0x04A0},
//	{0x0F12, 0x0400},
//	{0x0F12, 0x0001},
//	{0x0F12, 0x0400},
//	{0x0F12, 0x0001},
//	{0x0F12, 0x0940},
//	{0x0F12, 0x0001},
//	{SENSOR_TABLE_END, 0x00}
//};
//static struct s5k5cag_reg ColorEffect_None[] = {
//	{0x0028,	0x7000},
//	{0x002A,	0x021E},
//	{0x0F12,	0x0000},
//
//	{0x002A,	0x1000},
//	{0x0F12,	0x0001},
//	{SENSOR_TABLE_END, 0x00}
//};
//
static struct s5k5cag_reg ColorEffect_Mono[] = {
	{0x0028,	0x7000},
	{0x002A,	0x021E},
	{0x0F12,	0x0001},

	{0x002A,	0x1000},
	{0x0F12,	0x0001},
	{SENSOR_TABLE_END, 0x00}
};
//
//static struct s5k5cag_reg ColorEffect_Sepia[] = {
//	{0x0028,	0x7000},
//	{0x002A,	0x021E},
//	{0x0F12,	0x0003},
//
//	{0x002A,	0x1000},
//	{0x0F12,	0x0001},
//	{SENSOR_TABLE_END, 0x0000}
//};
//
//static struct s5k5cag_reg ColorEffect_Negative[] = {
//	{0x0028,	0x7000},
//	{0x002A,	0x021E},
//	{0x0F12,	0x0002},
//
//	{0x002A,	0x1000},
//	{0x0F12,	0x0001},
//
//	{SENSOR_TABLE_END, 0x0000}
//};
//
static struct s5k5cag_reg ColorEffect_Solarize[] = {
	{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg ColorEffect_Posterize[] = {
	{SENSOR_TABLE_END, 0x00}
};

//static struct s5k5cag_reg Whitebalance_Auto[] = {
//
////	{0xFCFC,	0x7000},
//	{0x0028,	0x7000},
//	{0x002A,	0x04D2},
//	{0x0F12,	0x067F},
//
//	{0x002A,	0x1000},
//	{0x0F12,	0x0001},
//
//	{SENSOR_TABLE_END, 0x00}
//};
//
//static struct s5k5cag_reg Whitebalance_Incandescent[] = {
//
//	//{0xFCFC,	0x7000},
//	{0x0028,	0x7000},
//	{0x002A,	0x04D2},
//	{0x0F12,	0x0677},
//
//	{0x002A,	0x04A0},
//	{0x0F12,	0x0220},
//	{0x002A,	0x04A2},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x04A4},
//	{0x0F12,	0x0100},
//	{0x002A,	0x04A6},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x04A8},
//	{0x0F12,	0x0221},
//	{0x002A,	0x04AA},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x1000},
//	{0x0F12,	0x0001},
//
//	{SENSOR_TABLE_END, 0x00}
//};
//
//static struct s5k5cag_reg Whitebalance_Daylight[] = {
//
//	//{0xFCFC,	0x7000},
//	{0x0028,	0x7000},
//	{0x002A,	0x04D2},
//	{0x0F12,	0x0677},
//
//	{0x002A,	0x04A0},
//	{0x0F12,	0x0164},
//	{0x002A,	0x04A2},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x04A4},
//	{0x0F12,	0x0100},
//	{0x002A,	0x04A6},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x04A8},
//	{0x0F12,	0x018C},
//	{0x002A,	0x04AA},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x1000},
//	{0x0F12,	0x0001},
//
//	{SENSOR_TABLE_END, 0x00}
//};
//
//static struct s5k5cag_reg Whitebalance_Fluorescent[] = {
//
////	{0xFCFC,	0x7000},
//	{0x0028,	0x7000},
//	{0x002A,	0x04D2},
//	{0x0F12,	0x0677},
//
//	{0x002A,	0x04A0},
//	{0x0F12,	0x01F0},
//	{0x002A,	0x04A2},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x04A4},
//	{0x0F12,	0x0100},
//	{0x002A,	0x04A6},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x04A8},
//	{0x0F12,	0x01F0},
//	{0x002A,	0x04AA},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x1000},
//	{0x0F12,	0x0001},
//
//	{SENSOR_TABLE_END, 0x00}
//};
//
//static struct s5k5cag_reg Whitebalance_CloudyDaylight[] = {
//
////	{0xFCFC,	0x7000},
//	{0x0028,	0x7000},
//	{0x002A,	0x04D2},
//	{0x0F12,	0x0677},
//
//	{0x002A,	0x04A0},
//	{0x0F12,	0x016E},
//	{0x002A,	0x04A2},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x04A4},
//	{0x0F12,	0x0100},
//	{0x002A,	0x04A6},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x04A8},
//	{0x0F12,	0x011E},
//	{0x002A,	0x04AA},
//	{0x0F12,	0x0001},
//
//	{0x002A,	0x1000},
//	{0x0F12,	0x0001},
//
//	{SENSOR_TABLE_END, 0x00}
//};
//
/* Antibanding Settings */
static struct s5k5cag_reg flicker_off[] = {
	{0xFCFC, 0xD000},
	{0x0028, 0x7000},
	{0x002A, 0x04BA},
	{0x0F12, 0x0001},
	{0x0F12, 0x0001},
	{SENSOR_TABLE_END, 0x00}          
};
//static struct s5k5cag_reg flicker_50hz[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x04BA},
//	{0x0F12, 0x0001},
//	{0x0F12, 0x0001},
//	{SENSOR_TABLE_END, 0x00}          
//};
//static struct s5k5cag_reg flicker_60hz[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x04BA},
//	{0x0F12, 0x0002},
//	{0x0F12, 0x0001},
//	{SENSOR_TABLE_END, 0x00}          
//};
//	
//#define tmp_data  0x0000
//
///* brightness Settings */
//static struct s5k5cag_reg brightness[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x020C},
//	{0x0F12, tmp_data},
//	{SENSOR_TABLE_END, 0x00}          
//};
//
///* exposure Settings */
//static struct s5k5cag_reg exposure[] = {
//	{0xFCFC, 0xD000},
//	{0x0028, 0x7000},
//	{0x002A, 0x0F70},
//	{0x0F12, tmp_data},
//	{SENSOR_TABLE_END, 0x00}          
//};
//
static struct s5k5cag_reg Exposure_2[] = {
{0xfcfc, 0xd000},
{0x0028, 0x7000},
{0x002a, 0x020C},
{0x0f12, 0x0040},
{0x0f12, 0x0000},
{0x002A, 0x0F70},
{0x0F12, 0x0046},
{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg Exposure_1[] = {
{0xfcfc, 0xd000},
{0x0028, 0x7000},
{0x002a, 0x020C},
{0x0f12, 0x0020},
{0x0f12, 0x0000},
{0x002A, 0x0F70},
{0x0F12, 0x0043},
{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg Exposure_0[] = {
{0xfcfc, 0xd000},
{0x0028, 0x7000},
{0x002a, 0x020C},
{0x0f12, 0x0000},
{0x0f12, 0x0000},
{0x002A, 0x0F70},
{0x0F12, 0x0040},
{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg Exposure_Negative_1[] = {
{0xfcfc, 0xd000},
{0x0028, 0x7000},
{0x002a, 0x020C},
{0x0f12, 0xffdc},
{0x0f12, 0x0000},
{0x002A, 0x0F70},
{0x0F12, 0x0038},
{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg Exposure_Negative_2[] = {
{0xfcfc, 0xd000},
{0x0028, 0x7000},
{0x002a, 0x020C},
{0x0f12, 0xffb8},
{0x0f12, 0x0000},
{0x002A, 0x0F70},
{0x0F12, 0x002f},
{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg scene_normal[] = {
{0xfcfc, 0xd000},                                                                        
{0x0028, 0x7000},//For Scene//                                                                                                
//{0x002A, 0x020C}, // &*&*&*CJ_20120521 mask to match EV UI setting                                                                                                            
//{0x0F12, 0x0000},// Brightness for controling the EV//                                                                        
{0x002A, 0x0210},                                                                                                             
{0x0F12, 0x0000},// Saturation//                                                                                              
{0x0F12, 0x0000},// Blur_Sharpness //                                                                                         
{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg sport[] = {
{0xfcfc, 0xd000},                                                                        
{0x0028, 0x7000},                                                                        
{0x002A, 0x12B8},                                                                        
{0x0F12, 0x2000},                                                                        
{0x002A, 0x0530},   	                                                                   
{0x0F12, 0x36B0},   //lt_uMaxExp1	32 30ms  9~10ea// 15fps  //                            
{0x002A, 0x0534},                                                                        
{0x0F12, 0x36B0},   //lt_uMaxExp2	67 65ms	18~20ea // 7.5fps //                           
{0x002A, 0x167C},                                                                        
{0x0F12, 0x36B0},   //9C40//MaxExp3  83 80ms  24~25ea //                                 
{0x002A, 0x1680},                                                                        
{0x0F12, 0x36B0},   //MaxExp4   125ms  38ea //                                           
{0x002A, 0x0538},                                                                        
{0x0F12, 0x36B0},   // 15fps //                                                          
{0x002A, 0x053C},                                                                        
{0x0F12, 0x36B0},   // 7.5fps //                                                         
{0x002A, 0x1684},                                                                        
{0x0F12, 0x36B0},   //CapMaxExp3 //                                                      
{0x002A, 0x1688},                                                                        
{0x0F12, 0x36B0},   //CapMaxExp4 //                                                      
{0x002A, 0x0540},                                                                        
{0x0F12, 0x0200},   //0170//0150//lt_uMaxAnGain1_700lux//                  
{0x0F12, 0x0200},   //0200//0400//lt_uMaxAnGain2_400lux//                   
{0x002A, 0x168C},                                                           
{0x0F12, 0x0200},   //0300//MaxAnGain3_200lux//                             
{0x0F12, 0x0200},   //MaxAnGain4 //                                         
{0x002A, 0x0544},                                                           
{0x0F12, 0x0100},                                                           
{0x0F12, 0x8000},                                                           
{0x002A, 0x04B4},                                                           
{0x0F12, 0x0001},                                                           
{0x0F12, 0x00C8},                                                           
{0x0F12, 0x0001},       

{0x002A, 0x023C}, 
//{0x0F12, 0x0000},  // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
{0x0F12, 0x0004},  // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
{0x002A, 0x0240}, 
{0x0F12, 0x0001},  // #REG_TC_GP_PrevOpenAfterChange
{0x002A, 0x0230}, 
{0x0F12, 0x0001},  // #REG_TC_GP_NewConfigSync // Update preview configuration
{0x002A, 0x023E}, 
{0x0F12, 0x0001},  // #REG_TC_GP_PrevConfigChanged
{0x002A, 0x0220}, 
{0x0F12, 0x0001},  // #REG_TC_GP_EnablePreview // Start preview
{0x0028, 0xd000},
{0x0028, 0xb0a0},
{0x0f12, 0x0000},
{0x0028, 0x7000},
{0x002A, 0x0222}, 
{0x0F12, 0x0001},  // #REG_TC_GP_EnablePreviewChanged                                                    
{SENSOR_TABLE_END, 0x0000}
};
static struct s5k5cag_reg sport_off[] = {
{0xfcfc, 0xd000},                                                                        
{0x0028, 0x7000},                                                                                                                           
{0x002A, 0x12B8},                                                                                                                           
{0x0F12, 0x1000},                                                                                                                           
{0x002A, 0x0530},   	                                                                                                                      
//{0x0F12, 0x3415},   //lt_uMaxExp1	32 30ms  9~10ea// 15fps  //                                                                               
{0x0F12, 0xD000},   //lt_uMaxExp1	32 30ms  9~10ea// 15fps  //                                                                               
{0x002A, 0x0534},                                                                                                                           
//{0x0F12, 0x682A},   //lt_uMaxExp2	67 65ms	18~20ea // 7.5fps //                                                                              
{0x0F12, 0x0000},   //lt_uMaxExp2	67 65ms	18~20ea // 7.5fps //                                                                              
{0x002A, 0x167C},                                                                                                                           
//{0x0F12, 0x8235},   //9C40//MaxExp3  83 80ms  24~25ea //                                                                                    
{0x0F12, 0xD000},   //9C40//MaxExp3  83 80ms  24~25ea //                                                                                    
{0x002A, 0x1680},                                                                                                                           
{0x0F12, 0xc350},   //MaxExp4   125ms  38ea //                                                                                              
{0x002A, 0x0538},                                                                                                                           
//{0x0F12, 0x3415},   // 15fps //                                                                                                             
{0x0F12, 0xD000},   // 15fps //                                                                                                             
{0x002A, 0x053C},                                                                                                                           
//{0x0F12, 0x682A},   // 7.5fps //                                                                                                            
{0x0F12, 0x0000},   // 7.5fps //                                                                                                            
{0x002A, 0x1684},                                                                                                                           
{0x0F12, 0x8235},   //CapMaxExp3 //                                                                                                         
{0x002A, 0x1688},                                                                                                                           
{0x0F12, 0xC350},   //CapMaxExp4 //                                                                                                         
{0x002A, 0x0540},                                                                                                                           
//{0x0F12, 0x01B3},    //0170//0150//lt_uMaxAnGain1_700lux//                                                                    
//{0x0F12, 0x01B3},    //0200//0400//lt_uMaxAnGain2_400lux//                                                                             
{0x0F12, 0x0170},    //0170//0150//lt_uMaxAnGain1_700lux//                                                                    
{0x0F12, 0x0280},    //0200//0400//lt_uMaxAnGain2_400lux//                                                                             
{0x002A, 0x168C},                                                                                                                     
//{0x0F12, 0x02A0},    //0300//MaxAnGain3_200lux//                                                                                       
//{0x0F12, 0x0710},    //MaxAnGain4 //                                                                                                   
{0x0F12, 0x0380},    //0300//MaxAnGain3_200lux//                                                                                       
{0x0F12, 0x0800},    //MaxAnGain4 //                                                                                                   
{0x002A, 0x0544},                                                                                                                     
//{0x0F12, 0x0100},                                                                                                                     
//{0x0F12, 0x8000},                                                                                                                     
{0x0F12, 0x0200},                                                                                                                     
{0x0F12, 0x1000},                                                                                                                     
{0x002A, 0x04B4},                                                                                                                     
//{0x0F12, 0x0001},                                                                                                                     
//{0x0F12, 0x0064},                                                                                                                     
//{0x0F12, 0x0001},                                                                                                                     
{0x0F12, 0x0000},                                                                                                                     
{0x0F12, 0x0000},                                                                                                                     
{0x0F12, 0x0001},        

{0x002A, 0x023C}, 
//{0x0F12, 0x0000},  // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
{0x0F12, 0x0004},  // #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
{0x002A, 0x0240}, 
{0x0F12, 0x0001},  // #REG_TC_GP_PrevOpenAfterChange
{0x002A, 0x0230}, 
{0x0F12, 0x0001},  // #REG_TC_GP_NewConfigSync // Update preview configuration
{0x002A, 0x023E}, 
{0x0F12, 0x0001},  // #REG_TC_GP_PrevConfigChanged
{0x002A, 0x0220}, 
{0x0F12, 0x0001},  // #REG_TC_GP_EnablePreview // Start preview
{0x0028, 0xd000},
{0x0028, 0xb0a0},
{0x0f12, 0x0000},
{0x0028, 0x7000},
{0x002A, 0x0222}, 
{0x0F12, 0x0001},  // #REG_TC_GP_EnablePreviewChanged                                                                                                              
{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg scene_landscape[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
//{0x002A, 0x020C}, // &*&*&*CJ_20120521 mask to match EV UI setting
//{0x0F12, 0x0000},
{0x002A, 0x0210},
{0x0F12, 0x001E},
{0x0F12, 0x000A},
{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg scene_portrait[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
//{0x002A, 0x020C}, // &*&*&*CJ_20120521 mask to match EV UI setting
//{0x0F12, 0x0000},
{0x002A, 0x0210},
{0x0F12, 0x0000},
{0x0F12, 0xFFF6},
{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg scene_sunset[] = {
{0xfcfc, 0xd000},
{0x0028, 0x7000},
{0x002a, 0x246E},
{0x0f12, 0x0000},
                 
{0x002a, 0x04A0},
{0x0f12, 0x05E0},
{0x0f12, 0x0001},
{0x0f12, 0x0400},
{0x0f12, 0x0001},
{0x0f12, 0x0520},
{0x0f12, 0x0001},

{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg scene_night[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x04B4},
{0x0F12, 0x0001},//REG_SF_USER_IsoType 0: ISO Auto 1: ISO Clasic 2: ISO Sport
{0x0F12, 0x0190},//REG_SF_USER_IsoVal                                        
{0x0F12, 0x0001},//REG_SF_USER_IsoChanged                                    

{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg scene_nightoff[] = {
{0xFCFC, 0xD000},
{0x0028, 0x7000},
{0x002A, 0x04B4},
{0x0F12, 0x0000},
{0x0F12, 0x0000},                                    
{0x0F12, 0x0001},                                  

{SENSOR_TABLE_END, 0x0000}
};

static struct s5k5cag_reg initial_list[] = {
//=================================================================================================
//* Name: S5K5CAGX EVT1.0 Initial Setfile
//* PLL mode: MCLK=24MHz / SYSCLK=30MHz / PCLK=60MHz
//* FPS : Preview YUV QXGA 7.5fps / Capture YUV QXGA 7.5fps
//* Analog and TnP modified(20100225)
//=================================================================================================
//ARM GO
	{SENSOR_WAIT_MS,100},
//Direct mode
{0xFCFC, 0xD000}, 
{0x0010, 0x0001}, //Reset
{0x1030, 0x0000}, //Clear host interrupt so main will wait
{0x0014, 0x0001}, //ARM go

//p100
 {SENSOR_WAIT_MS,100},
	{SENSOR_WAIT_MS,100},
 
//Min.10ms delay is required

//Start T&P part
//DO NOT DELETE T&P SECTION COMMENTS! They are required to debug T&P related issues.
//svn://transrdsrv/svn/svnroot/System/Software/tcevb/SDK+FW/ISP_5CA/Firmware
//Rev: WC
//Signature:
//md5 b093df4c68b392b85938fb2d5d1a3ed8 .btp
//md5 4013ff93e02c5ca42ee80f5d436679cb .htp
//md5 8c46487f45dee2566f9a04abe3fd8f52 .RegsMap.h
//md5 90230b6b3e71c02e34af139b81219a11 .RegsMap.bin
//md5 506b4144bd48cdb79cbecdda4f7176ba .base.RegsMap.h
//md5 fd8f92f13566c1a788746b23691c5f5f .base.RegsMap.bin
//
{0x0028, 0x7000},
{0x002A, 0x2CF8},
{0x0F12, 0xB570},
{0x0F12, 0x4927},
{0x0F12, 0x20C0},
{0x0F12, 0x8048},
{0x0F12, 0x4C25},
{0x0F12, 0x4926},
{0x0F12, 0x3420},
{0x0F12, 0x83A1},
{0x0F12, 0x1D09},
{0x0F12, 0x83E1},
{0x0F12, 0x4922},
{0x0F12, 0x3140},
{0x0F12, 0x8048},
{0x0F12, 0x4D21},
{0x0F12, 0x4822},
{0x0F12, 0x3560},
{0x0F12, 0x83A8},
{0x0F12, 0x1D00},
{0x0F12, 0x83E8},
{0x0F12, 0x4821},
{0x0F12, 0x2201},
{0x0F12, 0x2140},
{0x0F12, 0xF000},
{0x0F12, 0xFA24},
{0x0F12, 0x481F},
{0x0F12, 0x8360},
{0x0F12, 0x481C},
{0x0F12, 0x1F00},
{0x0F12, 0x8368},
{0x0F12, 0x481E},
{0x0F12, 0x4918},
{0x0F12, 0x8802},
{0x0F12, 0x3980},
{0x0F12, 0x804A},
{0x0F12, 0x8842},
{0x0F12, 0x808A},
{0x0F12, 0x8882},
{0x0F12, 0x80CA},
{0x0F12, 0x88C2},
{0x0F12, 0x810A},
{0x0F12, 0x8902},
{0x0F12, 0x4919},
{0x0F12, 0x80CA},
{0x0F12, 0x8942},
{0x0F12, 0x814A},
{0x0F12, 0x8982},
{0x0F12, 0x830A},
{0x0F12, 0x89C2},
{0x0F12, 0x834A},
{0x0F12, 0x8A00},
{0x0F12, 0x4915},
{0x0F12, 0x8188},
{0x0F12, 0x4915},
{0x0F12, 0x4816},
{0x0F12, 0xF000},
{0x0F12, 0xFA0C},
{0x0F12, 0x4915},
{0x0F12, 0x4816},
{0x0F12, 0x63C1},
{0x0F12, 0x4916},
{0x0F12, 0x6301},
{0x0F12, 0x4916},
{0x0F12, 0x3040},
{0x0F12, 0x6181},
{0x0F12, 0x4915},
{0x0F12, 0x4816},
{0x0F12, 0xF000},
{0x0F12, 0xFA00},
{0x0F12, 0x4915},
{0x0F12, 0x4816},
{0x0F12, 0xF000},
{0x0F12, 0xF9FC},
{0x0F12, 0x4915},
{0x0F12, 0x4816},
{0x0F12, 0xF000},
{0x0F12, 0xF9F8},
{0x0F12, 0xBC70},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0x0000},
{0x0F12, 0x1100},
{0x0F12, 0xD000},
{0x0F12, 0x267C},
{0x0F12, 0x0000},
{0x0F12, 0x2CE8},
{0x0F12, 0x0000},
{0x0F12, 0x1102},
{0x0F12, 0x0000},
{0x0F12, 0x6A02},
{0x0F12, 0x0000},
{0x0F12, 0x3364},
{0x0F12, 0x7000},
{0x0F12, 0xF400},
{0x0F12, 0xD000},
{0x0F12, 0xF520},
{0x0F12, 0xD000},
{0x0F12, 0x2DE9},
{0x0F12, 0x7000},
{0x0F12, 0x89A9},
{0x0F12, 0x0000},
{0x0F12, 0x2E3B},
{0x0F12, 0x7000},
{0x0F12, 0x0080},
{0x0F12, 0x7000},
{0x0F12, 0x2EE9},
{0x0F12, 0x7000},
{0x0F12, 0x2EFD},
{0x0F12, 0x7000},
{0x0F12, 0x2ECD},
{0x0F12, 0x7000},
{0x0F12, 0x013D},
{0x0F12, 0x0001},
{0x0F12, 0x2F83},
{0x0F12, 0x7000},
{0x0F12, 0xD789},
{0x0F12, 0x0000},
{0x0F12, 0x2FDB},
{0x0F12, 0x7000},
{0x0F12, 0x6D1B},
{0x0F12, 0x0000},
{0x0F12, 0xB570},
{0x0F12, 0x6804},
{0x0F12, 0x6845},
{0x0F12, 0x6881},
{0x0F12, 0x6840},
{0x0F12, 0x2900},
{0x0F12, 0x6880},
{0x0F12, 0xD007},
{0x0F12, 0x49C8},
{0x0F12, 0x8949},
{0x0F12, 0x084A},
{0x0F12, 0x1880},
{0x0F12, 0xF000},
{0x0F12, 0xF9C6},
{0x0F12, 0x80A0},
{0x0F12, 0xE000},
{0x0F12, 0x80A0},
{0x0F12, 0x88A0},
{0x0F12, 0x2800},
{0x0F12, 0xD010},
{0x0F12, 0x68A9},
{0x0F12, 0x6828},
{0x0F12, 0x084A},
{0x0F12, 0x1880},
{0x0F12, 0xF000},
{0x0F12, 0xF9BA},
{0x0F12, 0x8020},
{0x0F12, 0x1D2D},
{0x0F12, 0xCD03},
{0x0F12, 0x084A},
{0x0F12, 0x1880},
{0x0F12, 0xF000},
{0x0F12, 0xF9B3},
{0x0F12, 0x8060},
{0x0F12, 0xBC70},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0x2000},
{0x0F12, 0x8060},
{0x0F12, 0x8020},
{0x0F12, 0xE7F8},
{0x0F12, 0xB5F8},
{0x0F12, 0x0004},
{0x0F12, 0x48B8},
{0x0F12, 0x237D},
{0x0F12, 0x8B00},
{0x0F12, 0x4FB7},
{0x0F12, 0x015B},
{0x0F12, 0x4358},
{0x0F12, 0x8A39},
{0x0F12, 0x084A},
{0x0F12, 0x1880},
{0x0F12, 0xF000},
{0x0F12, 0xF99E},
{0x0F12, 0x210F},
{0x0F12, 0xF000},
{0x0F12, 0xF9A1},
{0x0F12, 0x49B3},
{0x0F12, 0x8C49},
{0x0F12, 0x090E},
{0x0F12, 0x0136},
{0x0F12, 0x4306},
{0x0F12, 0x49B1},
{0x0F12, 0x2C00},
{0x0F12, 0xD004},
{0x0F12, 0x2201},
{0x0F12, 0x0030},
{0x0F12, 0x0252},
{0x0F12, 0x4310},
{0x0F12, 0x8108},
{0x0F12, 0x48AE},
{0x0F12, 0x2C00},
{0x0F12, 0x8D00},
{0x0F12, 0xD001},
{0x0F12, 0x2501},
{0x0F12, 0xE000},
{0x0F12, 0x2500},
{0x0F12, 0x49AA},
{0x0F12, 0x4328},
{0x0F12, 0x8008},
{0x0F12, 0x207D},
{0x0F12, 0x00C0},
{0x0F12, 0xF000},
{0x0F12, 0xF98E},
{0x0F12, 0x2C00},
{0x0F12, 0x49A6},
{0x0F12, 0x0328},
{0x0F12, 0x4330},
{0x0F12, 0x8108},
{0x0F12, 0x48A1},
{0x0F12, 0x2C00},
{0x0F12, 0x8AC0},
{0x0F12, 0x01AA},
{0x0F12, 0x4310},
{0x0F12, 0x8088},
{0x0F12, 0x8A39},
{0x0F12, 0x48A2},
{0x0F12, 0xF000},
{0x0F12, 0xF971},
{0x0F12, 0x49A2},
{0x0F12, 0x8809},
{0x0F12, 0x4348},
{0x0F12, 0x0400},
{0x0F12, 0x0C00},
{0x0F12, 0xF000},
{0x0F12, 0xF978},
{0x0F12, 0x0020},
{0x0F12, 0xF000},
{0x0F12, 0xF97D},
{0x0F12, 0x489E},
{0x0F12, 0x7004},
{0x0F12, 0xBCF8},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0xB510},
{0x0F12, 0x0004},
{0x0F12, 0xF000},
{0x0F12, 0xF97C},
{0x0F12, 0x6020},
{0x0F12, 0x499A},
{0x0F12, 0x8B49},
{0x0F12, 0x0789},
{0x0F12, 0xD001},
{0x0F12, 0x0040},
{0x0F12, 0x6020},
{0x0F12, 0xBC10},
{0x0F12, 0xBC08},
{0x0F12, 0x4718},
{0x0F12, 0xB510},
{0x0F12, 0xF000},
{0x0F12, 0xF977},
{0x0F12, 0x4895},
{0x0F12, 0x498B},
{0x0F12, 0x8880},
{0x0F12, 0x0600},
{0x0F12, 0x1600},
{0x0F12, 0x8348},
{0x0F12, 0xE7F2},
{0x0F12, 0xB5F8},
{0x0F12, 0x000F},
{0x0F12, 0x4D8B},
{0x0F12, 0x3520},
{0x0F12, 0x2400},
{0x0F12, 0x572C},
{0x0F12, 0x0039},
{0x0F12, 0xF000},
{0x0F12, 0xF96F},
{0x0F12, 0x9000},
{0x0F12, 0x2600},
{0x0F12, 0x57AE},
{0x0F12, 0x4D82},
{0x0F12, 0x42A6},
{0x0F12, 0xD01B},
{0x0F12, 0x4C8A},
{0x0F12, 0x8AE0},
{0x0F12, 0x2800},
{0x0F12, 0xD013},
{0x0F12, 0x4883},
{0x0F12, 0x8A01},
{0x0F12, 0x8B80},
{0x0F12, 0x4378},
{0x0F12, 0xF000},
{0x0F12, 0xF931},
{0x0F12, 0x89A1},
{0x0F12, 0x1A41},
{0x0F12, 0x4884},
{0x0F12, 0x3820},
{0x0F12, 0x8AC0},
{0x0F12, 0x4348},
{0x0F12, 0x17C1},
{0x0F12, 0x0D89},
{0x0F12, 0x1808},
{0x0F12, 0x1280},
{0x0F12, 0x8B69},
{0x0F12, 0x1A08},
{0x0F12, 0x8368},
{0x0F12, 0xE003},
{0x0F12, 0x88A0},
{0x0F12, 0x0600},
{0x0F12, 0x1600},
{0x0F12, 0x8368},
{0x0F12, 0x201A},
{0x0F12, 0x5E28},
{0x0F12, 0x42B0},
{0x0F12, 0xD011},
{0x0F12, 0xF000},
{0x0F12, 0xF94F},
{0x0F12, 0x1D40},
{0x0F12, 0x00C3},
{0x0F12, 0x1A18},
{0x0F12, 0x214B},
{0x0F12, 0xF000},
{0x0F12, 0xF913},
{0x0F12, 0x211F},
{0x0F12, 0xF000},
{0x0F12, 0xF916},
{0x0F12, 0x211A},
{0x0F12, 0x5E69},
{0x0F12, 0x0FC9},
{0x0F12, 0x0149},
{0x0F12, 0x4301},
{0x0F12, 0x4873},
{0x0F12, 0x81C1},
{0x0F12, 0x9800},
{0x0F12, 0xE7A1},
{0x0F12, 0xB570},
{0x0F12, 0x6805},
{0x0F12, 0x2404},
{0x0F12, 0xF000},
{0x0F12, 0xF940},
{0x0F12, 0x2800},
{0x0F12, 0xD103},
{0x0F12, 0xF000},
{0x0F12, 0xF944},
{0x0F12, 0x2800},
{0x0F12, 0xD000},
{0x0F12, 0x2400},
{0x0F12, 0x3540},
{0x0F12, 0x88E8},
{0x0F12, 0x0500},
{0x0F12, 0xD403},
{0x0F12, 0x486A},
{0x0F12, 0x89C0},
{0x0F12, 0x2800},
{0x0F12, 0xD002},
{0x0F12, 0x2008},
{0x0F12, 0x4304},
{0x0F12, 0xE001},
{0x0F12, 0x2010},
{0x0F12, 0x4304},
{0x0F12, 0x4866},
{0x0F12, 0x8B80},
{0x0F12, 0x0700},
{0x0F12, 0x0F81},
{0x0F12, 0x2001},
{0x0F12, 0x2900},
{0x0F12, 0xD000},
{0x0F12, 0x4304},
{0x0F12, 0x4963},
{0x0F12, 0x8B0A},
{0x0F12, 0x42A2},
{0x0F12, 0xD004},
{0x0F12, 0x0762},
{0x0F12, 0xD502},
{0x0F12, 0x4A60},
{0x0F12, 0x3220},
{0x0F12, 0x8110},
{0x0F12, 0x830C},
{0x0F12, 0xE728},
{0x0F12, 0xB5F8},
{0x0F12, 0x2600},
{0x0F12, 0x4C5E},
{0x0F12, 0x495E},
{0x0F12, 0x8B20},
{0x0F12, 0x2800},
{0x0F12, 0xD101},
{0x0F12, 0x2001},
{0x0F12, 0x6088},
{0x0F12, 0x485C},
{0x0F12, 0x4D5B},
{0x0F12, 0x6028},
{0x0F12, 0x3080},
{0x0F12, 0x6068},
{0x0F12, 0x4858},
{0x0F12, 0x2100},
{0x0F12, 0x3840},
{0x0F12, 0x6101},
{0x0F12, 0x60C1},
{0x0F12, 0xF000},
{0x0F12, 0xF914},
{0x0F12, 0x68A8},
{0x0F12, 0x4F57},
{0x0F12, 0x2800},
{0x0F12, 0xD025},
{0x0F12, 0x4844},
{0x0F12, 0x4D53},
{0x0F12, 0x8A80},
{0x0F12, 0x6128},
{0x0F12, 0x8B20},
{0x0F12, 0x2120},
{0x0F12, 0xF000},
{0x0F12, 0xF8C0},
{0x0F12, 0x60E8},
{0x0F12, 0x2600},
{0x0F12, 0x616E},
{0x0F12, 0x2400},
{0x0F12, 0xE013},
{0x0F12, 0x4950},
{0x0F12, 0x0060},
{0x0F12, 0x1841},
{0x0F12, 0x2020},
{0x0F12, 0x5E08},
{0x0F12, 0x8BF9},
{0x0F12, 0x084A},
{0x0F12, 0x1880},
{0x0F12, 0xF000},
{0x0F12, 0xF8AB},
{0x0F12, 0x682B},
{0x0F12, 0x0202},
{0x0F12, 0x00A1},
{0x0F12, 0x505A},
{0x0F12, 0x0002},
{0x0F12, 0x4342},
{0x0F12, 0x0210},
{0x0F12, 0x686A},
{0x0F12, 0x5050},
{0x0F12, 0x1C64},
{0x0F12, 0x68E8},
{0x0F12, 0x4284},
{0x0F12, 0xD3E8},
{0x0F12, 0x60AE},
{0x0F12, 0xE049},
{0x0F12, 0x2400},
{0x0F12, 0x4844},
{0x0F12, 0x9000},
{0x0F12, 0xE033},
{0x0F12, 0x9800},
{0x0F12, 0x8845},
{0x0F12, 0x4940},
{0x0F12, 0x0060},
{0x0F12, 0x1841},
{0x0F12, 0x2020},
{0x0F12, 0x5E08},
{0x0F12, 0x8BF9},
{0x0F12, 0x084A},
{0x0F12, 0x1880},
{0x0F12, 0xF000},
{0x0F12, 0xF88C},
{0x0F12, 0x4A38},
{0x0F12, 0x00A1},
{0x0F12, 0x6812},
{0x0F12, 0x4696},
{0x0F12, 0x5852},
{0x0F12, 0x436A},
{0x0F12, 0x3280},
{0x0F12, 0x0A13},
{0x0F12, 0x22FF},
{0x0F12, 0x3201},
{0x0F12, 0x1B52},
{0x0F12, 0x4694},
{0x0F12, 0x4342},
{0x0F12, 0x189B},
{0x0F12, 0x4672},
{0x0F12, 0x5053},
{0x0F12, 0x0003},
{0x0F12, 0x4343},
{0x0F12, 0x4662},
{0x0F12, 0x4353},
{0x0F12, 0x482E},
{0x0F12, 0x6840},
{0x0F12, 0x5842},
{0x0F12, 0x436A},
{0x0F12, 0x3280},
{0x0F12, 0x0A12},
{0x0F12, 0x189A},
{0x0F12, 0x5042},
{0x0F12, 0x482A},
{0x0F12, 0x6800},
{0x0F12, 0x5840},
{0x0F12, 0x0001},
{0x0F12, 0x4341},
{0x0F12, 0x3180},
{0x0F12, 0x0A08},
{0x0F12, 0x4290},
{0x0F12, 0xD801},
{0x0F12, 0x1A10},
{0x0F12, 0x1986},
{0x0F12, 0x1C64},
{0x0F12, 0x4824},
{0x0F12, 0x68C0},
{0x0F12, 0x4284},
{0x0F12, 0xD3C7},
{0x0F12, 0x4C22},
{0x0F12, 0x68E1},
{0x0F12, 0x0848},
{0x0F12, 0x1980},
{0x0F12, 0xF000},
{0x0F12, 0xF85A},
{0x0F12, 0x3008},
{0x0F12, 0x0900},
{0x0F12, 0x6921},
{0x0F12, 0x4288},
{0x0F12, 0xD902},
{0x0F12, 0x9800},
{0x0F12, 0x8800},
{0x0F12, 0x6160},
{0x0F12, 0x491B},
{0x0F12, 0x6948},
{0x0F12, 0x2800},
{0x0F12, 0xDD01},
{0x0F12, 0x1E40},
{0x0F12, 0x6148},
{0x0F12, 0x4809},
{0x0F12, 0x8A80},
{0x0F12, 0x2800},
{0x0F12, 0xD00C},
{0x0F12, 0x4A19},
{0x0F12, 0x7B90},
{0x0F12, 0x2802},
{0x0F12, 0xD001},
{0x0F12, 0x2803},
{0x0F12, 0xD106},
{0x0F12, 0x6949},
{0x0F12, 0x2900},
{0x0F12, 0xDD03},
{0x0F12, 0x2100},
{0x0F12, 0x0040},
{0x0F12, 0x1880},
{0x0F12, 0x8081},
{0x0F12, 0xE6D4},
{0x0F12, 0x0C3C},
{0x0F12, 0x7000},
{0x0F12, 0x3364},
{0x0F12, 0x7000},
{0x0F12, 0x1D6C},
{0x0F12, 0x7000},
{0x0F12, 0x167C},
{0x0F12, 0x7000},
{0x0F12, 0xF400},
{0x0F12, 0xD000},
{0x0F12, 0x2C2C},
{0x0F12, 0x7000},
{0x0F12, 0x40A0},
{0x0F12, 0x00DD},
{0x0F12, 0xF520},
{0x0F12, 0xD000},
{0x0F12, 0x2C29},
{0x0F12, 0x7000},
{0x0F12, 0x1A54},
{0x0F12, 0x7000},
{0x0F12, 0x1564},
{0x0F12, 0x7000},
{0x0F12, 0xF2A0},
{0x0F12, 0xD000},
{0x0F12, 0x2894},
{0x0F12, 0x7000},
{0x0F12, 0x1224},
{0x0F12, 0x7000},
{0x0F12, 0xB000},
{0x0F12, 0xD000},
{0x0F12, 0x1E8C},
{0x0F12, 0x7000},
{0x0F12, 0x3240},
{0x0F12, 0x7000},
{0x0F12, 0x1EBC},
{0x0F12, 0x7000},
{0x0F12, 0x2050},
{0x0F12, 0x7000},
{0x0F12, 0x1D8C},
{0x0F12, 0x7000},
{0x0F12, 0x10E0},
{0x0F12, 0x7000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x38E5},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x1A3F},
{0x0F12, 0x0001},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xF004},
{0x0F12, 0xE51F},
{0x0F12, 0x1F48},
{0x0F12, 0x0001},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x36ED},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xF53F},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xF5D9},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x013D},
{0x0F12, 0x0001},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xF5C9},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xFAA9},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x36DD},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xD771},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0xD75B},
{0x0F12, 0x0000},
{0x0F12, 0x4778},
{0x0F12, 0x46C0},
{0x0F12, 0xC000},
{0x0F12, 0xE59F},
{0x0F12, 0xFF1C},
{0x0F12, 0xE12F},
{0x0F12, 0x6D1B},
{0x0F12, 0x0000},
{0x0F12, 0x82FA},
{0x0F12, 0x0000},
//
//Parameters Defined in T&P:
//TnP_SvnVersion                           2 7000323C SHORT
//Tune_TP                                 28 70003364 STRUCT
//Tune_TP_IO_DrivingCurrent_D0_D4_cs10     2 70003364 SHORT
//Tune_TP_IO_DrivingCurrent_D9_D5_cs10     2 70003366 SHORT
//Tune_TP_IO_DrivingCurrent_GPIO_cd10      2 70003368 SHORT
//Tune_TP_IO_DrivingCurrent_CLKs_cd10      2 7000336A SHORT
//Tune_TP_atop_dblr_reg_1                  2 7000336C SHORT
//Tune_TP_atop_dblr_reg_3                  2 7000336E SHORT
//Tune_TP_atop_ramp_reg_1                  2 70003370 SHORT
//Tune_TP_atop_ramp_reg_2                  2 70003372 SHORT
//Tune_TP_atop_rmp_offset_sig              2 70003374 SHORT
//Tune_TP_bEnablePrePostGammaAfControl     2 70003376 SHORT
//Tune_TP_AFC_SCD_Thresh                   2 70003378 SHORT
//Tune_TP_atop_dbus_reg                    2 7000337A SHORT
//Tune_TP_dblr_base_freq_mhz               2 7000337C SHORT
//Tune_TP_GL_sen_sOfs                      2 7000337E SHORT
//
//End T&P part


//============================================================
//CIS/APS/Analog setting- 400LSBSYSCLK 45MHz -091127
//============================================================
//This registers are for FACTORY ONLY. If you change it without prior notification,
//YOU are RESPONSIBLE for the FAILURE that will happen in the future.
//============================================================
{0x002A, 0x021A}, 
{0x0F12, 0x0000}, 
{0x002A, 0x157A}, 
{0x0F12, 0x0001}, 
{0x002A, 0x1578}, 
{0x0F12, 0x0001}, 
{0x002A, 0x1576}, 
{0x0F12, 0x0020}, 
{0x002A, 0x1574}, 
{0x0F12, 0x0006}, 
{0x002A, 0x156E}, 
{0x0F12, 0x0001},   //Slope calibration tolerance in units of 1/256
{0x002A, 0x1568}, 
{0x0F12, 0x00FC}, 
//ADC control
{0x002A, 0x155A},
{0x0F12, 0x01CC},  //ADC SAT of 450mV for 10bit default in EVT1
{0x002A, 0x157E},
{0x0F12, 0x0C80},  //3200 Max. Reset ramp DCLK counts (default 2048 0x800)
{0x0F12, 0x0578},  //1400 Max. Reset ramp DCLK counts for x3.5
{0x002A, 0x157C},
{0x0F12, 0x0190},  //400 Reset ramp for x1 in DCLK counts
{0x002A, 0x1570},
{0x0F12, 0x00A0},  //160 LSB
{0x0F12, 0x0010},  //reset threshold
{0x002A, 0x12C4},
{0x0F12, 0x006A},  //106 additional timing columns.
{0x002A, 0x12C8},
{0x0F12, 0x08AC},  //2220 ADC columns in normal mode including Hold & Latch
{0x0F12, 0x0050},  //80 addition of ADC columns in Y-ave mode (default 244 0x74)
//WRITE #senHal_ForceModeType			0001	// Long exposure mode
{0x002A, 0x1696}, 
{0x0F12, 0x0000},   //based on APS guidelines
{0x0F12, 0x0000},   //based on APS guidelines
{0x0F12, 0x00C6},   //default. 1492 used for ADC dark characteristics
{0x0F12, 0x00C6},   //default. 1492 used for ADC dark characteristics
{0x002A, 0x12B8}, 
{0x0F12, 0x1000},   //disable CINTR 0
{0x002A, 0x1690}, 
{0x0F12, 0x0001},   //when set double sampling is activated - requires different set of pointers
{0x002A, 0x12B0}, 
{0x0F12, 0x0055},   //comp and pixel bias control 0xF40E - default for EVT1
{0x0F12, 0x005A},   //comp and pixel bias control 0xF40E for binning mode
{0x002A, 0x337A}, 
{0x0F12, 0x0006},   //[7] - is used for rest-only mode (EVT0 value is 0xD and HW 0x6)
{0x0F12, 0x0068},   //Tune_TP_dblr_base_freq_mhz
{0x002A, 0x169E}, 
{0x0F12, 0x0007},   //[3:0]- specifies the target (default 7)- DCLK = 64MHz instead of 116MHz.
{0x002A, 0x336C}, 
{0x0F12, 0x1000},   //Enable DBLR Regulation
{0x0F12, 0x6998},   //VPIX 2.8
{0x0F12, 0x0078},   //[0] Static RC Filter
{0x0F12, 0x04FE},   //[7:4] Full RC Filter
{0x0F12, 0x8800},   //Add Load to CDS block
              
{0x002A, 0x3364}, 
{0x0F12, 0x0155}, //	   Set IO driving current 2mA for GS500
{0x0F12, 0x0155}, //	   Set IO driving current
{0x0F12, 0x1555}, //	   Set IO driving current
{0x0F12, 0x0FFF}, //	   Set IO driving current
 
//#Asserting CDS pointers - Long exposure MS Normal
//#Conditions: 10bit, ADC_SAT = 450mV ; ramp_del = 40 ; ramp_start = 60
{0x0028, 0x7000},  
{0x002A, 0x12D2},  
{0x0F12, 0x0003},   //#senHal_pContSenModesRegsArray[0][0] 2 700012D2
{0x0F12, 0x0003},   //#senHal_pContSenModesRegsArray[0][1] 2 700012D4
{0x0F12, 0x0003},   //#senHal_pContSenModesRegsArray[0][2] 2 700012D6
{0x0F12, 0x0003},   //#senHal_pContSenModesRegsArray[0][3] 2 700012D8
{0x0F12, 0x0884},   //#senHal_pContSenModesRegsArray[1][0] 2 700012DA
{0x0F12, 0x08CF},   //#senHal_pContSenModesRegsArray[1][1] 2 700012DC
{0x0F12, 0x0500},   //#senHal_pContSenModesRegsArray[1][2] 2 700012DE
{0x0F12, 0x054B},   //#senHal_pContSenModesRegsArray[1][3] 2 700012E0
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[2][0] 2 700012E2
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[2][1] 2 700012E4
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[2][2] 2 700012E6
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[2][3] 2 700012E8
{0x0F12, 0x0885},   //#senHal_pContSenModesRegsArray[3][0] 2 700012EA
{0x0F12, 0x0467},   //#senHal_pContSenModesRegsArray[3][1] 2 700012EC
{0x0F12, 0x0501},   //#senHal_pContSenModesRegsArray[3][2] 2 700012EE
{0x0F12, 0x02A5},   //#senHal_pContSenModesRegsArray[3][3] 2 700012F0
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[4][0] 2 700012F2
{0x0F12, 0x046A},   //#senHal_pContSenModesRegsArray[4][1] 2 700012F4
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[4][2] 2 700012F6
{0x0F12, 0x02A8},   //#senHal_pContSenModesRegsArray[4][3] 2 700012F8
{0x0F12, 0x0885},   //#senHal_pContSenModesRegsArray[5][0] 2 700012FA
{0x0F12, 0x08D0},   //#senHal_pContSenModesRegsArray[5][1] 2 700012FC
{0x0F12, 0x0501},   //#senHal_pContSenModesRegsArray[5][2] 2 700012FE
{0x0F12, 0x054C},   //#senHal_pContSenModesRegsArray[5][3] 2 70001300
{0x0F12, 0x0006},   //#senHal_pContSenModesRegsArray[6][0] 2 70001302
{0x0F12, 0x0020},   //#senHal_pContSenModesRegsArray[6][1] 2 70001304
{0x0F12, 0x0006},   //#senHal_pContSenModesRegsArray[6][2] 2 70001306
{0x0F12, 0x0020},   //#senHal_pContSenModesRegsArray[6][3] 2 70001308
{0x0F12, 0x0881},   //#senHal_pContSenModesRegsArray[7][0] 2 7000130A
{0x0F12, 0x0463},   //#senHal_pContSenModesRegsArray[7][1] 2 7000130C
{0x0F12, 0x04FD},   //#senHal_pContSenModesRegsArray[7][2] 2 7000130E
{0x0F12, 0x02A1},   //#senHal_pContSenModesRegsArray[7][3] 2 70001310
{0x0F12, 0x0006},   //#senHal_pContSenModesRegsArray[8][0] 2 70001312
{0x0F12, 0x0489},   //#senHal_pContSenModesRegsArray[8][1] 2 70001314
{0x0F12, 0x0006},   //#senHal_pContSenModesRegsArray[8][2] 2 70001316
{0x0F12, 0x02C7},   //#senHal_pContSenModesRegsArray[8][3] 2 70001318
{0x0F12, 0x0881},   //#senHal_pContSenModesRegsArray[9][0] 2 7000131A
{0x0F12, 0x08CC},   //#senHal_pContSenModesRegsArray[9][1] 2 7000131C
{0x0F12, 0x04FD},   //#senHal_pContSenModesRegsArray[9][2] 2 7000131E
{0x0F12, 0x0548},   //#senHal_pContSenModesRegsArray[9][3] 2 70001320
{0x0F12, 0x03A2},   //#senHal_pContSenModesRegsArray[10][0]2 70001322
{0x0F12, 0x01D3},   //#senHal_pContSenModesRegsArray[10][1]2 70001324
{0x0F12, 0x01E0},   //#senHal_pContSenModesRegsArray[10][2]2 70001326
{0x0F12, 0x00F2},   //#senHal_pContSenModesRegsArray[10][3]2 70001328
{0x0F12, 0x03F2},   //#senHal_pContSenModesRegsArray[11][0]2 7000132A
{0x0F12, 0x0223},   //#senHal_pContSenModesRegsArray[11][1]2 7000132C
{0x0F12, 0x0230},   //#senHal_pContSenModesRegsArray[11][2]2 7000132E
{0x0F12, 0x0142},   //#senHal_pContSenModesRegsArray[11][3]2 70001330
{0x0F12, 0x03A2},   //#senHal_pContSenModesRegsArray[12][0]2 70001332
{0x0F12, 0x063C},   //#senHal_pContSenModesRegsArray[12][1]2 70001334
{0x0F12, 0x01E0},   //#senHal_pContSenModesRegsArray[12][2]2 70001336
{0x0F12, 0x0399},   //#senHal_pContSenModesRegsArray[12][3]2 70001338
{0x0F12, 0x03F2},   //#senHal_pContSenModesRegsArray[13][0]2 7000133A
{0x0F12, 0x068C},   //#senHal_pContSenModesRegsArray[13][1]2 7000133C
{0x0F12, 0x0230},   //#senHal_pContSenModesRegsArray[13][2]2 7000133E
{0x0F12, 0x03E9},   //#senHal_pContSenModesRegsArray[13][3]2 70001340
{0x0F12, 0x0002},   //#senHal_pContSenModesRegsArray[14][0]2 70001342
{0x0F12, 0x0002},   //#senHal_pContSenModesRegsArray[14][1]2 70001344
{0x0F12, 0x0002},   //#senHal_pContSenModesRegsArray[14][2]2 70001346
{0x0F12, 0x0002},   //#senHal_pContSenModesRegsArray[14][3]2 70001348
{0x0F12, 0x003C},   //#senHal_pContSenModesRegsArray[15][0]2 7000134A
{0x0F12, 0x003C},   //#senHal_pContSenModesRegsArray[15][1]2 7000134C
{0x0F12, 0x003C},   //#senHal_pContSenModesRegsArray[15][2]2 7000134E
{0x0F12, 0x003C},   //#senHal_pContSenModesRegsArray[15][3]2 70001350
{0x0F12, 0x01D3},   //#senHal_pContSenModesRegsArray[16][0]2 70001352
{0x0F12, 0x01D3},   //#senHal_pContSenModesRegsArray[16][1]2 70001354
{0x0F12, 0x00F2},   //#senHal_pContSenModesRegsArray[16][2]2 70001356
{0x0F12, 0x00F2},   //#senHal_pContSenModesRegsArray[16][3]2 70001358
{0x0F12, 0x020B},   //#senHal_pContSenModesRegsArray[17][0]2 7000135A
{0x0F12, 0x024A},   //#senHal_pContSenModesRegsArray[17][1]2 7000135C
{0x0F12, 0x012A},   //#senHal_pContSenModesRegsArray[17][2]2 7000135E
{0x0F12, 0x0169},   //#senHal_pContSenModesRegsArray[17][3]2 70001360
{0x0F12, 0x0002},   //#senHal_pContSenModesRegsArray[18][0]2 70001362
{0x0F12, 0x046B},   //#senHal_pContSenModesRegsArray[18][1]2 70001364
{0x0F12, 0x0002},   //#senHal_pContSenModesRegsArray[18][2]2 70001366
{0x0F12, 0x02A9},   //#senHal_pContSenModesRegsArray[18][3]2 70001368
{0x0F12, 0x0419},   //#senHal_pContSenModesRegsArray[19][0]2 7000136A
{0x0F12, 0x04A5},   //#senHal_pContSenModesRegsArray[19][1]2 7000136C
{0x0F12, 0x0257},   //#senHal_pContSenModesRegsArray[19][2]2 7000136E
{0x0F12, 0x02E3},   //#senHal_pContSenModesRegsArray[19][3]2 70001370
{0x0F12, 0x0630},   //#senHal_pContSenModesRegsArray[20][0]2 70001372
{0x0F12, 0x063C},   //#senHal_pContSenModesRegsArray[20][1]2 70001374
{0x0F12, 0x038D},   //#senHal_pContSenModesRegsArray[20][2]2 70001376
{0x0F12, 0x0399},   //#senHal_pContSenModesRegsArray[20][3]2 70001378
{0x0F12, 0x0668},   //#senHal_pContSenModesRegsArray[21][0]2 7000137A
{0x0F12, 0x06B3},   //#senHal_pContSenModesRegsArray[21][1]2 7000137C
{0x0F12, 0x03C5},   //#senHal_pContSenModesRegsArray[21][2]2 7000137E
{0x0F12, 0x0410},   //#senHal_pContSenModesRegsArray[21][3]2 70001380
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[22][0]2 70001382
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[22][1]2 70001384
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[22][2]2 70001386
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[22][3]2 70001388
{0x0F12, 0x03A2},   //#senHal_pContSenModesRegsArray[23][0]2 7000138A
{0x0F12, 0x01D3},   //#senHal_pContSenModesRegsArray[23][1]2 7000138C
{0x0F12, 0x01E0},   //#senHal_pContSenModesRegsArray[23][2]2 7000138E
{0x0F12, 0x00F2},   //#senHal_pContSenModesRegsArray[23][3]2 70001390
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[24][0]2 70001392
{0x0F12, 0x0461},   //#senHal_pContSenModesRegsArray[24][1]2 70001394
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[24][2]2 70001396
{0x0F12, 0x029F},   //#senHal_pContSenModesRegsArray[24][3]2 70001398
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[25][0]2 7000139A
{0x0F12, 0x063C},   //#senHal_pContSenModesRegsArray[25][1]2 7000139C
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[25][2]2 7000139E
{0x0F12, 0x0399},   //#senHal_pContSenModesRegsArray[25][3]2 700013A0
{0x0F12, 0x003D},   //#senHal_pContSenModesRegsArray[26][0]2 700013A2
{0x0F12, 0x003D},   //#senHal_pContSenModesRegsArray[26][1]2 700013A4
{0x0F12, 0x003D},   //#senHal_pContSenModesRegsArray[26][2]2 700013A6
{0x0F12, 0x003D},   //#senHal_pContSenModesRegsArray[26][3]2 700013A8
{0x0F12, 0x01D0},   //#senHal_pContSenModesRegsArray[27][0]2 700013AA
{0x0F12, 0x01D0},   //#senHal_pContSenModesRegsArray[27][1]2 700013AC
{0x0F12, 0x00EF},   //#senHal_pContSenModesRegsArray[27][2]2 700013AE
{0x0F12, 0x00EF},   //#senHal_pContSenModesRegsArray[27][3]2 700013B0
{0x0F12, 0x020C},   //#senHal_pContSenModesRegsArray[28][0]2 700013B2
{0x0F12, 0x024B},   //#senHal_pContSenModesRegsArray[28][1]2 700013B4
{0x0F12, 0x012B},   //#senHal_pContSenModesRegsArray[28][2]2 700013B6
{0x0F12, 0x016A},   //#senHal_pContSenModesRegsArray[28][3]2 700013B8
{0x0F12, 0x039F},   //#senHal_pContSenModesRegsArray[29][0]2 700013BA
{0x0F12, 0x045E},   //#senHal_pContSenModesRegsArray[29][1]2 700013BC
{0x0F12, 0x01DD},   //#senHal_pContSenModesRegsArray[29][2]2 700013BE
{0x0F12, 0x029C},   //#senHal_pContSenModesRegsArray[29][3]2 700013C0
{0x0F12, 0x041A},   //#senHal_pContSenModesRegsArray[30][0]2 700013C2
{0x0F12, 0x04A6},   //#senHal_pContSenModesRegsArray[30][1]2 700013C4
{0x0F12, 0x0258},   //#senHal_pContSenModesRegsArray[30][2]2 700013C6
{0x0F12, 0x02E4},   //#senHal_pContSenModesRegsArray[30][3]2 700013C8
{0x0F12, 0x062D},   //#senHal_pContSenModesRegsArray[31][0]2 700013CA
{0x0F12, 0x0639},   //#senHal_pContSenModesRegsArray[31][1]2 700013CC
{0x0F12, 0x038A},   //#senHal_pContSenModesRegsArray[31][2]2 700013CE
{0x0F12, 0x0396},   //#senHal_pContSenModesRegsArray[31][3]2 700013D0
{0x0F12, 0x0669},   //#senHal_pContSenModesRegsArray[32][0]2 700013D2
{0x0F12, 0x06B4},   //#senHal_pContSenModesRegsArray[32][1]2 700013D4
{0x0F12, 0x03C6},   //#senHal_pContSenModesRegsArray[32][2]2 700013D6
{0x0F12, 0x0411},   //#senHal_pContSenModesRegsArray[32][3]2 700013D8
{0x0F12, 0x087C},   //#senHal_pContSenModesRegsArray[33][0]2 700013DA
{0x0F12, 0x08C7},   //#senHal_pContSenModesRegsArray[33][1]2 700013DC
{0x0F12, 0x04F8},   //#senHal_pContSenModesRegsArray[33][2]2 700013DE
{0x0F12, 0x0543},   //#senHal_pContSenModesRegsArray[33][3]2 700013E0
{0x0F12, 0x0040},   //#senHal_pContSenModesRegsArray[34][0]2 700013E2
{0x0F12, 0x0040},   //#senHal_pContSenModesRegsArray[34][1]2 700013E4
{0x0F12, 0x0040},   //#senHal_pContSenModesRegsArray[34][2]2 700013E6
{0x0F12, 0x0040},   //#senHal_pContSenModesRegsArray[34][3]2 700013E8
{0x0F12, 0x01D0},   //#senHal_pContSenModesRegsArray[35][0]2 700013EA
{0x0F12, 0x01D0},   //#senHal_pContSenModesRegsArray[35][1]2 700013EC
{0x0F12, 0x00EF},   //#senHal_pContSenModesRegsArray[35][2]2 700013EE
{0x0F12, 0x00EF},   //#senHal_pContSenModesRegsArray[35][3]2 700013F0
{0x0F12, 0x020F},   //#senHal_pContSenModesRegsArray[36][0]2 700013F2
{0x0F12, 0x024E},   //#senHal_pContSenModesRegsArray[36][1]2 700013F4
{0x0F12, 0x012E},   //#senHal_pContSenModesRegsArray[36][2]2 700013F6
{0x0F12, 0x016D},   //#senHal_pContSenModesRegsArray[36][3]2 700013F8
{0x0F12, 0x039F},   //#senHal_pContSenModesRegsArray[37][0]2 700013FA
{0x0F12, 0x045E},   //#senHal_pContSenModesRegsArray[37][1]2 700013FC
{0x0F12, 0x01DD},   //#senHal_pContSenModesRegsArray[37][2]2 700013FE
{0x0F12, 0x029C},   //#senHal_pContSenModesRegsArray[37][3]2 70001400
{0x0F12, 0x041D},   //#senHal_pContSenModesRegsArray[38][0]2 70001402
{0x0F12, 0x04A9},   //#senHal_pContSenModesRegsArray[38][1]2 70001404
{0x0F12, 0x025B},   //#senHal_pContSenModesRegsArray[38][2]2 70001406
{0x0F12, 0x02E7},   //#senHal_pContSenModesRegsArray[38][3]2 70001408
{0x0F12, 0x062D},   //#senHal_pContSenModesRegsArray[39][0]2 7000140A
{0x0F12, 0x0639},   //#senHal_pContSenModesRegsArray[39][1]2 7000140C
{0x0F12, 0x038A},   //#senHal_pContSenModesRegsArray[39][2]2 7000140E
{0x0F12, 0x0396},   //#senHal_pContSenModesRegsArray[39][3]2 70001410
{0x0F12, 0x066C},   //#senHal_pContSenModesRegsArray[40][0]2 70001412
{0x0F12, 0x06B7},   //#senHal_pContSenModesRegsArray[40][1]2 70001414
{0x0F12, 0x03C9},   //#senHal_pContSenModesRegsArray[40][2]2 70001416
{0x0F12, 0x0414},   //#senHal_pContSenModesRegsArray[40][3]2 70001418
{0x0F12, 0x087C},   //#senHal_pContSenModesRegsArray[41][0]2 7000141A
{0x0F12, 0x08C7},   //#senHal_pContSenModesRegsArray[41][1]2 7000141C
{0x0F12, 0x04F8},   //#senHal_pContSenModesRegsArray[41][2]2 7000141E
{0x0F12, 0x0543},   //#senHal_pContSenModesRegsArray[41][3]2 70001420
{0x0F12, 0x0040},   //#senHal_pContSenModesRegsArray[42][0]2 70001422
{0x0F12, 0x0040},   //#senHal_pContSenModesRegsArray[42][1]2 70001424
{0x0F12, 0x0040},   //#senHal_pContSenModesRegsArray[42][2]2 70001426
{0x0F12, 0x0040},   //#senHal_pContSenModesRegsArray[42][3]2 70001428
{0x0F12, 0x01D0},   //#senHal_pContSenModesRegsArray[43][0]2 7000142A
{0x0F12, 0x01D0},   //#senHal_pContSenModesRegsArray[43][1]2 7000142C
{0x0F12, 0x00EF},   //#senHal_pContSenModesRegsArray[43][2]2 7000142E
{0x0F12, 0x00EF},   //#senHal_pContSenModesRegsArray[43][3]2 70001430
{0x0F12, 0x020F},   //#senHal_pContSenModesRegsArray[44][0]2 70001432
{0x0F12, 0x024E},   //#senHal_pContSenModesRegsArray[44][1]2 70001434
{0x0F12, 0x012E},   //#senHal_pContSenModesRegsArray[44][2]2 70001436
{0x0F12, 0x016D},   //#senHal_pContSenModesRegsArray[44][3]2 70001438
{0x0F12, 0x039F},   //#senHal_pContSenModesRegsArray[45][0]2 7000143A
{0x0F12, 0x045E},   //#senHal_pContSenModesRegsArray[45][1]2 7000143C
{0x0F12, 0x01DD},   //#senHal_pContSenModesRegsArray[45][2]2 7000143E
{0x0F12, 0x029C},   //#senHal_pContSenModesRegsArray[45][3]2 70001440
{0x0F12, 0x041D},   //#senHal_pContSenModesRegsArray[46][0]2 70001442
{0x0F12, 0x04A9},   //#senHal_pContSenModesRegsArray[46][1]2 70001444
{0x0F12, 0x025B},   //#senHal_pContSenModesRegsArray[46][2]2 70001446
{0x0F12, 0x02E7},   //#senHal_pContSenModesRegsArray[46][3]2 70001448
{0x0F12, 0x062D},   //#senHal_pContSenModesRegsArray[47][0]2 7000144A
{0x0F12, 0x0639},   //#senHal_pContSenModesRegsArray[47][1]2 7000144C
{0x0F12, 0x038A},   //#senHal_pContSenModesRegsArray[47][2]2 7000144E
{0x0F12, 0x0396},   //#senHal_pContSenModesRegsArray[47][3]2 70001450
{0x0F12, 0x066C},   //#senHal_pContSenModesRegsArray[48][0]2 70001452
{0x0F12, 0x06B7},   //#senHal_pContSenModesRegsArray[48][1]2 70001454
{0x0F12, 0x03C9},   //#senHal_pContSenModesRegsArray[48][2]2 70001456
{0x0F12, 0x0414},   //#senHal_pContSenModesRegsArray[48][3]2 70001458
{0x0F12, 0x087C},   //#senHal_pContSenModesRegsArray[49][0]2 7000145A
{0x0F12, 0x08C7},   //#senHal_pContSenModesRegsArray[49][1]2 7000145C
{0x0F12, 0x04F8},   //#senHal_pContSenModesRegsArray[49][2]2 7000145E
{0x0F12, 0x0543},   //#senHal_pContSenModesRegsArray[49][3]2 70001460
{0x0F12, 0x003D},   //#senHal_pContSenModesRegsArray[50][0]2 70001462
{0x0F12, 0x003D},   //#senHal_pContSenModesRegsArray[50][1]2 70001464
{0x0F12, 0x003D},   //#senHal_pContSenModesRegsArray[50][2]2 70001466
{0x0F12, 0x003D},   //#senHal_pContSenModesRegsArray[50][3]2 70001468
{0x0F12, 0x01D2},   //#senHal_pContSenModesRegsArray[51][0]2 7000146A
{0x0F12, 0x01D2},   //#senHal_pContSenModesRegsArray[51][1]2 7000146C
{0x0F12, 0x00F1},   //#senHal_pContSenModesRegsArray[51][2]2 7000146E
{0x0F12, 0x00F1},   //#senHal_pContSenModesRegsArray[51][3]2 70001470
{0x0F12, 0x020C},   //#senHal_pContSenModesRegsArray[52][0]2 70001472
{0x0F12, 0x024B},   //#senHal_pContSenModesRegsArray[52][1]2 70001474
{0x0F12, 0x012B},   //#senHal_pContSenModesRegsArray[52][2]2 70001476
{0x0F12, 0x016A},   //#senHal_pContSenModesRegsArray[52][3]2 70001478
{0x0F12, 0x03A1},   //#senHal_pContSenModesRegsArray[53][0]2 7000147A
{0x0F12, 0x0460},   //#senHal_pContSenModesRegsArray[53][1]2 7000147C
{0x0F12, 0x01DF},   //#senHal_pContSenModesRegsArray[53][2]2 7000147E
{0x0F12, 0x029E},   //#senHal_pContSenModesRegsArray[53][3]2 70001480
{0x0F12, 0x041A},   //#senHal_pContSenModesRegsArray[54][0]2 70001482
{0x0F12, 0x04A6},   //#senHal_pContSenModesRegsArray[54][1]2 70001484
{0x0F12, 0x0258},   //#senHal_pContSenModesRegsArray[54][2]2 70001486
{0x0F12, 0x02E4},   //#senHal_pContSenModesRegsArray[54][3]2 70001488
{0x0F12, 0x062F},   //#senHal_pContSenModesRegsArray[55][0]2 7000148A
{0x0F12, 0x063B},   //#senHal_pContSenModesRegsArray[55][1]2 7000148C
{0x0F12, 0x038C},   //#senHal_pContSenModesRegsArray[55][2]2 7000148E
{0x0F12, 0x0398},   //#senHal_pContSenModesRegsArray[55][3]2 70001490
{0x0F12, 0x0669},   //#senHal_pContSenModesRegsArray[56][0]2 70001492
{0x0F12, 0x06B4},   //#senHal_pContSenModesRegsArray[56][1]2 70001494
{0x0F12, 0x03C6},   //#senHal_pContSenModesRegsArray[56][2]2 70001496
{0x0F12, 0x0411},   //#senHal_pContSenModesRegsArray[56][3]2 70001498
{0x0F12, 0x087E},   //#senHal_pContSenModesRegsArray[57][0]2 7000149A
{0x0F12, 0x08C9},   //#senHal_pContSenModesRegsArray[57][1]2 7000149C
{0x0F12, 0x04FA},   //#senHal_pContSenModesRegsArray[57][2]2 7000149E
{0x0F12, 0x0545},   //#senHal_pContSenModesRegsArray[57][3]2 700014A0
{0x0F12, 0x03A2},   //#senHal_pContSenModesRegsArray[58][0]2 700014A2
{0x0F12, 0x01D3},   //#senHal_pContSenModesRegsArray[58][1]2 700014A4
{0x0F12, 0x01E0},   //#senHal_pContSenModesRegsArray[58][2]2 700014A6
{0x0F12, 0x00F2},   //#senHal_pContSenModesRegsArray[58][3]2 700014A8
{0x0F12, 0x03AF},   //#senHal_pContSenModesRegsArray[59][0]2 700014AA
{0x0F12, 0x01E0},   //#senHal_pContSenModesRegsArray[59][1]2 700014AC
{0x0F12, 0x01ED},   //#senHal_pContSenModesRegsArray[59][2]2 700014AE
{0x0F12, 0x00FF},   //#senHal_pContSenModesRegsArray[59][3]2 700014B0
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[60][0]2 700014B2
{0x0F12, 0x0461},   //#senHal_pContSenModesRegsArray[60][1]2 700014B4
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[60][2]2 700014B6
{0x0F12, 0x029F},   //#senHal_pContSenModesRegsArray[60][3]2 700014B8
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[61][0]2 700014BA
{0x0F12, 0x046E},   //#senHal_pContSenModesRegsArray[61][1]2 700014BC
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[61][2]2 700014BE
{0x0F12, 0x02AC},   //#senHal_pContSenModesRegsArray[61][3]2 700014C0
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[62][0]2 700014C2
{0x0F12, 0x063C},   //#senHal_pContSenModesRegsArray[62][1]2 700014C4
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[62][2]2 700014C6
{0x0F12, 0x0399},   //#senHal_pContSenModesRegsArray[62][3]2 700014C8
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[63][0]2 700014CA
{0x0F12, 0x0649},   //#senHal_pContSenModesRegsArray[63][1]2 700014CC
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[63][2]2 700014CE
{0x0F12, 0x03A6},   //#senHal_pContSenModesRegsArray[63][3]2 700014D0
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[64][0]2 700014D2
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[64][1]2 700014D4
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[64][2]2 700014D6
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[64][3]2 700014D8
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[65][0]2 700014DA
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[65][1]2 700014DC
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[65][2]2 700014DE
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[65][3]2 700014E0
{0x0F12, 0x03AA},   //#senHal_pContSenModesRegsArray[66][0]2 700014E2
{0x0F12, 0x01DB},   //#senHal_pContSenModesRegsArray[66][1]2 700014E4
{0x0F12, 0x01E8},   //#senHal_pContSenModesRegsArray[66][2]2 700014E6
{0x0F12, 0x00FA},   //#senHal_pContSenModesRegsArray[66][3]2 700014E8
{0x0F12, 0x03B7},   //#senHal_pContSenModesRegsArray[67][0]2 700014EA
{0x0F12, 0x01E8},   //#senHal_pContSenModesRegsArray[67][1]2 700014EC
{0x0F12, 0x01F5},   //#senHal_pContSenModesRegsArray[67][2]2 700014EE
{0x0F12, 0x0107},   //#senHal_pContSenModesRegsArray[67][3]2 700014F0
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[68][0]2 700014F2
{0x0F12, 0x0469},   //#senHal_pContSenModesRegsArray[68][1]2 700014F4
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[68][2]2 700014F6
{0x0F12, 0x02A7},   //#senHal_pContSenModesRegsArray[68][3]2 700014F8
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[69][0]2 700014FA
{0x0F12, 0x0476},   //#senHal_pContSenModesRegsArray[69][1]2 700014FC
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[69][2]2 700014FE
{0x0F12, 0x02B4},   //#senHal_pContSenModesRegsArray[69][3]2 70001500
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[70][0]2 70001502
{0x0F12, 0x0644},   //#senHal_pContSenModesRegsArray[70][1]2 70001504
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[70][2]2 70001506
{0x0F12, 0x03A1},   //#senHal_pContSenModesRegsArray[70][3]2 70001508
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[71][0]2 7000150A
{0x0F12, 0x0651},   //#senHal_pContSenModesRegsArray[71][1]2 7000150C
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[71][2]2 7000150E
{0x0F12, 0x03AE},   //#senHal_pContSenModesRegsArray[71][3]2 70001510
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[72][0]2 70001512
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[72][1]2 70001514
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[72][2]2 70001516
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[72][3]2 70001518
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[73][0]2 7000151A
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[73][1]2 7000151C
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[73][2]2 7000151E
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[73][3]2 70001520
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[74][0]2 70001522
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[74][1]2 70001524
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[74][2]2 70001526
{0x0F12, 0x0001},   //#senHal_pContSenModesRegsArray[74][3]2 70001528
{0x0F12, 0x000F},   //#senHal_pContSenModesRegsArray[75][0]2 7000152A
{0x0F12, 0x000F},   //#senHal_pContSenModesRegsArray[75][1]2 7000152C
{0x0F12, 0x000F},   //#senHal_pContSenModesRegsArray[75][2]2 7000152E
{0x0F12, 0x000F},   //#senHal_pContSenModesRegsArray[75][3]2 70001530
{0x0F12, 0x05AD},   //#senHal_pContSenModesRegsArray[76][0]2 70001532
{0x0F12, 0x03DE},   //#senHal_pContSenModesRegsArray[76][1]2 70001534
{0x0F12, 0x030A},   //#senHal_pContSenModesRegsArray[76][2]2 70001536
{0x0F12, 0x021C},   //#senHal_pContSenModesRegsArray[76][3]2 70001538
{0x0F12, 0x062F},   //#senHal_pContSenModesRegsArray[77][0]2 7000153A
{0x0F12, 0x0460},   //#senHal_pContSenModesRegsArray[77][1]2 7000153C
{0x0F12, 0x038C},   //#senHal_pContSenModesRegsArray[77][2]2 7000153E
{0x0F12, 0x029E},   //#senHal_pContSenModesRegsArray[77][3]2 70001540
{0x0F12, 0x07FC},   //#senHal_pContSenModesRegsArray[78][0]2 70001542
{0x0F12, 0x0847},   //#senHal_pContSenModesRegsArray[78][1]2 70001544
{0x0F12, 0x0478},   //#senHal_pContSenModesRegsArray[78][2]2 70001546
{0x0F12, 0x04C3},   //#senHal_pContSenModesRegsArray[78][3]2 70001548
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[79][0]2 7000154A
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[79][1]2 7000154C
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[79][2]2 7000154E
{0x0F12, 0x0000},   //#senHal_pContSenModesRegsArray[79][3]2 70001550
//============================================================
//Analog Setting END
//============================================================

//============================================================
//ISP-FE Setting
//============================================================
{0x002A, 0x158A}, 
{0x0F12, 0xEAF0}, 
{0x002A, 0x15C6}, 
{0x0F12, 0x0020}, 
{0x0F12, 0x0060}, 
{0x002A, 0x15BC}, 
{0x0F12, 0x0200}, 

//Analog Offset for MSM
{0x002A, 0x1608}, 
{0x0F12, 0x0100}, //#gisp_msm_sAnalogOffset[0]
{0x0F12, 0x0100}, //#gisp_msm_sAnalogOffset[1]
{0x0F12, 0x0100}, //#gisp_msm_sAnalogOffset[2]
{0x0F12, 0x0100}, //#gisp_msm_sAnalogOffset[3]

//============================================================
//ISP-FE Setting END
//============================================================

//================================================================================================
//SET PLL
//================================================================================================
//How to set
//1. MCLK
//2. System CLK
//3. PCLK
//================================================================================================
//Set input CLK // 24MHz

{0x002A, 0x01CC}, 
{0x0F12, 0x5DC0}, //#REG_TC_IPRM_InClockLSBs
{0x0F12, 0x0000}, //#REG_TC_IPRM_InClockMSBs
{0x002A, 0x01EE}, 
{0x0F12, 0x0000}, //#REG_TC_IPRM_UseNPviClocks // Number of PLL setting
{0x0F12, 0x0002}, //#REG_TC_IPRM_UseNMiPiClocks
//Set system CLK // 30MHz
{0x002A, 0x01F6}, 
{0x0F12, 0x3A98}, //50M 1D4C 3A98	// #REG_TC_IPRM_OpClk4KHz_0
//Set pixel CLK // 60MHz (0x3A98)
{0x0F12, 0x3A88}, //#REG_TC_IPRM_MinOutRate4KHz_0
{0x0F12, 0x3AA8}, //#REG_TC_IPRM_MaxOutRate4KHz_0

//Set system CLK // 30MHz
{0x0F12, 0x2710}, //50M 1D4C	// #REG_TC_IPRM_OpClk4KHz_1
//Set pixel CLK // 60MHz (0x3A98)
{0x0F12, 0x2318}, //#REG_TC_IPRM_MinOutRate4KHz_1
{0x0F12, 0x2338}, //#REG_TC_IPRM_MaxOutRate4KHz_1
//Update PLL
{0x002A, 0x0208}, 
{0x0F12, 0x0001}, //#REG_TC_IPRM_InitParamsUpdated

//============================================================
//Frame rate setting
//============================================================
//How to set
//1. Exposure value
//dec2hex((1 / (frame rate you want(ms))) * 100d * 4d)
//2. Analog Digital gain
//dec2hex((Analog gain you want) * 256d)
//============================================================
//Set preview exposure time
//002A   0530 
//0F12   3415     //D000 3415//#lt_uMaxExp1 			30ms
//0F12   0000
//0F12   6590     //D000 6590//#lt_uMaxExp2 		  	40ms
//0F12   0000
//002A   167C 
//0F12   9AB0     //D000 9AB0//#evt1_lt_uMaxExp3 	40ms
//0F12   0000
//0F12   3880     //D000 3880//#evt1_lt_uMaxExp4 	40ms
//0F12   0001     //0001
////Set capture exposure time
//002A   0538 
//0F12   3415 //D000 3415#lt_uCapMaxExp1			30ms
//0F12   0000 
//0F12   6590 //D000 6590#lt_uCapMaxExp2      40ms
//0F12   0000 
//002A   1684 
//0F12   9AB0 /D000 9AB0/#evt1_lt_uCapMaxExp3 40ms
//0F12   0000 
//0F12   3880 //D000 13880#evt1_lt_uCapMaxExp4 40ms
//0F12   0001 
//
////Set gain
//002A   0540 
//0F12   0170 //0170 0150	// #lt_uMaxAnGain1
//0F12   0280 //0250 0280// #lt_uMaxAnGain2
//002A   168C 
//0F12   0380 //0380	02A0// #evt1_lt_uMaxAnGain3
//0F12   0800 //0800	0800// #evt1_lt_uMaxAnGain4
//002A   0544 
//0F12   0200 //0100 #lt_uMaxDigGain
//0F12   1000 //0800 #lt_uMaxTotGain

//Set preview exposure time
{0x002A, 0x0530}, 
{0x0F12, 0xD000},     //D000 3415//#lt_uMaxExp1 			30ms
{0x0F12, 0x0000},
{0x0F12, 0xD000},     //D000 6590//#lt_uMaxExp2 		  	40ms
{0x0F12, 0x0000},
{0x002A, 0x167C}, 
{0x0F12, 0xD000},     //D000 9AB0//#evt1_lt_uMaxExp3 	40ms
{0x0F12, 0x0000},
{0x0F12, 0xD000},     //D000 3880//#evt1_lt_uMaxExp4 	40ms
{0x0F12, 0x0000},     //0001

//Set capture exposure time
{0x002A, 0x0538}, 
{0x0F12, 0xD000}, //D000 3415#lt_uCapMaxExp1			30ms
{0x0F12, 0x0000}, 
{0x0F12, 0xD000}, //D000 6590#lt_uCapMaxExp2      40ms
{0x0F12, 0x0000}, 
{0x002A, 0x1684}, 
{0x0F12, 0xD000}, //D000 9AB0/#evt1_lt_uCapMaxExp3 40ms
{0x0F12, 0x0000}, 
{0x0F12, 0xD000}, //D000 13880#evt1_lt_uCapMaxExp4 40ms
{0x0F12, 0x0000}, 

//Set gain
{0x002A, 0x0540}, 
{0x0F12, 0x0170}, //0170 0150	// #lt_uMaxAnGain1
{0x0F12, 0x0280}, //0250 0280// #lt_uMaxAnGain2
{0x002A, 0x168C}, 
{0x0F12, 0x0380}, //0380	02A0// #evt1_lt_uMaxAnGain3
{0x0F12, 0x0800}, //0800	0800// #evt1_lt_uMaxAnGain4
{0x002A, 0x0544}, 
{0x0F12, 0x0200}, //#lt_uMaxDigGain
{0x0F12, 0x1000}, //#lt_uMaxTotGain

{0x002A, 0x1694}, 
{0x0F12, 0x0001}, //#evt1_senHal_bExpandForbid
{0x002A, 0x051A}, 
{0x0F12, 0x0111}, //#lt_uLimitHigh
{0x0F12, 0x00F0}, //#lt_uLimitLow

////================================================================================================
////SET PREVIEW CONFIGURATION_0
////# Foramt : YUV422
////# Size: 640 X 480
////# FPS : 30fps
////================================================================================================
//{0x002A, 0x026C},                                                                                                                    
//{0x0F12, 0x0280},  //0320 //#REG_0TC_PCFG_usWidth//1024                                                                                      
//{0x0F12, 0x01E0},  //0258 //#REG_0TC_PCFG_usHeight //768    026E                                                                             
//{0x0F12, 0x0005},  //#REG_0TC_PCFG_Format            0270                                                                             
//{0x0F12, 0x3AA8},  //#REG_0TC_PCFG_usMaxOut4KHzRate  0272                                                                               
//{0x0F12, 0x3A88},  //#REG_0TC_PCFG_usMinOut4KHzRate  0274                                                                               
//{0x0F12, 0x0100},  //#REG_0TC_PCFG_OutClkPerPix88    0276                                                                               
//{0x0F12, 0x0800},  //#REG_0TC_PCFG_uMaxBpp88         027                                                                                
//{0x0F12, 0x0052},  //0052 //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A                              
//{0x0F12, 0x0050},  //#REG_0TC_PCFG_OIFMask                                                                                              
//{0x0F12, 0x0600},  //#REG_0TC_PCFG_usJpegPacketSize                                                                                     
//{0x0F12, 0x0400},  //#REG_0TC_PCFG_usJpegTotalPackets                                                                                   
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_uClockInd                                                                                            
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_usFrTimeType                                                                                         
//{0x0F12, 0x0001},  //#REG_0TC_PCFG_FrRateQualityType                                                                                    
//{0x0F12, 0x029A},  //03E8  //0580 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //15fps                                                                                
//{0x0F12, 0x029A},  //029A 15fps //01c6 //#REG_0TC_PCFG_usMinFrTimeMsecMult10 // 014D 30fps                                                                        
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_bSmearOutput                                                                                         
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_sSaturation                                                                                          
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_sSharpBlur                                                                                           
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_sColorTemp                                                              
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_uDeviceGammaIndex                                                       
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_uPrevMirror                                                             
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_uCaptureMirror                                                                                                                 
//{0x0F12, 0x0000},  //#REG_0TC_PCFG_uRotation   

//================================================================================================
//SET PREVIEW CONFIGURATION_0
//# Foramt : YUV422
//# Size: 2048 X 1536
//# FPS : 7.5
//================================================================================================
	{0x002A, 0x026C},
	{0x0F12, 0x0800}, //#REG_0TC_PCFG_usWidth/
	{0x0F12, 0x0600}, //#REG_0TC_PCFG_usHeight    		026E
	{0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270
	{0x0F12, 0x3AA8}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
	{0x0F12, 0x3A88}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274
	{0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276
	{0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027
	{0x0F12, 0x0052}, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
	{0x0F12, 0x027E}, //#REG_0TC_PCFG_OIFMask
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegPacketSize
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
	{0x0F12, 0x0002}, //#REG_0TC_PCFG_FrRateQualityType
	{0x0F12, 0x0534}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
	{0x0F12, 0x0534}, //#REG_0TC_PCFG_usMinFrTimeMsecMult10
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uPrevMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation

////================================================================================================
////SET PREVIEW CONFIGURATION_1
////# Foramt : YUV422
////# Size: 720 X 576
////# FPS : 30fps
////================================================================================================
//{0x002A, 0x029C}, 
//{0x0F12, 0x02D0}, //#REG_0TC_PCFG_usWidth
//{0x0F12, 0x0240}, //#REG_0TC_PCFG_usHeight		    026E
//{0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270
//{0x0F12, 0x3AA8}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
//{0x0F12, 0x3A88}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274
//{0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276
//{0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027
//{0x0F12, 0x0052}, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
//{0x0F12, 0x027E}, //#REG_0TC_PCFG_OIFMask
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegPacketSize
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
//{0x0F12, 0x0001}, //#REG_0TC_PCFG_FrRateQualityType
//{0x0F12, 0x014D}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
//{0x0F12, 0x014D}, //#REG_0TC_PCFG_usMinFrTimeMsecMult10
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
//{0x0F12, 0x0003}, //#REG_0TC_PCFG_uPrevMirror
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation

//================================================================================================
//SET PREVIEW CONFIGURATION_1
//# Foramt : YUV422
//# Size: 1920 X 1080
//# FPS : 7.5 ~ 15fps
//================================================================================================
	{0x002A, 0x029C},
	{0x0F12, 0x0780}, //#REG_0TC_PCFG_usWidth
	{0x0F12, 0x0438}, //#REG_0TC_PCFG_usHeight		    026E
	{0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270
	{0x0F12, 0x3AA8}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
	{0x0F12, 0x3A88}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274
	{0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276
	{0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027
	{0x0F12, 0x0052}, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
	{0x0F12, 0x027E}, //#REG_0TC_PCFG_OIFMask
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegPacketSize
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
	{0x0F12, 0x0002}, //#REG_0TC_PCFG_FrRateQualityType
	{0x0F12, 0x0534}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
	{0x0F12, 0x029A}, //#REG_0TC_PCFG_usMinFrTimeMsecMult10
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uPrevMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation

//param_start SET PREVIEW CONFIGURATION_2
////================================================================================================
////SET PREVIEW CONFIGURATION_2
////# Foramt : YUV422
////# Size: 768 X 576
////# FPS : 30fps
////================================================================================================
//{0x002A, 0x02CC}, 
//{0x0F12, 0x0300}, //#REG_0TC_PCFG_usWidth
//{0x0F12, 0x0240}, //#REG_0TC_PCFG_usHeight		    026E
//{0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270
//{0x0F12, 0x3AA8}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
//{0x0F12, 0x3A88}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274
//{0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276
//{0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027
//{0x0F12, 0x0052}, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
//{0x0F12, 0x027E}, //#REG_0TC_PCFG_OIFMask
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegPacketSize
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
//{0x0F12, 0x0001}, //#REG_0TC_PCFG_FrRateQualityType
//{0x0F12, 0x014D}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
//{0x0F12, 0x014D}, //#REG_0TC_PCFG_usMinFrTimeMsecMult10
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
//{0x0F12, 0x0003}, //#REG_0TC_PCFG_uPrevMirror
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation

//================================================================================================
//SET PREVIEW CONFIGURATION_2
//# Foramt : YUV422
//# Size: 1280 X 720
//# FPS : 7.5 ~ 30fps
//================================================================================================
	{0x002A, 0x02CC},
	{0x0F12, 0x0500}, //#REG_0TC_PCFG_usWidth
	{0x0F12, 0x02D0}, //#REG_0TC_PCFG_usHeight		    026E
	{0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270
	{0x0F12, 0x3AA8}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
	{0x0F12, 0x3A88}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274
	{0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276
	{0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027
	{0x0F12, 0x0052}, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
	{0x0F12, 0x027E}, //#REG_0TC_PCFG_OIFMask
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegPacketSize
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
	{0x0F12, 0x0002}, //#REG_0TC_PCFG_FrRateQualityType
	{0x0F12, 0x0534}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
	{0x0F12, 0x014D}, //#REG_0TC_PCFG_usMinFrTimeMsecMult10
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uPrevMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation

//param_start SET PREVIEW CONFIGURATION_3
////================================================================================================
////SET PREVIEW CONFIGURATION_3
////# Foramt : YUV422
////# Size: 800 X 600
////# FPS : 15fps
////================================================================================================
//{0x002A, 0x02FC}, 
//{0x0F12, 0x0320}, //#REG_0TC_PCFG_usWidth
//{0x0F12, 0x0258}, //#REG_0TC_PCFG_usHeight		    026E
//{0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270
//{0x0F12, 0x3AA8}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
//{0x0F12, 0x3A88}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274
//{0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276
//{0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027
//{0x0F12, 0x0052}, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
//{0x0F12, 0x027E}, //#REG_0TC_PCFG_OIFMask
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegPacketSize
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
//{0x0F12, 0x0001}, //#REG_0TC_PCFG_FrRateQualityType
//{0x0F12, 0x014D}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
//{0x0F12, 0x014D}, //#REG_0TC_PCFG_usMinFrTimeMsecMult10
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
//{0x0F12, 0x0003}, //#REG_0TC_PCFG_uPrevMirror
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation

//================================================================================================
//SET PREVIEW CONFIGURATION_3
//# Foramt : YUV422
//# Size: 704 X 576
//# FPS : 30fps
//================================================================================================
	{0x002A, 0x02FC},
	{0x0F12, 0x02C0}, //#REG_0TC_PCFG_usWidth
	{0x0F12, 0x0240}, //#REG_0TC_PCFG_usHeight		    026E
	{0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270
	{0x0F12, 0x3AA8}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
	{0x0F12, 0x3A88}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274
	{0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276
	{0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027
	{0x0F12, 0x0052}, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
	{0x0F12, 0x027E}, //#REG_0TC_PCFG_OIFMask
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegPacketSize
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
	{0x0F12, 0x0001}, //#REG_0TC_PCFG_FrRateQualityType
	{0x0F12, 0x014D}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
	{0x0F12, 0x014D}, //#REG_0TC_PCFG_usMinFrTimeMsecMult10
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uPrevMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation

//param_start SET PREVIEW CONFIGURATION_4
////================================================================================================
////SET PREVIEW CONFIGURATION_4
////# Foramt : YUV422
////# Size: 992 X 560
////# FPS : 15fps
////================================================================================================
//{0x002A, 0x032C}, 
//{0x0F12, 0x03E0}, //#REG_0TC_PCFG_usWidth
//{0x0F12, 0x0230}, //#REG_0TC_PCFG_usHeight		    026E
//{0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270
//{0x0F12, 0x3AA8}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
//{0x0F12, 0x3A88}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274
//{0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276
//{0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027
//{0x0F12, 0x0052}, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
//{0x0F12, 0x027E}, //#REG_0TC_PCFG_OIFMask
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegPacketSize
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
//{0x0F12, 0x0001}, //#REG_0TC_PCFG_FrRateQualityType
//{0x0F12, 0x014D}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
//{0x0F12, 0x014D}, //#REG_0TC_PCFG_usMinFrTimeMsecMult10
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
//{0x0F12, 0x0003}, //#REG_0TC_PCFG_uPrevMirror
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror
//{0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation

//================================================================================================
//SET PREVIEW CONFIGURATION_4
//# Foramt : YUV422
//# Size: 640 X 480
//# FPS : 30fps
//================================================================================================
	{0x002A, 0x032C},
	{0x0F12, 0x0280}, //#REG_0TC_PCFG_usWidth
	{0x0F12, 0x01E0}, //#REG_0TC_PCFG_usHeight		    026E
	{0x0F12, 0x0005}, //#REG_0TC_PCFG_Format            0270
	{0x0F12, 0x3AA8}, //#REG_0TC_PCFG_usMaxOut4KHzRate  0272
	{0x0F12, 0x3A88}, //#REG_0TC_PCFG_usMinOut4KHzRate  0274
	{0x0F12, 0x0100}, //#REG_0TC_PCFG_OutClkPerPix88    0276
	{0x0F12, 0x0800}, //#REG_0TC_PCFG_uMaxBpp88         027
	{0x0F12, 0x0052}, //#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
	{0x0F12, 0x027E}, //#REG_0TC_PCFG_OIFMask
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegPacketSize
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usJpegTotalPackets
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uClockInd
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_usFrTimeType
	{0x0F12, 0x0001}, //#REG_0TC_PCFG_FrRateQualityType
	{0x0F12, 0x014D}, //03E8 //#REG_0TC_PCFG_usMaxFrTimeMsecMult10
	{0x0F12, 0x014D}, //#REG_0TC_PCFG_usMinFrTimeMsecMult10
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_bSmearOutput
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSaturation
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sSharpBlur
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_sColorTemp
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uDeviceGammaIndex
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uPrevMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uCaptureMirror
	{0x0F12, 0x0000}, //#REG_0TC_PCFG_uRotation
	
////================================================================================================
////SET CAPTURE CONFIGURATION_0
////# Foramt :JPEG
////# Size: 1920 X 1080
////# FPS : 7.5 ~ 15fps
////================================================================================================
//{0x002A, 0x035C}, 
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
//{0x0F12, 0x0780}, //#REG_0TC_CCFG_usWidth
//{0x0F12, 0x0438}, //#REG_0TC_CCFG_usHeight
//{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
//{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
//{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
//{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
//{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
//{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
//{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
//{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
//{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
//{0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType
//{0x0F12, 0x0534}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
//{0x0F12, 0x029A}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex
//
//param_start SET CAPTURE CONFIGURATION_0
//================================================================================================
//SET CAPTURE CONFIGURATION_0
//# Foramt :YUV422
//# Size: 2048 X 1536
//# FPS : 7.5
//================================================================================================
{0x002A, 0x035C}, 
{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
{0x0F12, 0x0800}, //#REG_0TC_CCFG_usWidth
{0x0F12, 0x0600}, //#REG_0TC_CCFG_usHeight
{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
{0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType
{0x0F12, 0x0534}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
{0x0F12, 0x0534}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex

////param_start SET CAPTURE CONFIGURATION_1
////================================================================================================
////SET CAPTURE CONFIGURATION_1
////# Foramt :JPEG
////# Size: 1280 X 720
////# FPS : 7.5 ~ 15fps
////================================================================================================
//{0x002A, 0x0388}, 
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
//{0x0F12, 0x0500}, //#REG_0TC_CCFG_usWidth
//{0x0F12, 0x02D0}, //#REG_0TC_CCFG_usHeight
//{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
//{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
//{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
//{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
//{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
//{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
//{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
//{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
//{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
//{0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType
//{0x0F12, 0x0535}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
//{0x0F12, 0x029A}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex

//================================================================================================
//SET CAPTURE CONFIGURATION_1
//# Foramt :YUV422
//# Size: 1920 X 1080
//# FPS : 7.5 ~ 15fps
//================================================================================================
{0x002A, 0x0388}, 
{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
{0x0F12, 0x0780}, //#REG_0TC_CCFG_usWidth
{0x0F12, 0x0438}, //#REG_0TC_CCFG_usHeight
{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
{0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType
{0x0F12, 0x0534}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
{0x0F12, 0x029A}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex


////param_start SET CAPTURE CONFIGURATION_2
////================================================================================================
////SET CAPTURE CONFIGURATION_2
////# Foramt :JPEG
////# Size: 1280 X 1024
////# FPS : 7.5 ~ 15fps
////================================================================================================
//{0x002A, 0x03B4}, 
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
//{0x0F12, 0x0500}, //#REG_0TC_CCFG_usWidth
//{0x0F12, 0x0400}, //#REG_0TC_CCFG_usHeight
//{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
//{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
//{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
//{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
//{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
//{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
//{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
//{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
//{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
//{0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType
//{0x0F12, 0x0535}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
//{0x0F12, 0x029A}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex

//================================================================================================
//SET CAPTURE CONFIGURATION_2
//# Foramt :YUV422
//# Size: 1280 X 720
//# FPS : 7.5 ~ 30fps
//================================================================================================
	{0x002A, 0x03B4},
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
	{0x0F12, 0x0500}, //#REG_0TC_CCFG_usWidth
	{0x0F12, 0x02D0}, //#REG_0TC_CCFG_usHeight
	{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
	{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
	{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
	{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
	{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
	{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
	{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
	{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
	{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
	{0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType
	{0x0F12, 0x0534}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
	{0x0F12, 0x014D}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex

//param_start SET CAPTURE CONFIGURATION_3
////================================================================================================
////SET CAPTURE CONFIGURATION_3
////# Foramt :JPEG
////# Size: 1600 X 1200
////# FPS : 7.5 ~ 15fps
////================================================================================================
//{0x002A, 0x03E0}, 
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
//{0x0F12, 0x0640}, //#REG_0TC_CCFG_usWidth
//{0x0F12, 0x04B0}, //#REG_0TC_CCFG_usHeight
//{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
//{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
//{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
//{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
//{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
//{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
//{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
//{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
//{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
//{0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType
//{0x0F12, 0x0535}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
//{0x0F12, 0x029A}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex

//================================================================================================
//SET CAPTURE CONFIGURATION_3
//# Foramt :YUV422
//# Size: 704 X 576
//# FPS : 30fps
//================================================================================================
	{0x002A, 0x03E0},
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
	{0x0F12, 0x02c0}, //#REG_0TC_CCFG_usWidth
	{0x0F12, 0x0240}, //#REG_0TC_CCFG_usHeight
	{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
	{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
	{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
	{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
	{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
	{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
	{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
	{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
	{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
	{0x0F12, 0x0001}, //#REG_0TC_CCFG_FrRateQualityType
	{0x0F12, 0x014D}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
	{0x0F12, 0x014D}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex

//param_start SET CAPTURE CONFIGURATION_4
////================================================================================================
////SET CAPTURE CONFIGURATION_4
////# Foramt :JPEG
////# Size: 2048 X 1536
////# FPS : 7.5 ~ 15fps
////================================================================================================
//{0x002A, 0x040C}, 
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
//{0x0F12, 0x0800}, //#REG_0TC_CCFG_usWidth
//{0x0F12, 0x0600}, //#REG_0TC_CCFG_usHeight
//{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
//{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
//{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
//{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
//{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
//{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
//{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
//{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
//{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
//{0x0F12, 0x0002}, //#REG_0TC_CCFG_FrRateQualityType
//{0x0F12, 0x0535}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
//{0x0F12, 0x029A}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
//{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex

//================================================================================================
//SET CAPTURE CONFIGURATION_4
//# Foramt :YUV422
//# Size: 640 X 480
//# FPS : 30fps
//================================================================================================
	{0x002A, 0x040C},
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_uCaptureModeJpEG
	{0x0F12, 0x0280}, //#REG_0TC_CCFG_usWidth
	{0x0F12, 0x01E0}, //#REG_0TC_CCFG_usHeight
	{0x0F12, 0x0005}, //#REG_0TC_CCFG_Format//5:YUV9:JPEG
	{0x0F12, 0x3AA8}, //#REG_0TC_CCFG_usMaxOut4KHzRate
	{0x0F12, 0x3A88}, //#REG_0TC_CCFG_usMinOut4KHzRate
	{0x0F12, 0x0100}, //#REG_0TC_CCFG_OutClkPerPix88
	{0x0F12, 0x0800}, //#REG_0TC_CCFG_uMaxBpp88
	{0x0F12, 0x0052}, //#REG_0TC_CCFG_PVIMask
	{0x0F12, 0x0050}, //#REG_0TC_CCFG_OIFMask   edison
	{0x0F12, 0x0600}, //#REG_0TC_CCFG_usJpegPacketSize
	{0x0F12, 0x0400}, //#REG_0TC_CCFG_usJpegTotalPackets
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_uClockInd
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_usFrTimeType
	{0x0F12, 0x0001}, //#REG_0TC_CCFG_FrRateQualityType
	{0x0F12, 0x014D}, //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps
	{0x0F12, 0x014D}, //#REG_0TC_CCFG_usMinFrTimeMsecMult10 //15fps
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_bSmearOutput
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSaturation
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_sSharpBlur
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_sColorTemp
	{0x0F12, 0x0000}, //#REG_0TC_CCFG_uDeviceGammaIndex


//================================================================================================
//SET JPEG & SPOOF
//================================================================================================
{0x002A, 0x0454}, 
{0x0F12, 0x0055}, //#REG_TC_BRC_usCaptureQuality // JPEG BRC (BitRateControl) value // 85
//WRITE D000007A         0A02     // JPEG CLK off
//WRITE 70000450  0001  //#BRC_BRC_Type // JPEG inspection off

//================================================================================================
//SET THUMBNAIL
//# Foramt : RGB565
//# Size: VGA
//================================================================================================
{0x0F12, 0x0000}, //0001	// thumbnail disable
{0x0F12, 0x0140}, //Width//320 //640
{0x0F12, 0x00F0}, //Height //240 //480
{0x0F12, 0x0000}, //Thumbnail format : 0RGB565 1 RGB888 2 Full-YUV (0-255)
//================================================================================================
//SET AE
//================================================================================================
//AE target
{0x002A, 0x0F70}, 
{0x0F12, 0x0040}, //003A	// #TVAR_ae_BrAve
//AE mode
{0x002A, 0x0F76},
{0x0F12, 0x000F}, //Disable illumination & contrast  // #ae_StatMode
//AE weight
{0x002A, 0x0F7E}, 
{0x0F12, 0x0101},    // #ae_WeightTbl_16_0_
{0x0F12, 0x0101},    // #ae_WeightTbl_16_1_
{0x0F12, 0x0101},    // #ae_WeightTbl_16_2_
{0x0F12, 0x0101},    // #ae_WeightTbl_16_3_
{0x0F12, 0x0101},    // #ae_WeightTbl_16_4_
{0x0F12, 0x0101},    // #ae_WeightTbl_16_5_
{0x0F12, 0x0101},    // #ae_WeightTbl_16_6_
{0x0F12, 0x0101},    // #ae_WeightTbl_16_7_
{0x0F12, 0x0101},    // #ae_WeightTbl_16_8_
{0x0F12, 0x0303},    // #ae_WeightTbl_16_9_
{0x0F12, 0x0303},    // #ae_WeightTbl_16_10
{0x0F12, 0x0101},    // #ae_WeightTbl_16_11
{0x0F12, 0x0101},    // #ae_WeightTbl_16_12
{0x0F12, 0x0303},    // #ae_WeightTbl_16_13
{0x0F12, 0x0303},    // #ae_WeightTbl_16_14
{0x0F12, 0x0101},    // #ae_WeightTbl_16_15
{0x0F12, 0x0101},    // #ae_WeightTbl_16_16
{0x0F12, 0x0303},    // #ae_WeightTbl_16_17
{0x0F12, 0x0303},    // #ae_WeightTbl_16_18
{0x0F12, 0x0101},    // #ae_WeightTbl_16_19
{0x0F12, 0x0101},    // #ae_WeightTbl_16_20
{0x0F12, 0x0303},    // #ae_WeightTbl_16_21
{0x0F12, 0x0303},    // #ae_WeightTbl_16_22
{0x0F12, 0x0101},    // #ae_WeightTbl_16_23
{0x0F12, 0x0101},    // #ae_WeightTbl_16_24
{0x0F12, 0x0101},    // #ae_WeightTbl_16_25
{0x0F12, 0x0101},    // #ae_WeightTbl_16_26
{0x0F12, 0x0101},    // #ae_WeightTbl_16_27
{0x0F12, 0x0101},    // #ae_WeightTbl_16_28
{0x0F12, 0x0101},    // #ae_WeightTbl_16_29
{0x0F12, 0x0101},    // #ae_WeightTbl_16_30
{0x0F12, 0x0101},    // #ae_WeightTbl_16_31
//================================================================================================
//SET FLICKER
//================================================================================================
{0x002A, 0x0C18},
{0x0F12, 0x0000}, //0001: 60Hz start auto / 0000: 50Hz start auto
{0x002A, 0x04D2},
{0x0F12, 0x067F},
//================================================================================================
// SET GAS
//================================================================================================
// GAS alpha
// R, Gr, Gb, B per light source
{0x002A, 0x06CE},
{0x0F12, 0x0100},	//0150//0100	// #TVAR_ash_GASAlpha[0] // Horizon
{0x0F12, 0x0100},	//0113//0100	// #TVAR_ash_GASAlpha[1]
{0x0F12, 0x0100}, 	//011D//0100	// #TVAR_ash_GASAlpha[2]
{0x0F12, 0x0100},  //0100//0100	// #TVAR_ash_GASAlpha[3]
{0x0F12, 0x0100},  //0146//0100	// #TVAR_ash_GASAlpha[4] // IncandA
{0x0F12, 0x0100},  //0113//0100	// #TVAR_ash_GASAlpha[5]
{0x0F12, 0x0100}, 	//011D//0100	// #TVAR_ash_GASAlpha[6]
{0x0F12, 0x0100}, 	//0109//0100	// #TVAR_ash_GASAlpha[7]
{0x0F12, 0x0100}, 	//0120//0100	// #TVAR_ash_GASAlpha[8] // WW
{0x0F12, 0x0100}, 	//00E6//0100	// #TVAR_ash_GASAlpha[9]
{0x0F12, 0x0100}, 	//00F0//0100	// #TVAR_ash_GASAlpha[10]
{0x0F12, 0x0100}, 	//00DD//0100	// #TVAR_ash_GASAlpha[11]
{0x0F12, 0x0100}, 	//00D7//0100	// #TVAR_ash_GASAlpha[12]// CWF
{0x0F12, 0x0100}, 	//00F9//0100	// #TVAR_ash_GASAlpha[13]
{0x0F12, 0x0100}, 	//00F7//0100	// #TVAR_ash_GASAlpha[14]
{0x0F12, 0x0100}, 	//00EF//0100	// #TVAR_ash_GASAlpha[15]
{0x0F12, 0x0100}, 	//00F0//0100	// #TVAR_ash_GASAlpha[16]// D50
{0x0F12, 0x0100}, 	//0103//0100	// #TVAR_ash_GASAlpha[17]
{0x0F12, 0x0100}, 	//00FC//0100	// #TVAR_ash_GASAlpha[18]
{0x0F12, 0x0100}, 	//010C//0100	// #TVAR_ash_GASAlpha[19]
{0x0F12, 0x0100}, 	//0100//0100	// #TVAR_ash_GASAlpha[20]// D65
{0x0F12, 0x0100}, 	//00FF//0100	// #TVAR_ash_GASAlpha[21]
{0x0F12, 0x0100}, 	//00F3//0100	// #TVAR_ash_GASAlpha[22]
{0x0F12, 0x0100}, 	//010F//0100	// #TVAR_ash_GASAlpha[23]
{0x0F12, 0x0100}, 	//0100//0100	// #TVAR_ash_GASAlpha[24]// D75
{0x0F12, 0x0100}, 	//0109//0100	// #TVAR_ash_GASAlpha[25]
{0x0F12, 0x0100}, 	//00FA//0100	// #TVAR_ash_GASAlpha[26]
{0x0F12, 0x0100}, 	//011E//0100	// #TVAR_ash_GASAlpha[27]
{0x0F12, 0x0100}, 	//00C8//0100	// #TVAR_ash_GASOutdoorAlpha[0] // Outdoor
{0x0F12, 0x0100}, 	//00F0//0100	// #TVAR_ash_GASOutdoorAlpha[1]
{0x0F12, 0x0100}, 	//00F8//0100	// #TVAR_ash_GASOutdoorAlpha[2]
{0x0F12, 0x0100}, 	//00F8//0100	// #TVAR_ash_GASOutdoorAlpha[3]
// GAS beta
{0x0F12, 0x0000}, //005A//0000	// #ash_GASBeta[0]// Horizon
{0x0F12, 0x0000}, //002D//0000	// #ash_GASBeta[1]
{0x0F12, 0x0000}, //002D//0000	// #ash_GASBeta[2]
{0x0F12, 0x0000}, //0000//0000	// #ash_GASBeta[3]
{0x0F12, 0x0000}, //005B//0000	// #ash_GASBeta[4]// IncandA
{0x0F12, 0x0000}, //0029//0000	// #ash_GASBeta[5]
{0x0F12, 0x0000}, //002A//0000	// #ash_GASBeta[6]
{0x0F12, 0x0000}, //0000//0000	// #ash_GASBeta[7]
{0x0F12, 0x0000}, //0040//0000	// #ash_GASBeta[8]// WW
{0x0F12, 0x0000}, //002E//0000	// #ash_GASBeta[9]
{0x0F12, 0x0000}, //0031//0000	// #ash_GASBeta[10]
{0x0F12, 0x0000}, //0000//0000	// #ash_GASBeta[11]
{0x0F12, 0x0000}, //0025//0000	// #ash_GASBeta[12] // CWF
{0x0F12, 0x0000}, //0029//0000	// #ash_GASBeta[13]
{0x0F12, 0x0000}, //0027//0000	// #ash_GASBeta[14]
{0x0F12, 0x0000}, //0000//0000	// #ash_GASBeta[15]
{0x0F12, 0x0000}, //0037//0000	// #ash_GASBeta[16] // D50
{0x0F12, 0x0000}, //001F//0000	// #ash_GASBeta[17]
{0x0F12, 0x0000}, //001C//0000	// #ash_GASBeta[18]
{0x0F12, 0x0000}, //0000//0000	// #ash_GASBeta[19]
{0x0F12, 0x0000}, //0031//0000	// #ash_GASBeta[20] // D65
{0x0F12, 0x0000}, //001A//0000	// #ash_GASBeta[21]
{0x0F12, 0x0000}, //0017//0000	// #ash_GASBeta[22]
{0x0F12, 0x0000}, //0000//0000	// #ash_GASBeta[23]
{0x0F12, 0x0000}, //0031//0010	// #ash_GASBeta[24] // D75
{0x0F12, 0x0000}, //0019//0000	// #ash_GASBeta[25]
{0x0F12, 0x0000}, //0014//0000	// #ash_GASBeta[26]
{0x0F12, 0x0000}, //0000//0000	// #ash_GASBeta[27]
{0x0F12, 0x0000}, //0035//0000	// #ash_GASOutdoorBeta[0] // Outdoor
{0x0F12, 0x0000}, //001D//0000	// #ash_GASOutdoorBeta[1]
{0x0F12, 0x0000}, //001A//0000	// #ash_GASOutdoorBeta[2]
{0x0F12, 0x0000}, //0000//0000	// #ash_GASOutdoorBeta[3]

// Parabloic function
{0x002A, 0x075A},
{0x0F12, 0x0000}, 	//0000	// #ash_bParabolicEstimation
{0x0F12, 0x0400}, 	//0400	// #ash_uParabolicCenterX
{0x0F12, 0x0300}, 	//0300	// #ash_uParabolicCenterY
{0x0F12, 0x0010}, 	//0010	// #ash_uParabolicScalingA
{0x0F12, 0x0011}, 	//0011	// #ash_uParabolicScalingB

//ash_CGrasAlphas
{0x002A, 0x06C6}, 
{0x0F12, 0x0100}, 	//0100	//ash_CGrasAlphas_0_
{0x0F12, 0x0100}, 	//0100	//ash_CGrasAlphas_1_
{0x0F12, 0x0100}, 	//0100	//ash_CGrasAlphas_2_
{0x0F12, 0x0100}, 	//0100	//ash_CGrasAlphas_3_

{0x002A, 0x074E}, 
{0x0F12, 0x0001},  // #ash_bLumaMode //use Beta : 0001 not use Beta : 0000
{0x002A, 0x0D30}, 
{0x0F12, 0x02A8},  //02A7	// #awbb_GLocusR
{0x0F12, 0x0347}, 	//0343	// #awbb_GLocusB

// GAS LUT start address // 7000_347C
{0x002A, 0x0754}, 
{0x0F12, 0x347C}, 
{0x0F12, 0x7000}, 
// GAS LUT
{0x002A, 0x347C},  
{0x0F12, 0x0262},  //#TVAR_ash_pGAS[0]
{0x0F12, 0x01E1},  //#TVAR_ash_pGAS[1]
{0x0F12, 0x018C},  //#TVAR_ash_pGAS[2]
{0x0F12, 0x013E},  //#TVAR_ash_pGAS[3]
{0x0F12, 0x0108},  //#TVAR_ash_pGAS[4]
{0x0F12, 0x00E5},  //#TVAR_ash_pGAS[5]
{0x0F12, 0x00D6},  //#TVAR_ash_pGAS[6]
{0x0F12, 0x00D4},  //#TVAR_ash_pGAS[7]
{0x0F12, 0x00E4},  //#TVAR_ash_pGAS[8]
{0x0F12, 0x0107},  //#TVAR_ash_pGAS[9]
{0x0F12, 0x0142},  //#TVAR_ash_pGAS[10]
{0x0F12, 0x0192},  //#TVAR_ash_pGAS[11]
{0x0F12, 0x01E0},  //#TVAR_ash_pGAS[12]
{0x0F12, 0x0208},  //#TVAR_ash_pGAS[13]
{0x0F12, 0x019E},  //#TVAR_ash_pGAS[14]
{0x0F12, 0x013F},  //#TVAR_ash_pGAS[15]
{0x0F12, 0x00F4},  //#TVAR_ash_pGAS[16]
{0x0F12, 0x00BE},  //#TVAR_ash_pGAS[17]
{0x0F12, 0x009D},  //#TVAR_ash_pGAS[18]
{0x0F12, 0x008C},  //#TVAR_ash_pGAS[19]
{0x0F12, 0x008C},  //#TVAR_ash_pGAS[20]
{0x0F12, 0x00A0},  //#TVAR_ash_pGAS[21]
{0x0F12, 0x00C4},  //#TVAR_ash_pGAS[22]
{0x0F12, 0x00FF},  //#TVAR_ash_pGAS[23]
{0x0F12, 0x014E},  //#TVAR_ash_pGAS[24]
{0x0F12, 0x01A0},  //#TVAR_ash_pGAS[25]
{0x0F12, 0x01CA},  //#TVAR_ash_pGAS[26]
{0x0F12, 0x0160},  //#TVAR_ash_pGAS[27]
{0x0F12, 0x0101},  //#TVAR_ash_pGAS[28]
{0x0F12, 0x00B4},  //#TVAR_ash_pGAS[29]
{0x0F12, 0x007F},  //#TVAR_ash_pGAS[30]
{0x0F12, 0x005D},  //#TVAR_ash_pGAS[31]
{0x0F12, 0x004D},  //#TVAR_ash_pGAS[32]
{0x0F12, 0x0050},  //#TVAR_ash_pGAS[33]
{0x0F12, 0x0066},  //#TVAR_ash_pGAS[34]
{0x0F12, 0x008C},  //#TVAR_ash_pGAS[35]
{0x0F12, 0x00C6},  //#TVAR_ash_pGAS[36]
{0x0F12, 0x0116},  //#TVAR_ash_pGAS[37]
{0x0F12, 0x016F},  //#TVAR_ash_pGAS[38]
{0x0F12, 0x019C},  //#TVAR_ash_pGAS[39]
{0x0F12, 0x0133},  //#TVAR_ash_pGAS[40]
{0x0F12, 0x00D0},  //#TVAR_ash_pGAS[41]
{0x0F12, 0x0087},  //#TVAR_ash_pGAS[42]
{0x0F12, 0x0051},  //#TVAR_ash_pGAS[43]
{0x0F12, 0x002E},  //#TVAR_ash_pGAS[44]
{0x0F12, 0x0020},  //#TVAR_ash_pGAS[45]
{0x0F12, 0x0025},  //#TVAR_ash_pGAS[46]
{0x0F12, 0x003B},  //#TVAR_ash_pGAS[47]
{0x0F12, 0x0064},  //#TVAR_ash_pGAS[48]
{0x0F12, 0x00A2},  //#TVAR_ash_pGAS[49]
{0x0F12, 0x00F2},  //#TVAR_ash_pGAS[50]
{0x0F12, 0x014B},  //#TVAR_ash_pGAS[51]
{0x0F12, 0x0186},  //#TVAR_ash_pGAS[52]
{0x0F12, 0x0118},  //#TVAR_ash_pGAS[53]
{0x0F12, 0x00B3},  //#TVAR_ash_pGAS[54]
{0x0F12, 0x006C},  //#TVAR_ash_pGAS[55]
{0x0F12, 0x0034},  //#TVAR_ash_pGAS[56]
{0x0F12, 0x0015},  //#TVAR_ash_pGAS[57]
{0x0F12, 0x0007},  //#TVAR_ash_pGAS[58]
{0x0F12, 0x000B},  //#TVAR_ash_pGAS[59]
{0x0F12, 0x0021},  //#TVAR_ash_pGAS[60]
{0x0F12, 0x004D},  //#TVAR_ash_pGAS[61]
{0x0F12, 0x008F},  //#TVAR_ash_pGAS[62]
{0x0F12, 0x00DE},  //#TVAR_ash_pGAS[63]
{0x0F12, 0x0139},  //#TVAR_ash_pGAS[64]
{0x0F12, 0x017C},  //#TVAR_ash_pGAS[65]
{0x0F12, 0x010D},  //#TVAR_ash_pGAS[66]
{0x0F12, 0x00AA},  //#TVAR_ash_pGAS[67]
{0x0F12, 0x0063},  //#TVAR_ash_pGAS[68]
{0x0F12, 0x002D},  //#TVAR_ash_pGAS[69]
{0x0F12, 0x000D},  //#TVAR_ash_pGAS[70]
{0x0F12, 0x0000},  //#TVAR_ash_pGAS[71]
{0x0F12, 0x0005},  //#TVAR_ash_pGAS[72]
{0x0F12, 0x001C},  //#TVAR_ash_pGAS[73]
{0x0F12, 0x004B},  //#TVAR_ash_pGAS[74]
{0x0F12, 0x008C},  //#TVAR_ash_pGAS[75]
{0x0F12, 0x00DD},  //#TVAR_ash_pGAS[76]
{0x0F12, 0x013B},  //#TVAR_ash_pGAS[77]
{0x0F12, 0x0183},  //#TVAR_ash_pGAS[78]
{0x0F12, 0x011A},  //#TVAR_ash_pGAS[79]
{0x0F12, 0x00B7},  //#TVAR_ash_pGAS[80]
{0x0F12, 0x0070},  //#TVAR_ash_pGAS[81]
{0x0F12, 0x0039},  //#TVAR_ash_pGAS[82]
{0x0F12, 0x0019},  //#TVAR_ash_pGAS[83]
{0x0F12, 0x000B},  //#TVAR_ash_pGAS[84]
{0x0F12, 0x0011},  //#TVAR_ash_pGAS[85]
{0x0F12, 0x002B},  //#TVAR_ash_pGAS[86]
{0x0F12, 0x0058},  //#TVAR_ash_pGAS[87]
{0x0F12, 0x009B},  //#TVAR_ash_pGAS[88]
{0x0F12, 0x00EA},  //#TVAR_ash_pGAS[89]
{0x0F12, 0x0149},  //#TVAR_ash_pGAS[90]
{0x0F12, 0x01A1},  //#TVAR_ash_pGAS[91]
{0x0F12, 0x0136},  //#TVAR_ash_pGAS[92]
{0x0F12, 0x00D4},  //#TVAR_ash_pGAS[93]
{0x0F12, 0x008C},  //#TVAR_ash_pGAS[94]
{0x0F12, 0x0057},  //#TVAR_ash_pGAS[95]
{0x0F12, 0x0036},  //#TVAR_ash_pGAS[96]
{0x0F12, 0x002A},  //#TVAR_ash_pGAS[97]
{0x0F12, 0x0030},  //#TVAR_ash_pGAS[98]
{0x0F12, 0x004B},  //#TVAR_ash_pGAS[99]
{0x0F12, 0x0078},  //#TVAR_ash_pGAS[100]
{0x0F12, 0x00B8},  //#TVAR_ash_pGAS[101]
{0x0F12, 0x010E},  //#TVAR_ash_pGAS[102]
{0x0F12, 0x016D},  //#TVAR_ash_pGAS[103]
{0x0F12, 0x01CD},  //#TVAR_ash_pGAS[104]
{0x0F12, 0x0168},  //#TVAR_ash_pGAS[105]
{0x0F12, 0x0104},  //#TVAR_ash_pGAS[106]
{0x0F12, 0x00BC},  //#TVAR_ash_pGAS[107]
{0x0F12, 0x008B},  //#TVAR_ash_pGAS[108]
{0x0F12, 0x006B},  //#TVAR_ash_pGAS[109]
{0x0F12, 0x005F},  //#TVAR_ash_pGAS[110]
{0x0F12, 0x0064},  //#TVAR_ash_pGAS[111]
{0x0F12, 0x0080},  //#TVAR_ash_pGAS[112]
{0x0F12, 0x00AD},  //#TVAR_ash_pGAS[113]
{0x0F12, 0x00EC},  //#TVAR_ash_pGAS[114]
{0x0F12, 0x0145},  //#TVAR_ash_pGAS[115]
{0x0F12, 0x01A1},  //#TVAR_ash_pGAS[116]
{0x0F12, 0x020A},  //#TVAR_ash_pGAS[117]
{0x0F12, 0x01A3},  //#TVAR_ash_pGAS[118]
{0x0F12, 0x014E},  //#TVAR_ash_pGAS[119]
{0x0F12, 0x0105},  //#TVAR_ash_pGAS[120]
{0x0F12, 0x00D3},  //#TVAR_ash_pGAS[121]
{0x0F12, 0x00B6},  //#TVAR_ash_pGAS[122]
{0x0F12, 0x00AA},  //#TVAR_ash_pGAS[123]
{0x0F12, 0x00B2},  //#TVAR_ash_pGAS[124]
{0x0F12, 0x00C8},  //#TVAR_ash_pGAS[125]
{0x0F12, 0x00F5},  //#TVAR_ash_pGAS[126]
{0x0F12, 0x0139},  //#TVAR_ash_pGAS[127]
{0x0F12, 0x0192},  //#TVAR_ash_pGAS[128]
{0x0F12, 0x01E7},  //#TVAR_ash_pGAS[129]
{0x0F12, 0x0262},  //#TVAR_ash_pGAS[130]
{0x0F12, 0x01EE},  //#TVAR_ash_pGAS[131]
{0x0F12, 0x01A7},  //#TVAR_ash_pGAS[132]
{0x0F12, 0x0160},  //#TVAR_ash_pGAS[133]
{0x0F12, 0x012D},  //#TVAR_ash_pGAS[134]
{0x0F12, 0x0110},  //#TVAR_ash_pGAS[135]
{0x0F12, 0x0101},  //#TVAR_ash_pGAS[136]
{0x0F12, 0x0106},  //#TVAR_ash_pGAS[137]
{0x0F12, 0x011F},  //#TVAR_ash_pGAS[138]
{0x0F12, 0x014B},  //#TVAR_ash_pGAS[139]
{0x0F12, 0x0191},  //#TVAR_ash_pGAS[140]
{0x0F12, 0x01DD},  //#TVAR_ash_pGAS[141]
{0x0F12, 0x023A},  //#TVAR_ash_pGAS[142]
{0x0F12, 0x01E0},  //#TVAR_ash_pGAS[143]
{0x0F12, 0x017A},  //#TVAR_ash_pGAS[144]
{0x0F12, 0x0137},  //#TVAR_ash_pGAS[145]
{0x0F12, 0x00FF},  //#TVAR_ash_pGAS[146]
{0x0F12, 0x00D4},  //#TVAR_ash_pGAS[147]
{0x0F12, 0x00B4},  //#TVAR_ash_pGAS[148]
{0x0F12, 0x00A7},  //#TVAR_ash_pGAS[149]
{0x0F12, 0x00A7},  //#TVAR_ash_pGAS[150]
{0x0F12, 0x00B2},  //#TVAR_ash_pGAS[151]
{0x0F12, 0x00CE},  //#TVAR_ash_pGAS[152]
{0x0F12, 0x00F9},  //#TVAR_ash_pGAS[153]
{0x0F12, 0x0132},  //#TVAR_ash_pGAS[154]
{0x0F12, 0x017D},  //#TVAR_ash_pGAS[155]
{0x0F12, 0x0196},  //#TVAR_ash_pGAS[156]
{0x0F12, 0x013E},  //#TVAR_ash_pGAS[157]
{0x0F12, 0x00F8},  //#TVAR_ash_pGAS[158]
{0x0F12, 0x00C1},  //#TVAR_ash_pGAS[159]
{0x0F12, 0x0098},  //#TVAR_ash_pGAS[160]
{0x0F12, 0x007C},  //#TVAR_ash_pGAS[161]
{0x0F12, 0x006D},  //#TVAR_ash_pGAS[162]
{0x0F12, 0x006E},  //#TVAR_ash_pGAS[163]
{0x0F12, 0x007B},  //#TVAR_ash_pGAS[164]
{0x0F12, 0x0094},  //#TVAR_ash_pGAS[165]
{0x0F12, 0x00C1},  //#TVAR_ash_pGAS[166]
{0x0F12, 0x00FD},  //#TVAR_ash_pGAS[167]
{0x0F12, 0x0142},  //#TVAR_ash_pGAS[168]
{0x0F12, 0x0164},  //#TVAR_ash_pGAS[169]
{0x0F12, 0x0115},  //#TVAR_ash_pGAS[170]
{0x0F12, 0x00C9},  //#TVAR_ash_pGAS[171]
{0x0F12, 0x0090},  //#TVAR_ash_pGAS[172]
{0x0F12, 0x0068},  //#TVAR_ash_pGAS[173]
{0x0F12, 0x004B},  //#TVAR_ash_pGAS[174]
{0x0F12, 0x003E},  //#TVAR_ash_pGAS[175]
{0x0F12, 0x003F},  //#TVAR_ash_pGAS[176]
{0x0F12, 0x0050},  //#TVAR_ash_pGAS[177]
{0x0F12, 0x006D},  //#TVAR_ash_pGAS[178]
{0x0F12, 0x0097},  //#TVAR_ash_pGAS[179]
{0x0F12, 0x00D5},  //#TVAR_ash_pGAS[180]
{0x0F12, 0x011D},  //#TVAR_ash_pGAS[181]
{0x0F12, 0x0146},  //#TVAR_ash_pGAS[182]
{0x0F12, 0x00F0},  //#TVAR_ash_pGAS[183]
{0x0F12, 0x00A4},  //#TVAR_ash_pGAS[184]
{0x0F12, 0x006D},  //#TVAR_ash_pGAS[185]
{0x0F12, 0x0043},  //#TVAR_ash_pGAS[186]
{0x0F12, 0x0027},  //#TVAR_ash_pGAS[187]
{0x0F12, 0x001B},  //#TVAR_ash_pGAS[188]
{0x0F12, 0x001E},  //#TVAR_ash_pGAS[189]
{0x0F12, 0x0030},  //#TVAR_ash_pGAS[190]
{0x0F12, 0x004F},  //#TVAR_ash_pGAS[191]
{0x0F12, 0x007B},  //#TVAR_ash_pGAS[192]
{0x0F12, 0x00B8},  //#TVAR_ash_pGAS[193]
{0x0F12, 0x0103},  //#TVAR_ash_pGAS[194]
{0x0F12, 0x0130},  //#TVAR_ash_pGAS[195]
{0x0F12, 0x00D9},  //#TVAR_ash_pGAS[196]
{0x0F12, 0x008F},  //#TVAR_ash_pGAS[197]
{0x0F12, 0x0058},  //#TVAR_ash_pGAS[198]
{0x0F12, 0x002F},  //#TVAR_ash_pGAS[199]
{0x0F12, 0x0012},  //#TVAR_ash_pGAS[200]
{0x0F12, 0x0006},  //#TVAR_ash_pGAS[201]
{0x0F12, 0x000A},  //#TVAR_ash_pGAS[202]
{0x0F12, 0x001D},  //#TVAR_ash_pGAS[203]
{0x0F12, 0x003F},  //#TVAR_ash_pGAS[204]
{0x0F12, 0x006E},  //#TVAR_ash_pGAS[205]
{0x0F12, 0x00A9},  //#TVAR_ash_pGAS[206]
{0x0F12, 0x00F4},  //#TVAR_ash_pGAS[207]
{0x0F12, 0x0128},  //#TVAR_ash_pGAS[208]
{0x0F12, 0x00D0},  //#TVAR_ash_pGAS[209]
{0x0F12, 0x0087},  //#TVAR_ash_pGAS[210]
{0x0F12, 0x0052},  //#TVAR_ash_pGAS[211]
{0x0F12, 0x0028},  //#TVAR_ash_pGAS[212]
{0x0F12, 0x000B},  //#TVAR_ash_pGAS[213]
{0x0F12, 0x0000},  //#TVAR_ash_pGAS[214]
{0x0F12, 0x0005},  //#TVAR_ash_pGAS[215]
{0x0F12, 0x001A},  //#TVAR_ash_pGAS[216]
{0x0F12, 0x003D},  //#TVAR_ash_pGAS[217]
{0x0F12, 0x006D},  //#TVAR_ash_pGAS[218]
{0x0F12, 0x00AA},  //#TVAR_ash_pGAS[219]
{0x0F12, 0x00F6},  //#TVAR_ash_pGAS[220]
{0x0F12, 0x012D},  //#TVAR_ash_pGAS[221]
{0x0F12, 0x00D8},  //#TVAR_ash_pGAS[222]
{0x0F12, 0x008E},  //#TVAR_ash_pGAS[223]
{0x0F12, 0x0058},  //#TVAR_ash_pGAS[224]
{0x0F12, 0x0030},  //#TVAR_ash_pGAS[225]
{0x0F12, 0x0014},  //#TVAR_ash_pGAS[226]
{0x0F12, 0x0009},  //#TVAR_ash_pGAS[227]
{0x0F12, 0x000F},  //#TVAR_ash_pGAS[228]
{0x0F12, 0x0026},  //#TVAR_ash_pGAS[229]
{0x0F12, 0x004A},  //#TVAR_ash_pGAS[230]
{0x0F12, 0x0079},  //#TVAR_ash_pGAS[231]
{0x0F12, 0x00B9},  //#TVAR_ash_pGAS[232]
{0x0F12, 0x0103},  //#TVAR_ash_pGAS[233]
{0x0F12, 0x0143},  //#TVAR_ash_pGAS[234]
{0x0F12, 0x00EF},  //#TVAR_ash_pGAS[235]
{0x0F12, 0x00A3},  //#TVAR_ash_pGAS[236]
{0x0F12, 0x006D},  //#TVAR_ash_pGAS[237]
{0x0F12, 0x0047},  //#TVAR_ash_pGAS[238]
{0x0F12, 0x002C},  //#TVAR_ash_pGAS[239]
{0x0F12, 0x0023},  //#TVAR_ash_pGAS[240]
{0x0F12, 0x002A},  //#TVAR_ash_pGAS[241]
{0x0F12, 0x0040},  //#TVAR_ash_pGAS[242]
{0x0F12, 0x0063},  //#TVAR_ash_pGAS[243]
{0x0F12, 0x0092},  //#TVAR_ash_pGAS[244]
{0x0F12, 0x00D4},  //#TVAR_ash_pGAS[245]
{0x0F12, 0x0122},  //#TVAR_ash_pGAS[246]
{0x0F12, 0x0161},  //#TVAR_ash_pGAS[247]
{0x0F12, 0x0112},  //#TVAR_ash_pGAS[248]
{0x0F12, 0x00C7},  //#TVAR_ash_pGAS[249]
{0x0F12, 0x008F},  //#TVAR_ash_pGAS[250]
{0x0F12, 0x006B},  //#TVAR_ash_pGAS[251]
{0x0F12, 0x0055},  //#TVAR_ash_pGAS[252]
{0x0F12, 0x004C},  //#TVAR_ash_pGAS[253]
{0x0F12, 0x0053},  //#TVAR_ash_pGAS[254]
{0x0F12, 0x0069},  //#TVAR_ash_pGAS[255]
{0x0F12, 0x008C},  //#TVAR_ash_pGAS[256]
{0x0F12, 0x00BC},  //#TVAR_ash_pGAS[257]
{0x0F12, 0x0101},  //#TVAR_ash_pGAS[258]
{0x0F12, 0x0150},  //#TVAR_ash_pGAS[259]
{0x0F12, 0x018D},  //#TVAR_ash_pGAS[260]
{0x0F12, 0x013F},  //#TVAR_ash_pGAS[261]
{0x0F12, 0x00FB},  //#TVAR_ash_pGAS[262]
{0x0F12, 0x00C7},  //#TVAR_ash_pGAS[263]
{0x0F12, 0x00A3},  //#TVAR_ash_pGAS[264]
{0x0F12, 0x008B},  //#TVAR_ash_pGAS[265]
{0x0F12, 0x0086},  //#TVAR_ash_pGAS[266]
{0x0F12, 0x0089},  //#TVAR_ash_pGAS[267]
{0x0F12, 0x00A1},  //#TVAR_ash_pGAS[268]
{0x0F12, 0x00C3},  //#TVAR_ash_pGAS[269]
{0x0F12, 0x00F7},  //#TVAR_ash_pGAS[270]
{0x0F12, 0x013D},  //#TVAR_ash_pGAS[271]
{0x0F12, 0x0186},  //#TVAR_ash_pGAS[272]
{0x0F12, 0x01D8},  //#TVAR_ash_pGAS[273]
{0x0F12, 0x0177},  //#TVAR_ash_pGAS[274]
{0x0F12, 0x013F},  //#TVAR_ash_pGAS[275]
{0x0F12, 0x0109},  //#TVAR_ash_pGAS[276]
{0x0F12, 0x00E4},  //#TVAR_ash_pGAS[277]
{0x0F12, 0x00CC},  //#TVAR_ash_pGAS[278]
{0x0F12, 0x00C6},  //#TVAR_ash_pGAS[279]
{0x0F12, 0x00CB},  //#TVAR_ash_pGAS[280]
{0x0F12, 0x00E3},  //#TVAR_ash_pGAS[281]
{0x0F12, 0x010A},  //#TVAR_ash_pGAS[282]
{0x0F12, 0x0142},  //#TVAR_ash_pGAS[283]
{0x0F12, 0x0181},  //#TVAR_ash_pGAS[284]
{0x0F12, 0x01CD},  //#TVAR_ash_pGAS[285]
{0x0F12, 0x01CE},  //#TVAR_ash_pGAS[286]
{0x0F12, 0x0160},  //#TVAR_ash_pGAS[287]
{0x0F12, 0x011E},  //#TVAR_ash_pGAS[288]
{0x0F12, 0x00E5},  //#TVAR_ash_pGAS[289]
{0x0F12, 0x00C0},  //#TVAR_ash_pGAS[290]
{0x0F12, 0x00A9},  //#TVAR_ash_pGAS[291]
{0x0F12, 0x00A3},  //#TVAR_ash_pGAS[292]
{0x0F12, 0x00A8},  //#TVAR_ash_pGAS[293]
{0x0F12, 0x00BA},  //#TVAR_ash_pGAS[294]
{0x0F12, 0x00DA},  //#TVAR_ash_pGAS[295]
{0x0F12, 0x010D},  //#TVAR_ash_pGAS[296]
{0x0F12, 0x014E},  //#TVAR_ash_pGAS[297]
{0x0F12, 0x0196},  //#TVAR_ash_pGAS[298]
{0x0F12, 0x0184},  //#TVAR_ash_pGAS[299]
{0x0F12, 0x012A},  //#TVAR_ash_pGAS[300]
{0x0F12, 0x00E5},  //#TVAR_ash_pGAS[301]
{0x0F12, 0x00AE},  //#TVAR_ash_pGAS[302]
{0x0F12, 0x008A},  //#TVAR_ash_pGAS[303]
{0x0F12, 0x0073},  //#TVAR_ash_pGAS[304]
{0x0F12, 0x0068},  //#TVAR_ash_pGAS[305]
{0x0F12, 0x006F},  //#TVAR_ash_pGAS[306]
{0x0F12, 0x0082},  //#TVAR_ash_pGAS[307]
{0x0F12, 0x00A1},  //#TVAR_ash_pGAS[308]
{0x0F12, 0x00D2},  //#TVAR_ash_pGAS[309]
{0x0F12, 0x0113},  //#TVAR_ash_pGAS[310]
{0x0F12, 0x015C},  //#TVAR_ash_pGAS[311]
{0x0F12, 0x0157},  //#TVAR_ash_pGAS[312]
{0x0F12, 0x0103},  //#TVAR_ash_pGAS[313]
{0x0F12, 0x00BB},  //#TVAR_ash_pGAS[314]
{0x0F12, 0x0085},  //#TVAR_ash_pGAS[315]
{0x0F12, 0x005E},  //#TVAR_ash_pGAS[316]
{0x0F12, 0x0045},  //#TVAR_ash_pGAS[317]
{0x0F12, 0x003A},  //#TVAR_ash_pGAS[318]
{0x0F12, 0x0041},  //#TVAR_ash_pGAS[319]
{0x0F12, 0x0056},  //#TVAR_ash_pGAS[320]
{0x0F12, 0x0077},  //#TVAR_ash_pGAS[321]
{0x0F12, 0x00A6},  //#TVAR_ash_pGAS[322]
{0x0F12, 0x00E7},  //#TVAR_ash_pGAS[323]
{0x0F12, 0x0130},  //#TVAR_ash_pGAS[324]
{0x0F12, 0x013D},  //#TVAR_ash_pGAS[325]
{0x0F12, 0x00E7},  //#TVAR_ash_pGAS[326]
{0x0F12, 0x009B},  //#TVAR_ash_pGAS[327]
{0x0F12, 0x0066},  //#TVAR_ash_pGAS[328]
{0x0F12, 0x003E},  //#TVAR_ash_pGAS[329]
{0x0F12, 0x0023},  //#TVAR_ash_pGAS[330]
{0x0F12, 0x0018},  //#TVAR_ash_pGAS[331]
{0x0F12, 0x001E},  //#TVAR_ash_pGAS[332]
{0x0F12, 0x0034},  //#TVAR_ash_pGAS[333]
{0x0F12, 0x0056},  //#TVAR_ash_pGAS[334]
{0x0F12, 0x0085},  //#TVAR_ash_pGAS[335]
{0x0F12, 0x00C5},  //#TVAR_ash_pGAS[336]
{0x0F12, 0x0112},  //#TVAR_ash_pGAS[337]
{0x0F12, 0x012F},  //#TVAR_ash_pGAS[338]
{0x0F12, 0x00D9},  //#TVAR_ash_pGAS[339]
{0x0F12, 0x008D},  //#TVAR_ash_pGAS[340]
{0x0F12, 0x0057},  //#TVAR_ash_pGAS[341]
{0x0F12, 0x002D},  //#TVAR_ash_pGAS[342]
{0x0F12, 0x0011},  //#TVAR_ash_pGAS[343]
{0x0F12, 0x0005},  //#TVAR_ash_pGAS[344]
{0x0F12, 0x0009},  //#TVAR_ash_pGAS[345]
{0x0F12, 0x001F},  //#TVAR_ash_pGAS[346]
{0x0F12, 0x0044},  //#TVAR_ash_pGAS[347]
{0x0F12, 0x0075},  //#TVAR_ash_pGAS[348]
{0x0F12, 0x00B2},  //#TVAR_ash_pGAS[349]
{0x0F12, 0x00FD},  //#TVAR_ash_pGAS[350]
{0x0F12, 0x0134},  //#TVAR_ash_pGAS[351]
{0x0F12, 0x00D7},  //#TVAR_ash_pGAS[352]
{0x0F12, 0x008C},  //#TVAR_ash_pGAS[353]
{0x0F12, 0x0056},  //#TVAR_ash_pGAS[354]
{0x0F12, 0x002A},  //#TVAR_ash_pGAS[355]
{0x0F12, 0x000D},  //#TVAR_ash_pGAS[356]
{0x0F12, 0x0000},  //#TVAR_ash_pGAS[357]
{0x0F12, 0x0004},  //#TVAR_ash_pGAS[358]
{0x0F12, 0x001A},  //#TVAR_ash_pGAS[359]
{0x0F12, 0x0040},  //#TVAR_ash_pGAS[360]
{0x0F12, 0x0070},  //#TVAR_ash_pGAS[361]
{0x0F12, 0x00AE},  //#TVAR_ash_pGAS[362]
{0x0F12, 0x00F9},  //#TVAR_ash_pGAS[363]
{0x0F12, 0x0142},  //#TVAR_ash_pGAS[364]
{0x0F12, 0x00EB},  //#TVAR_ash_pGAS[365]
{0x0F12, 0x009E},  //#TVAR_ash_pGAS[366]
{0x0F12, 0x0064},  //#TVAR_ash_pGAS[367]
{0x0F12, 0x0038},  //#TVAR_ash_pGAS[368]
{0x0F12, 0x0018},  //#TVAR_ash_pGAS[369]
{0x0F12, 0x000B},  //#TVAR_ash_pGAS[370]
{0x0F12, 0x000E},  //#TVAR_ash_pGAS[371]
{0x0F12, 0x0025},  //#TVAR_ash_pGAS[372]
{0x0F12, 0x004A},  //#TVAR_ash_pGAS[373]
{0x0F12, 0x0079},  //#TVAR_ash_pGAS[374]
{0x0F12, 0x00B6},  //#TVAR_ash_pGAS[375]
{0x0F12, 0x0101},  //#TVAR_ash_pGAS[376]
{0x0F12, 0x0163},  //#TVAR_ash_pGAS[377]
{0x0F12, 0x010A},  //#TVAR_ash_pGAS[378]
{0x0F12, 0x00BB},  //#TVAR_ash_pGAS[379]
{0x0F12, 0x0080},  //#TVAR_ash_pGAS[380]
{0x0F12, 0x0053},  //#TVAR_ash_pGAS[381]
{0x0F12, 0x0034},  //#TVAR_ash_pGAS[382]
{0x0F12, 0x0027},  //#TVAR_ash_pGAS[383]
{0x0F12, 0x002A},  //#TVAR_ash_pGAS[384]
{0x0F12, 0x003E},  //#TVAR_ash_pGAS[385]
{0x0F12, 0x0060},  //#TVAR_ash_pGAS[386]
{0x0F12, 0x008E},  //#TVAR_ash_pGAS[387]
{0x0F12, 0x00CE},  //#TVAR_ash_pGAS[388]
{0x0F12, 0x011C},  //#TVAR_ash_pGAS[389]
{0x0F12, 0x018A},  //#TVAR_ash_pGAS[390]
{0x0F12, 0x0136},  //#TVAR_ash_pGAS[391]
{0x0F12, 0x00E7},  //#TVAR_ash_pGAS[392]
{0x0F12, 0x00AB},  //#TVAR_ash_pGAS[393]
{0x0F12, 0x007F},  //#TVAR_ash_pGAS[394]
{0x0F12, 0x0061},  //#TVAR_ash_pGAS[395]
{0x0F12, 0x0051},  //#TVAR_ash_pGAS[396]
{0x0F12, 0x0054},  //#TVAR_ash_pGAS[397]
{0x0F12, 0x0066},  //#TVAR_ash_pGAS[398]
{0x0F12, 0x0088},  //#TVAR_ash_pGAS[399]
{0x0F12, 0x00B4},  //#TVAR_ash_pGAS[400]
{0x0F12, 0x00F7},  //#TVAR_ash_pGAS[401]
{0x0F12, 0x0141},  //#TVAR_ash_pGAS[402]
{0x0F12, 0x01BD},  //#TVAR_ash_pGAS[403]
{0x0F12, 0x016A},  //#TVAR_ash_pGAS[404]
{0x0F12, 0x0122},  //#TVAR_ash_pGAS[405]
{0x0F12, 0x00E8},  //#TVAR_ash_pGAS[406]
{0x0F12, 0x00BD},  //#TVAR_ash_pGAS[407]
{0x0F12, 0x009E},  //#TVAR_ash_pGAS[408]
{0x0F12, 0x008F},  //#TVAR_ash_pGAS[409]
{0x0F12, 0x008F},  //#TVAR_ash_pGAS[410]
{0x0F12, 0x009E},  //#TVAR_ash_pGAS[411]
{0x0F12, 0x00BE},  //#TVAR_ash_pGAS[412]
{0x0F12, 0x00EC},  //#TVAR_ash_pGAS[413]
{0x0F12, 0x012F},  //#TVAR_ash_pGAS[414]
{0x0F12, 0x0171},  //#TVAR_ash_pGAS[415]
{0x0F12, 0x0211},  //#TVAR_ash_pGAS[416]
{0x0F12, 0x01AA},  //#TVAR_ash_pGAS[417]
{0x0F12, 0x0170},  //#TVAR_ash_pGAS[418]
{0x0F12, 0x0135},  //#TVAR_ash_pGAS[419]
{0x0F12, 0x0106},  //#TVAR_ash_pGAS[420]
{0x0F12, 0x00E5},  //#TVAR_ash_pGAS[421]
{0x0F12, 0x00D6},  //#TVAR_ash_pGAS[422]
{0x0F12, 0x00D2},  //#TVAR_ash_pGAS[423]
{0x0F12, 0x00E2},  //#TVAR_ash_pGAS[424]
{0x0F12, 0x0101},  //#TVAR_ash_pGAS[425]
{0x0F12, 0x0134},  //#TVAR_ash_pGAS[426]
{0x0F12, 0x016D},  //#TVAR_ash_pGAS[427]
{0x0F12, 0x01B6},  //#TVAR_ash_pGAS[428]
{0x0F12, 0x0190},  //#TVAR_ash_pGAS[429]
{0x0F12, 0x0139},  //#TVAR_ash_pGAS[430]
{0x0F12, 0x00FF},  //#TVAR_ash_pGAS[431]
{0x0F12, 0x00CD},  //#TVAR_ash_pGAS[432]
{0x0F12, 0x00AD},  //#TVAR_ash_pGAS[433]
{0x0F12, 0x009A},  //#TVAR_ash_pGAS[434]
{0x0F12, 0x0093},  //#TVAR_ash_pGAS[435]
{0x0F12, 0x0098},  //#TVAR_ash_pGAS[436]
{0x0F12, 0x00A7},  //#TVAR_ash_pGAS[437]
{0x0F12, 0x00C1},  //#TVAR_ash_pGAS[438]
{0x0F12, 0x00EF},  //#TVAR_ash_pGAS[439]
{0x0F12, 0x012C},  //#TVAR_ash_pGAS[440]
{0x0F12, 0x016C},  //#TVAR_ash_pGAS[441]
{0x0F12, 0x0151},  //#TVAR_ash_pGAS[442]
{0x0F12, 0x010A},  //#TVAR_ash_pGAS[443]
{0x0F12, 0x00C7},  //#TVAR_ash_pGAS[444]
{0x0F12, 0x009A},  //#TVAR_ash_pGAS[445]
{0x0F12, 0x007D},  //#TVAR_ash_pGAS[446]
{0x0F12, 0x006C},  //#TVAR_ash_pGAS[447]
{0x0F12, 0x0064},  //#TVAR_ash_pGAS[448]
{0x0F12, 0x0069},  //#TVAR_ash_pGAS[449]
{0x0F12, 0x0077},  //#TVAR_ash_pGAS[450]
{0x0F12, 0x0090},  //#TVAR_ash_pGAS[451]
{0x0F12, 0x00BB},  //#TVAR_ash_pGAS[452]
{0x0F12, 0x00F5},  //#TVAR_ash_pGAS[453]
{0x0F12, 0x0136},  //#TVAR_ash_pGAS[454]
{0x0F12, 0x0124},  //#TVAR_ash_pGAS[455]
{0x0F12, 0x00E0},  //#TVAR_ash_pGAS[456]
{0x0F12, 0x009C},  //#TVAR_ash_pGAS[457]
{0x0F12, 0x0072},  //#TVAR_ash_pGAS[458]
{0x0F12, 0x0056},  //#TVAR_ash_pGAS[459]
{0x0F12, 0x0044},  //#TVAR_ash_pGAS[460]
{0x0F12, 0x003C},  //#TVAR_ash_pGAS[461]
{0x0F12, 0x0040},  //#TVAR_ash_pGAS[462]
{0x0F12, 0x004F},  //#TVAR_ash_pGAS[463]
{0x0F12, 0x0068},  //#TVAR_ash_pGAS[464]
{0x0F12, 0x008D},  //#TVAR_ash_pGAS[465]
{0x0F12, 0x00C8},  //#TVAR_ash_pGAS[466]
{0x0F12, 0x0107},  //#TVAR_ash_pGAS[467]
{0x0F12, 0x0105},  //#TVAR_ash_pGAS[468]
{0x0F12, 0x00C0},  //#TVAR_ash_pGAS[469]
{0x0F12, 0x0080},  //#TVAR_ash_pGAS[470]
{0x0F12, 0x0056},  //#TVAR_ash_pGAS[471]
{0x0F12, 0x0037},  //#TVAR_ash_pGAS[472]
{0x0F12, 0x0023},  //#TVAR_ash_pGAS[473]
{0x0F12, 0x001A},  //#TVAR_ash_pGAS[474]
{0x0F12, 0x001E},  //#TVAR_ash_pGAS[475]
{0x0F12, 0x002E},  //#TVAR_ash_pGAS[476]
{0x0F12, 0x0049},  //#TVAR_ash_pGAS[477]
{0x0F12, 0x006D},  //#TVAR_ash_pGAS[478]
{0x0F12, 0x00A5},  //#TVAR_ash_pGAS[479]
{0x0F12, 0x00E6},  //#TVAR_ash_pGAS[480]
{0x0F12, 0x00F6},  //#TVAR_ash_pGAS[481]
{0x0F12, 0x00B0},  //#TVAR_ash_pGAS[482]
{0x0F12, 0x0070},  //#TVAR_ash_pGAS[483]
{0x0F12, 0x0046},  //#TVAR_ash_pGAS[484]
{0x0F12, 0x0025},  //#TVAR_ash_pGAS[485]
{0x0F12, 0x0011},  //#TVAR_ash_pGAS[486]
{0x0F12, 0x0007},  //#TVAR_ash_pGAS[487]
{0x0F12, 0x0008},  //#TVAR_ash_pGAS[488]
{0x0F12, 0x0018},  //#TVAR_ash_pGAS[489]
{0x0F12, 0x0035},  //#TVAR_ash_pGAS[490]
{0x0F12, 0x005E},  //#TVAR_ash_pGAS[491]
{0x0F12, 0x0094},  //#TVAR_ash_pGAS[492]
{0x0F12, 0x00D0},  //#TVAR_ash_pGAS[493]
{0x0F12, 0x00F4},  //#TVAR_ash_pGAS[494]
{0x0F12, 0x00AA},  //#TVAR_ash_pGAS[495]
{0x0F12, 0x006D},  //#TVAR_ash_pGAS[496]
{0x0F12, 0x0043},  //#TVAR_ash_pGAS[497]
{0x0F12, 0x0022},  //#TVAR_ash_pGAS[498]
{0x0F12, 0x000A},  //#TVAR_ash_pGAS[499]
{0x0F12, 0x0000},  //#TVAR_ash_pGAS[500]
{0x0F12, 0x0002},  //#TVAR_ash_pGAS[501]
{0x0F12, 0x0012},  //#TVAR_ash_pGAS[502]
{0x0F12, 0x0030},  //#TVAR_ash_pGAS[503]
{0x0F12, 0x0057},  //#TVAR_ash_pGAS[504]
{0x0F12, 0x008B},  //#TVAR_ash_pGAS[505]
{0x0F12, 0x00C9},  //#TVAR_ash_pGAS[506]
{0x0F12, 0x00FB},  //#TVAR_ash_pGAS[507]
{0x0F12, 0x00B8},  //#TVAR_ash_pGAS[508]
{0x0F12, 0x007A},  //#TVAR_ash_pGAS[509]
{0x0F12, 0x004D},  //#TVAR_ash_pGAS[510]
{0x0F12, 0x002B},  //#TVAR_ash_pGAS[511]
{0x0F12, 0x0013},  //#TVAR_ash_pGAS[512]
{0x0F12, 0x0009},  //#TVAR_ash_pGAS[513]
{0x0F12, 0x000C},  //#TVAR_ash_pGAS[514]
{0x0F12, 0x001C},  //#TVAR_ash_pGAS[515]
{0x0F12, 0x0038},  //#TVAR_ash_pGAS[516]
{0x0F12, 0x0060},  //#TVAR_ash_pGAS[517]
{0x0F12, 0x0091},  //#TVAR_ash_pGAS[518]
{0x0F12, 0x00CF},  //#TVAR_ash_pGAS[519]
{0x0F12, 0x0119},  //#TVAR_ash_pGAS[520]
{0x0F12, 0x00D4},  //#TVAR_ash_pGAS[521]
{0x0F12, 0x0091},  //#TVAR_ash_pGAS[522]
{0x0F12, 0x0064},  //#TVAR_ash_pGAS[523]
{0x0F12, 0x0042},  //#TVAR_ash_pGAS[524]
{0x0F12, 0x002C},  //#TVAR_ash_pGAS[525]
{0x0F12, 0x0022},  //#TVAR_ash_pGAS[526]
{0x0F12, 0x0024},  //#TVAR_ash_pGAS[527]
{0x0F12, 0x0034},  //#TVAR_ash_pGAS[528]
{0x0F12, 0x004F},  //#TVAR_ash_pGAS[529]
{0x0F12, 0x0073},  //#TVAR_ash_pGAS[530]
{0x0F12, 0x00A8},  //#TVAR_ash_pGAS[531]
{0x0F12, 0x00E7},  //#TVAR_ash_pGAS[532]
{0x0F12, 0x0141},  //#TVAR_ash_pGAS[533]
{0x0F12, 0x00FF},  //#TVAR_ash_pGAS[534]
{0x0F12, 0x00B7},  //#TVAR_ash_pGAS[535]
{0x0F12, 0x0088},  //#TVAR_ash_pGAS[536]
{0x0F12, 0x006A},  //#TVAR_ash_pGAS[537]
{0x0F12, 0x0055},  //#TVAR_ash_pGAS[538]
{0x0F12, 0x004B},  //#TVAR_ash_pGAS[539]
{0x0F12, 0x004C},  //#TVAR_ash_pGAS[540]
{0x0F12, 0x005A},  //#TVAR_ash_pGAS[541]
{0x0F12, 0x0072},  //#TVAR_ash_pGAS[542]
{0x0F12, 0x0095},  //#TVAR_ash_pGAS[543]
{0x0F12, 0x00D0},  //#TVAR_ash_pGAS[544]
{0x0F12, 0x010F},  //#TVAR_ash_pGAS[545]
{0x0F12, 0x0172},  //#TVAR_ash_pGAS[546]
{0x0F12, 0x012C},  //#TVAR_ash_pGAS[547]
{0x0F12, 0x00EF},  //#TVAR_ash_pGAS[548]
{0x0F12, 0x00C0},  //#TVAR_ash_pGAS[549]
{0x0F12, 0x009F},  //#TVAR_ash_pGAS[550]
{0x0F12, 0x008B},  //#TVAR_ash_pGAS[551]
{0x0F12, 0x007F},  //#TVAR_ash_pGAS[552]
{0x0F12, 0x0080},  //#TVAR_ash_pGAS[553]
{0x0F12, 0x008B},  //#TVAR_ash_pGAS[554]
{0x0F12, 0x00A4},  //#TVAR_ash_pGAS[555]
{0x0F12, 0x00CC},  //#TVAR_ash_pGAS[556]
{0x0F12, 0x0109},  //#TVAR_ash_pGAS[557]
{0x0F12, 0x0141},  //#TVAR_ash_pGAS[558]
{0x0F12, 0x01AE},  //#TVAR_ash_pGAS[559]
{0x0F12, 0x0165},  //#TVAR_ash_pGAS[560]
{0x0F12, 0x0137},  //#TVAR_ash_pGAS[561]
{0x0F12, 0x0108},  //#TVAR_ash_pGAS[562]
{0x0F12, 0x00E3},  //#TVAR_ash_pGAS[563]
{0x0F12, 0x00CB},  //#TVAR_ash_pGAS[564]
{0x0F12, 0x00BE},  //#TVAR_ash_pGAS[565]
{0x0F12, 0x00BC},  //#TVAR_ash_pGAS[566]
{0x0F12, 0x00C7},  //#TVAR_ash_pGAS[567]
{0x0F12, 0x00E2},  //#TVAR_ash_pGAS[568]
{0x0F12, 0x010B},  //#TVAR_ash_pGAS[569]
{0x0F12, 0x0141},  //#TVAR_ash_pGAS[570]
{0x0F12, 0x017D},  //#TVAR_ash_pGAS[571]

//	param_start	TVAR_ash_AwbAshCord
{0x002A, 0x06B8},  
{0x0F12, 0x00D0},  //00C0//00D0	// #TVAR_ash_AwbAshCord_0_
{0x0F12, 0x0102},  //00E0//0102	// #TVAR_ash_AwbAshCord_1_
{0x0F12, 0x010E},  //00F0//010E	// #TVAR_ash_AwbAshCord_2_
{0x0F12, 0x0137},  //0129//0137	// #TVAR_ash_AwbAshCord_3_
{0x0F12, 0x0171},  //0156//0171	// #TVAR_ash_AwbAshCord_4_
{0x0F12, 0x0198},  //017F//0198	// #TVAR_ash_AwbAshCord_5_
{0x0F12, 0x01A8},  //018F//01A8	// #TVAR_ash_AwbAshCord_6_

//================================================================================================
// SET CCM    
//================================================================================================
{0x002A, 0x06A6},   //
{0x0F12, 0x00B2},   ////00C0//00D8 // #SARR_AwbCcmCord_0_  h
{0x0F12, 0x00CA},   ////00E0//00FC // #SARR_AwbCcmCord_1_  i
{0x0F12, 0x00FF},   ////00F0//0120 // #SARR_AwbCcmCord_2_  TL84
{0x0F12, 0x0120},   ////0129//014C // #SARR_AwbCcmCord_3_  CWF
{0x0F12, 0x015B},   ////0156//0184 // #SARR_AwbCcmCord_4_  D50
{0x0F12, 0x0170},   ////017F//01AD // #SARR_AwbCcmCord_5_  D60
              
// CCM start address // 7000_33A4
{0x002A, 0x0698},   // SET CCM  
{0x0F12, 0x33A4},   
{0x0F12, 0x7000}, 
// Horizon  
{0x002A, 0x33A4},   
{0x0F12, 0x021F},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0xFF1B},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0xFF43},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0xFE9F},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0x025C},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0xFECF},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0xFF9B},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0xFF73},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0x0202},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0x0100},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0x014C},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0xFE6B},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0x020D},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0xFEA6},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0x01FE},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0xFE50},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0x018D},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0x0172},   //#TVAR_wbt_pBaseCcms                            
{0x0F12, 0x021F},   //#TVAR_wbt_pBaseCcms[18] // Inca                
{0x0F12, 0xFF1B},   //#TVAR_wbt_pBaseCcms[19]                        
{0x0F12, 0xFF43},   //#TVAR_wbt_pBaseCcms[20]                        
{0x0F12, 0xFE9F},   //#TVAR_wbt_pBaseCcms[21]                        
{0x0F12, 0x025C},   //#TVAR_wbt_pBaseCcms[22]                        
{0x0F12, 0xFECF},   //#TVAR_wbt_pBaseCcms[23]                        
{0x0F12, 0xFF9B},   //#TVAR_wbt_pBaseCcms[24]                        
{0x0F12, 0xFF73},   //#TVAR_wbt_pBaseCcms[25]                        
{0x0F12, 0x0202},   //#TVAR_wbt_pBaseCcms[26]                        
{0x0F12, 0x0100},   //#TVAR_wbt_pBaseCcms[27]                        
{0x0F12, 0x014C},   //#TVAR_wbt_pBaseCcms[28]                        
{0x0F12, 0xFE6B},   //#TVAR_wbt_pBaseCcms[29]                        
{0x0F12, 0x020D},   //#TVAR_wbt_pBaseCcms[30]                        
{0x0F12, 0xFEA6},   //#TVAR_wbt_pBaseCcms[31]                        
{0x0F12, 0x01FE},   //#TVAR_wbt_pBaseCcms[32]                        
{0x0F12, 0xFE50},   //#TVAR_wbt_pBaseCcms[33]                        
{0x0F12, 0x018D},   //#TVAR_wbt_pBaseCcms[34]                        
{0x0F12, 0x0172},   //#TVAR_wbt_pBaseCcms[35]                        
{0x0F12, 0x0222},   //#TVAR_wbt_pBaseCcms[36] // WW                  
{0x0F12, 0xFF5B},   //#TVAR_wbt_pBaseCcms[37]                        
{0x0F12, 0xFF8C},   //#TVAR_wbt_pBaseCcms[38]                        
{0x0F12, 0xFEF0},   //#TVAR_wbt_pBaseCcms[39]                        
{0x0F12, 0x024B},   //#TVAR_wbt_pBaseCcms[40]                        
{0x0F12, 0xFF10},   //#TVAR_wbt_pBaseCcms[41]                        
{0x0F12, 0xFFD2},   //#TVAR_wbt_pBaseCcms[42]                        
{0x0F12, 0xFFB7},   //#TVAR_wbt_pBaseCcms[43]                        
{0x0F12, 0x01FD},   //#TVAR_wbt_pBaseCcms[44]                        
{0x0F12, 0x0108},   //#TVAR_wbt_pBaseCcms[45]                        
{0x0F12, 0x015C},   //#TVAR_wbt_pBaseCcms[46]                        
{0x0F12, 0xFED1},   //#TVAR_wbt_pBaseCcms[47]                        
{0x0F12, 0x0222},   //#TVAR_wbt_pBaseCcms[48]                        
{0x0F12, 0xFF03},   //#TVAR_wbt_pBaseCcms[49]                        
{0x0F12, 0x0226},   //#TVAR_wbt_pBaseCcms[50]                        
{0x0F12, 0xFECC},   //#TVAR_wbt_pBaseCcms[51]                        
{0x0F12, 0x0198},   //#TVAR_wbt_pBaseCcms[52]                        
{0x0F12, 0x0171},   //#TVAR_wbt_pBaseCcms[53]                        
{0x0F12, 0x0206},   //#TVAR_wbt_pBaseCcms[54] // CWF                 
{0x0F12, 0xFF64},   //#TVAR_wbt_pBaseCcms[55]                        
{0x0F12, 0xFF7C},   //#TVAR_wbt_pBaseCcms[56]                        
{0x0F12, 0xFED4},   //#TVAR_wbt_pBaseCcms[57]                        
{0x0F12, 0x023A},   //#TVAR_wbt_pBaseCcms[58]                        
{0x0F12, 0xFF10},   //#TVAR_wbt_pBaseCcms[59]                        
{0x0F12, 0xFFD0},   //#TVAR_wbt_pBaseCcms[60]                        
{0x0F12, 0xFFA6},   //#TVAR_wbt_pBaseCcms[61]                        
{0x0F12, 0x01E4},   //#TVAR_wbt_pBaseCcms[62]                        
{0x0F12, 0x00F0},   //#TVAR_wbt_pBaseCcms[63]                        
{0x0F12, 0x015B},   //#TVAR_wbt_pBaseCcms[64]                        
{0x0F12, 0xFECE},   //#TVAR_wbt_pBaseCcms[65]                        
{0x0F12, 0x0225},   //#TVAR_wbt_pBaseCcms[66]                        
{0x0F12, 0xFF01},   //#TVAR_wbt_pBaseCcms[67]                        
{0x0F12, 0x0215},   //#TVAR_wbt_pBaseCcms[68]                        
{0x0F12, 0xFEC3},   //#TVAR_wbt_pBaseCcms[69]                        
{0x0F12, 0x017C},   //#TVAR_wbt_pBaseCcms[70]                        
{0x0F12, 0x0167},   //#TVAR_wbt_pBaseCcms[71]                        
{0x0F12, 0x01D6},   //#TVAR_wbt_pBaseCcms[72] // D50                 
{0x0F12, 0xFF94},   //#TVAR_wbt_pBaseCcms[73]                        
{0x0F12, 0xFFCC},   //#TVAR_wbt_pBaseCcms[74]                        
{0x0F12, 0xFF1F},   //#TVAR_wbt_pBaseCcms[75]                        
{0x0F12, 0x021F},   //#TVAR_wbt_pBaseCcms[76]                        
{0x0F12, 0xFF1F},   //#TVAR_wbt_pBaseCcms[77]                        
{0x0F12, 0xFFE4},   //#TVAR_wbt_pBaseCcms[78]                        
{0x0F12, 0xFFED},   //#TVAR_wbt_pBaseCcms[79]                        
{0x0F12, 0x01C8},   //#TVAR_wbt_pBaseCcms[80]                        
{0x0F12, 0x010F},   //#TVAR_wbt_pBaseCcms[81]                        
{0x0F12, 0x0150},   //#TVAR_wbt_pBaseCcms[82]                        
{0x0F12, 0xFF16},   //#TVAR_wbt_pBaseCcms[83]                        
{0x0F12, 0x0210},   //#TVAR_wbt_pBaseCcms[84]                        
{0x0F12, 0xFF5D},   //#TVAR_wbt_pBaseCcms[85]                        
{0x0F12, 0x0244},   //#TVAR_wbt_pBaseCcms[86]                        
{0x0F12, 0xFF10},   //#TVAR_wbt_pBaseCcms[87]                        
{0x0F12, 0x0190},   //#TVAR_wbt_pBaseCcms[88]                        
{0x0F12, 0x0145},   //#TVAR_wbt_pBaseCcms[89]                        
{0x0F12, 0x01D6},   //#TVAR_wbt_pBaseCcms[90] // D65                 
{0x0F12, 0xFF94},   //#TVAR_wbt_pBaseCcms[91]                        
{0x0F12, 0xFFCC},   //#TVAR_wbt_pBaseCcms[92]                        
{0x0F12, 0xFF1F},   //#TVAR_wbt_pBaseCcms[93]                        
{0x0F12, 0x021F},   //#TVAR_wbt_pBaseCcms[94]                        
{0x0F12, 0xFF1F},   //#TVAR_wbt_pBaseCcms[95]                        
{0x0F12, 0xFFE4},   //#TVAR_wbt_pBaseCcms[96]                        
{0x0F12, 0xFFED},   //#TVAR_wbt_pBaseCcms[97]                        
{0x0F12, 0x01C8},   //#TVAR_wbt_pBaseCcms[98]                        
{0x0F12, 0x010F},   //#TVAR_wbt_pBaseCcms[99]                        
{0x0F12, 0x0150},   //#TVAR_wbt_pBaseCcms[100]                       
{0x0F12, 0xFF16},   //#TVAR_wbt_pBaseCcms[101]                       
{0x0F12, 0x0210},   //#TVAR_wbt_pBaseCcms[102]                       
{0x0F12, 0xFF5D},   //#TVAR_wbt_pBaseCcms[103]                       
{0x0F12, 0x0244},   //#TVAR_wbt_pBaseCcms[104]                       
{0x0F12, 0xFF10},   //#TVAR_wbt_pBaseCcms[105]                       
{0x0F12, 0x0190},   //#TVAR_wbt_pBaseCcms[106]                       
{0x0F12, 0x0145},   //#TVAR_wbt_pBaseCcms[107]                       
{0x002A, 0x06A0},   // Outdoor CCM address // 7000_3380              
{0x0F12, 0x3380},   
{0x0F12, 0x7000},   
{0x002A, 0x3380},   
{0x0F12, 0x01D6},   //#TVAR_wbt_pOutdoorCcm[0]// Outdoor CCM     
{0x0F12, 0xFF94},   //#TVAR_wbt_pOutdoorCcm[1]                   
{0x0F12, 0xFFCC},   //#TVAR_wbt_pOutdoorCcm[2]                   
{0x0F12, 0xFF1F},   //#TVAR_wbt_pOutdoorCcm[3]                   
{0x0F12, 0x021F},   //#TVAR_wbt_pOutdoorCcm[4]                   
{0x0F12, 0xFF1F},   //#TVAR_wbt_pOutdoorCcm[5]                   
{0x0F12, 0xFFE4},   //#TVAR_wbt_pOutdoorCcm[6]                   
{0x0F12, 0xFFED},   //#TVAR_wbt_pOutdoorCcm[7]                   
{0x0F12, 0x01C8},   //#TVAR_wbt_pOutdoorCcm[8]                   
{0x0F12, 0x012B},   //010F    //#TVAR_wbt_pOutdoorCcm[9]                   
{0x0F12, 0x0119},   //#TVAR_wbt_pOutdoorCcm[10]                  
{0x0F12, 0xFF71},   //#TVAR_wbt_pOutdoorCcm[11]                  
{0x0F12, 0x0210},   //#TVAR_wbt_pOutdoorCcm[12]                  
{0x0F12, 0xFF5D},   //#TVAR_wbt_pOutdoorCcm[13]                  
{0x0F12, 0x0244},   //#TVAR_wbt_pOutdoorCcm[14]                  
{0x0F12, 0xFF10},   //#TVAR_wbt_pOutdoorCcm[15]                  
{0x0F12, 0x0190},   //#TVAR_wbt_pOutdoorCcm[16]                  
{0x0F12, 0x0145},   //#TVAR_wbt_pOutdoorCcm[17]   
//================================================================================================
// SET AWB
//================================================================================================
// Indoor boundary
//
{0x002A, 0x0C48},  
{0x0F12, 0x038B},  //awbb_IndoorGrZones_m_BGrid[0]
{0x0F12, 0x03C0},  //awbb_IndoorGrZones_m_BGrid[1]
{0x0F12, 0x033D},  //awbb_IndoorGrZones_m_BGrid[2]
{0x0F12, 0x03C5},  //awbb_IndoorGrZones_m_BGrid[3]
{0x0F12, 0x0303},  //awbb_IndoorGrZones_m_BGrid[4]
{0x0F12, 0x03AE},  //awbb_IndoorGrZones_m_BGrid[5]
{0x0F12, 0x02CF},  //awbb_IndoorGrZones_m_BGrid[6]
{0x0F12, 0x0387},  //awbb_IndoorGrZones_m_BGrid[7]
{0x0F12, 0x02A0},  //awbb_IndoorGrZones_m_BGrid[8]
{0x0F12, 0x0360},  //awbb_IndoorGrZones_m_BGrid[9]
{0x0F12, 0x027C},  //awbb_IndoorGrZones_m_BGrid[10]
{0x0F12, 0x0335},  //awbb_IndoorGrZones_m_BGrid[11]
{0x0F12, 0x025D},  //awbb_IndoorGrZones_m_BGrid[12]
{0x0F12, 0x030A},  //awbb_IndoorGrZones_m_BGrid[13]
{0x0F12, 0x0243},  //awbb_IndoorGrZones_m_BGrid[14]
{0x0F12, 0x02E5},  //awbb_IndoorGrZones_m_BGrid[15]
{0x0F12, 0x0227},  //awbb_IndoorGrZones_m_BGrid[16]
{0x0F12, 0x02BD},  //awbb_IndoorGrZones_m_BGrid[17]
{0x0F12, 0x020E},  //awbb_IndoorGrZones_m_BGrid[18]
{0x0F12, 0x029E},  //awbb_IndoorGrZones_m_BGrid[19]
{0x0F12, 0x01F7},  //awbb_IndoorGrZones_m_BGrid[20]
{0x0F12, 0x027F},  //awbb_IndoorGrZones_m_BGrid[21]
{0x0F12, 0x01E3},  //awbb_IndoorGrZones_m_BGrid[22]
{0x0F12, 0x0262},  //awbb_IndoorGrZones_m_BGrid[23]
{0x0F12, 0x01D1},  //awbb_IndoorGrZones_m_BGrid[24]
{0x0F12, 0x024D},  //awbb_IndoorGrZones_m_BGrid[25]
{0x0F12, 0x01BD},  //awbb_IndoorGrZones_m_BGrid[26]
{0x0F12, 0x0232},  //awbb_IndoorGrZones_m_BGrid[27]
{0x0F12, 0x01B2},  //awbb_IndoorGrZones_m_BGrid[28]
{0x0F12, 0x021A},  //awbb_IndoorGrZones_m_BGrid[29]
{0x0F12, 0x01B3},  //awbb_IndoorGrZones_m_BGrid[30]
{0x0F12, 0x0201},  //awbb_IndoorGrZones_m_BGrid[31]
{0x0F12, 0x01BC},  //awbb_IndoorGrZones_m_BGrid[32]
{0x0F12, 0x01DD},  //awbb_IndoorGrZones_m_BGrid[33]
{0x0F12, 0x0000},  //awbb_IndoorGrZones_m_BGrid[34]
{0x0F12, 0x0000},  //awbb_IndoorGrZones_m_BGrid[35]
{0x0F12, 0x0000},  //awbb_IndoorGrZones_m_BGrid[36]
{0x0F12, 0x0000},  //awbb_IndoorGrZones_m_BGrid[37]
{0x0F12, 0x0000},  //awbb_IndoorGrZones_m_BGrid[38]
{0x0F12, 0x0000},  //awbb_IndoorGrZones_m_BGrid[39]
{0x0F12, 0x0005},  // #awbb_IndoorGrZones_m_GridStep
{0x002A, 0x0CA0},  
{0x0F12, 0x011A},  //00E8	// #awbb_IndoorGrZones_m_Boffs

// Outdoor boudary

{0x002A, 0x0CA4},  
{0x0F12, 0x026F},  //awbb_OutdoorGrZones_m_BGrid[0]
{0x0F12, 0x029C},  //awbb_OutdoorGrZones_m_BGrid[1]
{0x0F12, 0x0238},  //awbb_OutdoorGrZones_m_BGrid[2]
{0x0F12, 0x0284},  //awbb_OutdoorGrZones_m_BGrid[3]
{0x0F12, 0x0206},  //awbb_OutdoorGrZones_m_BGrid[4]
{0x0F12, 0x0250},  //awbb_OutdoorGrZones_m_BGrid[5]
{0x0F12, 0x01D6},  //awbb_OutdoorGrZones_m_BGrid[6]
{0x0F12, 0x0226},  //awbb_OutdoorGrZones_m_BGrid[7]
{0x0F12, 0x01BC},  //awbb_OutdoorGrZones_m_BGrid[8]
{0x0F12, 0x01F6},  //awbb_OutdoorGrZones_m_BGrid[9]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[10]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[11]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[12]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[13]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[14]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[15]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[16]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[17]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[18]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[19]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[20]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[21]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[22]
{0x0F12, 0x0000},  //awbb_OutdoorGrZones_m_BGrid[23]
{0x0F12, 0x0005},  // #awbb_OutdoorGrZones_m_GridStep
{0x002A, 0x0CDC},  
{0x0F12, 0x0212},  //0204	// #awbb_OutdoorGrZones_m_Boffs
 
//Outdoor detecor
{0x002A, 0x0DA0},  
{0x0F12, 0x0005},  
{0x002A, 0x0D88},  
{0x0F12, 0x0048},  
{0x0F12, 0x0084},  
{0x0F12, 0x0001},  
{0x0F12, 0x00CF},  
{0x0F12, 0xFFAB},  
{0x0F12, 0x00EB},  
{0x0F12, 0xFF66},  
{0x0F12, 0x0100},  
{0x0F12, 0xFF0F},  
{0x0F12, 0x011F},  
{0x0F12, 0x0E74},  
{0x002A, 0x0DA8},  
{0x0F12, 0x1701},  
{0x002A, 0x0DA4},  
{0x0F12, 0x0691},  

// LowBr boundry
{0x002A, 0x0CE0},  
{0x0F12, 0x0376},  // #awbb_LowBrGrZones_m_BGrid[0]
{0x0F12, 0x03F4},  // #awbb_LowBrGrZones_m_BGrid[1]
{0x0F12, 0x0304},  // #awbb_LowBrGrZones_m_BGrid[2]
{0x0F12, 0x03F4},  // #awbb_LowBrGrZones_m_BGrid[3]
{0x0F12, 0x029A},  // #awbb_LowBrGrZones_m_BGrid[4]
{0x0F12, 0x03E6},  // #awbb_LowBrGrZones_m_BGrid[5]
{0x0F12, 0x024E},  // #awbb_LowBrGrZones_m_BGrid[6]
{0x0F12, 0x039A},  // #awbb_LowBrGrZones_m_BGrid[7]
{0x0F12, 0x020E},  // #awbb_LowBrGrZones_m_BGrid[8]
{0x0F12, 0x034C},  // #awbb_LowBrGrZones_m_BGrid[9]
{0x0F12, 0x01E0},  // #awbb_LowBrGrZones_m_BGrid[10]
{0x0F12, 0x02FF},  // #awbb_LowBrGrZones_m_BGrid[11]
{0x0F12, 0x01AD},  // #awbb_LowBrGrZones_m_BGrid[12]
{0x0F12, 0x02B8},  // #awbb_LowBrGrZones_m_BGrid[13]
{0x0F12, 0x018A},  // #awbb_LowBrGrZones_m_BGrid[14]
{0x0F12, 0x0284},  // #awbb_LowBrGrZones_m_BGrid[15]
{0x0F12, 0x0187},  // #awbb_LowBrGrZones_m_BGrid[16]
{0x0F12, 0x025A},  // #awbb_LowBrGrZones_m_BGrid[17]
{0x0F12, 0x018D},  // #awbb_LowBrGrZones_m_BGrid[18]
{0x0F12, 0x01F6},  // #awbb_LowBrGrZones_m_BGrid[19]
{0x0F12, 0x0000},  // #awbb_LowBrGrZones_m_BGrid[20]
{0x0F12, 0x0000},  // #awbb_LowBrGrZones_m_BGrid[21]
{0x0F12, 0x0000},  // #awbb_LowBrGrZones_m_BGrid[22]
{0x0F12, 0x0000},  // #awbb_LowBrGrZones_m_BGrid[23]
{0x0F12, 0x0006},  // #awbb_LowBrGrZones_m_GridStep
{0x002A, 0x0D18},  
{0x0F12, 0x00FA},  //00FE	// #awbb_LowBrGrZones_m_Boffs
                            
// AWB ETC                  
{0x002A, 0x0D1C}, 
{0x0F12, 0x0340},  //037C	// #awbb_CrclLowT_R_c
{0x002A, 0x0D20},  
{0x0F12, 0x016C},  //0157	// #awbb_CrclLowT_B_c
{0x002A, 0x0D24},  
{0x0F12, 0x49D5},   //3EB8	// #awbb_CrclLowT_Rad_c
                            
{0x002A, 0x0D2C},  
{0x0F12, 0x0131},  //013D	// #awbb_IntcR
{0x0F12, 0x012C},  //011E	// #awbb_IntcB
 
{0x002A, 0x0D28},  
{0x0F12, 0x0290},  // #awbb_OutdoorWP_r
{0x0F12, 0x0240},  // #awbb_OutdoorWP_b

{0x002A, 0x0D5C},  
{0x0F12, 0x7FFF},  // #awbb_LowTempRB
{0x0F12, 0x0050},  // #awbb_LowTemp_RBzone

{0x002A, 0x0D46},  
{0x0F12, 0x0546},  // #awbb_MvEq_RBthresh

// AWB initialpoint
{0x002A, 0x0E44},  
{0x0F12, 0x053C},  // #define awbb_GainsInit_0_
{0x0F12, 0x0400},  // #define awbb_GainsInit_1_
{0x0F12, 0x055C},  // #define awbb_GainsInit_2_
// Set AWB gloal offset
{0x002A, 0x0E36},  
{0x0F12, 0x0000},  //#awbb_RGainOff
{0x0F12, 0x0000},  //#awbb_BGainOff
{0x0F12, 0x0000},  //#awbb_GGainOff

//================================================================================================
// SET GRID OFFSET
//================================================================================================
// Not used
{0x002A, 0x0E4A},  
{0x0F12, 0x0002},  // #awbb_GridEnable

{0x002A, 0x0DD4},  
{0x0F12, 0x0032},  //awbb_GridCorr_R[0] //
{0x0F12, 0x0032},  //awbb_GridCorr_R[1] //
{0x0F12, 0x0032},  //awbb_GridCorr_R[2] //
{0x0F12, 0xFFE2},  //awbb_GridCorr_R[3] //  001E
{0x0F12, 0x001E},  //awbb_GridCorr_R[4] //
{0x0F12, 0x0064},  //awbb_GridCorr_R[5] //
{0x0F12, 0x0000},  //awbb_GridCorr_R[6] //  0032
{0x0F12, 0x0000},  //awbb_GridCorr_R[7] //  0032
{0x0F12, 0x0032},  //awbb_GridCorr_R[8] //
{0x0F12, 0xFFCE},  //awbb_GridCorr_R[9] // 001E FFE2
{0x0F12, 0x0000},  //awbb_GridCorr_R[10] //001E
{0x0F12, 0x0064},  //awbb_GridCorr_R[11] //
{0x0F12, 0x0032},  //awbb_GridCorr_R[12] //
{0x0F12, 0x0032},  //awbb_GridCorr_R[13] //
{0x0F12, 0x0032},  //awbb_GridCorr_R[14] //
{0x0F12, 0x001E},  //awbb_GridCorr_R[15] //
{0x0F12, 0x001E},  //awbb_GridCorr_R[16] //
{0x0F12, 0x0064},  //awbb_GridCorr_R[17] //
{0x0F12, 0xFFE2},  //awbb_GridCorr_B[0] ////
{0x0F12, 0x0000},  //awbb_GridCorr_B[1] //
{0x0F12, 0x0014},  //awbb_GridCorr_B[2] //
{0x0F12, 0x001E},  //awbb_GridCorr_B[3] //
{0x0F12, 0xFFCE},  //awbb_GridCorr_B[4] //
{0x0F12, 0xFFCE},  //awbb_GridCorr_B[5] //
{0x0F12, 0xFFE2},  //awbb_GridCorr_B[6] //
{0x0F12, 0x0000},  //awbb_GridCorr_B[7] //
{0x0F12, 0x0014},  //awbb_GridCorr_B[8] //
{0x0F12, 0x001E},  //awbb_GridCorr_B[9] //
{0x0F12, 0xFFCE},  //awbb_GridCorr_B[10] //
{0x0F12, 0xFFCE},  //awbb_GridCorr_B[11] //
{0x0F12, 0xFFE2},  //awbb_GridCorr_B[12] //
{0x0F12, 0x0000},  //awbb_GridCorr_B[13] //
{0x0F12, 0x0014},  //awbb_GridCorr_B[14] //
{0x0F12, 0x001E},  //awbb_GridCorr_B[15] //
{0x0F12, 0xFFCE},  //awbb_GridCorr_B[16] //
{0x0F12, 0xFFCE},  //awbb_GridCorr_B[17] //


{0x0F12, 0x02D9},  //awbb_GridConst_1[0] //
{0x0F12, 0x0357},  //awbb_GridConst_1[1] //
{0x0F12, 0x03D1},  //awbb_GridConst_1[2] //

{0x0F12, 0x0DF6},  //0E4F//0DE9//0DE9//awbb_GridConst_2[0] //
{0x0F12, 0x0EB9},  //0EDD//0EDD//0EDD//awbb_GridConst_2[1] //
{0x0F12, 0x0F42},  //0F42//0F42//0F42//awbb_GridConst_2[2] //
{0x0F12, 0x0F4E},  //0F4E//0F4E//0F54//awbb_GridConst_2[3] //
{0x0F12, 0x0F99},  //0F99//0F99//0FAE//awbb_GridConst_2[4] //
{0x0F12, 0x1006},  //1006//1006//1011//awbb_GridConst_2[5] //

{0x0F12, 0x00AC},  //00BA//awbb_GridCoeff_R_1
{0x0F12, 0x00BD},  //00AF//awbb_GridCoeff_B_1
{0x0F12, 0x0049},  //0049//awbb_GridCoeff_R_2
{0x0F12, 0x00F5},  //00F5//awbb_GridCoeff_B_2

//================================================================================================
// SET GAMMA
//================================================================================================
//Our //old	//STW
{0x002A, 0x05A0}, 
{0x0F12, 0x0000}, 	//0000	//#SARR_usGammaLutRGBIndoor_0__0_
{0x0F12, 0x0008}, 	//0008	//#SARR_usGammaLutRGBIndoor_0__1_
{0x0F12, 0x0013}, 	//001E	//#SARR_usGammaLutRGBIndoor_0__2_
{0x0F12, 0x002C}, 	//0040	//#SARR_usGammaLutRGBIndoor_0__3_
{0x0F12, 0x0061}, 	//0078	//#SARR_usGammaLutRGBIndoor_0__4_
{0x0F12, 0x00C8}, 	//00D4	//#SARR_usGammaLutRGBIndoor_0__5_
{0x0F12, 0x0113}, 	//0126	//#SARR_usGammaLutRGBIndoor_0__6_
{0x0F12, 0x0132}, 	//014C	//#SARR_usGammaLutRGBIndoor_0__7_
{0x0F12, 0x014C}, 	//016F	//#SARR_usGammaLutRGBIndoor_0__8_
{0x0F12, 0x0179}, 	//01AC	//#SARR_usGammaLutRGBIndoor_0__9_
{0x0F12, 0x01A4}, 	//01DE	//#SARR_usGammaLutRGBIndoor_0__10_
{0x0F12, 0x01CD}, 	//0208	//#SARR_usGammaLutRGBIndoor_0__11_
{0x0F12, 0x01F4}, 	//022A	//#SARR_usGammaLutRGBIndoor_0__12_
{0x0F12, 0x0239}, 	//0260	//#SARR_usGammaLutRGBIndoor_0__13_
{0x0F12, 0x0278}, 	//0294	//#SARR_usGammaLutRGBIndoor_0__14_
{0x0F12, 0x02E0}, 	//02EE	//#SARR_usGammaLutRGBIndoor_0__15_
{0x0F12, 0x0333}, 	//033E	//#SARR_usGammaLutRGBIndoor_0__16_
{0x0F12, 0x037B}, 	//0384	//#SARR_usGammaLutRGBIndoor_0__17_
{0x0F12, 0x03BF}, 	//03C4	//#SARR_usGammaLutRGBIndoor_0__18_
{0x0F12, 0x03FF}, 	//03FF	//#SARR_usGammaLutRGBIndoor_0__19_
{0x0F12, 0x0000}, 	//0000	//#SARR_usGammaLutRGBIndoor_1__0_
{0x0F12, 0x0008}, 	//0008	//#SARR_usGammaLutRGBIndoor_1__1_
{0x0F12, 0x0013}, 	//001E	//#SARR_usGammaLutRGBIndoor_1__2_
{0x0F12, 0x002C}, 	//0040	//#SARR_usGammaLutRGBIndoor_1__3_
{0x0F12, 0x0061}, 	//0078	//#SARR_usGammaLutRGBIndoor_1__4_
{0x0F12, 0x00C8}, 	//00D4	//#SARR_usGammaLutRGBIndoor_1__5_
{0x0F12, 0x0113}, 	//0126	//#SARR_usGammaLutRGBIndoor_1__6_
{0x0F12, 0x0132}, 	//014C	//#SARR_usGammaLutRGBIndoor_1__7_
{0x0F12, 0x014C}, 	//016F	//#SARR_usGammaLutRGBIndoor_1__8_
{0x0F12, 0x0179}, 	//01AC	//#SARR_usGammaLutRGBIndoor_1__9_
{0x0F12, 0x01A4}, 	//01DE	//#SARR_usGammaLutRGBIndoor_1__10_
{0x0F12, 0x01CD}, 	//0208	//#SARR_usGammaLutRGBIndoor_1__11_
{0x0F12, 0x01F4}, 	//022A	//#SARR_usGammaLutRGBIndoor_1__12_
{0x0F12, 0x0239}, 	//0260	//#SARR_usGammaLutRGBIndoor_1__13_
{0x0F12, 0x0278}, 	//0294	//#SARR_usGammaLutRGBIndoor_1__14_
{0x0F12, 0x02E0}, 	//02EE	//#SARR_usGammaLutRGBIndoor_1__15_
{0x0F12, 0x0333}, 	//033E	//#SARR_usGammaLutRGBIndoor_1__16_
{0x0F12, 0x037B}, 	//0384	//#SARR_usGammaLutRGBIndoor_1__17_
{0x0F12, 0x03BF}, 	//03C4	//#SARR_usGammaLutRGBIndoor_1__18_
{0x0F12, 0x03FF}, 	//03FF	//#SARR_usGammaLutRGBIndoor_1__19_
{0x0F12, 0x0000}, 	//0000	//#SARR_usGammaLutRGBIndoor_2__0_
{0x0F12, 0x0008}, 	//0008	//#SARR_usGammaLutRGBIndoor_2__1_
{0x0F12, 0x0013}, 	//001E	//#SARR_usGammaLutRGBIndoor_2__2_
{0x0F12, 0x002C}, 	//0040	//#SARR_usGammaLutRGBIndoor_2__3_
{0x0F12, 0x0061}, 	//0078	//#SARR_usGammaLutRGBIndoor_2__4_
{0x0F12, 0x00C8}, 	//00D4	//#SARR_usGammaLutRGBIndoor_2__5_
{0x0F12, 0x0113}, 	//0126	//#SARR_usGammaLutRGBIndoor_2__6_
{0x0F12, 0x0132}, 	//014C	//#SARR_usGammaLutRGBIndoor_2__7_
{0x0F12, 0x014C}, 	//016F	//#SARR_usGammaLutRGBIndoor_2__8_
{0x0F12, 0x0179}, 	//01AC	//#SARR_usGammaLutRGBIndoor_2__9_
{0x0F12, 0x01A4}, 	//01DE	//#SARR_usGammaLutRGBIndoor_2__10_
{0x0F12, 0x01CD}, 	//0208	//#SARR_usGammaLutRGBIndoor_2__11_
{0x0F12, 0x01F4}, 	//022A	//#SARR_usGammaLutRGBIndoor_2__12_
{0x0F12, 0x0239}, 	//0260	//#SARR_usGammaLutRGBIndoor_2__13_
{0x0F12, 0x0278}, 	//0294	//#SARR_usGammaLutRGBIndoor_2__14_
{0x0F12, 0x02E0}, 	//02EE	//#SARR_usGammaLutRGBIndoor_2__15_
{0x0F12, 0x0333}, 	//033E	//#SARR_usGammaLutRGBIndoor_2__16_
{0x0F12, 0x037B}, 	//0384	//#SARR_usGammaLutRGBIndoor_2__17_
{0x0F12, 0x03BF}, 	//03C4	//#SARR_usGammaLutRGBIndoor_2__18_
{0x0F12, 0x03FF}, 	//03FF	//#SARR_usGammaLutRGBIndoor_2__19_

{0x002A, 0x1034},  
{0x0F12, 0x00C0},  //00B5	// #SARR_IllumType[0]
{0x0F12, 0x00E0},  //00CF	// #SARR_IllumType[1]
{0x0F12, 0x00F0},  //0116	// #SARR_IllumType[2]
{0x0F12, 0x0129},  //0140	// #SARR_IllumType[3]
{0x0F12, 0x0156},  //0150	// #SARR_IllumType[4]
{0x0F12, 0x017F},  //0174	// #SARR_IllumType[5]
{0x0F12, 0x018F},  //018E	// #SARR_IllumType[6]

{0x0F12, 0x0120},  //00B8	// #SARR_IllumTypeF[0]
{0x0F12, 0x0120},  //00BA	// #SARR_IllumTypeF[1]
{0x0F12, 0x0120},  //00C0	// #SARR_IllumTypeF[2]
{0x0F12, 0x0100},  //00F0	// #SARR_IllumTypeF[3]
{0x0F12, 0x0100},  //0100	// #SARR_IllumTypeF[4]
{0x0F12, 0x0100},  //0100	// #SARR_IllumTypeF[5]
{0x0F12, 0x0100},  //0100	// #SARR_IllumTypeF[6]
 
//================================================================================================
// SET AFIT
//================================================================================================
// Noise index
{0x002A, 0x0764}, 
{0x0F12, 0x0049}, 	//0041	// #afit_uNoiseIndInDoor[0] // 64
{0x0F12, 0x005F}, 	//0063	// #afit_uNoiseIndInDoor[1] // 165
{0x0F12, 0x0138}, 	//00BB	// #afit_uNoiseIndInDoor[2] // 377
{0x0F12, 0x01E0}, 	//0193	// #afit_uNoiseIndInDoor[3] // 616
{0x0F12, 0x0220}, 	//02BC	// #afit_uNoiseIndInDoor[4] // 700
// AFIT table start address // 7000_07C4
{0x002A, 0x0770}, 
{0x0F12, 0x07C4}, 
{0x0F12, 0x7000}, 
// AFIT table (Variables)
{0x002A, 0x07C4}, 
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[0]  AFIT16_BRIGHTNESS   
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[1]  AFIT16_CONTRAST     
{0x0F12, 0x0018}, 	//0000	// #TVAR_afit_pBaseVals[2]  AFIT16_SATURATION  23 10
{0x0F12, 0x0006}, 	//0000	// #TVAR_afit_pBaseVals[3]  AFIT16_SHARP_BLUR   
{0x0F12, 0xFFF6}, 	//0000	// #TVAR_afit_pBaseVals[4]
{0x0F12, 0x00C4}, 	//00C1	// #TVAR_afit_pBaseVals[5]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[6]
{0x0F12, 0x009C}, 	//009C	// #TVAR_afit_pBaseVals[7]
{0x0F12, 0x017C}, 	//017C	// #TVAR_afit_pBaseVals[8]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[9]
{0x0F12, 0x000C}, 	//000C	// #TVAR_afit_pBaseVals[10]
{0x0F12, 0x0010}, 	//0010	// #TVAR_afit_pBaseVals[11]
{0x0F12, 0x012C}, 	//012C	// #TVAR_afit_pBaseVals[12]
{0x0F12, 0x03E8}, 	//03E8	// #TVAR_afit_pBaseVals[13]
{0x0F12, 0x0046}, 	//0046	// #TVAR_afit_pBaseVals[14]
{0x0F12, 0x005A}, 	//005A	// #TVAR_afit_pBaseVals[15]
{0x0F12, 0x0070}, 	//0070	// #TVAR_afit_pBaseVals[16]
{0x0F12, 0x0019}, 	//0028	// #TVAR_afit_pBaseVals[17]
{0x0F12, 0x0019}, 	//0028	// #TVAR_afit_pBaseVals[18]
{0x0F12, 0x01AA}, 	//01AA	// #TVAR_afit_pBaseVals[19]
{0x0F12, 0x0064}, 	//003C	// #TVAR_afit_pBaseVals[20]
{0x0F12, 0x0064}, 	//003C	// #TVAR_afit_pBaseVals[21]
{0x0F12, 0x000A}, 	//0005	// #TVAR_afit_pBaseVals[22]
{0x0F12, 0x000A}, 	//0005	// #TVAR_afit_pBaseVals[23]
{0x0F12, 0x0032}, 	//003C	// #TVAR_afit_pBaseVals[24]
{0x0F12, 0x0012}, 	//0014	// #TVAR_afit_pBaseVals[25]
{0x0F12, 0x002A}, 	//003C	// #TVAR_afit_pBaseVals[26]
{0x0F12, 0x0024}, 	//001E	// #TVAR_afit_pBaseVals[27]
{0x0F12, 0x002A}, 	//003C	// #TVAR_afit_pBaseVals[28]
{0x0F12, 0x0024}, 	//001E	// #TVAR_afit_pBaseVals[29]
{0x0F12, 0x0A24}, 	//0A24	// #TVAR_afit_pBaseVals[30]
{0x0F12, 0x1701}, 	//1701	// #TVAR_afit_pBaseVals[31]
{0x0F12, 0x0229}, 	//0229	// #TVAR_afit_pBaseVals[32]
{0x0F12, 0x1403}, 	//1403	// #TVAR_afit_pBaseVals[33]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[34]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[35]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[36]
{0x0F12, 0x00FF}, 	//00FF	// #TVAR_afit_pBaseVals[37]
{0x0F12, 0x043B}, 	//003B	// #TVAR_afit_pBaseVals[38]
{0x0F12, 0x1414}, 	//1414	// #TVAR_afit_pBaseVals[39]
{0x0F12, 0x0301}, 	//0301	// #TVAR_afit_pBaseVals[40]
{0x0F12, 0xFF07}, 	//FF07	// #TVAR_afit_pBaseVals[41]
{0x0F12, 0x051E}, 	//081E	// #TVAR_afit_pBaseVals[42]
{0x0F12, 0x0A1E}, 	//0A1E	// #TVAR_afit_pBaseVals[43]
{0x0F12, 0x0F0F}, 	//0F0F	// #TVAR_afit_pBaseVals[44]
{0x0F12, 0x0A05}, 	//0A00	// #TVAR_afit_pBaseVals[45]
{0x0F12, 0x0A3C}, 	//0A3C	// #TVAR_afit_pBaseVals[46]
{0x0F12, 0x0A28}, 	//0528	// #TVAR_afit_pBaseVals[47]
{0x0F12, 0x0002}, 	//0002	// #TVAR_afit_pBaseVals[48]
{0x0F12, 0x00FF}, 	//00FF	// #TVAR_afit_pBaseVals[49]
{0x0F12, 0x1102}, 	//1102	// #TVAR_afit_pBaseVals[50]
{0x0F12, 0x001B}, 	//001B	// #TVAR_afit_pBaseVals[51]
{0x0F12, 0x0900}, 	//0900	// #TVAR_afit_pBaseVals[52]
{0x0F12, 0x0600}, 	//0600	// #TVAR_afit_pBaseVals[53]
{0x0F12, 0x0504}, 	//0504	// #TVAR_afit_pBaseVals[54]
{0x0F12, 0x0305}, 	//0306	// #TVAR_afit_pBaseVals[55]
{0x0F12, 0x3C03}, 	//4603	// #TVAR_afit_pBaseVals[56]
{0x0F12, 0x006E}, 	//0078	// #TVAR_afit_pBaseVals[57]
{0x0F12, 0x0178}, 	//0078	// #TVAR_afit_pBaseVals[58]
{0x0F12, 0x0080}, 	//0080	// #TVAR_afit_pBaseVals[59]
{0x0F12, 0x1414}, 	//1414	// #TVAR_afit_pBaseVals[60]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[61]
{0x0F12, 0x5002}, 	//4601	// #TVAR_afit_pBaseVals[62]
{0x0F12, 0x7850}, 	//6E44	// #TVAR_afit_pBaseVals[63]
{0x0F12, 0x2878}, 	//2864	// #TVAR_afit_pBaseVals[64]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[65]
{0x0F12, 0x1403}, 	//0003	// #TVAR_afit_pBaseVals[66]
{0x0F12, 0x1E0C}, 	//1E00	// #TVAR_afit_pBaseVals[67]
{0x0F12, 0x070A}, 	//0714	// #TVAR_afit_pBaseVals[68]
{0x0F12, 0x32FF}, 	//32FF	// #TVAR_afit_pBaseVals[69]
{0x0F12, 0x4104}, 	//1404	// #TVAR_afit_pBaseVals[70]
{0x0F12, 0x123C}, 	//0F14	// #TVAR_afit_pBaseVals[71]
{0x0F12, 0x4012}, 	//400F	// #TVAR_afit_pBaseVals[72]
{0x0F12, 0x0204}, 	//0204	// #TVAR_afit_pBaseVals[73]
{0x0F12, 0x1E03}, 	//1403	// #TVAR_afit_pBaseVals[74]
{0x0F12, 0x011E}, 	//0114	// #TVAR_afit_pBaseVals[75]
{0x0F12, 0x0201}, 	//0101	// #TVAR_afit_pBaseVals[76]
{0x0F12, 0x5050}, 	//4446	// #TVAR_afit_pBaseVals[77]
{0x0F12, 0x3C3C}, 	//646E	// #TVAR_afit_pBaseVals[78]
{0x0F12, 0x0028}, 	//0028	// #TVAR_afit_pBaseVals[79]
{0x0F12, 0x030A}, 	//030A	// #TVAR_afit_pBaseVals[80]
{0x0F12, 0x0714}, 	//0000	// #TVAR_afit_pBaseVals[81]
{0x0F12, 0x0A1E}, 	//141E	// #TVAR_afit_pBaseVals[82]
{0x0F12, 0xFF07}, 	//FF07	// #TVAR_afit_pBaseVals[83]
{0x0F12, 0x0432}, 	//0432	// #TVAR_afit_pBaseVals[84]
{0x0F12, 0x4050}, 	//0000	// #TVAR_afit_pBaseVals[85]
{0x0F12, 0x0F0F}, 	//0F0F	// #TVAR_afit_pBaseVals[86]
{0x0F12, 0x0440}, 	//0440	// #TVAR_afit_pBaseVals[87]
{0x0F12, 0x0302}, 	//0302	// #TVAR_afit_pBaseVals[88]
{0x0F12, 0x1E1E}, 	//1414	// #TVAR_afit_pBaseVals[89]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[90]
{0x0F12, 0x5002}, 	//4601	// #TVAR_afit_pBaseVals[91]
{0x0F12, 0x3C50}, 	//6E44	// #TVAR_afit_pBaseVals[92]
{0x0F12, 0x283C}, 	//2864	// #TVAR_afit_pBaseVals[93]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[94]
{0x0F12, 0x1403}, 	//0003	// #TVAR_afit_pBaseVals[95]
{0x0F12, 0x1E07}, 	//1E00	// #TVAR_afit_pBaseVals[96]
{0x0F12, 0x070A}, 	//0714	// #TVAR_afit_pBaseVals[97]
{0x0F12, 0x32FF}, 	//32FF	// #TVAR_afit_pBaseVals[98]
{0x0F12, 0x5004}, 	//0004	// #TVAR_afit_pBaseVals[99]
{0x0F12, 0x0F40}, 	//0F00	// #TVAR_afit_pBaseVals[100]
{0x0F12, 0x400F}, 	//400F	// #TVAR_afit_pBaseVals[101]
{0x0F12, 0x0204}, 	//0204	// #TVAR_afit_pBaseVals[102]
{0x0F12, 0x0003}, 	//0003	// #TVAR_afit_pBaseVals[103]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[104] AFIT16_BRIGHTNESS   
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[105] AFIT16_CONTRAST     
{0x0F12, 0x0018}, 	//0000	// #TVAR_afit_pBaseVals[106] AFIT16_SATURATION 23  10
{0x0F12, 0x0006}, 	//0000	// #TVAR_afit_pBaseVals[107] AFIT16_SHARP_BLUR   
{0x0F12, 0xFFF6}, 	//0000	// #TVAR_afit_pBaseVals[108]
{0x0F12, 0x00C4}, 	//00C1	// #TVAR_afit_pBaseVals[109]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[110]
{0x0F12, 0x009C}, 	//009C	// #TVAR_afit_pBaseVals[111]
{0x0F12, 0x017C}, 	//017C	// #TVAR_afit_pBaseVals[112]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[113]
{0x0F12, 0x000C}, 	//000C	// #TVAR_afit_pBaseVals[114]
{0x0F12, 0x0010}, 	//0010	// #TVAR_afit_pBaseVals[115]
{0x0F12, 0x012C}, 	//012C	// #TVAR_afit_pBaseVals[116]
{0x0F12, 0x03E8}, 	//03E8	// #TVAR_afit_pBaseVals[117]
{0x0F12, 0x0046}, 	//0046	// #TVAR_afit_pBaseVals[118]
{0x0F12, 0x005A}, 	//005A	// #TVAR_afit_pBaseVals[119]
{0x0F12, 0x0070}, 	//0070	// #TVAR_afit_pBaseVals[120]
{0x0F12, 0x000F}, 	//0000	// #TVAR_afit_pBaseVals[121]
{0x0F12, 0x000F}, 	//0000	// #TVAR_afit_pBaseVals[122]
{0x0F12, 0x01AA}, 	//01AA	// #TVAR_afit_pBaseVals[123]
{0x0F12, 0x003C}, 	//001E	// #TVAR_afit_pBaseVals[124]
{0x0F12, 0x003C}, 	//001E	// #TVAR_afit_pBaseVals[125]
{0x0F12, 0x0005}, 	//0005	// #TVAR_afit_pBaseVals[126]
{0x0F12, 0x0005}, 	//0005	// #TVAR_afit_pBaseVals[127]
{0x0F12, 0x0046}, 	//0064	// #TVAR_afit_pBaseVals[128]
{0x0F12, 0x0019}, 	//0028	// #TVAR_afit_pBaseVals[129]
{0x0F12, 0x002A}, 	//003C	// #TVAR_afit_pBaseVals[130]
{0x0F12, 0x0024}, 	//001E	// #TVAR_afit_pBaseVals[131]
{0x0F12, 0x002A}, 	//003C	// #TVAR_afit_pBaseVals[132]
{0x0F12, 0x0024}, 	//001E	// #TVAR_afit_pBaseVals[133]
{0x0F12, 0x0A24}, 	//0A24	// #TVAR_afit_pBaseVals[134]
{0x0F12, 0x1701}, 	//1701	// #TVAR_afit_pBaseVals[135]
{0x0F12, 0x0229}, 	//0229	// #TVAR_afit_pBaseVals[136]
{0x0F12, 0x1403}, 	//1403	// #TVAR_afit_pBaseVals[137]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[138]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[139]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[140]
{0x0F12, 0x00FF}, 	//00FF	// #TVAR_afit_pBaseVals[141]
{0x0F12, 0x043B}, 	//003B	// #TVAR_afit_pBaseVals[142]
{0x0F12, 0x1414}, 	//1414	// #TVAR_afit_pBaseVals[143]
{0x0F12, 0x0301}, 	//0301	// #TVAR_afit_pBaseVals[144]
{0x0F12, 0xFF07}, 	//FF07	// #TVAR_afit_pBaseVals[145]
{0x0F12, 0x051E}, 	//081E	// #TVAR_afit_pBaseVals[146]
{0x0F12, 0x0A1E}, 	//0A1E	// #TVAR_afit_pBaseVals[147]
{0x0F12, 0x0F0F}, 	//0F0F	// #TVAR_afit_pBaseVals[148]
{0x0F12, 0x0A03}, 	//0A00	// #TVAR_afit_pBaseVals[149]
{0x0F12, 0x0A3C}, 	//0A3C	// #TVAR_afit_pBaseVals[150]
{0x0F12, 0x0A28}, 	//0528	// #TVAR_afit_pBaseVals[151]
{0x0F12, 0x0002}, 	//0002	// #TVAR_afit_pBaseVals[152]
{0x0F12, 0x00FF}, 	//00FF	// #TVAR_afit_pBaseVals[153]
{0x0F12, 0x1102}, 	//1102	// #TVAR_afit_pBaseVals[154]
{0x0F12, 0x001B}, 	//001B	// #TVAR_afit_pBaseVals[155]
{0x0F12, 0x0900}, 	//0900	// #TVAR_afit_pBaseVals[156]
{0x0F12, 0x0600}, 	//0600	// #TVAR_afit_pBaseVals[157]
{0x0F12, 0x0504}, 	//0504	// #TVAR_afit_pBaseVals[158]
{0x0F12, 0x0305}, 	//0306	// #TVAR_afit_pBaseVals[159]
{0x0F12, 0x4603}, 	//5003	// #TVAR_afit_pBaseVals[160]
{0x0F12, 0x0080}, 	//0080	// #TVAR_afit_pBaseVals[161]
{0x0F12, 0x0180}, 	//0080	// #TVAR_afit_pBaseVals[162]
{0x0F12, 0x0080}, 	//0080	// #TVAR_afit_pBaseVals[163]
{0x0F12, 0x1919}, 	//2323	// #TVAR_afit_pBaseVals[164]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[165]
{0x0F12, 0x3C02}, 	//3C01	// #TVAR_afit_pBaseVals[166]
{0x0F12, 0x643C}, 	//553A	// #TVAR_afit_pBaseVals[167]
{0x0F12, 0x2864}, 	//2853	// #TVAR_afit_pBaseVals[168]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[169]
{0x0F12, 0x1403}, 	//0003	// #TVAR_afit_pBaseVals[170]
{0x0F12, 0x1E0C}, 	//1E04	// #TVAR_afit_pBaseVals[171]
{0x0F12, 0x070A}, 	//0714	// #TVAR_afit_pBaseVals[172]
{0x0F12, 0x32FF}, 	//32FF	// #TVAR_afit_pBaseVals[173]
{0x0F12, 0x4104}, 	//1404	// #TVAR_afit_pBaseVals[174]
{0x0F12, 0x123C}, 	//0F14	// #TVAR_afit_pBaseVals[175]
{0x0F12, 0x4012}, 	//400F	// #TVAR_afit_pBaseVals[176]
{0x0F12, 0x0204}, 	//0204	// #TVAR_afit_pBaseVals[177]
{0x0F12, 0x1E03}, 	//1E03	// #TVAR_afit_pBaseVals[178]
{0x0F12, 0x011E}, 	//011E	// #TVAR_afit_pBaseVals[179]
{0x0F12, 0x0201}, 	//0101	// #TVAR_afit_pBaseVals[180]
{0x0F12, 0x3232}, 	//3A3C	// #TVAR_afit_pBaseVals[181]
{0x0F12, 0x3C3C}, 	//585A	// #TVAR_afit_pBaseVals[182]
{0x0F12, 0x0028}, 	//0028	// #TVAR_afit_pBaseVals[183]
{0x0F12, 0x030A}, 	//030A	// #TVAR_afit_pBaseVals[184]
{0x0F12, 0x0714}, 	//0000	// #TVAR_afit_pBaseVals[185]
{0x0F12, 0x0A1E}, 	//141E	// #TVAR_afit_pBaseVals[186]
{0x0F12, 0xFF07}, 	//FF07	// #TVAR_afit_pBaseVals[187]
{0x0F12, 0x0432}, 	//0432	// #TVAR_afit_pBaseVals[188]
{0x0F12, 0x4050}, 	//0000	// #TVAR_afit_pBaseVals[189]
{0x0F12, 0x0F0F}, 	//0F0F	// #TVAR_afit_pBaseVals[190]
{0x0F12, 0x0440}, 	//0440	// #TVAR_afit_pBaseVals[191]
{0x0F12, 0x0302}, 	//0302	// #TVAR_afit_pBaseVals[192]
{0x0F12, 0x1E1E}, 	//1E1E	// #TVAR_afit_pBaseVals[193]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[194]
{0x0F12, 0x3202}, 	//3C01	// #TVAR_afit_pBaseVals[195]
{0x0F12, 0x3C32}, 	//5A3A	// #TVAR_afit_pBaseVals[196]
{0x0F12, 0x283C}, 	//2858	// #TVAR_afit_pBaseVals[197]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[198]
{0x0F12, 0x1403}, 	//0003	// #TVAR_afit_pBaseVals[199]
{0x0F12, 0x1E07}, 	//1E00	// #TVAR_afit_pBaseVals[200]
{0x0F12, 0x070A}, 	//0714	// #TVAR_afit_pBaseVals[201]
{0x0F12, 0x32FF}, 	//32FF	// #TVAR_afit_pBaseVals[202]
{0x0F12, 0x5004}, 	//0004	// #TVAR_afit_pBaseVals[203]
{0x0F12, 0x0F40}, 	//0F00	// #TVAR_afit_pBaseVals[204]
{0x0F12, 0x400F}, 	//400F	// #TVAR_afit_pBaseVals[205]
{0x0F12, 0x0204}, 	//0204	// #TVAR_afit_pBaseVals[206]
{0x0F12, 0x0003}, 	//0003	// #TVAR_afit_pBaseVals[207]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[208]  AFIT16_BRIGHTNESS   
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[209]  AFIT16_CONTRAST     
//{0x0F12, 0x001A}, 	//0000	// #TVAR_afit_pBaseVals[210]  AFIT16_SATURATION 28 10 
//{0x0F12, 0xFFEC}, 	//0000	// #TVAR_afit_pBaseVals[210]  AFIT16_SATURATION 28 10 //20120613
//{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[210]  AFIT16_SATURATION 28 10 //20120613
//{0x0F12, 0xFFF8}, 	//0000	// #TVAR_afit_pBaseVals[210]  AFIT16_SATURATION 28 10 //20120613
{0x0F12, 0xFFFC}, 	//0000	// #TVAR_afit_pBaseVals[210]  AFIT16_SATURATION 28 10 //20120613
{0x0F12, 0x0005}, 	//0000	// #TVAR_afit_pBaseVals[211]  AFIT16_SHARP_BLUR   
{0x0F12, 0xFFF6}, 	//0000	// #TVAR_afit_pBaseVals[212]
{0x0F12, 0x00C4}, 	//00C1	// #TVAR_afit_pBaseVals[213]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[214]
{0x0F12, 0x009C}, 	//009C	// #TVAR_afit_pBaseVals[215]
{0x0F12, 0x017C}, 	//017C	// #TVAR_afit_pBaseVals[216]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[217]
{0x0F12, 0x000C}, 	//000C	// #TVAR_afit_pBaseVals[218]
{0x0F12, 0x0010}, 	//0010	// #TVAR_afit_pBaseVals[219]
{0x0F12, 0x012C}, 	//012C	// #TVAR_afit_pBaseVals[220]
{0x0F12, 0x03E8}, 	//03E8	// #TVAR_afit_pBaseVals[221]
{0x0F12, 0x0046}, 	//0046	// #TVAR_afit_pBaseVals[222]
{0x0F12, 0x0078}, 	//005A	// #TVAR_afit_pBaseVals[223]
{0x0F12, 0x0070}, 	//0070	// #TVAR_afit_pBaseVals[224]
{0x0F12, 0x0004}, 	//0000	// #TVAR_afit_pBaseVals[225]
{0x0F12, 0x0004}, 	//0000	// #TVAR_afit_pBaseVals[226]
{0x0F12, 0x01AA}, 	//01AA	// #TVAR_afit_pBaseVals[227]
{0x0F12, 0x001E}, 	//001E	// #TVAR_afit_pBaseVals[228]
{0x0F12, 0x001E}, 	//001E	// #TVAR_afit_pBaseVals[229]
{0x0F12, 0x0005}, 	//0005	// #TVAR_afit_pBaseVals[230]
{0x0F12, 0x0005}, 	//0005	// #TVAR_afit_pBaseVals[231]
{0x0F12, 0x0064}, 	//00C8	// #TVAR_afit_pBaseVals[232]
{0x0F12, 0x001B}, 	//0055	// #TVAR_afit_pBaseVals[233]
{0x0F12, 0x002A}, 	//003C	// #TVAR_afit_pBaseVals[234]
{0x0F12, 0x0024}, 	//001E	// #TVAR_afit_pBaseVals[235]
{0x0F12, 0x002A}, 	//003C	// #TVAR_afit_pBaseVals[236]
{0x0F12, 0x0024}, 	//001E	// #TVAR_afit_pBaseVals[237]
{0x0F12, 0x0A24}, 	//0A24	// #TVAR_afit_pBaseVals[238]
{0x0F12, 0x1701}, 	//1701	// #TVAR_afit_pBaseVals[239]
{0x0F12, 0x0229}, 	//0229	// #TVAR_afit_pBaseVals[240]
{0x0F12, 0x1403}, 	//1403	// #TVAR_afit_pBaseVals[241]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[242]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[243]
{0x0F12, 0x0000}, 	//0202	// #TVAR_afit_pBaseVals[244]
{0x0F12, 0x00FF}, 	//00FF	// #TVAR_afit_pBaseVals[245]
{0x0F12, 0x043B}, 	//003B	// #TVAR_afit_pBaseVals[246]
{0x0F12, 0x1414}, 	//1414	// #TVAR_afit_pBaseVals[247]
{0x0F12, 0x0301}, 	//0301	// #TVAR_afit_pBaseVals[248]
{0x0F12, 0xFF07}, 	//FF07	// #TVAR_afit_pBaseVals[249]
{0x0F12, 0x051E}, 	//081E	// #TVAR_afit_pBaseVals[250]
{0x0F12, 0x0A1E}, 	//0A1E	// #TVAR_afit_pBaseVals[251]
{0x0F12, 0x0F0F}, 	//0F0F	// #TVAR_afit_pBaseVals[252]
{0x0F12, 0x0A03}, 	//0A00	// #TVAR_afit_pBaseVals[253]
{0x0F12, 0x0A3C}, 	//3C78	// #TVAR_afit_pBaseVals[254]
{0x0F12, 0x0528}, 	//0328	// #TVAR_afit_pBaseVals[255]
{0x0F12, 0x0002}, 	//0002	// #TVAR_afit_pBaseVals[256]
{0x0F12, 0x00FF}, 	//00FF	// #TVAR_afit_pBaseVals[257]
{0x0F12, 0x1102}, 	//1102	// #TVAR_afit_pBaseVals[258]
{0x0F12, 0x001B}, 	//001B	// #TVAR_afit_pBaseVals[259]
{0x0F12, 0x0900}, 	//0900	// #TVAR_afit_pBaseVals[260]
{0x0F12, 0x0600}, 	//0600	// #TVAR_afit_pBaseVals[261]
{0x0F12, 0x0504}, 	//0504	// #TVAR_afit_pBaseVals[262]
{0x0F12, 0x0305}, 	//0306	// #TVAR_afit_pBaseVals[263]
{0x0F12, 0x4603}, 	//5A03	// #TVAR_afit_pBaseVals[264]
{0x0F12, 0x0080}, 	//0080	// #TVAR_afit_pBaseVals[265]
{0x0F12, 0x0180}, 	//0080	// #TVAR_afit_pBaseVals[266]
{0x0F12, 0x0080}, 	//0080	// #TVAR_afit_pBaseVals[267]
{0x0F12, 0x2323}, 	//3C3C	// #TVAR_afit_pBaseVals[268]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[269]
{0x0F12, 0x2A02}, 	//2401	// #TVAR_afit_pBaseVals[270]
{0x0F12, 0x3C2A}, 	//281E	// #TVAR_afit_pBaseVals[271]
{0x0F12, 0x283C}, 	//2828	// #TVAR_afit_pBaseVals[272]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[273]
{0x0F12, 0x1403}, 	//1403	// #TVAR_afit_pBaseVals[274]
{0x0F12, 0x1E0C}, 	//140A	// #TVAR_afit_pBaseVals[275]
{0x0F12, 0x070A}, 	//0808	// #TVAR_afit_pBaseVals[276]
{0x0F12, 0x32FF}, 	//1EC8	// #TVAR_afit_pBaseVals[277]
{0x0F12, 0x4B04}, 	//3C04	// #TVAR_afit_pBaseVals[278]
{0x0F12, 0x0F40}, 	//0C40	// #TVAR_afit_pBaseVals[279]
{0x0F12, 0x400F}, 	//400A	// #TVAR_afit_pBaseVals[280]
{0x0F12, 0x0204}, 	//0204	// #TVAR_afit_pBaseVals[281]
{0x0F12, 0x2303}, 	//2803	// #TVAR_afit_pBaseVals[282]
{0x0F12, 0x0123}, 	//0128	// #TVAR_afit_pBaseVals[283]
{0x0F12, 0x0201}, 	//0101	// #TVAR_afit_pBaseVals[284]
{0x0F12, 0x262A}, 	//2224	// #TVAR_afit_pBaseVals[285]
{0x0F12, 0x2C2C}, 	//3236	// #TVAR_afit_pBaseVals[286]
{0x0F12, 0x0028}, 	//0028	// #TVAR_afit_pBaseVals[287]
{0x0F12, 0x030A}, 	//030A	// #TVAR_afit_pBaseVals[288]
{0x0F12, 0x0714}, 	//0410	// #TVAR_afit_pBaseVals[289]
{0x0F12, 0x0A1E}, 	//141E	// #TVAR_afit_pBaseVals[290]
{0x0F12, 0xFF07}, 	//FF07	// #TVAR_afit_pBaseVals[291]
{0x0F12, 0x0432}, 	//0432	// #TVAR_afit_pBaseVals[292]
{0x0F12, 0x4050}, 	//4050	// #TVAR_afit_pBaseVals[293]
{0x0F12, 0x0F0F}, 	//0F0F	// #TVAR_afit_pBaseVals[294]
{0x0F12, 0x0440}, 	//0440	// #TVAR_afit_pBaseVals[295]
{0x0F12, 0x0302}, 	//0302	// #TVAR_afit_pBaseVals[296]
{0x0F12, 0x2323}, 	//2828	// #TVAR_afit_pBaseVals[297]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[298]
{0x0F12, 0x2A02}, 	//2401	// #TVAR_afit_pBaseVals[299]
{0x0F12, 0x2C26}, 	//3622	// #TVAR_afit_pBaseVals[300]
{0x0F12, 0x282C}, 	//2832	// #TVAR_afit_pBaseVals[301]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[302]
{0x0F12, 0x1403}, 	//1003	// #TVAR_afit_pBaseVals[303]
{0x0F12, 0x1E07}, 	//1E04	// #TVAR_afit_pBaseVals[304]
{0x0F12, 0x070A}, 	//0714	// #TVAR_afit_pBaseVals[305]
{0x0F12, 0x32FF}, 	//32FF	// #TVAR_afit_pBaseVals[306]
{0x0F12, 0x5004}, 	//5004	// #TVAR_afit_pBaseVals[307]
{0x0F12, 0x0F40}, 	//0F40	// #TVAR_afit_pBaseVals[308]
{0x0F12, 0x400F}, 	//400F	// #TVAR_afit_pBaseVals[309]
{0x0F12, 0x0204}, 	//0204	// #TVAR_afit_pBaseVals[310]
{0x0F12, 0x0003}, 	//0003	// #TVAR_afit_pBaseVals[311]
//{0x0F12, 0x000A}, 	//0000	// #TVAR_afit_pBaseVals[312] AFIT16_BRIGHTNESS  0003 
//{0x0F12, 0x001A}, 	//0000	// #TVAR_afit_pBaseVals[312] AFIT16_BRIGHTNESS  0003 //20120608
{0x0F12, 0x0010}, 	//0000	// #TVAR_afit_pBaseVals[312] AFIT16_BRIGHTNESS  0003 //20120613
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[313] AFIT16_CONTRAST     
//{0x0F12, 0x001A}, 	//0000	// #TVAR_afit_pBaseVals[314] AFIT16_SATURATION 28  10
//{0x0F12, 0x0010}, 	//0000	// #TVAR_afit_pBaseVals[314] AFIT16_SATURATION 28  10	//20120608
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[314] AFIT16_SATURATION 28  10	//20120613
//{0x0F12, 0xFFEC}, 	//0000	// #TVAR_afit_pBaseVals[314] AFIT16_SATURATION 28  10	//20120613
//{0x0F12, 0xFFE2}, 	//0000	// #TVAR_afit_pBaseVals[314] AFIT16_SATURATION 28  10	//20120613
//{0x0F12, 0xFFC4}, 	//0000	// #TVAR_afit_pBaseVals[314] AFIT16_SATURATION 28  10	//20120613
{0x0F12, 0x0002}, 	//0000	// #TVAR_afit_pBaseVals[315] AFIT16_SHARP_BLUR   
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[316]
{0x0F12, 0x00C4}, 	//00C1	// #TVAR_afit_pBaseVals[317]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[318]
{0x0F12, 0x009C}, 	//009C	// #TVAR_afit_pBaseVals[319]
{0x0F12, 0x017C}, 	//017C	// #TVAR_afit_pBaseVals[320]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[321]
{0x0F12, 0x000C}, 	//000C	// #TVAR_afit_pBaseVals[322]
{0x0F12, 0x0010}, 	//0010	// #TVAR_afit_pBaseVals[323]
{0x0F12, 0x00C8}, 	//00C8	// #TVAR_afit_pBaseVals[324]
{0x0F12, 0x0384}, 	//03E8	// #TVAR_afit_pBaseVals[325]
{0x0F12, 0x0046}, 	//0046	// #TVAR_afit_pBaseVals[326]
{0x0F12, 0x0082}, 	//0050	// #TVAR_afit_pBaseVals[327]
{0x0F12, 0x0070}, 	//0070	// #TVAR_afit_pBaseVals[328]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[329]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[330]
{0x0F12, 0x01AA}, 	//01AA	// #TVAR_afit_pBaseVals[331]
{0x0F12, 0x001E}, 	//0014	// #TVAR_afit_pBaseVals[332]
{0x0F12, 0x001E}, 	//0014	// #TVAR_afit_pBaseVals[333]
{0x0F12, 0x000A}, 	//000A	// #TVAR_afit_pBaseVals[334]
{0x0F12, 0x000A}, 	//000A	// #TVAR_afit_pBaseVals[335]
{0x0F12, 0x010E}, 	//00C8	// #TVAR_afit_pBaseVals[336]
{0x0F12, 0x0028}, 	//0041	// #TVAR_afit_pBaseVals[337]
{0x0F12, 0x0032}, 	//002D	// #TVAR_afit_pBaseVals[338]
{0x0F12, 0x0028}, 	//0019	// #TVAR_afit_pBaseVals[339]
{0x0F12, 0x0032}, 	//002D	// #TVAR_afit_pBaseVals[340]
{0x0F12, 0x0028}, 	//0019	// #TVAR_afit_pBaseVals[341]
{0x0F12, 0x0A24}, 	//0A24	// #TVAR_afit_pBaseVals[342]
{0x0F12, 0x1701}, 	//1701	// #TVAR_afit_pBaseVals[343]
{0x0F12, 0x0229}, 	//0229	// #TVAR_afit_pBaseVals[344]
{0x0F12, 0x1403}, 	//1403	// #TVAR_afit_pBaseVals[345]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[346]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[347]
{0x0F12, 0x0504}, 	//0300	// #TVAR_afit_pBaseVals[348]
{0x0F12, 0x00FF}, 	//00FF	// #TVAR_afit_pBaseVals[349]
{0x0F12, 0x043B}, 	//003B	// #TVAR_afit_pBaseVals[350]
{0x0F12, 0x1414}, 	//1414	// #TVAR_afit_pBaseVals[351]
{0x0F12, 0x0301}, 	//0301	// #TVAR_afit_pBaseVals[352]
{0x0F12, 0xFF07}, 	//FF07	// #TVAR_afit_pBaseVals[353]
{0x0F12, 0x051E}, 	//081E	// #TVAR_afit_pBaseVals[354]
{0x0F12, 0x0A1E}, 	//0A1E	// #TVAR_afit_pBaseVals[355]
{0x0F12, 0x0F0F}, 	//0F0F	// #TVAR_afit_pBaseVals[356]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[357]
{0x0F12, 0x0A3C}, 	//3C78	// #TVAR_afit_pBaseVals[358]
{0x0F12, 0x0532}, 	//0828	// #TVAR_afit_pBaseVals[359]
{0x0F12, 0x0002}, 	//0001	// #TVAR_afit_pBaseVals[360]
{0x0F12, 0x00FF}, 	//00FF	// #TVAR_afit_pBaseVals[361]
{0x0F12, 0x1002}, 	//1002	// #TVAR_afit_pBaseVals[362]
{0x0F12, 0x001E}, 	//001E	// #TVAR_afit_pBaseVals[363]
{0x0F12, 0x0900}, 	//0900	// #TVAR_afit_pBaseVals[364]
{0x0F12, 0x0600}, 	//0600	// #TVAR_afit_pBaseVals[365]
{0x0F12, 0x0504}, 	//0504	// #TVAR_afit_pBaseVals[366]
{0x0F12, 0x0305}, 	//0307	// #TVAR_afit_pBaseVals[367]
{0x0F12, 0x4602}, 	//6402	// #TVAR_afit_pBaseVals[368]
{0x0F12, 0x0080}, 	//0080	// #TVAR_afit_pBaseVals[369]
{0x0F12, 0x0180}, 	//0080	// #TVAR_afit_pBaseVals[370]
{0x0F12, 0x0080}, 	//0080	// #TVAR_afit_pBaseVals[371]
{0x0F12, 0x2328}, 	//4646	// #TVAR_afit_pBaseVals[372]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[373]
{0x0F12, 0x2A02}, 	//2401	// #TVAR_afit_pBaseVals[374]
{0x0F12, 0x2228}, 	//191E	// #TVAR_afit_pBaseVals[375]
{0x0F12, 0x2822}, 	//3217	// #TVAR_afit_pBaseVals[376]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[377]
{0x0F12, 0x1903}, 	//1403	// #TVAR_afit_pBaseVals[378]
{0x0F12, 0x1E0F}, 	//140C	// #TVAR_afit_pBaseVals[379]
{0x0F12, 0x070A}, 	//0808	// #TVAR_afit_pBaseVals[380]
{0x0F12, 0x32FF}, 	//1EC8	// #TVAR_afit_pBaseVals[381]
{0x0F12, 0x9604}, 	//5004	// #TVAR_afit_pBaseVals[382]
{0x0F12, 0x0F42}, 	//0C40	// #TVAR_afit_pBaseVals[383]
{0x0F12, 0x400F}, 	//400A	// #TVAR_afit_pBaseVals[384]
{0x0F12, 0x0504}, 	//0204	// #TVAR_afit_pBaseVals[385]
{0x0F12, 0x2805}, 	//3C03	// #TVAR_afit_pBaseVals[386]
{0x0F12, 0x0123}, 	//013C	// #TVAR_afit_pBaseVals[387]
{0x0F12, 0x0201}, 	//0101	// #TVAR_afit_pBaseVals[388]
{0x0F12, 0x2024}, 	//1C1E	// #TVAR_afit_pBaseVals[389]
{0x0F12, 0x1C1C}, 	//1E22	// #TVAR_afit_pBaseVals[390]
{0x0F12, 0x0028}, 	//0028	// #TVAR_afit_pBaseVals[391]
{0x0F12, 0x030A}, 	//030A	// #TVAR_afit_pBaseVals[392]
{0x0F12, 0x0A0A}, 	//0214	// #TVAR_afit_pBaseVals[393]
{0x0F12, 0x0A2D}, 	//0E14	// #TVAR_afit_pBaseVals[394]
{0x0F12, 0xFF07}, 	//FF06	// #TVAR_afit_pBaseVals[395]
{0x0F12, 0x0432}, 	//0432	// #TVAR_afit_pBaseVals[396]
{0x0F12, 0x4050}, 	//4052	// #TVAR_afit_pBaseVals[397]
{0x0F12, 0x0F0F}, 	//150C	// #TVAR_afit_pBaseVals[398]
{0x0F12, 0x0440}, 	//0440	// #TVAR_afit_pBaseVals[399]
{0x0F12, 0x0302}, 	//0302	// #TVAR_afit_pBaseVals[400]
{0x0F12, 0x2328}, 	//3C3C	// #TVAR_afit_pBaseVals[401]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[402]
{0x0F12, 0x3C02}, 	//1E01	// #TVAR_afit_pBaseVals[403]
{0x0F12, 0x1C3C}, 	//221C	// #TVAR_afit_pBaseVals[404]
{0x0F12, 0x281C}, 	//281E	// #TVAR_afit_pBaseVals[405]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[406]
{0x0F12, 0x0A03}, 	//1403	// #TVAR_afit_pBaseVals[407]
{0x0F12, 0x2D0A}, 	//1402	// #TVAR_afit_pBaseVals[408]
{0x0F12, 0x070A}, 	//060E	// #TVAR_afit_pBaseVals[409]
{0x0F12, 0x32FF}, 	//32FF	// #TVAR_afit_pBaseVals[410]
{0x0F12, 0x5004}, 	//5204	// #TVAR_afit_pBaseVals[411]
{0x0F12, 0x0F40}, 	//0C40	// #TVAR_afit_pBaseVals[412]
{0x0F12, 0x400F}, 	//4015	// #TVAR_afit_pBaseVals[413]
{0x0F12, 0x0204}, 	//0204	// #TVAR_afit_pBaseVals[414]
{0x0F12, 0x0003}, 	//0003	// #TVAR_afit_pBaseVals[415]
//{0x0F12, 0x000A}, 	//0000	// #TVAR_afit_pBaseVals[416]  AFIT16_BRIGHTNESS 0000  
//{0x0F12, 0x001A}, 	//0000	// #TVAR_afit_pBaseVals[416]  AFIT16_BRIGHTNESS 0000  //20120608
{0x0F12, 0x0010}, 	//0000	// #TVAR_afit_pBaseVals[416]  AFIT16_BRIGHTNESS 0000  //20120613
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[417]  AFIT16_CONTRAST     
//{0x0F12, 0x001A}, 	//0000	// #TVAR_afit_pBaseVals[418]  AFIT16_SATURATION 28 10  
//{0x0F12, 0x0012}, 	//0000	// #TVAR_afit_pBaseVals[418]  AFIT16_SATURATION 28 10  //20120608
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[418]  AFIT16_SATURATION 28 10  //20120608
//{0x0F12, 0xFFEC}, 	//0000	// #TVAR_afit_pBaseVals[418]  AFIT16_SATURATION 28 10  //20120613
//{0x0F12, 0xFFE2}, 	//0000	// #TVAR_afit_pBaseVals[418]  AFIT16_SATURATION 28 10  //20120613
//{0x0F12, 0xFFC4}, 	//0000	// #TVAR_afit_pBaseVals[418]  AFIT16_SATURATION 28 10  //20120613
{0x0F12, 0x0002}, 	//0000	// #TVAR_afit_pBaseVals[419]  AFIT16_SHARP_BLUR   
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[420]
{0x0F12, 0x00C4}, 	//00C1	// #TVAR_afit_pBaseVals[421]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[422]
{0x0F12, 0x009C}, 	//009C	// #TVAR_afit_pBaseVals[423]
{0x0F12, 0x017C}, 	//017C	// #TVAR_afit_pBaseVals[424]
{0x0F12, 0x03FF}, 	//03FF	// #TVAR_afit_pBaseVals[425]
{0x0F12, 0x000C}, 	//000C	// #TVAR_afit_pBaseVals[426]
{0x0F12, 0x0010}, 	//0010	// #TVAR_afit_pBaseVals[427]
{0x0F12, 0x00C8}, 	//0032	// #TVAR_afit_pBaseVals[428]
{0x0F12, 0x0320}, 	//028A	// #TVAR_afit_pBaseVals[429]
{0x0F12, 0x0046}, 	//0032	// #TVAR_afit_pBaseVals[430]
{0x0F12, 0x015E}, 	//01F4	// #TVAR_afit_pBaseVals[431]
{0x0F12, 0x0070}, 	//0070	// #TVAR_afit_pBaseVals[432]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[433]
{0x0F12, 0x0000}, 	//0000	// #TVAR_afit_pBaseVals[434]
{0x0F12, 0x01AA}, 	//01AA	// #TVAR_afit_pBaseVals[435]
{0x0F12, 0x0014}, 	//003C	// #TVAR_afit_pBaseVals[436]
{0x0F12, 0x0014}, 	//0050	// #TVAR_afit_pBaseVals[437]
{0x0F12, 0x000A}, 	//0000	// #TVAR_afit_pBaseVals[438]
{0x0F12, 0x000A}, 	//0000	// #TVAR_afit_pBaseVals[439]
{0x0F12, 0x0140}, 	//003C	// #TVAR_afit_pBaseVals[440]
{0x0F12, 0x003C}, 	//0014	// #TVAR_afit_pBaseVals[441]
{0x0F12, 0x0032}, 	//0046	// #TVAR_afit_pBaseVals[442]
{0x0F12, 0x0023}, 	//0019	// #TVAR_afit_pBaseVals[443]
{0x0F12, 0x0023}, 	//0046	// #TVAR_afit_pBaseVals[444]
{0x0F12, 0x0032}, 	//0019	// #TVAR_afit_pBaseVals[445]
{0x0F12, 0x0A24}, 	//0A24	// #TVAR_afit_pBaseVals[446]
{0x0F12, 0x1701}, 	//1701	// #TVAR_afit_pBaseVals[447]
{0x0F12, 0x0229}, 	//0229	// #TVAR_afit_pBaseVals[448]
{0x0F12, 0x1403}, 	//0503	// #TVAR_afit_pBaseVals[449]
{0x0F12, 0x0000}, 	//080F	// #TVAR_afit_pBaseVals[450]
{0x0F12, 0x0000}, 	//0808	// #TVAR_afit_pBaseVals[451]
{0x0F12, 0x0505}, 	//0000	// #TVAR_afit_pBaseVals[452]
{0x0F12, 0x00FF}, 	//00FF	// #TVAR_afit_pBaseVals[453]
{0x0F12, 0x043B}, 	//002D	// #TVAR_afit_pBaseVals[454]
{0x0F12, 0x1414}, 	//1414	// #TVAR_afit_pBaseVals[455]
{0x0F12, 0x0301}, 	//0301	// #TVAR_afit_pBaseVals[456]
{0x0F12, 0xFF07}, 	//FF07	// #TVAR_afit_pBaseVals[457]
{0x0F12, 0x051E}, 	//061E	// #TVAR_afit_pBaseVals[458]
{0x0F12, 0x0A1E}, 	//0A1E	// #TVAR_afit_pBaseVals[459]
{0x0F12, 0x0000}, 	//0606	// #TVAR_afit_pBaseVals[460]
{0x0F12, 0x0A00}, 	//0A01	// #TVAR_afit_pBaseVals[461]
{0x0F12, 0x143C}, 	//378B	// #TVAR_afit_pBaseVals[462]
{0x0F12, 0x0532}, 	//1028	// #TVAR_afit_pBaseVals[463]
{0x0F12, 0x0002}, 	//0001	// #TVAR_afit_pBaseVals[464]
{0x0F12, 0x0096}, 	//00FF	// #TVAR_afit_pBaseVals[465]
{0x0F12, 0x1002}, 	//1002	// #TVAR_afit_pBaseVals[466]
{0x0F12, 0x001E}, 	//001E	// #TVAR_afit_pBaseVals[467]
{0x0F12, 0x0900}, 	//0900	// #TVAR_afit_pBaseVals[468]
{0x0F12, 0x0600}, 	//0600	// #TVAR_afit_pBaseVals[469]
{0x0F12, 0x0504}, 	//0504	// #TVAR_afit_pBaseVals[470]
{0x0F12, 0x0305}, 	//0307	// #TVAR_afit_pBaseVals[471]
{0x0F12, 0x6402}, 	//6402	// #TVAR_afit_pBaseVals[472]
{0x0F12, 0x0080}, 	//0080	// #TVAR_afit_pBaseVals[473]
{0x0F12, 0x0180}, 	//0080	// #TVAR_afit_pBaseVals[474]
{0x0F12, 0x0080}, 	//0080	// #TVAR_afit_pBaseVals[475]
{0x0F12, 0x5050}, 	//5050	// #TVAR_afit_pBaseVals[476]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[477]
{0x0F12, 0x1C02}, 	//3201	// #TVAR_afit_pBaseVals[478]
{0x0F12, 0x191C}, 	//1432	// #TVAR_afit_pBaseVals[479]
{0x0F12, 0x2819}, 	//2112	// #TVAR_afit_pBaseVals[480]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[481]
{0x0F12, 0x1E03}, 	//1404	// #TVAR_afit_pBaseVals[482]
{0x0F12, 0x1E0F}, 	//1408	// #TVAR_afit_pBaseVals[483]
{0x0F12, 0x0508}, 	//070C	// #TVAR_afit_pBaseVals[484]
{0x0F12, 0x32FF}, 	//32C8	// #TVAR_afit_pBaseVals[485]
{0x0F12, 0xAA04}, 	//5004	// #TVAR_afit_pBaseVals[486]
{0x0F12, 0x1452}, 	//0F40	// #TVAR_afit_pBaseVals[487]
{0x0F12, 0x4015}, 	//4014	// #TVAR_afit_pBaseVals[488]
{0x0F12, 0x0604}, 	//0604	// #TVAR_afit_pBaseVals[489]
{0x0F12, 0x5006}, 	//4606	// #TVAR_afit_pBaseVals[490]
{0x0F12, 0x0150}, 	//0146	// #TVAR_afit_pBaseVals[491]
{0x0F12, 0x0201}, 	//0101	// #TVAR_afit_pBaseVals[492]
{0x0F12, 0x1E1E}, 	//1C18	// #TVAR_afit_pBaseVals[493]
{0x0F12, 0x1212}, 	//1819	// #TVAR_afit_pBaseVals[494]
{0x0F12, 0x0028}, 	//0028	// #TVAR_afit_pBaseVals[495]
{0x0F12, 0x030A}, 	//030A	// #TVAR_afit_pBaseVals[496]
{0x0F12, 0x0A10}, 	//0514	// #TVAR_afit_pBaseVals[497]
{0x0F12, 0x0819}, 	//0C14	// #TVAR_afit_pBaseVals[498]
{0x0F12, 0xFF05}, 	//FF05	// #TVAR_afit_pBaseVals[499]
{0x0F12, 0x0432}, 	//0432	// #TVAR_afit_pBaseVals[500]
{0x0F12, 0x4052}, 	//4052	// #TVAR_afit_pBaseVals[501]
{0x0F12, 0x1514}, 	//1514	// #TVAR_afit_pBaseVals[502]
{0x0F12, 0x0440}, 	//0440	// #TVAR_afit_pBaseVals[503]
{0x0F12, 0x0302}, 	//0302	// #TVAR_afit_pBaseVals[504]
{0x0F12, 0x5050}, 	//4646	// #TVAR_afit_pBaseVals[505]
{0x0F12, 0x0101}, 	//0101	// #TVAR_afit_pBaseVals[506]
{0x0F12, 0x1E02}, 	//1801	// #TVAR_afit_pBaseVals[507]
{0x0F12, 0x121E}, 	//191C	// #TVAR_afit_pBaseVals[508]
{0x0F12, 0x2812}, 	//2818	// #TVAR_afit_pBaseVals[509]
{0x0F12, 0x0A00}, 	//0A00	// #TVAR_afit_pBaseVals[510]
{0x0F12, 0x1003}, 	//1403	// #TVAR_afit_pBaseVals[511]
{0x0F12, 0x190A}, 	//1405	// #TVAR_afit_pBaseVals[512]
{0x0F12, 0x0508}, 	//050C	// #TVAR_afit_pBaseVals[513]
{0x0F12, 0x32FF}, 	//32FF	// #TVAR_afit_pBaseVals[514]
{0x0F12, 0x5204}, 	//5204	// #TVAR_afit_pBaseVals[515]
{0x0F12, 0x1440}, 	//1440	// #TVAR_afit_pBaseVals[516]
{0x0F12, 0x4015}, 	//4015	// #TVAR_afit_pBaseVals[517]
{0x0F12, 0x0204}, 	//0204	// #TVAR_afit_pBaseVals[518]
{0x0F12, 0x0003}, 	//0003	// #TVAR_afit_pBaseVals[519] 
//AFIT table (Constants)
{0x0F12, 0x7FFA}, //#afit_pConstBaseVals[0]
{0x0F12, 0xFFBD}, //#afit_pConstBaseVals[1]
{0x0F12, 0x26FE}, //#afit_pConstBaseVals[2]
{0x0F12, 0xF7BC}, //#afit_pConstBaseVals[3]
{0x0F12, 0x7E06}, //#afit_pConstBaseVals[4]
{0x0F12, 0x00D3}, //#afit_pConstBaseVals[5]
//Update Changed Registers
{0x002A, 0x0664}, 
{0x0F12, 0x013E}, //013E	//seti_uContrastCenter

//WRITE	700004D6	0001	// #REG_TC_DBG_ReInitCmd

//Fill RAM with alternative op-codes
{0x002A, 0x2CE8}, 
{0x0F12, 0x0007}, //Modify LSB to control AWBB_YThreshLow
{0x0F12, 0x00E2}, 
{0x0F12, 0x0005}, //Modify LSB to control AWBB_YThreshLowBrLow
{0x0F12, 0x00E2}, 

//////////////////////////////////////////////////////////////////////////
//AFC - fix if ( G_AFC_Confidence[G_AFC_SuspectedState] > AFC_CONFIDENCE_HIGH_THR ) condition to if ( G_AFC_Confidence[G_AFC_SuspectedState] > (10<<10) )
//Fill RAM with alternative op-codes

{0x002A, 0x2CE6}, 
{0x0F12, 0x220A}, //3 ==> A Modify AFC_CONFIDENCE_HIGH_THR
{0x002A, 0x3378}, 
{0x0F12, 0x0030}, //#Tune_TP_AFC_SCD_Thresh ==> Set > 0 to activate the SCD T&P code for AFC.

{0x002A, 0x10E2}, 
{0x0F12, 0x00C0}, //192, decay factor (using af_search_usCapturePolicy) ==> change to 230 ??
{0x002A, 0x10E0}, 
{0x0F12, 0x0008}, //8 frames wait for scene stable (using M_af_search_usFineMaxScale) ==> can vary between 4..10

//force zeroing of "G_AFC_Conf50_VerySlow" and "G_AFC_Conf60_VerySlow", (using M_af_stat_usG2)

{0x002A,0x0C1E}, 
{0x0F12,0x0018}, //change "alpha" parameter from "0x10" to "0x16"

	
{SENSOR_TABLE_END, 0x00}
          
};     


#endif /* ifndef S5K5CAG_REGS_H */
