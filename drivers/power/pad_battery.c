/*
 * drivers/power/pad_battery.c
 *
 * Gas Gauge driver for TI's BQ20Z45
 *
 * Copyright (c) 2010, NVIDIA Corporation.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/power_supply.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/wakelock.h>

#include <mach/gpio.h>
#include <linux/timer.h>
#include "../../arch/arm/mach-tegra/gpio-names.h"
#include "../../arch/arm/mach-tegra/wakeups-t3.h"
#include <mach/board-asus-t30-misc.h>
#include <linux/delay.h>

#define SMBUS_RETRY                         (3)
#define GPIOPIN_BATTERY_DETECT              TEGRA_GPIO_PN4
#define GPIOPIN_LOW_BATTERY_DETECT          TEGRA_GPIO_PS4
#define BATTERY_POLLING_RATE                (120)
#define DELAY_FOR_CORRECT_CHARGER_STATUS	(4)
#define TEMP_KELVIN_TO_CELCIUS              (2731)
#define MAXIMAL_VALID_BATTERY_TEMP          (200)

#define USB_NO_Cable                        0
#define USB_DETECT_CABLE                    1 
#define USB_SHIFT                           0
#define AC_SHIFT                            1 
#define USB_Cable                           ((1 << (USB_SHIFT)) | (USB_DETECT_CABLE))
#define USB_AC_Adapter                      ((1 << (AC_SHIFT)) | (USB_DETECT_CABLE))
#define USB_CALBE_DETECT_MASK               (USB_Cable  | USB_DETECT_CABLE)

unsigned battery_cable_status = 0;
unsigned battery_docking_status = 0;
unsigned battery_driver_ready = 0;

static int ac_on;
static int usb_on;
extern int asuspec_battery_monitor(char *cmd);
static unsigned int battery_current;
static unsigned int battery_remaining_capacity;

module_param(battery_current, uint, 0644);
module_param(battery_remaining_capacity, uint, 0644);

enum {
    REG_MANUFACTURER_DATA,  	
	REG_STATE_OF_HEALTH,
	REG_TEMPERATURE,
	REG_VOLTAGE,
	REG_CURRENT,
	REG_TIME_TO_EMPTY,
	REG_TIME_TO_FULL,
	REG_STATUS,
	REG_CAPACITY,
	REG_SERIAL_NUMBER,
	REG_MAX
};
typedef enum {
	Charger_Type_Battery = 0,
	Charger_Type_AC,
	Charger_Type_USB,
	Charger_Type_Docking_AC,
	Charger_Type_Num,
	Charger_Type_Force32 = 0x7FFFFFFF
} Charger_Type;

#define BATTERY_MANUFACTURER_SIZE	12
#define BATTERY_NAME_SIZE		8

/* manufacturer access defines */
#define MANUFACTURER_ACCESS_STATUS	0x0006
#define MANUFACTURER_ACCESS_SLEEP	0x0011

/* battery status value bits */
#define BATTERY_CHARGING 			0x40
#define BATTERY_FULL_CHARGED		0x20
#define BATTERY_FULL_DISCHARGED 	0x10
#define PAD_BAT_DATA(_psp, _addr, _min_value, _max_value)	\
	{							\
		.psp = POWER_SUPPLY_PROP_##_psp,		\
		.addr = _addr,					\
		.min_value = _min_value,			\
		.max_value = _max_value,			\
	}

static struct pad_device_data {
	enum power_supply_property psp;
	u8 addr;
	int min_value;
	int max_value;
} pad_data[] = {
    [REG_MANUFACTURER_DATA]	= PAD_BAT_DATA(PRESENT, 0, 0, 65535),
    [REG_STATE_OF_HEALTH]	= PAD_BAT_DATA(HEALTH, 0, 0, 65535),
	[REG_TEMPERATURE]       = PAD_BAT_DATA(TEMP, 0x08, 0, 65535),
	[REG_VOLTAGE]           = PAD_BAT_DATA(VOLTAGE_NOW, 0x09, 0, 20000),
	[REG_CURRENT]           = PAD_BAT_DATA(CURRENT_NOW, 0x0A, -32768, 32767),
	[REG_TIME_TO_EMPTY]     = PAD_BAT_DATA(TIME_TO_EMPTY_AVG, 0x12, 0, 65535),
	[REG_TIME_TO_FULL]      = PAD_BAT_DATA(TIME_TO_FULL_AVG, 0x13, 0, 65535),
	[REG_STATUS]            = PAD_BAT_DATA(STATUS, 0x16, 0, 65535),
	[REG_CAPACITY]          = PAD_BAT_DATA(CAPACITY, 0x0d, 0, 100),//battery HW request
	[REG_SERIAL_NUMBER]     = PAD_BAT_DATA(SERIAL_NUMBER, 0x1C, 0, 65535),
};

static enum power_supply_property pad_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
};

unsigned (*get_usb_cable_status_cb) (void);

void check_cabe_type(void)
{
    if(battery_cable_status == USB_AC_Adapter){
		ac_on = 1;
	    usb_on = 0;
	} else if (battery_cable_status == USB_Cable){
		ac_on = 0;
		usb_on = 1;
	} else {
		ac_on = 0;
		usb_on = 0;
	}
}
static int pad_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val);

static enum power_supply_property power_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int pad_get_psp(int reg_offset, enum power_supply_property psp,union power_supply_propval *val);

static int power_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	int ret = 0;
	switch (psp) {

	case POWER_SUPPLY_PROP_ONLINE:
		   if(psy->type == POWER_SUPPLY_TYPE_MAINS && ac_on)
		   	val->intval =  1;
		   else if (psy->type == POWER_SUPPLY_TYPE_USB && usb_on)
			val->intval =  1;
		   else if (psy->type == POWER_SUPPLY_TYPE_DOCK_AC&& battery_docking_status)
			val->intval =  1;
		   else 
		   	val->intval = 0;
		break;

	default:
		return -EINVAL;
	}
	return ret;
}

static char *supply_list[] = {
	"battery",
	"ac",
	"usb",
};

static struct power_supply pad_supply[] = {
	{
	    .name		= "battery",
	    .type		= POWER_SUPPLY_TYPE_BATTERY,
	    .properties	= pad_properties,
	    .num_properties	= ARRAY_SIZE(pad_properties),
	    .get_property	= pad_get_property,
    },
	{
		.name        = "ac",
		.type        = POWER_SUPPLY_TYPE_MAINS,
		.supplied_to = supply_list,
		.num_supplicants = ARRAY_SIZE(supply_list),
		.properties = power_properties,
		.num_properties = ARRAY_SIZE(power_properties),
		.get_property = power_get_property,
	},
	{
		.name        = "usb",
		.type        = POWER_SUPPLY_TYPE_USB,
		.supplied_to = supply_list,
		.num_supplicants = ARRAY_SIZE(supply_list),
		.properties = power_properties,
		.num_properties = ARRAY_SIZE(power_properties),
		.get_property = power_get_property,
	},
	{
		.name        = "docking_ac",
		.type        = POWER_SUPPLY_TYPE_DOCK_AC,
		.properties = power_properties,
		.num_properties = ARRAY_SIZE(power_properties),
		.get_property = power_get_property,
	},
};

static struct pad_device_info {
	struct i2c_client	*client;
	struct delayed_work thermal_stress_test;
	struct delayed_work pmu_stress_test;
	struct delayed_work status_poll_work;
	struct delayed_work low_low_bat_work;
	struct timer_list charger_pad_dock_detect_timer ;
	int smbus_status;
	int battery_present;
	int low_battery_present;
	unsigned int old_capacity;
	unsigned int cap_err;
	unsigned int old_temperature;
	unsigned int temp_err;
	int gpio_battery_detect;
	int gpio_low_battery_detect;
	int irq_low_battery_detect;
	int irq_battery_detect;
	bool dock_charger_pad_interrupt_enabled;
	spinlock_t lock;
	unsigned int prj_id;
	struct wake_lock low_battery_wake_lock;
	struct wake_lock cable_event_wake_lock;
	int bat_status;
	int bat_temp;
	int bat_vol;
	int bat_current;
	int bat_capacity;
} *pad_device;
	
void register_usb_cable_status_cb(unsigned (*fn) (void))
{
	if (!get_usb_cable_status_cb)
		get_usb_cable_status_cb = fn;
}

unsigned get_usb_cable_status(void)
{
	if (!get_usb_cable_status_cb) {
		pr_err("pad_battery: get_usb_cable_status_cb is NULL\n");
		return 0;
	}
	return get_usb_cable_status_cb();
}

static ssize_t show_battery_charger_status(struct device *dev, struct device_attribute *devattr, char *buf)
{
	if(pad_device->smbus_status < 0)
	{
		return sprintf(buf, "%d\n", 0);
	}
	else
	{
		return sprintf(buf, "%d\n", 1);
	}
}
static DEVICE_ATTR(battery_charger, S_IWUSR | S_IRUGO, show_battery_charger_status, NULL);

static struct attribute *battery_charger_attributes[] = {
	&dev_attr_battery_charger.attr,
	NULL
};

static const struct attribute_group battery_charger_group = {
	.attrs = battery_charger_attributes,
};

int pad_smbus_read_data(int reg_offset,int byte)
{
    s32 ret = -EINVAL;
    int count = 0; 
    do{
	    if(byte)
            ret=i2c_smbus_read_byte_data(pad_device->client,pad_data[reg_offset].addr);
	    else
	        ret=i2c_smbus_read_word_data(pad_device->client,pad_data[reg_offset].addr);

    }while((ret < 0)&& (++count <= SMBUS_RETRY));
    return ret;
}

int pad_smbus_write_data(int reg_offset,int byte, unsigned int value)
{
    s32 ret = -EINVAL;
    int count = 0; 

    do{
	    if(byte){
            ret = i2c_smbus_write_byte_data(pad_device->client, pad_data[reg_offset].addr,value&0xFF);
	    } else {
	        ret = i2c_smbus_write_word_data(pad_device->client, pad_data[reg_offset].addr,value&0xFFFF);
	    }

    }while((ret < 0)&& (++count <= SMBUS_RETRY));
    return ret;
}

struct workqueue_struct *battery_work_queue = NULL;

static ssize_t show_battery_smbus_status(struct device *dev, struct device_attribute *devattr, char *buf)
{
	int status =! pad_device->smbus_status;
	return sprintf(buf, "%d\n", status);
}
static DEVICE_ATTR(battery_smbus, S_IWUSR | S_IRUGO, show_battery_smbus_status,NULL);

static struct attribute *battery_smbus_attributes[] = {
	&dev_attr_battery_smbus.attr,
	NULL
};

static const struct attribute_group battery_smbus_group = {
	.attrs = battery_smbus_attributes,
};

static void battery_status_poll(struct work_struct *work)
{
    struct pad_device_info *battery_device = container_of(work, struct pad_device_info, status_poll_work.work);
	if(!battery_driver_ready)
	    printk("pad_battery: %s: driver not ready\n", __func__);
	power_supply_changed(&pad_supply[Charger_Type_Battery]);
	/* Schedule next poll */
    pad_device->battery_present =! (gpio_get_value(pad_device->gpio_battery_detect));
	if(pad_device->battery_present)
		queue_delayed_work(battery_work_queue, &battery_device->status_poll_work, BATTERY_POLLING_RATE*HZ);
}

static irqreturn_t battery_detect_isr(int irq, void *dev_id)
{
	pad_device->battery_present =! (gpio_get_value(pad_device->gpio_battery_detect));
	printk("pad_battery: %s: battery %s \n", __func__, pad_device->battery_present?"instered":"removed" );
	if(pad_device->battery_present)
		queue_delayed_work(battery_work_queue, &pad_device->status_poll_work, BATTERY_POLLING_RATE*HZ);
	return IRQ_HANDLED;
}

static void low_low_battery_check(struct work_struct *work)
{
    cancel_delayed_work(&pad_device->status_poll_work);
	queue_delayed_work(battery_work_queue, &pad_device->status_poll_work, 0.1*HZ);
	msleep(2000);
	enable_irq(pad_device->irq_low_battery_detect);
}

static irqreturn_t low_battery_detect_isr(int irq, void *dev_id)
{
	disable_irq_nosync(pad_device->irq_low_battery_detect);
	pad_device->low_battery_present = gpio_get_value(pad_device->gpio_low_battery_detect);
	printk("pad_battery: %s: battery is %x\n", __func__, pad_device->low_battery_present);
	wake_lock_timeout(&pad_device->low_battery_wake_lock, 10*HZ);
	queue_delayed_work(battery_work_queue, &pad_device->low_low_bat_work, 0.1*HZ);
	return IRQ_HANDLED;
}

static void setup_detect_irq(void)
{
    s32 ret = 0;
	
	pad_device->battery_present = 0;
	pad_device->low_battery_present = 0;
       ret = gpio_request(pad_device->gpio_battery_detect, "battery_detect");
	if (ret < 0) {
		printk("request battery_detect gpio failed\n");
		pad_device->gpio_battery_detect = -1;
		goto setup_low_bat_irq;
	}
	
	pad_device->irq_battery_detect = gpio_to_irq(pad_device->gpio_battery_detect);
	if (pad_device->irq_battery_detect < 0) {
		printk("invalid battery_detect GPIO\n");
		pad_device->gpio_battery_detect = -1;
		pad_device->irq_battery_detect = -1;
		goto setup_low_bat_irq;
	}
	ret = gpio_direction_input(pad_device->gpio_battery_detect);
	if (ret < 0) {
			printk("failed to configure GPIO\n");
			gpio_free(pad_device->gpio_battery_detect);
			pad_device->gpio_battery_detect = -1;
			goto setup_low_bat_irq;
	}
	ret = request_irq(pad_device->irq_battery_detect, battery_detect_isr,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "pad-battery (detect)", NULL);
	if (ret < 0) {
            printk("failed to request battery_detect irq\n");
	}
	 pad_device->battery_present =! (gpio_get_value(pad_device->gpio_battery_detect));
	printk("pad_battery: %s: battery_present = %x\n", __func__, pad_device->battery_present);
setup_low_bat_irq:

	return;
}

static void setup_low_battery_irq(void)
{
    s32 ret = 0;

	pad_device->gpio_low_battery_detect = GPIOPIN_LOW_BATTERY_DETECT;
    ret = gpio_request( pad_device->gpio_low_battery_detect, "low_battery_detect");
	if (ret < 0) {
		printk("request low_battery_detect gpio failed\n");
		pad_device->gpio_low_battery_detect = -1;
		goto exit;
	}

	pad_device->irq_low_battery_detect = gpio_to_irq(pad_device->gpio_low_battery_detect);
	if (pad_device->irq_low_battery_detect < 0) {
		printk("invalid low_battery_detect gpio\n");
		pad_device->gpio_low_battery_detect = -1;
		pad_device->irq_low_battery_detect = -1;
		goto exit;
	}

	ret = gpio_direction_input(pad_device->gpio_low_battery_detect);
	if (ret < 0) {
			printk("failed to configure low_battery_detect gpio\n");
			gpio_free(pad_device->gpio_battery_detect);
			pad_device->gpio_low_battery_detect= -1;
			goto exit;
	}
	ret = request_irq(pad_device->irq_low_battery_detect, low_battery_detect_isr,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "bq20z45-battery (low battery)", NULL);
       pad_device->low_battery_present=gpio_get_value(pad_device->gpio_low_battery_detect);
	if (ret < 0) {
            printk("failed to request low_battery_detect irq\n");
	}
	enable_irq_wake(pad_device->irq_low_battery_detect);
exit:
	return;
}

void battery_callback(unsigned usb_cable_state)
{
    int old_cable_status = battery_cable_status;
    printk("pad_battery: %s: usb_cable_state = %x\n", __func__, usb_cable_state ) ;
    battery_cable_status = usb_cable_state ;
	if(!battery_driver_ready){
		printk("pad_battery: %s: battery driver not ready\n", __func__) ;
		return;
	}
	check_cabe_type();

	wake_lock_timeout(&pad_device->cable_event_wake_lock, DELAY_FOR_CORRECT_CHARGER_STATUS * HZ);

	if(!battery_cable_status){
		if (old_cable_status == USB_AC_Adapter){
			power_supply_changed(&pad_supply[Charger_Type_AC]);
		} else if (old_cable_status == USB_Cable){
			power_supply_changed(&pad_supply[Charger_Type_USB]);
		}
	} else if (battery_cable_status == USB_Cable){
		power_supply_changed(&pad_supply[Charger_Type_USB]);
	} else if (battery_cable_status == USB_AC_Adapter){
		power_supply_changed(&pad_supply[Charger_Type_AC]);
	}
	cancel_delayed_work(&pad_device->status_poll_work);

	queue_delayed_work(battery_work_queue, &pad_device->status_poll_work, 2*HZ);
}

static irqreturn_t charger_pad_dock_interrupt(int irq, void *dev_id)
{
	pr_info("charger_pad_dock_interrupt\n");
	mod_timer(&pad_device->charger_pad_dock_detect_timer, jiffies + (5*HZ));
	return IRQ_HANDLED;
}

static int docking_status = 0;
static void charger_pad_dock_detection(unsigned long unused)
{
//	dock_in_value = gpio_get_value(TEGRA_GPIO_PU4);
//	charger_pad_dock_value = gpio_get_value(TEGRA_GPIO_PS5);

	if(docking_status && gpio_get_value(TEGRA_GPIO_PU4) == 0 && gpio_get_value(TEGRA_GPIO_PS5) == 0){
		battery_docking_status=true;
	} else {
		battery_docking_status=false;
	}

	check_cabe_type();
	power_supply_changed(&pad_supply[Charger_Type_Docking_AC]);
	if(battery_driver_ready){
		 cancel_delayed_work(&pad_device->status_poll_work);
		 queue_delayed_work(battery_work_queue, &pad_device->status_poll_work, 0.2*HZ);
	}

}

int docking_callback(int docking_in)
{
	if(!battery_driver_ready)
		return -1;
	
	printk("pad_battery: %s: docked in = %u, TEGRA_GPIO_PU4 is %x\n", __func__, docking_in, gpio_get_value(TEGRA_GPIO_PU4));
	docking_status = docking_in;

	if(docking_status){
		if(!pad_device->dock_charger_pad_interrupt_enabled){
			enable_irq(gpio_to_irq(TEGRA_GPIO_PS5));
			pad_device->dock_charger_pad_interrupt_enabled=true;
			printk("pad_battery: %s: enable_irq for TEGRA_GPIO_PS5\n", __func__) ;
		}
	}else if(pad_device->dock_charger_pad_interrupt_enabled){
		disable_irq(gpio_to_irq(TEGRA_GPIO_PS5));
		pad_device->dock_charger_pad_interrupt_enabled=false;
		printk("pad_battery: %s: disable_irq for TEGRA_GPIO_PS5\n", __func__) ;
	}

	if(battery_driver_ready){
		cancel_delayed_work_sync(&pad_device->status_poll_work);
		power_supply_changed(&pad_supply[Charger_Type_Battery]);
		 mod_timer(&pad_device->charger_pad_dock_detect_timer, jiffies +(5*HZ));
	}
	return 0;
}
EXPORT_SYMBOL(docking_callback);

void init_docking_charging_irq(void)
{
    int rc;

	rc = gpio_request(TEGRA_GPIO_PS5, "dock_charging");
	if (rc < 0)
		pr_err("TEGRA_GPIO_PS5 GPIO%d request fault!%d\n", TEGRA_GPIO_PS5,rc);
	
	rc = gpio_direction_input(TEGRA_GPIO_PS5);
	if (rc)
	    pr_err("gpio_direction_input failed for input TEGRA_GPIO_PS5=%d\n", TEGRA_GPIO_PS5);

	rc = request_irq(gpio_to_irq(TEGRA_GPIO_PS5), charger_pad_dock_interrupt, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "dock_charging" , NULL);
	pad_device->dock_charger_pad_interrupt_enabled = true;
	if (rc < 0)
	    pr_err("Could not register for TEGRA_GPIO_PS5 interrupt, irq = %d, rc = %d\n", gpio_to_irq(TEGRA_GPIO_PS5), rc);
}

static int pad_get_health(enum power_supply_property psp,
	union power_supply_propval *val)
{	
	if (psp == POWER_SUPPLY_PROP_PRESENT) {
		val->intval = pad_device->battery_present;
	} else if (psp == POWER_SUPPLY_PROP_HEALTH) {
		if( pad_device->battery_present)
			val->intval = POWER_SUPPLY_HEALTH_GOOD;
		else
			val->intval = POWER_SUPPLY_HEALTH_DEAD;
	}
	return 0;
}

static int pad_get_psp(int reg_offset, enum power_supply_property psp,
	union power_supply_propval *val)
{
	s32 ret;
	int smb_retry = 0;

	if(pad_device->battery_present){
			do{
				if (psp == POWER_SUPPLY_PROP_VOLTAGE_NOW){
					pad_device->smbus_status = pad_device->bat_vol=asuspec_battery_monitor("voltage");
					battery_current = asuspec_battery_monitor("current");
					battery_remaining_capacity = asuspec_battery_monitor("remaining_capacity");
				}
				else if (psp == POWER_SUPPLY_PROP_STATUS)
					pad_device->smbus_status = pad_device->bat_status = asuspec_battery_monitor("status");
				else  if (psp == POWER_SUPPLY_PROP_TEMP)
					pad_device->smbus_status = pad_device->bat_temp = asuspec_battery_monitor("temperature");
			}while((pad_device->smbus_status < 0) && ( ++smb_retry <= SMBUS_RETRY));
	}else{
			pad_device->smbus_status = -1;
			printk("pad_get_psp: pad_device->battery_present = %u\n",pad_device->battery_present);
	}

	if (pad_device->smbus_status < 0) {
		dev_err(&pad_device->client->dev,
			"%s: i2c read for %d failed\n", __func__, reg_offset);
		
		if(psp == POWER_SUPPLY_PROP_TEMP && (++pad_device->temp_err<=3) &&(pad_device->old_temperature!=0xFF)){
			val->intval = pad_device->old_temperature;
			printk("read battery's tempurate fail use old temperature = %u pad_device->temp_err = %u\n", val->intval, pad_device->temp_err);
			return 0;
		}
		else
		return -EINVAL;
	}

	if (psp == POWER_SUPPLY_PROP_VOLTAGE_NOW) {
		val->intval = pad_device->bat_vol;
	}

	if (psp == POWER_SUPPLY_PROP_STATUS) {
		ret = pad_device->bat_status;

		/* mask the upper byte and then find the actual status */
		if (!(ret & BATTERY_CHARGING) && (ac_on || battery_docking_status) ){/*DSG*/
			val->intval = POWER_SUPPLY_STATUS_CHARGING;
			if (pad_device->old_capacity == 100)
				val->intval = POWER_SUPPLY_STATUS_FULL;
		}
		else if (ret & BATTERY_FULL_CHARGED)//fc
			val->intval = POWER_SUPPLY_STATUS_FULL;
		else if (ret & BATTERY_FULL_DISCHARGED)//fd
			val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
		else 
			val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
	}else if (psp == POWER_SUPPLY_PROP_TEMP) {
		ret = pad_device->bat_temp;
		ret -= TEMP_KELVIN_TO_CELCIUS;

		if ((ret/10) >= MAXIMAL_VALID_BATTERY_TEMP && (pad_device->temp_err == 0)){
			ret = 300;
			printk("[Warning] pad_get_psp  battery temp=%u, set to 30.0\n", ret);
			WARN_ON(1);
			pad_device->temp_err++;
		}
		else
			pad_device->temp_err = 0;

		pad_device->old_temperature = val->intval = ret;
	}

	return 0;
}

static int pad_get_capacity(union power_supply_propval *val)
{
	s32 ret;
	s32 temp_capacity;
	int smb_retry = 0;

	if(pad_device->battery_present){
		do{
			pad_device->smbus_status = pad_device->bat_capacity = asuspec_battery_monitor("capacity");
		}while((pad_device->smbus_status < 0) && ( ++smb_retry <= SMBUS_RETRY));

	}else{
			pad_device->smbus_status = -1;
			printk("pad_get_capacity: pad_device->battery_present=%u\n",pad_device->battery_present);
	}

	if (pad_device->smbus_status < 0) {
		dev_err(&pad_device->client->dev, "%s: i2c read for %d "
			"failed pad_device->cap_err=%u\n", __func__, REG_CAPACITY, pad_device->cap_err);
		if(pad_device->cap_err > 5 || (pad_device->old_capacity == 0xFF))
		return -EINVAL;
		else{
			val->intval = pad_device->old_capacity;
			pad_device->cap_err++;
			printk("read capacity fail, use old capacity=%u pad_device->cap_err=%u\n",val->intval,pad_device->cap_err);
			return 0;
		}
	}

	ret = pad_device->bat_capacity;
        /* pad spec says that this can be >100 %
         * even if max value is 100 % */
	temp_capacity = ((ret >= 100) ? 100 : ret);

	/* start: for mapping %99 to 100%. Lose 84%*/
	if(temp_capacity==99)
		temp_capacity=100;
	if(temp_capacity >=84 && temp_capacity <=98)
		temp_capacity++;
	/* for mapping %99 to 100% */

	 /* lose 26% 47% 58%,69%,79% */
	if(temp_capacity >70 && temp_capacity <80)
		temp_capacity-=1;
	else if(temp_capacity >60&& temp_capacity <=70)
		temp_capacity-=2;
	else if(temp_capacity >50&& temp_capacity <=60)
		temp_capacity-=3;
	else if(temp_capacity >30&& temp_capacity <=50)
		temp_capacity-=4;
	else if(temp_capacity >=0&& temp_capacity <=30)
		temp_capacity-=5;

	/*Re-check capacity to avoid  that temp_capacity <0*/
	temp_capacity = ((temp_capacity <0) ? 0 : temp_capacity);
	val->intval = temp_capacity;

	pad_device->old_capacity = val->intval;
	pad_device->cap_err = 0;
	return 0;
}

static int pad_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
	u8 count;
	switch (psp) {
		case POWER_SUPPLY_PROP_PRESENT:
		case POWER_SUPPLY_PROP_HEALTH:
			if (pad_get_health(psp, val))
				return -EINVAL;
			break;

		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
			break;

		case POWER_SUPPLY_PROP_CAPACITY:
			if (pad_get_capacity(val))
				return -EINVAL;
			break;

		case POWER_SUPPLY_PROP_STATUS:
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		case POWER_SUPPLY_PROP_CURRENT_NOW:
		case POWER_SUPPLY_PROP_TEMP:
		case POWER_SUPPLY_PROP_TIME_TO_EMPTY_AVG:
		case POWER_SUPPLY_PROP_TIME_TO_FULL_AVG:
		case POWER_SUPPLY_PROP_SERIAL_NUMBER:
			for (count = 0; count < REG_MAX; count++) {
				if (psp == pad_data[count].psp)
					break;
			}
			
			if (pad_get_psp(count, psp, val))
				return -EINVAL;
			break;

		default:
			dev_err(&pad_device->client->dev,
				"%s: INVALID property psp=%u\n", __func__,psp);
			return -EINVAL;
	}
	return 0;
}

void config_thermal_power(void)
{
	int ret;

	ret = gpio_request(TEGRA_GPIO_PU3, "thermal_power_u3");
	if (ret < 0)
		 pr_err("%s: gpio_request failed for gpio %s\n", __func__, "TEGRA_GPIO_PU3");
	gpio_direction_output(TEGRA_GPIO_PU3, 1);
	pr_info("gpio %d set to %d\n", TEGRA_GPIO_PU3, gpio_get_value(TEGRA_GPIO_PU3));
}

static int pad_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc;
	int i = 0;

	printk("pad_probe: client->addr=%x \n", client->addr);
	pad_device = kzalloc(sizeof(*pad_device), GFP_KERNEL);

	if (!pad_device) {
		return -ENOMEM;
	}

	memset(pad_device, 0, sizeof(*pad_device));
	pad_device->client = client;
	i2c_set_clientdata(client, pad_device);
    pad_device->smbus_status = 0;
    pad_device->cap_err = 0;
    pad_device->temp_err = 0;
	pad_device->old_capacity = 0xFF;
    pad_device->old_temperature = 0xFF;
	pad_device->low_battery_present = 0;
	pad_device->gpio_battery_detect = GPIOPIN_BATTERY_DETECT;

	for (i = 0; i < ARRAY_SIZE(pad_supply); i++) {
		rc = power_supply_register(&client->dev, &pad_supply[i]);
		if (rc) {
			pr_err("Failed to register power supply\n");
			while (i--)
			power_supply_unregister(&pad_supply[i]);
			kfree(pad_device);
			return rc;
		}
	}

	dev_info(&pad_device->client->dev,"%s: battery driver registered\n", client->name);
    spin_lock_init(&pad_device->lock);
	INIT_DELAYED_WORK(&pad_device->status_poll_work, battery_status_poll) ;
	INIT_DELAYED_WORK(&pad_device->low_low_bat_work, low_low_battery_check) ;
    battery_work_queue = create_singlethread_workqueue("battery_workqueue");
	setup_timer(&pad_device->charger_pad_dock_detect_timer, charger_pad_dock_detection, 0);

	/* Register sysfs hooks */
	if (sysfs_create_group(&client->dev.kobj, &battery_smbus_group )) {
		dev_err(&client->dev, "Not able to create the sysfs\n");
	}

	/* Register sysfs */
	if(sysfs_create_group(&client->dev.kobj, &battery_charger_group)) {
		dev_err(&client->dev, "pad_battery_probe: unable to create battery_group sysfs\n");
	}

	init_docking_charging_irq();
	battery_cable_status = get_usb_cable_status();
	cancel_delayed_work(&pad_device->status_poll_work);
	setup_detect_irq();
	setup_low_battery_irq();
	wake_lock_init(&pad_device->low_battery_wake_lock, WAKE_LOCK_SUSPEND, "low_battery_detection");
	wake_lock_init(&pad_device->cable_event_wake_lock, WAKE_LOCK_SUSPEND, "battery_cable_event");
	battery_driver_ready = 1;

	if(pad_device->battery_present)
		queue_delayed_work(battery_work_queue, &pad_device->status_poll_work, 15*HZ);
	if( tegra3_get_project_id() == TEGRA3_PROJECT_TF201)
		config_thermal_power();
	return 0;
}

static int pad_remove(struct i2c_client *client)
{
	struct pad_device_info *pad_device;
    int i = 0;
	
	pad_device = i2c_get_clientdata(client);
	del_timer_sync(&pad_device->charger_pad_dock_detect_timer);
	for (i = 0; i < ARRAY_SIZE(pad_supply); i++) {
		power_supply_unregister(&pad_supply[i]);
	}

	if (pad_device) {
		wake_lock_destroy(&pad_device->low_battery_wake_lock);
		kfree(pad_device);
		pad_device = NULL;
	}

	return 0;
}

static int pad_suspend(struct device *dev)
{
	cancel_delayed_work_sync(&pad_device->status_poll_work);
	del_timer_sync(&pad_device->charger_pad_dock_detect_timer);
    flush_workqueue(battery_work_queue);

	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201)
		gpio_direction_output(TEGRA_GPIO_PU3, 0);
	return 0;
}

/* any smbus transaction will wake up pad */

static int pad_resume(struct device *dev)
{
	pad_device->battery_present =! (gpio_get_value(pad_device->gpio_battery_detect));
	cancel_delayed_work(&pad_device->status_poll_work);
	queue_delayed_work(battery_work_queue, &pad_device->status_poll_work, 5*HZ);

	if(tegra3_get_project_id() == TEGRA3_PROJECT_TF201)
		gpio_direction_output(TEGRA_GPIO_PU3, 1);
	return 0;
}

static const struct i2c_device_id pad_id[] = {
	{ "pad-battery", 0 },
	{},
};

static const struct dev_pm_ops pad_pm_ops = {
	.suspend	= pad_suspend,
	.resume		= pad_resume,
};

static struct i2c_driver pad_battery_driver = {
	.probe		= pad_probe,
	.remove 	= pad_remove,
	.id_table	= pad_id,
	.driver = {
		.name	= "pad-battery",
#ifdef CONFIG_PM
		.pm		= &pad_pm_ops,
#endif
	},
};
static int __init pad_battery_init(void)
{
	return i2c_add_driver(&pad_battery_driver);
}
module_init(pad_battery_init);

static void __exit pad_battery_exit(void)
{
	i2c_del_driver(&pad_battery_driver);
}
module_exit(pad_battery_exit);

MODULE_AUTHOR("NVIDIA Corporation");
MODULE_DESCRIPTION("PAD battery monitor driver");
MODULE_LICENSE("GPL");
