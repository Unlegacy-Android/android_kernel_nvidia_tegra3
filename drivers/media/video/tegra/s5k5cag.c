/*
 * s5k5cag.c - s5k5cag sensor driver
 *
 * Copyright (c) 2011-2012, NVIDIA, All Rights Reserved.
 *
 * Contributors:
 *      erik lilliebjerg <elilliebjerg@nvidia.com>
 *
 * Leverage ov2710.c
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */
#define DEBUG
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/export.h>
#include <linux/module.h>
#include <media/s5k5cag.h>
#include <media/yuv_sensor_cl2n.h>
#include "s5k5cag_regs.h"

#define S5K5CAG_SENSOR_NAME "s5k5cag"

enum {
      Ratio_4_3 = 0,
      Ratio_16_9
};

struct s5k5cag_info {
	int mode;
	struct i2c_client *i2c_client;
	struct yuv_sensor_platform_data *pdata;
	u16 last_ratio;
};

static struct s5k5cag_info *s5k5cag_sensor_info;
int init_check_s5k5cag = 1;
static int openflage4ratio = 0; //&*&*&*CJ1_add: fixed remind bug 1796&1769 for ratio issue.

static int s5k5cag_read_reg(struct i2c_client *client, u16 reg, u16 *val)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[4] = {0};


	if (!client->adapter)
		return -ENODEV;
		

	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 2;
	msg->buf = data;

	/* Write addr - high byte goes out first */
	data[0] = (u8) (reg >> 8);;
	data[1] = (u8) (reg & 0xff);
	err = i2c_transfer(client->adapter, msg, 1);

	/* Read back data */
	if (err >= 0) {
		msg->len = 4;
		msg->flags = I2C_M_RD;
		err = i2c_transfer(client->adapter, msg, 1);
	}
	if (err >= 0) {
		*val = 0;
		/* high byte comes first */
			*val = data[1] + (data[0] << 8);
			
		return 0;
	}
	//v4l_err(client, "read from offset 0x%x error %d\n", reg, err);
	return err;
}

static int s5k5cag_write_reg(struct i2c_client *client, u16 addr, u16 val)
{
	int err;
	struct i2c_msg msg;
	unsigned char data[4];
	int retry = 0;

	if (!client->adapter)
		return -ENODEV;

	data[0] = (u8) (addr >> 8);
	data[1] = (u8) (addr & 0xff);
	data[2] = (u8) (val >> 8);
	data[3] = (u8) (val & 0xff);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 4;
	msg.buf = data;

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			return 0;
		retry++;
		pr_err("%s: i2c transfer failed, retrying %x %x %d\n",
				__func__, addr, val, err);
		msleep(3);
	} while (retry <= SENSOR_MAX_RETRIES);

	return err;
}

static int s5k5cag_reg_poll16(struct i2c_client *client, u16 write_reg, u16 write_val, u16 read_reg,
                              u16 mask, u16 val, int delay, int timeout)
{
	    u16 currval = 0;
			while (timeout) 
			{
	
					s5k5cag_write_reg(client, write_reg, write_val);
	        s5k5cag_read_reg(client, read_reg, &currval);
	        
	
	        if ((currval & mask) == val)
	                return 0;
	
	        msleep(delay);
	        timeout--;
			}
	
	        pr_err("[err](%s)write_reg=0x%x, write_val=0x%x, read_reg=0x%x, currval=0x%x \n",__FUNCTION__,
	        write_reg, write_val, read_reg,currval);
	
	return -ETIMEDOUT;

}

static int s5k5cag_write_table(struct i2c_client *client,
		const struct s5k5cag_reg table[],
		const struct s5k5cag_reg override_list[],
		int num_override_regs)
{
	int err;
	const struct s5k5cag_reg *next;
	int i;
	u16 val;

	for (next = table; next->addr != SENSOR_TABLE_END; next++) {
		if (next->addr == SENSOR_WAIT_MS) {
			msleep(next->val);
			continue;
		}

		val = next->val;

		/* When an override list is passed in, replace the reg */
		/* value to write if the reg is in the list            */
		if (override_list) {
			for (i = 0; i < num_override_regs; i++) {
				if (next->addr == override_list[i].addr) {
					val = override_list[i].val;
					break;
				}
			}
		}

		err = s5k5cag_write_reg(client, next->addr, val);
		if (err)
			return err;
	}
	return 0;
}


static int s5k5cag_set_mode(struct s5k5cag_info *info, struct sensor_mode *mode)
{
	
	s5k5cag_write_reg(info->i2c_client, 0xFCFC, 0xD000);
	s5k5cag_write_reg(info->i2c_client, 0x0028, 0x7000);

	if (mode->xres < 1024 && mode->yres < 768) //context_A /* 1024*768*/
	{

		if(mode->xres == 640 && mode->yres == 480)
		{
	printk("[s5k5cag][context_A:1] %s: xres=%u, yres=%u, \n",
			__func__, mode->xres, mode->yres);
			s5k5cag_write_reg(info->i2c_client, 0x002A,0x023C);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0000);	// #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
			s5k5cag_write_reg(info->i2c_client, 0x002A,0x0240);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);	//_TC_GP_PrevOpenAfterChange
			s5k5cag_write_reg(info->i2c_client, 0x002A,0x0230);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);  	// TC_GP_NewConfigSync // Update preview configuration
			s5k5cag_write_reg(info->i2c_client, 0x002A,0x023E);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);	// #REG_TC_GP_PrevConfigChanged
			s5k5cag_write_reg(info->i2c_client, 0x002A,0x0220);
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);	// #REG_TC_GP_EnablePreview // Start preview
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);	// #REG_TC_GP_EnablePreviewChanged
		}
		else
		{
	printk("[s5k5cag][context_A:2] %s: xres=%u, yres=%u, \n",
			__func__, mode->xres, mode->yres);
			
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x026C);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->xres);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->yres);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0005);	//#REG_0TC_PCFG_Format            0270
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x3AA8);	//#REG_0TC_PCFG_usMaxOut4KHzRate  0272
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x3A88);	//#REG_0TC_PCFG_usMinOut4KHzRate  0274
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0100);	//#REG_0TC_PCFG_OutClkPerPix88    0276
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0800);	//#REG_0TC_PCFG_uMaxBpp88         027
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0052);	//#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x027E);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_usJpegPacketSize
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_usJpegTotalPackets
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uClockInd
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_usFrTimeType
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//#REG_0TC_PCFG_FrRateQualityType
			
//			if(work_mode == NIGHT_MODE)
//				s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0535);	//#REG_0TC_PCFG_usMaxFrTimeMsecMult10
//			else
				s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x014D);	//#REG_0TC_PCFG_usMaxFrTimeMsecMult10

			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x014d);	//#REG_0TC_PCFG_usMinFrTimeMsecMult10
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_bSmearOutput
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_sSaturation
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_sSharpBlur
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_sColorTemp
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uDeviceGammaIndex
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uPrevMirror
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uCaptureMirror
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uRotation		

			s5k5cag_write_reg(info->i2c_client, 0x002A,0x023C);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0000);	// #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
			s5k5cag_write_reg(info->i2c_client, 0x002A,0x0240);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);	//_TC_GP_PrevOpenAfterChange
			s5k5cag_write_reg(info->i2c_client, 0x002A,0x0230);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);  	// TC_GP_NewConfigSync // Update preview configuration
			s5k5cag_write_reg(info->i2c_client, 0x002A,0x023E);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);	// #REG_TC_GP_PrevConfigChanged
          
			//s5k5cag_write_reg(info->i2c_client, 0xB0A0, 0x0000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,0x0220);
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);	// #REG_TC_GP_EnablePreview // Start preview
			s5k5cag_write_reg(info->i2c_client, 0x0F12,0x0001);	// #REG_TC_GP_EnablePreviewChanged
		}

	}
	else	//context_B
	{
		if((mode->xres==1920&&mode->yres==1080)||
			 (mode->xres==1280&&mode->yres==720)||
			 (mode->xres==1280&&mode->yres==1024)||
			 (mode->xres==1600&&mode->yres==1200)||
			 (mode->xres==2048&&mode->yres==1536))
		{
			u32 config_no;
			if(mode->xres==1920&&mode->yres==1080)
				config_no = 0;
			else if(mode->xres==1280&&mode->yres==720)
				config_no = 1;
			else if(mode->xres==1280&&mode->yres==1024)
				config_no = 2;
			else if(mode->xres==1600&&mode->yres==1200)
				config_no = 3;
			else if(mode->xres==2048&&mode->yres==1536)
				config_no = 4;
			
	printk("[s5k5cag][context_B:1] %s: xres=%u, yres=%u,config_no=%d \n",
			__func__, mode->xres, mode->yres,config_no);
			
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0244);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, config_no);
		  s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0240);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		  s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0230);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		  s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0246);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		  s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0224);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		}
		else
		{
	printk("[s5k5cag][context_B:2] %s: xres=%u, yres=%u \n",
			__func__, mode->xres, mode->yres);
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x035C);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->xres);
		  s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->yres);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0005);	//#REG_0TC_CCFG_Format//5:YUV9:JPEG              
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x3AA8);	//#REG_0TC_CCFG_usMaxOut4KHzRate                 
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x3A88);	//#REG_0TC_CCFG_usMinOut4KHzRate                 
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0100);	//#REG_0TC_CCFG_OutClkPerPix88                   
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0800);	//#REG_0TC_CCFG_uMaxBpp88                        
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0052);	//#REG_0TC_CCFG_PVIMask                          
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0050);	//#REG_0TC_CCFG_OIFMask                          
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0600);	//#REG_0TC_CCFG_usJpegPacketSize                 
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0400);	//08FC	//#REG_0TC_CCFG_usJpegTotalPackets     
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_uClockInd                      
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_usFrTimeType                   
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0002);	//#REG_0TC_CCFG_FrRateQualityType              
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0534);	//#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps 
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x029a);	//#REG_0TC_CCFG_usMinFrTimeMsecMult10 //13.5fps
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_bSmearOutput                   
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_sSaturation                    
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_sSharpBlur                     
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_sColorTemp                     
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_uDeviceGammaIndex

			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0244); 	//Normal capture// 
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000); 	//config 0 // REG_TC_GP_ActiveCapConfig    0/1 
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0240); 
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//_TC_GP_PrevOpenAfterChange
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0230);                    
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//REG_TC_GP_NewConfigSync          
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0246);                    
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);   //Synchronize FW with new capture configuration
       
			//s5k5cag_write_reg(info->i2c_client, 0xB0A0, 0x0000);
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0224);                    
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// REG_TC_GP_EnableCapture         
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);    // REG_TC_GP_EnableCaptureChanged
		}
		
	}

	s5k5cag_write_reg(info->i2c_client, 0x0028, 0xD000);
	s5k5cag_write_reg(info->i2c_client, 0x002A, 0x1000);
	s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
	msleep(100);
	
	return 0;
}

#if 0
static int s5k5cag_set_preview(struct s5k5cag_info *info, struct sensor_mode *mode)
{
		u16 x,y,w,h;
		
		s5k5cag_write_reg(info->i2c_client, 0xFCFC, 0xD000);
		s5k5cag_write_reg(info->i2c_client, 0x0028, 0x7000);  				  	

		//+++
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x035C);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->xres);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->yres);
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0246);                    
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);  //Synchronize FW with new capture configuration
		//---
		
		printk("[s5k5cag](%s): xres=%u, yres=%u, \n",
				__func__, mode->xres, mode->yres);
		
		if((2048*mode->yres)>(1536*mode->xres)) //change w
		{
				h = 1536;
				y = 0;
				w = ((h*mode->xres)/mode->yres);if(w%2)w++;
				x = (2048 - w)>>1;
		}
		else	//change h
		{
				w = 2048;
				x = 0;
				h = ((w*mode->yres)/mode->xres);if(h%2)h++;
				y = (1536 - h)>>1;
		}						
		//printk("[s5k5cag](%s)w=0x%x, h=0x%x, x=0x%x, y=0x%x \n",__FUNCTION__,w,h,x,y);	
		
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0232);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, w);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, h);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, x);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, y);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		msleep(100);
		
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x026C);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->xres);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->yres);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0005);	//#REG_0TC_PCFG_Format            0270
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x3AA8);	//#REG_0TC_PCFG_usMaxOut4KHzRate  0272
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x3A88);	//#REG_0TC_PCFG_usMinOut4KHzRate  0274
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0100);	//#REG_0TC_PCFG_OutClkPerPix88    0276
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0800);	//#REG_0TC_PCFG_uMaxBpp88         027
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0052);	//#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
		
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x027E);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_usJpegPacketSize
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_usJpegTotalPackets
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uClockInd
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_usFrTimeType
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_FrRateQualityType

		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0534);	//#REG_0TC_PCFG_usMaxFrTimeMsecMult10
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x029A);	//#REG_0TC_PCFG_usMinFrTimeMsecMult10
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_bSmearOutput
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_sSaturation
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_sSharpBlur
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_sColorTemp
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uDeviceGammaIndex
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uPrevMirror
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uCaptureMirror
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_PCFG_uRotatiom

//		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x023C);
//		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	// #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0240);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//_TC_GP_PrevOpenAfterChange
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0230);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);  // TC_GP_NewConfigSync // Update preview configuration
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x023E);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_PrevConfigChanged

		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0220);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_EnablePreview // Start preview
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_EnablePreviewChanged

		s5k5cag_write_reg(info->i2c_client, 0x0028, 0xD000);
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x1000);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		msleep(100);

		return 0;
}

static int s5k5cag_set_capture(struct s5k5cag_info *info, struct sensor_mode *mode)
{
		u16 x,y,w,h;

		s5k5cag_write_reg(info->i2c_client, 0xFCFC, 0xD000);
		s5k5cag_write_reg(info->i2c_client, 0x0028, 0x7000);

		//+++
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x026C);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->xres);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->yres);
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x023E);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_PrevConfigChanged
		//---

		printk("[s5k5cag] (%s): xres=%u, yres=%u, \n",
				__func__, mode->xres, mode->yres);

		if((2048*mode->yres)>(1536*mode->xres)) //change w
		{
				h = 1536;
				y = 0;
				w = ((h*mode->xres)/mode->yres);if(w%2)w++;
				x = (2048 - w)>>1;
		}
		else	//change h
		{
				w = 2048;
				x = 0;
				h = ((w*mode->yres)/mode->xres);if(h%2)h++;
				y = (1536 - h)>>1;
		}						
		//printk("[s5k5cag](%s)w=0x%x, h=0x%x, x=0x%x, y=0x%x\n",__FUNCTION__,w,h,x,y);		
		
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0232);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, w);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, h);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, x);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, y);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		msleep(100);
		
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x035C);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->xres);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, mode->yres);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0005);	//#REG_0TC_CCFG_Format//5:YUV9:JPEG              
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x3AA8);	//#REG_0TC_CCFG_usMaxOut4KHzRate                 
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x3A88);	//#REG_0TC_CCFG_usMinOut4KHzRate                 
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0100);	//#REG_0TC_CCFG_OutClkPerPix88                   
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0800);	//#REG_0TC_CCFG_uMaxBpp88                        
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0052);	//#REG_0TC_CCFG_PVIMask     
                    
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0050);	//#REG_0TC_CCFG_OIFMask                          
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0600);	//#REG_0TC_CCFG_usJpegPacketSize                 
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0400);	//#REG_0TC_CCFG_usJpegTotalPackets     
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_uClockInd                      
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_usFrTimeType                   
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_FrRateQualityType  
            
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0534);	//#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps 
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x029a);	//#REG_0TC_CCFG_usMinFrTimeMsecMult10 //13.5fps
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_bSmearOutput                   
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_sSaturation                    
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_sSharpBlur                     
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_sColorTemp                     
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);	//#REG_0TC_CCFG_uDeviceGammaIndex

//		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0244);  //Normal capture// 
//		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0000);  //config 0 // REG_TC_GP_ActiveCapConfig    0/1 
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0240); 
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//_TC_GP_PrevOpenAfterChange
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0230);                    
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//REG_TC_GP_NewConfigSync          
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0246);                    
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);  //Synchronize FW with new capture configuration
     
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0224);                    
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// REG_TC_GP_EnableCapture         
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);  // REG_TC_GP_EnableCaptureChanged
  
		s5k5cag_write_reg(info->i2c_client, 0x0028, 0xD000);
		s5k5cag_write_reg(info->i2c_client, 0x002A, 0x1000);
		s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
		msleep(100);		
  	
		return 0;
}
#else
static int s5k5cag_set_preview(struct s5k5cag_info *info, struct sensor_mode *mode)
{
	
			u16 x=0,y=0,w=0,h=0;
			u16 config=0;
			u16 ratio=0;
			
			s5k5cag_write_reg(info->i2c_client, 0xFCFC, 0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x0028, 0x7000);  				  	

			
			if(mode->xres==2048&&mode->yres==1536)
			{
				config = 0;
				x = 0;
				y = 0;
				w = 2048;
				h = 1536;
				ratio = Ratio_4_3;
			}
			else if(mode->xres==1920&&mode->yres==1080)
			{
				config = 1;
				x = 0;
				y = 192;
				w = 2048;
				h = 1152;
				ratio = Ratio_16_9;
			}
			else if(mode->xres==1280&&mode->yres==720)
			{
				config = 2;
				x = 0;
				y = 192;
				w = 2048;
				h = 1152;
				ratio = Ratio_16_9;
			}
			else if(mode->xres==704&&mode->yres==576)
			{
				config = 3;
				x = 0;
				y = 0;
				w = 2048;
				h = 1536;
				ratio = Ratio_4_3;
			}
			else if(mode->xres==640&&mode->yres==480)
			{
				config = 4;
				x = 0;
				y = 0;
				w = 2048;
				h = 1536;
				ratio = Ratio_4_3;
			}
			
			printk("###############################[s5k5cag](%s): xres=%u, yres=%u, config=%d, ratio=%d, last_ratio=%d \n",
					__func__, mode->xres, mode->yres,config,ratio,info->last_ratio);

			if((ratio != info->last_ratio)||openflage4ratio==1)	//&*&*&*CJ1_add: fixed remind bug 1796&1769 for ratio issue.
			{			
					printk("[s5k5cag](%s)(%d): set Ratio \n",__func__,__LINE__);
					
					s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0244);  //Normal capture// 
					s5k5cag_write_reg(info->i2c_client, 0x0F12, config);  //config 0 // REG_TC_GP_ActiveCapConfig    0/1 
					s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0246);                    
					s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);  //Synchronize FW with new capture configuration		
					
					s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0232);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, w);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, h);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, x);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, y);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
/*
					s5k5cag_write_reg(info->i2c_client, 0xFCFC, 0xD000);
					s5k5cag_write_reg(info->i2c_client, 0x002C, 0x7000);
					s5k5cag_reg_poll16(info->i2c_client, 0x002E, 0x0242, 0x0F12, 0xFFFF, 0, 20, 10);
					s5k5cag_reg_poll16(info->i2c_client, 0x002E, 0x0248, 0x0F12, 0xFFFF, 0, 20, 10);
					s5k5cag_reg_poll16(info->i2c_client, 0x002A, 0x0242, 0x0F12, 0xFFFF, 0, 20, 50);
					s5k5cag_reg_poll16(info->i2c_client, 0x002A, 0x0248, 0x0F12, 0xFFFF, 0, 20, 50);
*/
			msleep(100);
				openflage4ratio = 0;	//&*&*&*CJ1_add: fixed remind bug 1796&1769 for ratio issue.
			}
			info->last_ratio = ratio;
											    	
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x023C);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12, config);	// #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0240);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//_TC_GP_PrevOpenAfterChange
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0230);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);  // TC_GP_NewConfigSync // Update preview configuration
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x023E);        
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_PrevConfigChanged        
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0220);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_EnablePreview // Start preview
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_EnablePreviewChanged
			    	
			s5k5cag_write_reg(info->i2c_client, 0x0028, 0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x1000);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
/*
			s5k5cag_write_reg(info->i2c_client, 0x0028,	0x7000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,	0x0224);
			s5k5cag_write_reg(info->i2c_client, 0x0f12,	0x0000);
			
			s5k5cag_write_reg(info->i2c_client, 0x0028,	0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,	0xB0A0);
			s5k5cag_write_reg(info->i2c_client, 0x0F12,	0x0000);
			
			s5k5cag_write_reg(info->i2c_client, 0x0028,	0x7000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,	0x0226);
			s5k5cag_write_reg(info->i2c_client, 0x0f12,	0x0001);

			s5k5cag_write_reg(info->i2c_client, 0xFCFC, 0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x002C, 0x7000);
			s5k5cag_reg_poll16(info->i2c_client, 0x002E, 0x0242, 0x0F12, 0xFFFF, 0, 20, 10);
			s5k5cag_reg_poll16(info->i2c_client, 0x002E, 0x0248, 0x0F12, 0xFFFF, 0, 20, 10);
			s5k5cag_reg_poll16(info->i2c_client, 0x002A, 0x0242, 0x0F12, 0xFFFF, 0, 20, 50);
			s5k5cag_reg_poll16(info->i2c_client, 0x002A, 0x0248, 0x0F12, 0xFFFF, 0, 20, 50);
			msleep(1);	
*/			  	
			msleep(100);	
			return 0;
}

static int s5k5cag_set_capture(struct s5k5cag_info *info, struct sensor_mode *mode)
{
			u16 x=0,y=0,w=0,h=0;
			u16 config=0;
			u16 ratio=0;
		
			s5k5cag_write_reg(info->i2c_client, 0xFCFC, 0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x0028, 0x7000);  	


			if(mode->xres==2048&&mode->yres==1536)
			{
				config = 0;
				x = 0;
				y = 0;
				w = 2048;
				h = 1536;
				ratio = Ratio_4_3;
			}
			else if(mode->xres==1920&&mode->yres==1080)
			{
				config = 1;
				x = 0;
				y = 192;
				w = 2048;
				h = 1152;
				ratio = Ratio_16_9;
			}
			else if(mode->xres==1280&&mode->yres==720)
			{
				config = 2;
				x = 0;
				y = 192;
				w = 2048;
				h = 1152;
				ratio = Ratio_16_9;
			}
			else if(mode->xres==704&&mode->yres==576)
			{
				config = 3;
				x = 0;
				y = 0;
				w = 2048;
				h = 1536;
				ratio = Ratio_4_3;
			}
			else if(mode->xres==640&&mode->yres==480)
			{
				config = 4;
				x = 0;
				y = 0;
				w = 2048;
				h = 1536;
				ratio = Ratio_4_3;
			}

			printk("###############################[s5k5cag](%s): xres=%u, yres=%u, config=%d, ratio=%d, last_ratio=%d \n",
					__func__, mode->xres, mode->yres,config,ratio,info->last_ratio);


			if(ratio != info->last_ratio)
			{
					printk("[s5k5cag](%s)(%d): set Ratio \n",__func__,__LINE__);

					s5k5cag_write_reg(info->i2c_client, 0x002A, 0x023C);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, config);	// #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
					s5k5cag_write_reg(info->i2c_client, 0x002A, 0x023E);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_PrevConfigChanged

					s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0232);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, w);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, h);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, x);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, y);
					s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
/*
					s5k5cag_write_reg(info->i2c_client, 0xFCFC, 0xD000);
					s5k5cag_write_reg(info->i2c_client, 0x002C, 0x7000);
					s5k5cag_reg_poll16(info->i2c_client, 0x002E, 0x0242, 0x0F12, 0xFFFF, 0, 20, 10);
					s5k5cag_reg_poll16(info->i2c_client, 0x002E, 0x0248, 0x0F12, 0xFFFF, 0, 20, 10);
					s5k5cag_reg_poll16(info->i2c_client, 0x002A, 0x0242, 0x0F12, 0xFFFF, 0, 20, 50);
					s5k5cag_reg_poll16(info->i2c_client, 0x002A, 0x0248, 0x0F12, 0xFFFF, 0, 20, 50);
*/
					msleep(100);			
			}
			info->last_ratio = ratio;
    	
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0244);  //Normal capture// 
			s5k5cag_write_reg(info->i2c_client, 0x0F12, config);  //config 0 // REG_TC_GP_ActiveCapConfig    0/1 
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0240); 
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//_TC_GP_PrevOpenAfterChange
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0230);                    
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//REG_TC_GP_NewConfigSync          
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0246);                    
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);  //Synchronize FW with new capture configuration    	 
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0224);                    
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// REG_TC_GP_EnableCapture         
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);  // REG_TC_GP_EnableCaptureChanged
    	
			s5k5cag_write_reg(info->i2c_client, 0x0028, 0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x1000);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
/*
			s5k5cag_write_reg(info->i2c_client, 0x0028,	0x7000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,	0x0224);
			s5k5cag_write_reg(info->i2c_client, 0x0f12,	0x0001);
			
			s5k5cag_write_reg(info->i2c_client, 0x0028,	0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,	0xB0A0);
			s5k5cag_write_reg(info->i2c_client, 0x0F12,	0x0000);
			
			s5k5cag_write_reg(info->i2c_client, 0x0028,	0x7000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,	0x0226);
			s5k5cag_write_reg(info->i2c_client, 0x0f12,	0x0001);
			
			s5k5cag_write_reg(info->i2c_client, 0xFCFC, 0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x002C, 0x7000);
			s5k5cag_reg_poll16(info->i2c_client, 0x002E, 0x0242, 0x0F12, 0xFFFF, 0, 20, 10);
			s5k5cag_reg_poll16(info->i2c_client, 0x002E, 0x0248, 0x0F12, 0xFFFF, 0, 20, 10);
			s5k5cag_reg_poll16(info->i2c_client, 0x002A, 0x0242, 0x0F12, 0xFFFF, 0, 20, 50);
			s5k5cag_reg_poll16(info->i2c_client, 0x002A, 0x0248, 0x0F12, 0xFFFF, 0, 20, 50);
			msleep(1);
*/
			msleep(100);		
			
			return 0;
}
#endif

#define INFO_chipId1 		0x0040	//0x00000040
#define INFO_chipVer 		0x0042	//0x00000042
#define INFO_SVNversion 0x0048	//0x00000048
#define INFO_Date 			0x004E	//0x0000004E
#define INFO_chipId1_value 		0x05CA
#define INFO_chipVer_value 		0x00B0
#define INFO_SVNversion_value 0x7B83
#define INFO_Date_vale 				0x971d

static long s5k5cag_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
	int err=0;
	struct s5k5cag_info *info = file->private_data;
	int poll_timeout;

	if(init_check_s5k5cag)
	{
			poll_timeout = s5k5cag_reg_poll16(info->i2c_client, 0xFCFC, 0x0000, INFO_chipId1,		0xFFFF, INFO_chipId1_value, 5, 10);
			if(poll_timeout)
			{
				printk("[s5k5cag]poll_timeout1-->power off/on angin,poll_timeout=%d \n",poll_timeout);
				s5k5cag_sensor_info->pdata->power_off();
				msleep(20);
				s5k5cag_sensor_info->pdata->power_on(40);
				msleep(100);
			}
			s5k5cag_reg_poll16(info->i2c_client, 0xFCFC, 0x0000, INFO_chipId1,		0xFFFF, INFO_chipId1_value, 5, 10);
			s5k5cag_reg_poll16(info->i2c_client, 0xFCFC, 0x0000, INFO_chipVer, 		0xFFFF, INFO_chipVer_value, 5, 10);

			s5k5cag_write_table(s5k5cag_sensor_info->i2c_client, initial_list, NULL, 0);
			printk("[s5k5cag]init_check_s5k5cag done \n");
			init_check_s5k5cag = 0;

			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x023C);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0004);	// #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0240);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	//_TC_GP_PrevOpenAfterChange
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0230);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);  // TC_GP_NewConfigSync // Update preview configuration
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x023E);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_PrevConfigChanged
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x0220);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_EnablePreview // Start preview
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);	// #REG_TC_GP_EnablePreviewChanged

			s5k5cag_write_reg(info->i2c_client, 0x0028, 0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x002A, 0x1000);
			s5k5cag_write_reg(info->i2c_client, 0x0F12, 0x0001);
			//msleep(100);

			s5k5cag_write_reg(info->i2c_client, 0x0028,	0x7000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,	0x0224);
			s5k5cag_write_reg(info->i2c_client, 0x0f12,	0x0000);

			s5k5cag_write_reg(info->i2c_client, 0x0028,	0xD000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,	0xB0A0);
			s5k5cag_write_reg(info->i2c_client, 0x0F12,	0x0000);

			s5k5cag_write_reg(info->i2c_client, 0x0028,	0x7000);
			s5k5cag_write_reg(info->i2c_client, 0x002A,	0x0226);
			s5k5cag_write_reg(info->i2c_client, 0x0f12,	0x0001);

			s5k5cag_write_table(info->i2c_client, colorfx_none, NULL, 0);
			s5k5cag_write_table(info->i2c_client, wb_auto_init, NULL, 0);
			s5k5cag_write_table(info->i2c_client, scene_nightoff, NULL, 0);
			s5k5cag_write_table(info->i2c_client, sport_off, NULL, 0);
			s5k5cag_write_table(info->i2c_client, scene_normal, NULL, 0);
	    s5k5cag_write_table(info->i2c_client, Exposure_0, NULL, 0);
	    s5k5cag_write_table(info->i2c_client, flicker_off, NULL, 0);
	}

	pr_info("s5k5cag %s:cmd=0x%x \n",__func__,cmd);
	switch (cmd) {
		case SENSOR_IOCTL_SET_MODE:
			{
				struct sensor_mode mode;
				if (copy_from_user(&mode,
							(const void __user *)arg,
							sizeof(struct sensor_mode))) {
					return -EFAULT;
				}

				return s5k5cag_set_mode(info, &mode);
			}
		case SENSOR_IOCTL_SET_CAPTURE:
			{
				struct sensor_mode mode;
				if (copy_from_user(&mode,
							(const void __user *)arg,
							sizeof(struct sensor_mode))) {
					return -EFAULT;
				}

				return s5k5cag_set_capture(info, &mode);
			}
		case SENSOR_IOCTL_SET_PREVIEW:
			{
				struct sensor_mode mode;
				if (copy_from_user(&mode,
							(const void __user *)arg,
							sizeof(struct sensor_mode))) {
					return -EFAULT;
				}

				return s5k5cag_set_preview(info, &mode);
			}
		case SENSOR_IOCTL_GET_STATUS:
			{
				return 0;
			}
#if 1
		case SENSOR_IOCTL_GET_AF_STATUS:
			{
				return 0;
			}
		case SENSOR_IOCTL_SET_AF_MODE:
			{
				return 0;
			}
		case SENSOR_IOCTL_SET_COLOR_EFFECT:
			{
				u8 coloreffect;

				if (copy_from_user(&coloreffect,
							(const void __user *)arg,
							sizeof(coloreffect))) {
					return -EFAULT;
				}

				switch(coloreffect)
				{
					case YUV_ColorEffect_None:
						printk("Color Effect None\n");
						//err = s5k5cag_write_table(info->i2c_client, ColorEffect_None, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, colorfx_none, NULL, 0);
						break;
					case YUV_ColorEffect_Mono:
						printk("Color Effect Mono\n");
						err = s5k5cag_write_table(info->i2c_client, ColorEffect_Mono, NULL, 0);
						break;
					case YUV_ColorEffect_Sepia:
						printk("Color Effect Sepia\n");
						//err = s5k5cag_write_table(info->i2c_client, ColorEffect_Sepia, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, colorfx_sepia, NULL, 0);
						break;
					case YUV_ColorEffect_Negative:
						printk("Color Effect Negative\n");
						//err = s5k5cag_write_table(info->i2c_client, ColorEffect_Negative, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, colorfx_negative, NULL, 0);
						break;
					case YUV_ColorEffect_Solarize:
						printk("Color Effect Solarize\n");
						err = s5k5cag_write_table(info->i2c_client, ColorEffect_Solarize, NULL, 0);
						break;
					case YUV_ColorEffect_Posterize:
						printk("Color Effect Posterize\n");
						err = s5k5cag_write_table(info->i2c_client, ColorEffect_Posterize, NULL, 0);
						break;
					default:
						break;
				}

				if (err)
					return err;

				return 0;
			}
		case SENSOR_IOCTL_SET_WHITE_BALANCE:
			{
				u8 whitebalance;

				if (copy_from_user(&whitebalance,
							(const void __user *)arg,
							sizeof(whitebalance))) {
					return -EFAULT;
				}

				switch(whitebalance)
				{
					case YUV_Whitebalance_Auto:
						printk("enter auto white balance \n");
						//err = s5k5cag_write_table(info->i2c_client, Whitebalance_Auto, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, wb_auto, NULL, 0);
						break;
					case YUV_Whitebalance_Incandescent:
						printk("enter Incandescent white balance \n");
						//err = s5k5cag_write_table(info->i2c_client, Whitebalance_Incandescent, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, wb_incandescent, NULL, 0);
						break;
					case YUV_Whitebalance_Daylight:
						printk("enter Daylight white balance \n");
						//err = s5k5cag_write_table(info->i2c_client, Whitebalance_Daylight, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, wb_daylight, NULL, 0);
						break;
					case YUV_Whitebalance_Fluorescent:
						printk("enter Fluorescent white balance \n");
						//err = s5k5cag_write_table(info->i2c_client, Whitebalance_Fluorescent, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, wb_fluorescent, NULL, 0);
						break;
					case YUV_Whitebalance_CloudyDaylight:
						printk("enter CloudyDaylight white balance \n");
						//err = s5k5cag_write_table(info->i2c_client, Whitebalance_CloudyDaylight, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, wb_cloudy, NULL, 0);
						break;
					default:
						break;
				}

				if (err)
					return err;

				return 0;
			}
		case SENSOR_IOCTL_SET_SCENE_MODE:
			{
				u8 scene_mode;

				printk("SENSOR_IOCTL_SET_SCENE_MODE \n");

				if (copy_from_user(&scene_mode,
							(const void __user *)arg,
							sizeof(scene_mode))) {
					return -EFAULT;
				}

				switch(scene_mode)
				{
					case YUV_SceneMode_Auto:
						printk("enter scene mode auto \n");
						err = s5k5cag_write_table(info->i2c_client, scene_nightoff, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, sport_off, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, scene_normal, NULL, 0);
						break;
					case YUV_SceneMode_Action:
						printk("enter scene mode active \n");
						err = s5k5cag_write_table(info->i2c_client, scene_nightoff, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, sport, NULL, 0);
						break;
					case YUV_SceneMode_Portrait:
						printk("enter scene mode portrait \n");
						err = s5k5cag_write_table(info->i2c_client, scene_nightoff, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, sport_off, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, scene_portrait, NULL, 0);
						break;
					case YUV_SceneMode_Landscape:
						printk("enter scene mode landscape \n");
						err = s5k5cag_write_table(info->i2c_client, scene_nightoff, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, sport_off, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, scene_landscape, NULL, 0);
						break;
					case YUV_SceneMode_Sunset:
						printk("enter scene mode sunset \n");
						err = s5k5cag_write_table(info->i2c_client, scene_nightoff, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, sport_off, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, scene_sunset, NULL, 0);
						break;
					case YUV_SceneMode_Night:
						printk("enter scene mode night \n");
						err = s5k5cag_write_table(info->i2c_client, sport_off, NULL, 0);
						err = s5k5cag_write_table(info->i2c_client, scene_night, NULL, 0);
						break;
					default:
						break;
				}

				if (err)
					return err;

				return 0;
			}
#endif
        case SENSOR_IOCTL_SET_EXPOSURE:
        {
                int exposure;

		if (copy_from_user(&exposure,
				   (const void __user *)arg,
				   sizeof(exposure))) {
			return -EFAULT;
		}

                switch(exposure)
                {
                    case YUV_Exposure_0:
	                 err = s5k5cag_write_table(info->i2c_client, Exposure_0, NULL, 0);
                         break;
                    case YUV_Exposure_1:
	                 err = s5k5cag_write_table(info->i2c_client, Exposure_1, NULL, 0);
                         break;
                    case YUV_Exposure_2:
	                 err = s5k5cag_write_table(info->i2c_client, Exposure_2, NULL, 0);
                         break;
                    case YUV_Exposure_Negative_1:
	                 err = s5k5cag_write_table(info->i2c_client, Exposure_Negative_1, NULL, 0);
                         break;
                    case YUV_Exposure_Negative_2:
	                 err = s5k5cag_write_table(info->i2c_client, Exposure_Negative_2, NULL, 0);
                         break;
                    default:
                         break;
                }
	        if (err)
		   return err;

                return 0;
        }

		default:
			return -EINVAL;
	}

	return 0;
}


static int s5k5cag_open(struct inode *inode, struct file *file)
{
	pr_info("s5k5cag %s \n",__func__);	

	file->private_data = s5k5cag_sensor_info;
	
	openflage4ratio = 1;	//&*&*&*CJ1_add: fixed remind bug 1796&1769 for ratio issue.
//	s5k5cag_sensor_info->last_ratio = Ratio_4_3;	//&*&*&*CJ_mask: fixed buglliza Bug 34570 some time the screen will be disorderly
	
	if (s5k5cag_sensor_info->pdata && s5k5cag_sensor_info->pdata->power_on)
	{
		s5k5cag_sensor_info->pdata->power_on(20);	
		//init_check_s5k5cag = 1;
	}

	
	return 0;
}

int s5k5cag_release(struct inode *inode, struct file *file)
{
	pr_info("s5k5cag %s \n",__func__);
	
	if (s5k5cag_sensor_info->pdata && s5k5cag_sensor_info->pdata->power_off)
		s5k5cag_sensor_info->pdata->power_off();
	file->private_data = NULL;
	return 0;
}


static int s5k5cag_suspend(struct i2c_client *client,pm_message_t state)
{
	
	s5k5cag_sensor_info->pdata->suspend();
	pr_info("s5k5cag %s \n",__func__);	
	
	return 0;
}

static int s5k5cag_resume(struct i2c_client *client)
{
		
	s5k5cag_sensor_info->pdata->init();
	pr_info("s5k5cag %s \n",__func__);
	
	msleep(20);
	
	s5k5cag_sensor_info->pdata->power_on(0);	
	
	init_check_s5k5cag = 1;
	
	return 0;
}
static const struct file_operations s5k5cag_fileops = {
	.owner = THIS_MODULE,
	.open = s5k5cag_open,
	.unlocked_ioctl = s5k5cag_ioctl,
	.release = s5k5cag_release,
};

static struct miscdevice s5k5cag_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "s5k5cag",
	.fops = &s5k5cag_fileops,
};

static int s5k5cag_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err;
	
	

	pr_info("s5k5cag: probing sensor.\n");

	s5k5cag_sensor_info = kzalloc(sizeof(struct s5k5cag_info), GFP_KERNEL);
	if (!s5k5cag_sensor_info) {
		pr_err("s5k5cag: Unable to allocate memory!\n");
		return -ENOMEM;
	}

	err = misc_register(&s5k5cag_device);
	if (err) {
		pr_err("s5k5cag: Unable to register misc device!\n");
		kfree(s5k5cag_sensor_info);
		return err;
	}
	s5k5cag_sensor_info->last_ratio = Ratio_4_3;
	s5k5cag_sensor_info->pdata = client->dev.platform_data;
	s5k5cag_sensor_info->i2c_client = client;

	i2c_set_clientdata(client, s5k5cag_sensor_info);
	
	return 0;
}

static int s5k5cag_remove(struct i2c_client *client)
{
	struct s5k5cag_info *info;
	info = i2c_get_clientdata(client);
	misc_deregister(&s5k5cag_device);
	kfree(info);
	return 0;
}

static const struct i2c_device_id s5k5cag_id[] = {
	{ "s5k5cag", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, s5k5cag_id);

static struct i2c_driver s5k5cag_i2c_driver = {
	.driver = {
		.name = "s5k5cag",
		.owner = THIS_MODULE,
	},
	.probe = s5k5cag_probe,
	.remove = s5k5cag_remove,
	.id_table = s5k5cag_id,
	.suspend = s5k5cag_suspend,
	.resume = s5k5cag_resume,
};

static int __init s5k5cag_init(void)
{
	pr_info("s5k5cag sensor driver loading\n");
	return i2c_add_driver(&s5k5cag_i2c_driver);
}

static void __exit s5k5cag_exit(void)
{
	i2c_del_driver(&s5k5cag_i2c_driver);
}

module_init(s5k5cag_init);
module_exit(s5k5cag_exit);

