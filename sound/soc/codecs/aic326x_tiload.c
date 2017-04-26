/*
 * linux/sound/soc/codecs/AIC3262_tiload.c
 *
 *
 * Copyright (C) 2010 Texas Instruments, Inc.
 *
 *
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * History:
 *
 * Rev 0.1	Tiload support		TI	16-09-2010
 *
 * The Tiload programming support is added to AIC3262.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <sound/soc.h>
#include <sound/control.h>
#include <linux/switch.h>
#include <sound/jack.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include <linux/mfd/tlv320aic3xxx-core.h>
#include "tlv320aic326x.h"
#include "aic326x_tiload.h"

/* enable debug prints in the driver */
/*#define DEBUG */
#undef DEBUG

#ifdef DEBUG
#define dprintk(x...)	printk(x)
#else
#define dprintk(x...)
#endif

#ifdef AIC3262_TiLoad

/* Function prototypes */
#ifdef REG_DUMP_aic3262
static void aic3262_dump_page(struct i2c_client *i2c, u8 page);
#endif

/************** Dynamic aic3262 driver, TI LOAD support  ***************/

static struct cdev *aic3262_cdev;
static union aic3xxx_reg_union aic_reg;
static int aic3262_major;	/* Dynamic allocation of Mjr No. */
static int aic3262_opened;	/* Dynamic allocation of Mjr No. */
static struct snd_soc_codec *aic3262_codec;
struct class *tiload_class;
static unsigned int magic_num = 0xE0;

/******************************** Debug section *****************************/

#ifdef REG_DUMP_aic3262
/*
 * aic3262_dump_page: Read and display one codec register page, for
	debugging purpose
 * @i2c; i2c_client identifies a single device (i.e. aic3262) connected
	to an i2c bus.
 * @page: page number
 *
 */
static void aic3262_dump_page(struct i2c_client *i2c, u8 page)
{
	int i;
	u8 data;
	u8 test_page_array[8];

	dprintk("TiLoad DRIVER : %s\n", __func__);

	data = 0x0;
	i2c_master_send(i2c, data, 1);
	i2c_master_recv(i2c, test_page_array, 8);

	dprintk("\n------- aic3262 PAGE %d DUMP --------\n", page);
	for (i = 0; i < 8; i++)
		dprintk(" [ %d ] = 0x%x\n", i, test_page_array[i]);
}
#endif

/**
 * tiload_open: open method for aic3262-tiload programming interface
 * @in: Pointer to inode
 * @filp: pointer to file
 *
 * Return: Return 0 if success.
 */
static int tiload_open(struct inode *in, struct file *filp)
{
	dprintk("TiLoad DRIVER : %s\n", __func__);
	if (aic3262_opened) {
		dprintk("%s device is already opened\n", "aic3262");
		dprintk("%s: only one instance of driver is allowed\n",
			"aic3262");
		return -1;
	}
	aic3262_opened++;
	return 0;
}

/**
 * tiload_release close method for aic3262_tilaod programming interface
 * @in: Pointer to inode
 * @filp: pointer to file
 *
 * Return: Return 0 if success.
 */
static int tiload_release(struct inode *in, struct file *filp)
{
	dprintk("TiLoad DRIVER : %s\n", __func__);
	aic3262_opened--;
	return 0;
}

/**
 * tiload_read: read method for mini dsp programming interface
 * @file: pointer to file
 * @buf: pointer to user
 * @count: number of byte to be read
 * @offset: offset address
 *
 * Return: return value read
 */
static ssize_t tiload_read(struct file *file, char __user *buf,
			   size_t count, loff_t *offset)
{
	static char rd_data[128];
	char reg_addr;
	size_t size;
#ifdef DEBUG
	int i;
#endif
	struct aic3xxx *control = aic3262_codec->control_data;

	dprintk("TiLoad DRIVER : %s\n", __func__);
	if (count > 128) {
		dprintk("Max 128 bytes can be read\n");
		count = 128;
	}

	/* copy register address from user space  */
	size = copy_from_user(&reg_addr, buf, 1);
	if (size != 0) {
		dprintk("read: copy_from_user failure\n");
		return -1;
	}
	/* Send the address to device thats is to be read */

	aic_reg.aic3xxx_register.offset = reg_addr;
	size =
	    aic3xxx_bulk_read(control, aic_reg.aic3xxx_register_int, count,
				rd_data);

#ifdef DEBUG
	pr_err(KERN_ERR "read size = %d, reg_addr= %x , count = %d\n",
		(int)size, reg_addr, (int)count);
	for (i = 0; i < (int)size; i++)
		dprintk("rd_data[%d]=%x\n", i, rd_data[i]);
#endif
	if (size != count)
		dprintk("read %d registers from the codec\n", size);

	if (copy_to_user(buf, rd_data, size) != 0) {
		dprintk("copy_to_user failed\n");
		return -1;
	}

	return size;
}

/**
 * tiload_write: write method for aic3262_tiload programming interface
 * @file: pointer to file
 * @buf: pointer to user
 * @count: number of byte to be read
 * @offset: offset address
 *
 * Return: return byte written
 */
static ssize_t tiload_write(struct file *file, const char __user *buf,
			    size_t count, loff_t *offset)
{
	static char wr_data[128];
#ifdef DEBUG
	int i;
#endif
	struct aic3xxx *control = aic3262_codec->control_data;

	dprintk("TiLoad DRIVER : %s\n", __func__);
	/* copy buffer from user space  */
	if (copy_from_user(wr_data, buf, count)) {
		dprintk("copy_from_user failure\n");
		return -1;
	}
#ifdef DEBUG
	dprintk("write size = %d\n", (int)count);
	for (i = 0; i < (int)count; i++)
		dprintk("\nwr_data[%d]=%x\n", i, wr_data[i]);
#endif
	if (wr_data[0] == 0) {
		/*change of page seen, but will only be registered */
		aic_reg.aic3xxx_register.page = wr_data[1];
		return count;

	} else
	    if (wr_data[0] == 127) {
		/* change of book seen, but will not be sent for I2C write */
		aic_reg.aic3xxx_register.book = wr_data[1];
		return count;

	} else {
		aic_reg.aic3xxx_register.offset = wr_data[0];
		aic3xxx_bulk_write(control, aic_reg.aic3xxx_register_int,
				   count - 1, &wr_data[1]);
		return count;
	}
}

/**
 * tiload_ioctl: copy data to user and from user
 * @filp: pointer to file
 * @cmd: integer of type command
 * @arg: argument type
 *
 * Return: Return 0 on success
 */
static long tiload_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int num = 0;
	void __user *argp = (void __user *)arg;

	if (_IOC_TYPE(cmd) != aic3262_IOC_MAGIC)
		return -ENOTTY;

	dprintk("TiLoad DRIVER : %s\n", __func__);
	switch (cmd) {
	case aic3262_IOMAGICNUM_GET:
		num = copy_to_user(argp, &magic_num, sizeof(int));
		break;
	case aic3262_IOMAGICNUM_SET:
		num = copy_from_user(&magic_num, argp, sizeof(int));
		break;
	}
	return num;
}

/******* File operations structure for aic3262-tiload programming *********/
static struct file_operations aic3262_fops = {
	.owner = THIS_MODULE,
	.open = tiload_open,
	.release = tiload_release,
	.read = tiload_read,
	.write = tiload_write,
	.unlocked_ioctl = tiload_ioctl,
};

/**
 * aic3262_driver_init: Register a char driver for dynamic aic3262-tiload programming
 * @codec: pointer variable to codec having codec information
 *
 * Return: Return 0 on seccess
 */
int aic3262_driver_init(struct snd_soc_codec *codec)
{
	int result;

	dev_t dev = MKDEV(aic3262_major, 0);
	dprintk("TiLoad DRIVER : %s\n", __func__);
	aic3262_codec = codec;

	dprintk("allocating dynamic major number\n");

	result = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
	if (result < 0) {
		dprintk("cannot allocate major number %d\n", aic3262_major);
		return result;
	}

	tiload_class = class_create(THIS_MODULE, DEVICE_NAME);
	aic3262_major = MAJOR(dev);
	dprintk("allocated Major Number: %d\n", aic3262_major);

	aic3262_cdev = cdev_alloc();
	cdev_init(aic3262_cdev, &aic3262_fops);
	aic3262_cdev->owner = THIS_MODULE;
	aic3262_cdev->ops = &aic3262_fops;

	aic_reg.aic3xxx_register.page = 0;
	aic_reg.aic3xxx_register.book = 0;

	if (cdev_add(aic3262_cdev, dev, 1) < 0) {
		dprintk("aic3262_driver: cdev_add failed\n");
		unregister_chrdev_region(dev, 1);
		aic3262_cdev = NULL;
		return 1;
	}
	dprintk("Registered aic3262 TiLoad driver, Major number: %d\n",
		aic3262_major);
	return 0;
}

#endif
