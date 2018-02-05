#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>

#include <asm/gpio.h>

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

//----
#include <linux/input.h>
#include <linux/debugfs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/gpio.h>
//----

#include <linux/workqueue.h>
#include <linux/delay.h>
#include "dsp_fm34.h"
#include <mach/board-asus-t30-misc.h>

#undef DUMP_REG

#define BYPASS_DSP_FOR_NORMAL_RECORDING

#define DSP_IOC_MAGIC			0xf3
#define DSP_IOC_MAXNR			3
#define DSP_CONTROL 			_IOW(DSP_IOC_MAGIC, 1,int)
#define DSP_RECONFIG 			_IOW(DSP_IOC_MAGIC, 2,int)

#define START_RECORDING 		1
#define END_RECORDING 			0
#define PLAYBACK 				2
#define INPUT_SOURCE_NORMAL 	100
#define INPUT_SOURCE_VR 		101
#define OUTPUT_SOURCE_NORMAL 	200
#define OUTPUT_SOURCE_VOICE 	201
#define INPUT_SOURCE_NO_AGC 	300
#define INPUT_SOURCE_AGC 		301

#define HEADPHONE_NO_MIC 		0
#define HEADSET_WITH_MIC 		1

#define MAX_RETRY 				(5)

extern bool headset_alive;

struct i2c_client *fm34_client;
struct fm34_chip *dsp_chip;
struct delayed_work config_dsp_work;

static int input_source = INPUT_SOURCE_NORMAL;
static int output_source = OUTPUT_SOURCE_NORMAL;
static int input_agc = INPUT_SOURCE_NO_AGC;

static bool bConfigured=false;

bool isRecording = false;
EXPORT_SYMBOL(isRecording);

static int fm34_i2c_retry(struct i2c_client *fm34_i2c_client, u8* parameter, size_t size)
{
	int retry = 0;
	int ret = -1;

        ret = i2c_master_send(fm34_i2c_client, parameter, size);
        msleep(5);

	while(retry < MAX_RETRY && ret < 0){
		retry++;
		pr_info("i2c no ack retry time = %d\n", retry);
		ret = i2c_master_send(fm34_i2c_client, parameter, size);
		msleep(5);
	}

	if(retry == MAX_RETRY)
		pr_info("i2c retry fail, exceed maximum retry times = %d\n", MAX_RETRY);

	return ret;

}

static void fm34_reset(void)
{
	gpio_set_value(TEGRA_GPIO_PO3, 0);
	msleep(10);
	gpio_set_value(TEGRA_GPIO_PO3, 1);
	return;
}

static int fm34_configure(void)
{
	int ret=0;
	struct i2c_msg msg[3];
	u8 buf1;
	int config_length;
	u8 *config_table;

	if(!bConfigured){
		fm34_reset();
		msleep(100);

		gpio_set_value(TEGRA_GPIO_PBB6, 1); // Enable DSP
		msleep(20);

		//access chip to check if acknowledgement.
		buf1=0xC0;
		/* Write register */
		msg[0].addr = dsp_chip->client->addr;
		msg[0].flags = 0;
		msg[0].len = 1;
		msg[0].buf = &buf1;

		ret = i2c_transfer(dsp_chip->client->adapter, msg, 1);
		if(ret < 0){
			pr_info("DSP NOack, Failed to read 0x%x: %d\n", buf1, ret);
			msleep(50);
			fm34_reset();
			return ret;
		}
		else
			pr_info("DSP ACK, read 0x%x: %d\n", buf1, ret);

		if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T){
			pr_info("DSP: load TF700T parameters\n");
			config_length= sizeof(input_parameter_TF700T);
			config_table= (u8 *)input_parameter_TF700T;
		}else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201){
			pr_info("DSP: load TF201 parameters\n");
			config_length= sizeof(input_parameter_TF201);
			config_table= (u8 *)input_parameter_TF201;
		}else{
			pr_info("DSP: load default parameters\n");
			config_length= sizeof(input_parameter);
			config_table= (u8 *)input_parameter;
		}

		ret = fm34_i2c_retry(dsp_chip->client, config_table, config_length);
		pr_info("DSP: config_length = %d\n", config_length);

		if(ret == config_length){
			pr_info("DSP: configuration is done\n");
			bConfigured=true;
		}

		msleep(100);
		gpio_set_value(TEGRA_GPIO_PBB6, 0);
	}

	return ret;
}

static void fm34_reconfigure(struct work_struct *work)
{
	pr_info("DSP: ReConfiguration\n");
	bConfigured=false;
	fm34_configure();
}

static ssize_t fm34_show(struct device *class, struct device_attribute *attr, char *buf)
{
	struct fm34_chip *data = i2c_get_clientdata(to_i2c_client(class));

	return sprintf(buf, "%d\n", data->status);
}

static int fm34_check_i2c(struct i2c_client *client)
{
	int ret = -1;
	struct i2c_msg msg[3];
	u8 buf1;

	fm34_reset();
	msleep(100);
	gpio_set_value(TEGRA_GPIO_PBB6, 1); // Enable DSP
	msleep(20);

	//access chip to check if acknowledgement.
	buf1=0xC0;
	/* Write register */
	msg[0].addr = dsp_chip->client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &buf1;

	ret = i2c_transfer(dsp_chip->client->adapter, msg, 1);
	if(ret < 0){
		//pr_info("DSP NOack, Failed to read 0x%x: %d\n", buf1, ret);
		msleep(50);
		fm34_reset();
		return ret;
	} else /*if(ret == 1)*/ { /* reg 0xC0 value should be 1 */
		//pr_info("DSP ack, read 0x%x: %d\n", buf1, ret);
		return 0;
	}
}

static int fm34_chip_init(struct i2c_client *client)
{
	int rc = 0;

	//config RST# pin, default HIGH.
	rc = gpio_request(TEGRA_GPIO_PO3, "fm34_reset");
	if (rc) {
		pr_err("DSP: gpio_request failed for input %d\n", TEGRA_GPIO_PO3);
	}

	rc = gpio_direction_output(TEGRA_GPIO_PO3, 1) ;
	if (rc) {
		pr_err("DSP: gpio_direction_output failed for input %d\n", TEGRA_GPIO_PO3);
	}
	pr_info("DSP: GPIO_PO3 = %d , state = %d\n", TEGRA_GPIO_PO3, gpio_get_value(TEGRA_GPIO_PO3));

	gpio_set_value(TEGRA_GPIO_PO3, 1);

	//config PWDN# pin, default HIGH.
	rc = gpio_request(TEGRA_GPIO_PBB6, "fm34_pwdn");
	if (rc) {
		pr_err("DSP: gpio_request failed for input %d\n", TEGRA_GPIO_PBB6);
	}

	rc = gpio_direction_output(TEGRA_GPIO_PBB6, 1) ;
	if (rc) {
		pr_err("DSP: gpio_direction_output failed for input %d\n", TEGRA_GPIO_PBB6);
	}
	pr_info("DSP: GPIO_PBB6 = %d , state = %d\n", TEGRA_GPIO_PBB6, gpio_get_value(TEGRA_GPIO_PBB6));

	gpio_set_value(TEGRA_GPIO_PBB6, 1);

	return 0;
}

static int fm34_open(struct inode *inode, struct file *filp)
{
	return 0;          /* success */
}


static int fm34_release(struct inode *inode, struct file *filp)
{
	return 0;          /* success */
}

static long fm34_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	int retval = 0;
	static int recording_enabled = -1;

	if (_IOC_TYPE(cmd) != DSP_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > DSP_IOC_MAXNR) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 * access_ok: 1 (successful, accessable)
	 */

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

       /* cmd: the ioctl commend user-space asked */
	switch(cmd){
		case DSP_CONTROL:
			fm34_configure();

			switch(arg){
				case START_RECORDING:
					gpio_set_value(TEGRA_GPIO_PBB6, 1);
					msleep(20);
					if(headset_alive){/*Headset mode*/
							pr_info("DSP: Start recording(AMIC), bypass DSP\n");
							fm34_i2c_retry(dsp_chip->client, (u8 *)bypass_parameter,
										sizeof(bypass_parameter));
							gpio_set_value(TEGRA_GPIO_PBB6, 0);
					}else{/*Handsfree mode*/
#ifdef BYPASS_DSP_FOR_NORMAL_RECORDING
						pr_info("DSP: Start recording(DMIC), ");
						isRecording = true;
						if(input_source==INPUT_SOURCE_VR){
							pr_info("DSP: enable DSP since VR case (INPUT_SOURCE_VR)\n");
							fm34_i2c_retry(dsp_chip->client, (u8 *)enable_parameter,
										sizeof(enable_parameter));

							pr_info("DSP: Disable Noise Suppression\n");
							if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF700T_disable_NS,
										sizeof(TF700T_disable_NS));
							else
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF201_disable_NS,
										sizeof(TF201_disable_NS));
						}
						else if(output_source==OUTPUT_SOURCE_VOICE || input_agc==INPUT_SOURCE_AGC){
							pr_info("DSP: enable DSP since VOICE case (OUTPUT_SOURCE_VOICE)\n");
							fm34_i2c_retry(dsp_chip->client, (u8 *)enable_parameter,
										sizeof(enable_parameter));

							pr_info("DSP: Enable Noise Suppression\n");
							if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF700T_enable_NS,
										sizeof(TF700T_enable_NS));
							else
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF201_enable_NS,
										sizeof(TF201_enable_NS));
						}else{
							pr_info("DSP: bypass DSP since NORMAL recording\n");
							fm34_i2c_retry(dsp_chip->client, (u8 *)bypass_parameter,
										sizeof(bypass_parameter));
							gpio_set_value(TEGRA_GPIO_PBB6, 0);
						}

#else
#ifdef BYPASS_DSP_FOR_VR
                        pr_info("DSP: Start recording(DMIC), ");
                        if(input_source==INPUT_SOURCE_VR){
                            pr_info("DSP: bypass DSP since BYPASS_DSP_FOR_VR\n");
                            fm34_i2c_retry(dsp_chip->client, (u8 *)bypass_parameter,
										sizeof(bypass_parameter));
                            gpio_set_value(TEGRA_GPIO_PBB6, 0);
                        }else{
							pr_info("enable DSP\n");
							fm34_i2c_retry(dsp_chip->client, (u8 *)enable_parameter,
										 sizeof(enable_parameter));

							pr_info("DSP: Enable Noise Suppression\n");
							if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF700T_enable_NS,
										sizeof(TF700T_enable_NS));
							else
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF201_enable_NS,
										sizeof(TF201_enable_NS));
						}
#else
						pr_info("DSP: Start recording(DMIC), enable DSP\n");
						fm34_i2c_retry(dsp_chip->client, (u8 *)enable_parameter,
										sizeof(enable_parameter));

						if(input_source==INPUT_SOURCE_VR){
							pr_info("DSP: Disable Noise Suppression\n");
							if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF700T_disable_NS,
										sizeof(TF700T_disable_NS));
							else
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF201_disable_NS,
                                        sizeof(TF201_disable_NS));
							}else{
							pr_info("DSP: Enable Noise Suppression\n");
							if(tegra3_get_project_id() == TEGRA3_PROJECT_TF700T)
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF700T_enable_NS,
										sizeof(TF700T_enable_NS));
							else
								fm34_i2c_retry(dsp_chip->client, (u8 *)TF201_enable_NS,
										sizeof(TF201_enable_NS));
							}
#endif  //BYPASS_DSP_FOR_VR
#endif  //BYPASS_DSP_FOR_NORMAL_RECORDING

					}
					recording_enabled = START_RECORDING;
					break;

				case END_RECORDING:
					isRecording = false;
					if(recording_enabled == START_RECORDING){
						pr_info("DSP: End recording, bypass DSP\n");
						gpio_set_value(TEGRA_GPIO_PBB6, 1);
						msleep(20);
						fm34_i2c_retry(dsp_chip->client, (u8 *)bypass_parameter, sizeof(bypass_parameter));
						recording_enabled = END_RECORDING;
						gpio_set_value(TEGRA_GPIO_PBB6, 0);
					}
					else{
						pr_info("DSP: End recording, but do nothing with DSP because no recording activity found\n");
						recording_enabled = END_RECORDING;
					}
					break;

				case INPUT_SOURCE_NORMAL:
				case INPUT_SOURCE_VR:
					pr_info("DSP: Input source = %s\n", arg==INPUT_SOURCE_NORMAL? "NORMAL" : "VR");
					input_source=arg;
					break;
				case OUTPUT_SOURCE_NORMAL:
				case OUTPUT_SOURCE_VOICE:
					pr_info("DSP: Output source = %s\n", arg==OUTPUT_SOURCE_NORMAL? "NORMAL" : "VOICE");
					output_source=arg;
					break;
				case INPUT_SOURCE_AGC:
				case INPUT_SOURCE_NO_AGC:
					printk("DSP: Input AGC = %s\n",	 arg == INPUT_SOURCE_AGC ? "AGC" : "NON-AGC");
					input_agc = arg;
					break;
				case PLAYBACK:
					pr_info("DSP: Do nothing because playback path always be bypassed after a DSP patch\n");
				default:
				break;
			}
		break;

		case DSP_RECONFIG:
			pr_info("DSP: ReConfig parameters\n");
			fm34_reconfigure(NULL);
		break;

	  default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}
	return retval;
}

static struct file_operations fm34_fops = {
	.owner =    THIS_MODULE,
	.unlocked_ioctl =	fm34_ioctl,
	.open =		fm34_open,
	.release =	fm34_release,
};

static SENSOR_DEVICE_ATTR(dsp_status, S_IRUGO, fm34_show, NULL, 1);

static struct attribute *fm34_attr[] = {
	&sensor_dev_attr_dsp_status.dev_attr.attr,
	NULL
};

static int fm34_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct fm34_chip *data;
	int err;

	dev_dbg(&client->dev, "%s()\n", __func__);

	data = kzalloc(sizeof (struct fm34_chip), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Memory allocation fails\n");
		err = -ENOMEM;
		goto exit;
	}

	dsp_chip=data;
	data->status = 0;

	i2c_set_clientdata(client, data);
	data->client = client;
	fm34_client= data->client;

	data->misc_dev.minor  = MISC_DYNAMIC_MINOR;
	data->misc_dev.name = "dsp_fm34";
	data->misc_dev.fops = &fm34_fops;
	err = misc_register(&data->misc_dev);
		if (err) {
			pr_err("tegra_acc_probe: Unable to register %s misc device\n", data->misc_dev.name);
		goto exit_free;
			}

	/* Register sysfs hooks */
	data->attrs.attrs = fm34_attr;
	err = sysfs_create_group(&client->dev.kobj, &data->attrs);
	if (err) {
		dev_err(&client->dev, "Not able to create the sysfs\n");
		goto exit_free;
	}

	fm34_chip_init(dsp_chip->client);
	if( fm34_check_i2c(dsp_chip->client) == 0)
       	data->status = 1;
	else
		data->status = 0;

	bConfigured=false;
	INIT_DELAYED_WORK(&config_dsp_work, fm34_reconfigure);
	schedule_delayed_work(&config_dsp_work, 0);
	pr_info("DSP: %s()\n", __func__);

	return 0;

exit_free:
	kfree(data);
exit:
	return err;
}

static int fm34_remove(struct i2c_client *client)
{
	struct fm34_chip *data = i2c_get_clientdata(client);

	misc_deregister(&data->misc_dev);
	dev_dbg(&client->dev, "%s()\n", __func__);
	pr_info("DSP: %s()\n", __func__);
	sysfs_remove_group(&client->dev.kobj, &data->attrs);

	kfree(data);
	return 0;
}

static void fm34_power_switch(int state)
{
	unsigned dsp_1v8_power_control;

	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201)
		dsp_1v8_power_control = TEGRA_GPIO_PU5;
	else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF300T)
		dsp_1v8_power_control = TEGRA_GPIO_PP3;
	else
		return;
	if(state){
		gpio_set_value(dsp_1v8_power_control, 1);
		schedule_delayed_work(&config_dsp_work, 0);
	}
	else{
		gpio_set_value(dsp_1v8_power_control, 0);
		bConfigured=false;
	}

}

static void fm34_power_switch_init(void)
{
	unsigned dsp_1v8_power_control;
	int ret = 0;
	
	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201)
		dsp_1v8_power_control = TEGRA_GPIO_PU5;
	else if(tegra3_get_project_id() == TEGRA3_PROJECT_TF300T)
		dsp_1v8_power_control = TEGRA_GPIO_PP3;
	else
		return;

	//Enalbe dsp power 1.8V
	ret = gpio_request(dsp_1v8_power_control, "dsp_power_1v8_en");
	if (ret < 0)
		pr_err("DSP: %s: gpio_request failed for gpio %s\n",
			__func__, "DSP_POWER_1V8_EN_GPIO");
	gpio_direction_output(dsp_1v8_power_control, 1);
	pr_info("DSP: gpio %d set to %d\n", dsp_1v8_power_control, gpio_get_value(dsp_1v8_power_control));

}

static int fm34_suspend(struct device *dev)
{
	int ret = 0;

	printk("DSP: %s()+\n", __func__);
	gpio_set_value(TEGRA_GPIO_PBB6, 0); /* Bypass DSP*/
	fm34_power_switch(0);
	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201){
		printk("DSP: %s(): project TF201\n", __func__);
		gpio_set_value(TEGRA_GPIO_PO3, 0);
		//Set DAP2_FS to low in LP0 for voltage leaking.
		ret = gpio_request(TEGRA_GPIO_PA2, "dap2_fs");
		if (ret < 0)
			pr_err("%s: gpio_request failed for gpio %s\n",
				__func__, "DAP2_FS");
		gpio_direction_output(TEGRA_GPIO_PA2, 0);
		gpio_free(TEGRA_GPIO_PA2);
		//Set AUDIO_MCLK to low in LP0 for voltage leaking.
		ret = gpio_request(TEGRA_GPIO_PW4, "audio_mclk");
		if (ret < 0)
			pr_err("%s: gpio_request failed for gpio %s\n",
				__func__, "AUDIO_MCLK");
		gpio_direction_output(TEGRA_GPIO_PW4, 0);
		gpio_free(TEGRA_GPIO_PW4);
	}
	gpio_set_value(TEGRA_GPIO_PBB6, 0); /* Bypass DSP*/
	printk("DSP: %s()-\n", __func__);
	return 0;
}

static int fm34_resume(struct device *dev)
{
	printk("DSP: %s()+\n", __func__);

	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201){
		gpio_set_value(TEGRA_GPIO_PO3, 1);
	}
	fm34_power_switch(1);
	printk("DSP: %s()-\n", __func__);
	return 0;
}

static SIMPLE_DEV_PM_OPS(fm34_pm_ops, fm34_suspend, fm34_resume);

static const struct i2c_device_id fm34_id[] = {
	{"dsp_fm34", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, fm34_id);

static struct i2c_driver fm34_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "dsp_fm34",
		.pm	= &fm34_pm_ops,
	},
	.probe		= fm34_probe,
	.remove		= fm34_remove,
	.id_table	= fm34_id,
};

static int __init fm34_init(void)
{
		printk(KERN_INFO "%s\n", __func__);
		
		//Enalbe dsp power 1.8V
		fm34_power_switch_init();
		return i2c_add_driver(&fm34_driver);
}

static void __exit fm34_exit(void)
{
	i2c_del_driver(&fm34_driver);
}

module_init(fm34_init);
module_exit(fm34_exit);
