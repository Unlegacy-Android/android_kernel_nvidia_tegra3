/*
 * An I2C driver for SMSC Proximity Sensor CAP1106.
 *
 * Copyright (c) 2012, ASUSTek Corporation.
 * Copyright (c) 2016, André Pinela
 *
 */

#define DEBUG 1

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <linux/gpio.h>
#include <linux/input/cap1106.h>

struct cap1106_data {
        struct attribute_group attrs;
        struct i2c_client *client;
        struct delayed_work work;
        struct delayed_work checking_work;
        int enable;
        int obj_detect;
        int overflow_status;
        int force_enable;
        int gpio;
};

static int cap1106_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int cap1106_remove(struct i2c_client *client);
static int cap1106_suspend(struct device *dev);
static int cap1106_resume(struct device *dev);
static s32 cap1106_read_reg(struct i2c_client *client, u8 command);
static s32 cap1106_write_reg(struct i2c_client *client, u8 command, u8 value);
static irqreturn_t cap1106_interrupt_handler(int irq, void *dev);
static void cap1106_work_function(struct delayed_work *work);
static int cap1106_init_sensor(struct i2c_client *client);
static int cap1106_config_irq(struct i2c_client *client);
static void cap1106_enable_sensor(struct i2c_client *client, int enable);

static DEFINE_MUTEX(prox_mtx);
static struct cap1106_data *prox_data;
static struct workqueue_struct *prox_wq;
static struct switch_dev prox_sdev;
static long checking_work_period = 100; //default (ms)
static int is_wood_sensitivity = 0;
static int prev_c2_status = 0;
static int prev_c6_status = 0;
static int c2_acc_cnt = 0;
static int c6_acc_cnt = 0;
static int acc_limit = 10;

static ssize_t show_obj_detect(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                ret = sprintf(buf, "%d\n", data->obj_detect);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t show_sensitivity(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x1F);
                ret = sprintf(buf, "%02X\n", value);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t store_sensitivity(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        long value;

        if (strict_strtol(buf, 16, &value))
                return -EINVAL;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                cap1106_write_reg(client, 0x1F, value & 0x7F);
        }
        mutex_unlock(&prox_mtx);

        return strnlen(buf, count);
}

static ssize_t show_sensor_gain(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x00);
                ret = sprintf(buf, "%02X\n", (value & 0xC0) >> 6);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t store_sensor_gain(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        long gain_value;
        long reg_value;

        if (strict_strtol(buf, 16, &gain_value))
                return -EINVAL;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                reg_value = cap1106_read_reg(client, 0x00);
                cap1106_write_reg(client, 0x00, (reg_value & 0x3F) | ((gain_value & 0x3) << 6));
        }
        mutex_unlock(&prox_mtx);

        return strnlen(buf, count);
}

static ssize_t show_sensor_input_2_delta_count(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x11);
                ret = sprintf(buf, "%02X\n", value);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t show_sensor_input_2_th(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x31);
                ret = sprintf(buf, "%02X\n", value);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t store_sensor_input_2_th(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        long value;

        if (strict_strtol(buf, 16, &value))
                return -EINVAL;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                cap1106_write_reg(client, 0x31, value & 0x7F);
        }
        mutex_unlock(&prox_mtx);

        return strnlen(buf, count);
}

static ssize_t show_sensor_input_6_delta_count(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x15);
                ret = sprintf(buf, "%02X\n", value);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t show_sensor_input_6_th(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x35);
                ret = sprintf(buf, "%02X\n", value);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t store_sensor_input_6_th(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        long value;

        if (strict_strtol(buf, 16, &value))
                return -EINVAL;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                cap1106_write_reg(client, 0x35, value & 0x7F);
        }
        mutex_unlock(&prox_mtx);

        return strnlen(buf, count);
}

static ssize_t show_sensor_input_noise_th(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x38);
                ret = sprintf(buf, "%02X\n", value & 0x3);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t store_sensor_input_noise_th(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        long value;

        if (strict_strtol(buf, 16, &value))
                return -EINVAL;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                cap1106_write_reg(client, 0x38, value & 0x3);
        }
        mutex_unlock(&prox_mtx);

        return strnlen(buf, count);
}

static ssize_t show_sensor_input_status(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x03);
                ret = sprintf(buf, "%02X\n", value);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t show_sensing_cycle(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x24);
                ret = sprintf(buf, "%02X\n", value);
        } else {
                ret = sprintf(buf, "-1\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t store_sensing_cycle(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        long value;

        if (strict_strtol(buf, 16, &value))
                return -EINVAL;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                cap1106_write_reg(client, 0x24, value & 0x7F);
        }
        mutex_unlock(&prox_mtx);

        return strnlen(buf, count);
}

static ssize_t show_sensor_onoff(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int ret = 0;

        mutex_lock(&prox_mtx);
        ret = sprintf(buf, "%d\n", data->enable);
        mutex_unlock(&prox_mtx);
        return ret;
}

static ssize_t store_sensor_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        long enable;

        if (strict_strtol(buf, 10, &enable))
                return -EINVAL;

        if ((enable != 1) && (enable != 0))
                return -EINVAL;

        mutex_lock(&prox_mtx);
        data->force_enable = enable;
        cap1106_enable_sensor(client, enable);
        mutex_unlock(&prox_mtx);

        return strnlen(buf, count);
}

static ssize_t show_sensor_recal(struct device *dev, struct device_attribute *devattr, char *buf)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);
        int value;
        int ret = 0;

        mutex_lock(&prox_mtx);
        if (data->enable) {
                value = cap1106_read_reg(client, 0x26);
                ret = sprintf(buf, value == 0x0 ? "OK\n" : "FAIL\n");
        } else {
                ret = sprintf(buf, "FAIL\n");
        }
        mutex_unlock(&prox_mtx);

        return ret;
}

static ssize_t store_sensor_recal(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);

        mutex_lock(&prox_mtx);
        if (data->enable) {
                cap1106_write_reg(client, 0x26, 0x22);
        }
        mutex_unlock(&prox_mtx);

        return strnlen(buf, count);
}

DEVICE_ATTR(obj_detect, 0644, show_obj_detect, NULL);
DEVICE_ATTR(sensitivity, 0644, show_sensitivity, store_sensitivity);
DEVICE_ATTR(sensor_gain, 0644, show_sensor_gain, store_sensor_gain);
DEVICE_ATTR(sensor_input_2_delta_count, 0644, show_sensor_input_2_delta_count, NULL);
DEVICE_ATTR(sensor_input_2_th, 0644, show_sensor_input_2_th, store_sensor_input_2_th);
DEVICE_ATTR(sensor_input_6_delta_count, 0644, show_sensor_input_6_delta_count, NULL);
DEVICE_ATTR(sensor_input_6_th, 0644, show_sensor_input_6_th, store_sensor_input_6_th);
DEVICE_ATTR(sensor_input_noise_th, 0644, show_sensor_input_noise_th, store_sensor_input_noise_th);
DEVICE_ATTR(sensor_input_status, 0644, show_sensor_input_status, NULL);
DEVICE_ATTR(sensing_cycle, 0644, show_sensing_cycle, store_sensing_cycle);
DEVICE_ATTR(sensor_onoff, 0644, show_sensor_onoff, store_sensor_onoff);
DEVICE_ATTR(sensor_recal, 0644, show_sensor_recal, store_sensor_recal);

static struct attribute *cap1106_attr[] = {
                &dev_attr_obj_detect.attr,
                &dev_attr_sensitivity.attr,
                &dev_attr_sensor_gain.attr,
                &dev_attr_sensor_input_2_delta_count.attr,
                &dev_attr_sensor_input_2_th.attr,
                &dev_attr_sensor_input_6_delta_count.attr,
                &dev_attr_sensor_input_6_th.attr,
                &dev_attr_sensor_input_noise_th.attr,
                &dev_attr_sensor_input_status.attr,
                &dev_attr_sensing_cycle.attr,
                &dev_attr_sensor_onoff.attr,
                &dev_attr_sensor_recal.attr,
                NULL
};

static ssize_t print_prox_name(struct switch_dev *sdev, char *buf)
{
        return sprintf(buf, "%s\n", "prox_sar_det");
}

static ssize_t print_prox_state(struct switch_dev *sdev, char *buf)
{
        int state = -1;
        if (switch_get_state(sdev))
                state = 1;
        else
                state = 0;

        return sprintf(buf, "%d\n", state);
}

static void dump_registers(struct i2c_client *client)
{
        int value;
        value = cap1106_read_reg(client, 0x00);
        dev_dbg(&client->dev, "=== Main Control(0x00) is %x\n", value);
        value = cap1106_read_reg(client, 0x02);
        dev_dbg(&client->dev, "=== Genaral Status(0x02) is %x\n", value);
        value = cap1106_read_reg(client, 0x03);
        dev_dbg(&client->dev, "=== Sensor Input Status(0x03) is %x\n", value);
        value = cap1106_read_reg(client, 0x0A);
        dev_dbg(&client->dev, "=== Noise Flag Status(0x0A) is %x\n", value);
        value = cap1106_read_reg(client, 0x21);
        dev_dbg(&client->dev, "=== Sensor Input Enable Register(0x21) is %x\n", value);
        value = cap1106_read_reg(client, 0x44);
        dev_dbg(&client->dev, "=== configuration 2(0x44) is %x\n", value);
        value = cap1106_read_reg(client, 0xFD);
        dev_dbg(&client->dev, "=== Product ID(0xFD) is %x\n", value);
        value = cap1106_read_reg(client, 0xFE);
        dev_dbg(&client->dev, "=== Manufacturer ID(0xFE) is %x\n", value);
        value = cap1106_read_reg(client, 0xFF);
        dev_dbg(&client->dev, "=== Revision (0xFF) is %x\n", value);
}

static void cap1106_enable_sensor(struct i2c_client *client, int enable)
{
        long reg_value;
        //long status;

        struct cap1106_data *data = i2c_get_clientdata(client);

        if (data->enable != enable) {
                reg_value = cap1106_read_reg(client, 0x00);
                if (enable) {
                        cap1106_write_reg(client, 0x00, (reg_value & 0xEF) | (!enable << 4));
                        // Time to first conversion is 200ms (Max)
                        queue_delayed_work(prox_wq, &data->work, msecs_to_jiffies(200));
                        enable_irq(client->irq);
                        queue_delayed_work(prox_wq, &prox_data->checking_work, checking_work_period);
                } else {
                        disable_irq(client->irq);
                        cancel_delayed_work_sync(&data->work);
                        cancel_delayed_work_sync(&data->checking_work);
                        flush_workqueue(prox_wq);
                        switch_set_state(&prox_sdev, 0);
                        cap1106_write_reg(client, 0x00, (reg_value & 0xEF) | (!enable << 4));
                }
                data->enable = enable;
                dev_dbg(&client->dev, "sensor enabled state changed to: %d", enable);
        }
}

static s32 cap1106_read_reg(struct i2c_client *client, u8 command)
{
        return i2c_smbus_read_byte_data(client, command);
}

static s32 cap1106_write_reg(struct i2c_client *client, u8 command, u8 value)
{
        return i2c_smbus_write_byte_data(client, command, value);
}

static void cap1106_work_function(struct delayed_work *work)
{
        int status;
        int value_delta_2,value_delta_6;
        int bc2, bc6;
        struct cap1106_data *data = container_of(work, struct cap1106_data, work);

        disable_irq(data->client->irq);
        cap1106_write_reg(data->client, 0x00, 0x80); // Clear INT and Set Gain to MAX
        status = cap1106_read_reg(data->client, 0x03);
        value_delta_2 = cap1106_read_reg(prox_data->client, 0x11);
        value_delta_6 = cap1106_read_reg(prox_data->client, 0x15);
        bc2 = cap1106_read_reg(prox_data->client, 0x51);
        bc6 = cap1106_read_reg(prox_data->client, 0x55);
        dev_dbg(&data->client->dev, "status: 0x%02X, BC2=0x%02X, D2=0x%02X, BC6=0x%02X, D6=0x%02X\n", status, bc2, value_delta_2, bc6, value_delta_6);
        if (is_wood_sensitivity == 0) {
                data->obj_detect = ((status == 0x2) || (status == 0x20) || (status == 0x22));
                switch_set_state(&prox_sdev, data->obj_detect);
                if ((status == 0x2 && value_delta_2 == 0x7F)
                                || (status == 0x20 && value_delta_6 == 0x7F)
                                || (status == 0x22 && (value_delta_2 == 0x7F || value_delta_6 == 0x7F))) {
                        dev_dbg(&data->client->dev, "set to wood sensitivity------>\n");
                        //set sensitivity and threshold for wood touch
                        cap1106_write_reg(prox_data->client, 0x1f, 0x4f);
                        cap1106_write_reg(prox_data->client, 0x31, 0x50);
                        cap1106_write_reg(prox_data->client, 0x35, 0x50);
                        is_wood_sensitivity = 1;
                        data->overflow_status = status;
                        c2_acc_cnt = 0;
                        c6_acc_cnt = 0;
                } else {
                        if (value_delta_2 >= 0x08 && value_delta_2 <= 0x3F)
                                c2_acc_cnt++;
                        if (value_delta_6 >= 0x0a && value_delta_6 <= 0x3F)
                                c6_acc_cnt++;

                        dev_dbg(&data->client->dev, "c2_acc_cnt=%d, c6_acc_cnt=%d\n", c2_acc_cnt, c6_acc_cnt);
                        if (c2_acc_cnt >= acc_limit || c6_acc_cnt >= acc_limit) {
                                dev_dbg(&data->client->dev, "+++ FORCE RECALIBRATION +++\n");
                                cap1106_write_reg(data->client, 0x26, 0x22);
                                c2_acc_cnt = 0;
                                c6_acc_cnt = 0;
                        }
                }
                prev_c2_status = (status & 0x02);
                prev_c6_status = (status & 0x20);
        }
        enable_irq(data->client->irq);
}

static irqreturn_t cap1106_interrupt_handler(int irq, void *dev)
{
        struct cap1106_data *data = i2c_get_clientdata(dev);

        queue_delayed_work(prox_wq, &data->work, 0);

        return IRQ_HANDLED;
}

static int cap1106_config_irq(struct i2c_client *client)
{
        struct cap1106_data *data = i2c_get_clientdata(client);
        int rc = 0 ;

        rc = gpio_request(data->gpio, CAP1106_GPIO_NAME);
        if (rc < 0) {
                dev_err(&client->dev, "%s: gpio_request failed %d\n", __func__, rc);
                goto err_gpio_request_failed;
        }

        rc = gpio_direction_input(data->gpio);
        if (rc < 0) {
                dev_err(&client->dev, "%s: gpio_direction_input failed %d\n", __func__, rc);
                goto err_gpio_direction_input_failed;
        }

        client->irq = gpio_to_irq(data->gpio);
        if (&client->irq < 0) {
                rc = client->irq;
                dev_err(&client->dev, "%s: gpio_to_irq failed %d\n", __func__, client->irq);
                goto err_gpio_to_irq_failed;
        }

        rc = request_irq(client->irq, cap1106_interrupt_handler, IRQF_TRIGGER_FALLING, CAP1106_IRQ_NAME, client);
        if(rc){
                dev_err(&client->dev, "Could not register for %s interrupt, irq = %d, rc = %d\n", CAP1106_IRQ_NAME, client->irq, rc);
                goto err_gpio_request_irq_failed;
        }

        dev_info(&client->dev, "interrupt configuration done, irq=%d\n, ", client->irq);

        dump_registers(client);

        return 0 ;
        err_gpio_request_irq_failed:
        err_gpio_to_irq_failed:
        err_gpio_direction_input_failed:
        gpio_free(data->gpio);
        err_gpio_request_failed:
        return rc;
}

static int cap1106_init_sensor(struct i2c_client *client)
{
        struct cap1106_data *data = i2c_get_clientdata(client);
        u8 bIdx;
        int rc = 0;
        const u8 InitTable[] = {
                        0x1f, 0x1f, // Data sensitivity (need to be fine tune for real system).
                        0x20, 0x20, // MAX duration disable
                        0x21, 0x22, // Enable CS2+CS6.
                        0x22, 0xff, // MAX duration time to max , repeat period time to max
                        0x24, 0x39, // digital count update time to 140*64ms
                        0x27, 0x22, // Enable INT. for CS2+CS6.
                        0x28, 0x22, // disable repeat irq
                        0x2a, 0x00, // all channel run in the same time
                        0x31, 0x08, // Threshold of CS 2 (need to be fine tune for real system).
                        0x35, 0x0a, // Threshold of CS 6 (need to be fine tune for real system).
                        0x26, 0x22, // force re-cal
                        0x00, 0x00, // Reset INT. bit.
        };

        dev_dbg(&client->dev, "client->name: %s, client->addr: 0x%X\n", client->name, client->addr);

        for (bIdx = 0; bIdx < sizeof(InitTable) / sizeof(InitTable[0]); bIdx += 2) {
                if ((rc = cap1106_write_reg(client, InitTable[bIdx],
                                InitTable[bIdx + 1]))) {
                        dev_err(&client->dev, "=== Write Error, rc=0x%X\n", rc);
                        break;
                }
        }

        dev_info(&client->dev, "proximity SAR 3G detection sensor initialized (gpio value=%d)\n", gpio_get_value(data->gpio));

        dump_registers(client);

        return rc;
}

static void cap1106_checking_work_function(struct delayed_work *work) {
        struct cap1106_data *data = container_of(work, struct cap1106_data, work);
        int status;
        int value_delta_2;
        int value_delta_6;
        int bc2, bc6;

        if (is_wood_sensitivity == 1){
                mutex_lock(&prox_mtx);
                if (prox_data->enable) {
                        status = cap1106_read_reg(prox_data->client, 0x03);
                        value_delta_2 = cap1106_read_reg(prox_data->client, 0x11);
                        value_delta_6 = cap1106_read_reg(prox_data->client, 0x15);
                        bc2 = cap1106_read_reg(prox_data->client, 0x51);
                        bc6 = cap1106_read_reg(prox_data->client, 0x55);
                        dev_dbg(&data->client->dev, "status: 0x%02X, BC2=0x%02X, D2=0x%02X, BC6=0x%02X, D6=0x%02X\n", status, bc2, value_delta_2, bc6, value_delta_6);
                        if ((value_delta_2 == 0x00 && value_delta_6 == 0x00)
                                        || (value_delta_2 == 0xFF && value_delta_6 == 0xFF)
                                        || (value_delta_2 == 0x00 && value_delta_6 == 0xFF)
                                        || (value_delta_2 == 0xFF && value_delta_6 == 0x00)
                                        || (prox_data->overflow_status == 0x2 && (value_delta_2 > 0x50) && (value_delta_2 <= 0x7F))
                                        || (prox_data->overflow_status == 0x20 && (value_delta_6 > 0x50) && (value_delta_6 <= 0x7F))
                                        || (prox_data->overflow_status == 0x22 && (((value_delta_2 > 0x50) && (value_delta_2 <= 0x7F))
                                                        || ((value_delta_6 > 0x50) && (value_delta_6 <= 0x7F))))) {
                                dev_dbg(&data->client->dev, "unset is_wood_sensitivity to 0\n");
                                //set sensitivity and threshold for 2cm body distance
                                cap1106_write_reg(prox_data->client, 0x1f, 0x1f);
                                cap1106_write_reg(prox_data->client, 0x31, 0x08);
                                cap1106_write_reg(prox_data->client, 0x35, 0x0a);
                                is_wood_sensitivity = 0;
                                queue_delayed_work(prox_wq, &prox_data->work, 0);
                        }
                } else {
                        dev_dbg(&data->client->dev, "delta 2 = -1\n");
                }
                mutex_unlock(&prox_mtx);
        }
        queue_delayed_work(prox_wq, &prox_data->checking_work, checking_work_period);
}

static int cap1106_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

        struct cap1106_i2c_platform_data *pdata;
        struct device *dev = &client->dev;
        int rc = 0;

        pdata = dev->platform_data;
        if (!pdata)
                return -EINVAL;

        prox_data = kzalloc(sizeof(struct cap1106_data), GFP_KERNEL);
        if (!prox_data)
                return -ENOMEM;

        prox_wq = create_singlethread_workqueue("prox_wq");
        if(!prox_wq)
                return -ENOMEM;

        prox_data->client = client;

        /* Touch data processing workqueue initialization */
        INIT_DELAYED_WORK(&prox_data->work, cap1106_work_function);
        INIT_DELAYED_WORK(&prox_data->checking_work, cap1106_checking_work_function);

        i2c_set_clientdata(client, prox_data);
        prox_data->client->flags = 0;
        strlcpy(prox_data->client->name, CAP1106_NAME, I2C_NAME_SIZE);
        prox_data->enable = 0;
        prox_data->force_enable = 1;
        prox_data->gpio = pdata->irq_gpio;

        rc = cap1106_init_sensor(prox_data->client);
        if (rc) {
                dev_err(&client->dev, "sensor initialization failed!\n");
                goto err_init_sensor_failed;
        }

        prox_data->attrs.attrs = cap1106_attr;
        rc = sysfs_create_group(&prox_data->client->dev.kobj, &prox_data->attrs);
        if (rc) {
                dev_err(&client->dev, "create the sysfs group failed!\n");
                goto err_create_sysfs_group_failed;
        }

        /* register switch class */
        prox_sdev.name = NAME_RIL_PROX;
        prox_sdev.print_name = print_prox_name;
        prox_sdev.print_state = print_prox_state;

        rc = switch_dev_register(&prox_sdev);

        if (rc) {
                dev_err(&client->dev, "switch device registration failed!\n");
                goto err_register_switch_class_failed;
        }

        rc = cap1106_config_irq(prox_data->client);
        if (rc) {
                dev_err(&client->dev, "sensor interrupt configuration failed!\n");
                goto err_config_irq_failed;
        }

        prox_data->enable = 1;
        prox_data->overflow_status = 0x0;
        queue_delayed_work(prox_wq, &prox_data->work, msecs_to_jiffies(200));
        queue_delayed_work(prox_wq, &prox_data->checking_work, checking_work_period);

        dev_info(&client->dev, "probed\n");

        return 0;

        err_config_irq_failed:
        err_register_switch_class_failed:
        sysfs_remove_group(&prox_data->client->dev.kobj, &prox_data->attrs);
        err_create_sysfs_group_failed:
        err_init_sensor_failed:
        kfree(prox_data);
        destroy_workqueue(prox_wq);
        return rc;
}

static int cap1106_remove(struct i2c_client *client)
{
        switch_dev_unregister(&prox_sdev);

        sysfs_remove_group(&client->dev.kobj, &prox_data->attrs);

        free_irq(client->irq, client);

        kfree(prox_data);

        if (prox_wq)
                destroy_workqueue(prox_wq);

        return 0;
}

#ifdef CONFIG_PM
static int cap1106_suspend(struct device *dev)
{
        struct i2c_client *client = to_i2c_client(dev);

        mutex_lock(&prox_mtx);

        cap1106_enable_sensor(client, 0);

        mutex_unlock(&prox_mtx);

        return 0;
}

static int cap1106_resume(struct device *dev)
{
        struct i2c_client *client = to_i2c_client(dev);
        struct cap1106_data *data = i2c_get_clientdata(client);

        mutex_lock(&prox_mtx);

        if (data->force_enable)
                cap1106_enable_sensor(client, 1);

        mutex_unlock(&prox_mtx);

        return 0;
}

static const struct dev_pm_ops cap1106_pm = {
                .suspend = cap1106_suspend,
                .resume = cap1106_resume,
};
#endif

static const struct i2c_device_id cap1106_id[] = {
                { CAP1106_NAME, 0 },
                {}
};
MODULE_DEVICE_TABLE(i2c, cap1106_id);

static struct i2c_driver cap1106_driver = {
                .driver = {
                                .name   = CAP1106_NAME,
                                .owner  = THIS_MODULE,
#ifdef CONFIG_PM
                                .pm     = &cap1106_pm,
#endif
                },
                .probe		= cap1106_probe,
                .remove		= __devexit_p(cap1106_remove),
                .id_table	= cap1106_id,
};

module_i2c_driver(cap1106_driver);

MODULE_DESCRIPTION("SMSC Proximity Sensor CAP1106 Driver");
MODULE_LICENSE("GPL");
