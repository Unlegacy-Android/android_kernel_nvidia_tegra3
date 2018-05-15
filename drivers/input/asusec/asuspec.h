#ifndef _ASUSPEC_H
#define _ASUSPEC_H


/*
 * compiler option
 */
#define ASUSPEC_DEBUG			0

/*
 * Debug Utility
 */
#if ASUSPEC_DEBUG
#define ASUSPEC_INFO(format, arg...)	\
	printk(KERN_INFO "asuspec: [%s] " format , __FUNCTION__ , ## arg)
#define ASUSPEC_I2C_DATA(array, i)	\
					do {		\
						for (i = 0; i < array[0]+1; i++) \
							ASUSPEC_INFO("pad_ec_data[%d] = 0x%x\n", i, array[i]);	\
					} while(0)
#else
#define ASUSPEC_INFO(format, arg...)
#define ASUSPEC_I2C_DATA(array, i)
#endif

#define ASUSPEC_NOTICE(format, arg...)	\
	printk(KERN_NOTICE "asuspec: [%s] " format , __FUNCTION__ , ## arg)

#define ASUSPEC_ERR(format, arg...)	\
	printk(KERN_ERR "asuspec: [%s] " format , __FUNCTION__ , ## arg)

//-----------------------------------------

#define DRIVER_DESC     		"ASUS PAD EC driver"
#define PAD_SDEV_NAME			"pad"
#define APOWER_SDEV_NAME		"apower"

#define DELAY_TIME_MS		50
#define ASUSPEC_RETRY_COUNT		3
#define ASUSPEC_I2C_ERR_TOLERANCE	32

#define ASUSPEC_OBF_MASK			0x1
#define ASUSPEC_KEY_MASK			0x4
#define ASUSPEC_KBC_MASK			0x8
#define ASUSPEC_AUX_MASK			0x20
#define ASUSPEC_SCI_MASK			0x40
#define ASUSPEC_SMI_MASK			0x80
/************* SMI event ********************/
#define ASUSPEC_SMI_HANDSHAKING			0x50
#define ASUSPEC_SMI_WAKE			0x53
#define ASUSPEC_SMI_RESET			0x5F
#define APOWER_SMI_S3				0x83
#define APOWER_SMI_S5				0x85
#define APOWER_SMI_NOTIFY_SHUTDOWN		0x90
#define APOWER_SMI_RESUME			0x91
/*************APOWER switch state***************/
#define APOWER_IDLE			0
#define APOWER_RESUME			1
#define APOWER_SUSPEND			2
#define APOWER_POWEROFF			3
#define APOWER_NOTIFY_SHUTDOWN		4
/*************IO control setting***************/
#define ASUSPEC_IOCTL_HEAVY	2
#define ASUSPEC_IOCTL_NORMAL	1
#define ASUSPEC_IOCTL_END	0
#define ASUSPEC_IOC_MAGIC	0xf4
#define ASUSPEC_IOC_MAXNR	11
#define ASUSPEC_POLLING_DATA _IOR(ASUSPEC_IOC_MAGIC,	1,	int)
#define ASUSPEC_FW_UPDATE 	_IOR(ASUSPEC_IOC_MAGIC,	2,	int)
#define ASUSPEC_INIT 		_IOR(ASUSPEC_IOC_MAGIC,	3,	int)
#define ASUSPEC_FW_DUMMY	_IOR(ASUSPEC_IOC_MAGIC,	7,	int)
#define ASUSPEC_SWITCH_HDMI	_IOR(ASUSPEC_IOC_MAGIC,	10,	int)
#define ASUSPEC_WIN_SHUTDOWN	_IOR(ASUSPEC_IOC_MAGIC,	11,	int)
/*****************************************/
#define ASUSPEC_MAGIC_NUM	0x19850604

/************* EC FW update ***********/
#define EC_BUFF_LEN  256
/********************** ***********/

#define MB 1024*1024

/*
 * data struct
 */

struct asuspec_chip {
	struct input_dev	*indev;
	struct switch_dev 	pad_sdev;
	struct switch_dev 	apower_sdev;
	struct i2c_client	*client;
	struct mutex		lock;
	struct mutex		irq_lock;
	struct mutex		state_change_lock;
	struct delayed_work asuspec_fw_update_work;
	struct delayed_work asuspec_init_work;
	struct delayed_work asuspec_work;
	struct delayed_work asuspec_enter_s3_work;
	struct wake_lock 	wake_lock;
	struct timer_list	asuspec_timer;
	int polling_rate;
	int status;
	int ret_val;
	u8 ec_data[32];
	u8 i2c_data[32];
	u8 i2c_dm_data[32];
	u8 i2c_dm_battery[32];
	u8 i2c_dm_storage[32];
	char ec_model_name[32];
	char ec_version[32];
	char ec_pcba[32];
	int op_mode;	// 0: normal mode, 1: fw update mode
	int ec_ram_init;	// 0: not init, MAGIC_NUM: init successfully
	int ec_in_s3;	// 0: normal mode, 1: ec in deep sleep mode
	int i2c_err_count;
	int apwake_disabled;	// 0: normal mode, 1: apwake gets disabled
	int audio_recording;	// 0: not recording, 1: audio recording
	unsigned long storage_total;
	unsigned long storage_avail;
	unsigned int pad_pid;
	int apower_state;
};

#endif
