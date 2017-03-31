/*
 * kernel/drivers/media/video/tegra
 *
 * Aptina MT9M114 sensor driver
 *
 * Copyright (C) 2012 NVIDIA Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
#include <media/yuv_sensor_cl2n.h>
#include "mt9m114-cl2n_regs.h"

#define SENSOR_NAME "mt9m114"

struct sensor_info {
	int mode;
	struct i2c_client *i2c_client;
	struct yuv_sensor_platform_data *pdata;
};

static struct sensor_info *info;
int init_check_mt9m114;

static int mt9m114_reg_write8(struct i2c_client *client, u16 reg, u8 val)
{
	int retry = 0;
	struct i2c_msg msg;
	struct {
	        u16 reg;
	        u8 val;
	} __packed buf;
	int err;

	buf.reg = swab16(reg);
	buf.val = val;

	msg.addr        = client->addr;
	msg.flags       = 0;
	msg.len         = 3;
	msg.buf         = (u8 *)&buf;

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			return 0;
		retry++;
		pr_err("%s: i2c transfer failed, retrying %x %x %d\n",
				__func__, reg, val, err);
		msleep(3);
	} while (retry <= SENSOR_MAX_RETRIES);

	return err;
}


static int mt9m114_reg_write16(struct i2c_client *client, u16 reg, u16 val)
{
	int retry = 0;
	struct i2c_msg msg;
	struct {
	        u16 reg;
	        u16 val;
	} __packed buf;
	int err;

	buf.reg = swab16(reg);
	buf.val = swab16(val);

	msg.addr        = client->addr;
	msg.flags       = 0;
	msg.len         = 4;
	msg.buf         = (u8 *)&buf;

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			return 0;
		retry++;
		pr_err("%s: i2c transfer failed, retrying %x %x %d\n",
				__func__, reg, val, err);
		msleep(3);
	} while (retry <= SENSOR_MAX_RETRIES);

	return err;
}

static int mt9m114_reg_write32(struct i2c_client *client, u16 reg, u32 val)
{
	int retry = 0;
	struct i2c_msg msg;
	struct {
	        u16 reg;
	        u32 val;
	} __packed buf;
	int err;

	buf.reg = swab16(reg);
	buf.val = swab32(val);

	msg.addr        = client->addr;
	msg.flags       = 0;
	msg.len         = 6;
	msg.buf         = (u8 *)&buf;

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			return 0;
		retry++;
		pr_err("%s: i2c transfer failed, retrying %x %x %d\n",
				__func__, reg, val, err);
		msleep(3);
	} while (retry <= SENSOR_MAX_RETRIES);

	return err;
}

static int mt9m114_reg_read16(struct i2c_client *client, u16 reg, u16 *val)
{
  int ret;
  u16 rval;
  struct i2c_msg msg[] = 
  {
  	{
  		.addr   = client->addr,
  		.flags  = 0,
  		.len    = 2,
  		.buf    = (u8 *)&reg,
  	},
  	{
  	 	.addr   = client->addr,
  	 	.flags  = I2C_M_RD,
  	 	.len    = 2,
  	 	.buf    = (u8 *)&rval,
  	},
  };

  reg = swab16(reg);

  ret = i2c_transfer(client->adapter, msg, 2);

  if (ret < 0) {
          pr_err("(%s)Failed reading register 0x%04x!\n",__FUNCTION__, reg);
          return ret;
  }

  *val = swab16(rval);

  return 0;
}

static int mt9m114_reg_poll16(struct i2c_client *client, u16 reg,
                              u16 mask, u16 val, int delay, int timeout)
{
	while (timeout) {
	        u16 currval=0;

	        mt9m114_reg_read16(client, reg, &currval);

	        if ((currval & mask) == val)
	                return 0;

	        msleep(delay);
	        timeout--;
	}

	pr_err("(%s):Failed polling register 0x%04x for 0x%04x\n",__FUNCTION__,reg, val);

	return -ETIMEDOUT;

}

static int mt9m114_reg_write_table(struct i2c_client *client,
                                   const struct sensor_reg table[])
{
	const struct sensor_reg *next;

	for (next = table; next->addr != SENSOR_TABLE_END; next++)
	{

		if (next->addr == SENSOR_WAIT_MS)
		{
			msleep(next->val);
			continue;
		}

		if (next->addr == POLL_COMMAND_REGISTER)
		{
			mt9m114_reg_poll16(client, COMMAND_REGISTER, next->val, next->type, poll_delay, poll_timeout);
			continue;
		}

		switch (next->type)
		{
					case REG_U8:
					        mt9m114_reg_write8(client, next->addr, (u8)next->val);
					        break;

					case REG_U16:
					        mt9m114_reg_write16(client, next->addr, (u16)next->val);
					        break;

					case REG_U32:
					        mt9m114_reg_write32(client, next->addr, (u32)next->val);
					        break;
					default:
					        BUG_ON(1);
		}

	}
	return 0;

}

//static int sensor_read_reg(struct i2c_client *client, u16 addr, u16 *val)
//{
//	int err=0;
//	struct i2c_msg msg[2];
//	unsigned char data[4];
//
//	if (!client->adapter)
//		return -ENODEV;
//
//	msg[0].addr = client->addr;
//	msg[0].flags = 0;
//	msg[0].len = 2;
//	msg[0].buf = data;
//
//	/* high byte goes out first */
//	data[0] = (u8) (addr >> 8);;
//	data[1] = (u8) (addr & 0xff);
//
//	msg[1].addr = client->addr;
//	msg[1].flags = I2C_M_RD;
//	msg[1].len = 2;
//	msg[1].buf = data + 2;
//
//	err = i2c_transfer(client->adapter, msg, 2);
//
//	if (err != 2)
//		return -EINVAL;
//
//        swap(*(data+2),*(data+3)); //swap high and low byte to match table format
//	memcpy(val, data+2, 2);
//
//	return 0;
//}
//
//static int sensor_write_reg(struct i2c_client *client, u16 addr, u16 val)
//{
//	int err;
//	struct i2c_msg msg;
//	unsigned char data[4];
//	int retry = 0;
//
//	if (!client->adapter)
//		return -ENODEV;
//
//	data[0] = (u8) (addr >> 8);
//	data[1] = (u8) (addr & 0xff);
//  data[2] = (u8) (val >> 8);
//	data[3] = (u8) (val & 0xff);
//
//	msg.addr = client->addr;
//	msg.flags = 0;
//	msg.len = 4;
//	msg.buf = data;
//
//
//	do {
//		err = i2c_transfer(client->adapter, &msg, 1);
//		if (err == 1)
//			return 0;
//		retry++;
//		pr_err("%s: i2c transfer failed, retrying %x %x %d\n",
//				__func__, addr, val, err);
//		msleep(3);
//	} while (retry <= SENSOR_MAX_RETRIES);
//
//	return err;
//}
//
//static int sensor_write_table(struct i2c_client *client,
//			      const struct sensor_reg table[])
//{
//	int err;
//	const struct sensor_reg *next;
//	u16 val;
//
//	for (next = table; next->addr != SENSOR_TABLE_END; next++) {
//		if (next->addr == SENSOR_WAIT_MS) {
//			msleep(next->val);
//			continue;
//		}
//
//		val = next->val;
//
//		err = sensor_write_reg(client, next->addr, val);
//		if (err)
//			return err;
//	}
//	return 0;
//}



static int sensor_set_mode(struct sensor_info *info, struct sensor_mode *mode)
{

	printk("[Jimmy][context] %s: xres=%u, yres=%u, \n",
			__func__, mode->xres, mode->yres);

	if (mode->xres == 1280 && mode->yres == 720)
	{
		mt9m114_reg_write_table(info->i2c_client, size_1280x720);
	}
	else if (mode->xres == 640 && mode->yres == 480)
	{
		mt9m114_reg_write_table(info->i2c_client, size_640x480);
	}
	else if (mode->xres == 1280 && mode->yres == 960)
	{
		mt9m114_reg_write_table(info->i2c_client, size_1280x960);
	}
	else if (mode->xres == 720 && mode->yres == 576)
	{
		mt9m114_reg_write_table(info->i2c_client, size_720x576);
	}

		mt9m114_reg_write_table(info->i2c_client, Change_Config);
		//mt9m114_reg_write_table(info->i2c_client, Refresh);

	return 0;
}

static long sensor_ioctl(struct file *file,
			 unsigned int cmd, unsigned long arg)
{
	struct sensor_info *info = file->private_data;
        int err=0;

	pr_info("yuv %s\n",__func__);
	if(init_check_mt9m114)
	{
		mt9m114_reg_write_table(info->i2c_client, initial_list);
		mt9m114_reg_write_table(info->i2c_client, size_1280x960);//&*&*&*CJ_20120608:add to improve CTS stability.
		printk("[Jimmy]init_check_mt9m114 done \n");
		init_check_mt9m114 = 0;
	}

	pr_info("[Jimmy]mt9m114 %s:cmd=0x%x \n",__func__,cmd);

	switch (cmd)
	{
			case SENSOR_IOCTL_SET_MODE:
			{
				struct sensor_mode mode;
				if (copy_from_user(&mode,
						   (const void __user *)arg,
						   sizeof(struct sensor_mode))) {
					return -EFAULT;
				}

				return sensor_set_mode(info, &mode);
			}
			case SENSOR_IOCTL_GET_STATUS:
			{
				return 0;
			}
			case SENSOR_IOCTL_SET_COLOR_EFFECT:
			{
				int coloreffect;

				if (copy_from_user(&coloreffect,(const void __user *)arg, sizeof(coloreffect)))
				{
					return -EFAULT;
				}

					switch(coloreffect)
					{
					    case YUV_ColorEffect_None:
					   				err = mt9m114_reg_write_table(info->i2c_client, ColorEffect_None);
					         break;
					    case YUV_ColorEffect_Mono:
					   				err = mt9m114_reg_write_table(info->i2c_client, ColorEffect_Mono);
					         break;
					    case YUV_ColorEffect_Sepia:
					   				err = mt9m114_reg_write_table(info->i2c_client, ColorEffect_Sepia);
					         break;
					    case YUV_ColorEffect_Negative:
					  				err = mt9m114_reg_write_table(info->i2c_client, ColorEffect_Negative);
					         break;
					    case YUV_ColorEffect_Solarize:
					   				err = mt9m114_reg_write_table(info->i2c_client, ColorEffect_Solarize);
					         break;
					    case YUV_ColorEffect_Posterize:
					   				err = mt9m114_reg_write_table(info->i2c_client, ColorEffect_Posterize);
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
				int whitebalance;

				if (copy_from_user(&whitebalance,
						   (const void __user *)arg,
						   sizeof(whitebalance))) {
					return -EFAULT;
				}

					switch(whitebalance)
					{
					    case YUV_Whitebalance_Auto:
					   				err = mt9m114_reg_write_table(info->i2c_client, Whitebalance_Auto);
					         break;
					    case YUV_Whitebalance_Incandescent:
					   				err = mt9m114_reg_write_table(info->i2c_client, Whitebalance_Incandescent);
					         break;
					    case YUV_Whitebalance_Daylight:
					   				err = mt9m114_reg_write_table(info->i2c_client, Whitebalance_Daylight);
					         break;
					    case YUV_Whitebalance_Fluorescent:
					   				err = mt9m114_reg_write_table(info->i2c_client, Whitebalance_Fluorescent);
					         break;
							case YUV_Whitebalance_CloudyDaylight:
						 				err = mt9m114_reg_write_table(info->i2c_client, wb_cloudy);
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
//&*&*&*CJ1_20120521: mod for mt9m114 scene mode
							case YUV_SceneMode_Auto:
										printk("enter scene mode auto \n");
										err = mt9m114_reg_write_table(info->i2c_client, scene_nightoff);
										err = mt9m114_reg_write_table(info->i2c_client, scene_landscape_normal);
										err = mt9m114_reg_write_table(info->i2c_client, scene_portrait_normal);
										err = mt9m114_reg_write_table(info->i2c_client, scene_sunset_normal);
										err = mt9m114_reg_write_table(info->i2c_client, sport_off);
									 break;
							case YUV_SceneMode_Action:
										printk("enter scene mode active \n");
										err = mt9m114_reg_write_table(info->i2c_client, scene_nightoff);
										err = mt9m114_reg_write_table(info->i2c_client, scene_landscape_normal);
										err = mt9m114_reg_write_table(info->i2c_client, scene_portrait_normal);
										err = mt9m114_reg_write_table(info->i2c_client, scene_sunset_normal);
										err = mt9m114_reg_write_table(info->i2c_client, sport);
									 break;
							case YUV_SceneMode_Portrait:
										printk("enter scene mode portrait \n");
										err = mt9m114_reg_write_table(info->i2c_client, scene_nightoff);
										err = mt9m114_reg_write_table(info->i2c_client, scene_landscape_normal);
										err = mt9m114_reg_write_table(info->i2c_client, scene_sunset_normal);
										err = mt9m114_reg_write_table(info->i2c_client, sport_off);
										err = mt9m114_reg_write_table(info->i2c_client, scene_portrait);
									 break;
							case YUV_SceneMode_Landscape:
										printk("enter scene mode landscape \n");
										err = mt9m114_reg_write_table(info->i2c_client, scene_nightoff);
										err = mt9m114_reg_write_table(info->i2c_client, scene_portrait_normal);
										err = mt9m114_reg_write_table(info->i2c_client, scene_sunset_normal);
										err = mt9m114_reg_write_table(info->i2c_client, sport_off);
										err = mt9m114_reg_write_table(info->i2c_client, scene_landscape);
									 break;
							case YUV_SceneMode_Sunset:
										printk("enter scene mode sunset \n");
										err = mt9m114_reg_write_table(info->i2c_client, scene_nightoff);
										err = mt9m114_reg_write_table(info->i2c_client, scene_landscape_normal);
										err = mt9m114_reg_write_table(info->i2c_client, scene_portrait_normal);
										err = mt9m114_reg_write_table(info->i2c_client, sport_off);
										err = mt9m114_reg_write_table(info->i2c_client, scene_sunset);
									 break;
							case YUV_SceneMode_Night:
										printk("enter scene mode night \n");
										err = mt9m114_reg_write_table(info->i2c_client, scene_landscape_normal);
										err = mt9m114_reg_write_table(info->i2c_client, scene_portrait_normal);
										err = mt9m114_reg_write_table(info->i2c_client, scene_sunset_normal);
										err = mt9m114_reg_write_table(info->i2c_client, sport_off);
										err = mt9m114_reg_write_table(info->i2c_client, scene_night);
									 break;
//&*&*&*CJ1_20120521: mod for mt9m114 scene mode
							default:
									 break;
					}

				if (err)
					return err;

				return 0;
			}
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
						   				err = mt9m114_reg_write_table(info->i2c_client, Exposure_0);
						         break;
						    case YUV_Exposure_1:
						   				err = mt9m114_reg_write_table(info->i2c_client, Exposure_1);
						         break;
						    case YUV_Exposure_2:
						   				err = mt9m114_reg_write_table(info->i2c_client, Exposure_2);
						         break;
						    case YUV_Exposure_Negative_1:
						   				err = mt9m114_reg_write_table(info->i2c_client, Exposure_Negative_1);
						         break;
						    case YUV_Exposure_Negative_2:
						   				err = mt9m114_reg_write_table(info->i2c_client, Exposure_Negative_2);
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

static int sensor_open(struct inode *inode, struct file *file)
{

	pr_info("yuv %s\n",__func__);

	file->private_data = info;
	if (info->pdata && info->pdata->power_on)
	{
		info->pdata->power_on(0);
		init_check_mt9m114 = 1;
	}

	return 0;
}

int sensor_release(struct inode *inode, struct file *file)
{
	if (info->pdata && info->pdata->power_off)
		info->pdata->power_off();
	file->private_data = NULL;
	return 0;
}


static const struct file_operations sensor_fileops = {
	.owner = THIS_MODULE,
	.open = sensor_open,
	.unlocked_ioctl = sensor_ioctl,
	.release = sensor_release,
};

static struct miscdevice sensor_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = SENSOR_NAME,
	.fops = &sensor_fileops,
};

static int sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int err;

	pr_info("mt9m114 %s+++\n",__func__);

	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);

	if (!info) {
		pr_err("mt9m114_sensor : Unable to allocate memory!\n");
		return -ENOMEM;
	}

	err = misc_register(&sensor_device);
	if (err) {
		pr_err("mt9m114_sensor : Unable to register misc device!\n");
		kfree(info);
		return err;
	}

	info->pdata = client->dev.platform_data;
	info->i2c_client = client;

	i2c_set_clientdata(client, info);


	pr_info("mt9m114 %s---\n",__func__);
	return 0;
}

static int sensor_remove(struct i2c_client *client)
{
	struct sensor_info *info;

	pr_info("mt9m114 %s\n",__func__);
	info = i2c_get_clientdata(client);
	misc_deregister(&sensor_device);
	kfree(info);
	return 0;
}

static const struct i2c_device_id sensor_id[] = {
	{ SENSOR_NAME, 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, sensor_id);

static struct i2c_driver sensor_i2c_driver = {
	.driver = {
		.name = SENSOR_NAME,
		.owner = THIS_MODULE,
	},
	.probe = sensor_probe,
	.remove = sensor_remove,
	.id_table = sensor_id,
};

static int __init sensor_init(void)
{
	pr_info("mt9m114 %s\n",__func__);
	return i2c_add_driver(&sensor_i2c_driver);
}

static void __exit sensor_exit(void)
{
	pr_info("mt9m114 %s\n",__func__);
	i2c_del_driver(&sensor_i2c_driver);
}

module_init(sensor_init);
module_exit(sensor_exit);

