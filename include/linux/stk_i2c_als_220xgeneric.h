/*
 * stk_i2c_als220xgeneric.h
 *
 * $Id: stk_i2c_als.h,v 1.0 2010/08/09 11:12:08 jsgood Exp $
 *
 * Copyright (C) 2010 Patrick Chang <patrick_chang@sitronix.com.tw>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	SenseTek/Sitronix Ambient Light Sensor Driver
 *	based on stk2203.c
 */

#ifndef __STK_I2C_ALS220XGENERIC_H
#define __STK_I2C_ALS220XGENERIC_H


/* Define Reg Address */
#define STK_ALS_CMD_REG 0x01
#define STK_ALS_DT1_REG 0x02
#define STK_ALS_DT2_REG 0x03
#define STK_ALS_INT_REG 0x04


/* Define CMD */
#define STK_ALS_CMD_GAIN_SHIFT 6
#define STK_ALS_CMD_IT_SHIFT 2
#define STK_ALS_CMD_SD_SHIFT 0

#define STK_ALS_CMD_GAIN(x) ((x)<<STK_ALS_CMD_GAIN_SHIFT)
#define STK_ALS_CMD_IT(x) ((x)<<STK_ALS_CMD_IT_SHIFT)
#define STK_ALS_CMD_SD(x) ((x)<<STK_ALS_CMD_SD_SHIFT)

#define STK_ALS_CMD_GAIN_MASK 0xC0
#define STK_ALS_CMD_IT_MASK 0x0C
#define STK_ALS_CMD_SD_MASK 0x1


/* Define Data */
#define STK_ALS_DATA(DT1,DT2) ((DT1<<4)|(DT2)>>4)

/*Define Interrupt */
#define STK_ALS_INT_THD(x) ((x)<<6)
#define STK_ALS_INT_PRST(x) ((x)<<4)
#define STK_ALS_INT_FLAG(x) ((x)<<3)
#define STK_ALS_INT_DN(x) ((x)<<2)
#define STK_ALS_INT_SFQ(x) ((x)<<1)
#define STK_ALS_INT_ENF(x) ((x)<<0)

#define STK_ALS_INT_THD_MASK 0xC0
#define STK_ALS_INT_PRST_MASK 0x30
#define STK_ALS_INT_FLAG_MASK 0x08
#define STK_ALS_INT_DN_MASK 0x04
#define STK_ALS_INT_SFQ_MASK 0x02
#define STK_ALS_INT_ENF_MASK 0x01


struct stkals_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct work_struct work;
	int32_t irq;
	uint8_t enable_als_irq;
	int32_t als_lux_last;
	uint32_t als_delay;
};
#define ALS_MIN_DELAY 250
#define STK_IRQF_MODE IRQF_TRIGGER_FALLING

#endif /*__STK_I2C_ALS220XGENERIC_H*/
