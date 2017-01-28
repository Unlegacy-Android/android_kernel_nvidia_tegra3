#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <mach/pinmux.h>
#include <mach/pinmux-tegra30.h>
#include "acer_a1026.h"

#if 0
#define ACER_DBG(fmt, arg...) printk(KERN_INFO "[a1026]: %s " fmt "\n", __FUNCTION__, ## arg)
#else
#define ACER_DBG(fmt, arg...) do {} while (0)
#endif

#define DEBUG 0

static int execute_cmdmsg(unsigned int);
static ssize_t config_wakeup_gpio(void);
static void es305_set_uart_tx(int on);
static ssize_t a1026_suspend_es305(void);
int a1026_i2c_read(char *rxData, int length);
int a1026_i2c_write(char *txData, int length);
ssize_t chk_wakeup_a1026(void);

static struct mutex a1026_lock;
static int a1026_opened;
static int a1026_suspended;
static int a1026_config_wakeup_gpio = 0;
static int control_a1026_clk;
static int a1026_current_config = A1026_TABLE_SUSPEND;

static struct a1026_data {
	struct i2c_client *client;
	struct a1026_platform_data *pdata;
} a1026_data;

struct vp_ctxt {
	unsigned char *data;
	unsigned int img_size;
};

#define DEFAULT_PINMUX(_pingroup, _mux, _pupd, _tri, _io)	\
	{							\
		.pingroup	= TEGRA_PINGROUP_##_pingroup,	\
		.func		= TEGRA_MUX_##_mux,		\
		.pupd		= TEGRA_PUPD_##_pupd,		\
		.tristate	= TEGRA_TRI_##_tri,		\
		.io		= TEGRA_PIN_##_io,		\
		.lock		= TEGRA_PIN_LOCK_DEFAULT,	\
		.od		= TEGRA_PIN_OD_DEFAULT,		\
		.ioreset	= TEGRA_PIN_IO_RESET_DEFAULT,	\
	}

static struct tegra_pingroup_config ES_305_UART_MODE[] = {
	DEFAULT_PINMUX(ULPI_CLK,        UARTD,           NORMAL,    NORMAL,     OUTPUT),
};

static struct tegra_pingroup_config ES_305_GPIO_MODE[] = {
	DEFAULT_PINMUX(ULPI_CLK,        RSVD,            PULL_UP,    NORMAL,     OUTPUT),
};

#define AUDIENCE_A1026_NAME "audience_a1026"

extern void acer_set_bypass_switch(int state);

int a1026_i2c_read(char *rxData, int length)
{
	int rc;
	struct i2c_msg msgs[] = {
		{
			.addr = a1026_data.client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = rxData,
		},
	};

	rc = i2c_transfer(a1026_data.client->adapter, msgs, 1);
	if (rc < 0) {
		pr_err("%s: transfer error %d\n", __func__, rc);
		return rc;
	}

#if DEBUG
	{
		int i;
		for (i = 0; i < length; i++)
			pr_info("%s: rx[%d] = %2x\n", __func__, i, rxData[i]);
	}
#endif

	return 0;
}

int a1026_i2c_write(char *txData, int length)
{
	int rc;
	struct i2c_msg msg[] = {
		{
			.addr = a1026_data.client->addr,
			.flags = 0,
			.len = length,
			.buf = txData,
		},
	};

	rc = i2c_transfer(a1026_data.client->adapter, msg, 1);
	if (rc < 0) {
		pr_err("%s: transfer error %d\n", __func__, rc);
		return rc;
	}

#if DEBUG
	{
		int i;
		for (i = 0; i < length; i++)
			pr_info("%s: rx[%d] = %2x\n", __func__, i, txData[i]);
	}
#endif

	return 0;
}

static void a1026_i2c_sw_reset(unsigned int reset_cmd)
{
	int rc = 0;
	unsigned char msgbuf[4];

	msgbuf[0] = (reset_cmd >> 24) & 0xFF;
	msgbuf[1] = (reset_cmd >> 16) & 0xFF;
	msgbuf[2] = (reset_cmd >> 8) & 0xFF;
	msgbuf[3] = reset_cmd & 0xFF;

	pr_info("%s: %08x\n", __func__, reset_cmd);

	rc = a1026_i2c_write(msgbuf, 4);
	if (!rc)
		msleep(20);
}

static ssize_t a1026_bootup_init(struct file *file, struct a1026img *img)
{
	struct vp_ctxt vp;
	int rc, pass = 0;
	int remaining;
	int retry = RETRY_CNT;
	unsigned char *index;
	char buf[2];

	if (img->img_size > A1026_MAX_FW_SIZE) {
		pr_err("%s: invalid a1026 image size %d\n", __func__,
			img->img_size);
		return -EINVAL;
	}

	vp.data = kmalloc(img->img_size, GFP_KERNEL);

	if (!vp.data) {
		pr_err("%s: out of memory\n", __func__);
		return -ENOMEM;
	}

	vp.img_size = img->img_size;
	if (copy_from_user(vp.data, img->buf, img->img_size)) {
		pr_err("%s: copy from user failed\n", __func__);
		kfree(vp.data);
		return -EFAULT;
	}

	while (retry--) {
		/* Enable A1026 clock */
		if (control_a1026_clk)
			gpio_set_value(a1026_data.pdata->gpio_a1026_clk, 1);
		mdelay(10);

		/* Reset A1026 chip */
		gpio_set_value(a1026_data.pdata->gpio_a1026_reset, 0);
		msleep(1);

		/* Take out of reset */
		gpio_set_value(a1026_data.pdata->gpio_a1026_reset, 1);

		msleep(50); /* Delay before send I2C command */

		/* Boot Cmd to A1026 */
		buf[0] = A1026_msg_BOOT >> 8;
		buf[1] = A1026_msg_BOOT & 0xff;

		rc = a1026_i2c_write(buf, 2);
		if (rc < 0) {
			pr_err("%s: set boot mode error (%d retries left)\n",
				__func__, retry);
			continue;
		}

		mdelay(1); /* use polling */
		rc = a1026_i2c_read(buf, 1);
		if (rc < 0) {
			pr_err("%s: boot mode ack error (%d retries left)\n",
				__func__, retry);
			continue;
		}

		if (buf[0] != A1026_msg_BOOT_ACK) {
			pr_err("%s: not a boot-mode ack (%d retries left)\n",
				__func__, retry);
			continue;
		}

		remaining = vp.img_size / 32;
		index = vp.data;

		pr_info("%s: starting to load image (%d passes)...\n",
			__func__,
			remaining + !!(vp.img_size % 32));

		for (; remaining; remaining--, index += 32) {
			rc = a1026_i2c_write(index, 32);
			if (rc < 0)
				break;
		}

		if (rc >= 0 && vp.img_size % 32)
			rc = a1026_i2c_write(index, vp.img_size % 32);

		if (rc < 0) {
			pr_err("%s: fw load error %d (%d retries left)\n",
				__func__, rc, retry);
			continue;
		}

		msleep(80); /* Delay time before issue a Sync Cmd */

		pr_info("%s: firmware loaded successfully\n", __func__);

		rc = execute_cmdmsg(A1026_msg_Sync);
		if (rc < 0) {
			pr_err("%s: sync command error %d (%d retries left)\n",
				__func__, rc, retry);
			continue;
		}

		pass = 1;
		break;
	}

	rc = config_wakeup_gpio();
	msleep(10);

	if (!a1026_suspend_es305())
		pr_info("%s: a1026 suspend command okay \n", __func__);
	else {
		pr_err("%s: suspend procedure error!!! \n", __func__);
	}

	if (pass && !rc)
		pr_info("%s: initialized!\n", __func__);
	else
		pr_err("%s: initialization failed\n", __func__);

	kfree(vp.data);
	return rc;
}

int execute_cmdmsg(unsigned int msg)
{
	int rc = 0;
	int retries, pass = 0;
	unsigned char msgbuf[4];
	unsigned char chkbuf[4];
	unsigned int sw_reset = 0;

	sw_reset = ((A1026_msg_Reset << 16) | RESET_IMMEDIATE);

	msgbuf[0] = (msg >> 24) & 0xFF;
	msgbuf[1] = (msg >> 16) & 0xFF;
	msgbuf[2] = (msg >> 8) & 0xFF;
	msgbuf[3] = msg & 0xFF;

	memcpy(chkbuf, msgbuf, 4);

	rc = a1026_i2c_write(msgbuf, 4);
	if (rc < 0) {
		pr_err("%s: error %d\n", __func__, rc);
		a1026_i2c_sw_reset(sw_reset);
		return rc;
	}

	/* We don't need to get Ack after sending out a suspend command */
	if (msg == A1026_msg_Sleep)
		return rc;

	retries = POLLING_RETRY_CNT;
	while (retries--) {
		rc = 0;

		msleep(20); /* use polling */
		memset(msgbuf, 0, sizeof(msgbuf));

		rc = a1026_i2c_read(msgbuf, 4);

		if (rc < 0) {
			pr_err("%s: ack-read error %d (%d retries)\n", __func__,
				rc, retries);
			continue;
		}

		if (msgbuf[0] == chkbuf[0]  && msgbuf[1] == chkbuf[1] &&
			msgbuf[2] == chkbuf[2]  && msgbuf[3] == chkbuf[3]) {
			pr_info("msgbuf=0x%02x 0x%02x 0x%02x 0x%02x\n",msgbuf[0],msgbuf[1],msgbuf[2],msgbuf[3]);
			pass = 1;
			break;
		} else if (msgbuf[0] == 0xff && msgbuf[1] == 0xff) {
			pr_err("%s: illegal cmd %08x\n", __func__, msg);
			rc = -EINVAL;
			break;
		} else if (msgbuf[0] == 0x00 && msgbuf[1] == 0x00) {
			pr_info("%s: not ready (%d retries)\n", __func__,
				retries);
			rc = -EBUSY;
		} else {
			pr_info("ack buf=0x%02x 0x%02x 0x%02x 0x%02x \n\n\n",msgbuf[0],msgbuf[1],msgbuf[2],msgbuf[3]);
			pr_info("%s: cmd/ack mismatch: (%d retries left)\n",
				__func__,
				retries);

			rc = -EBUSY;
		}
	}

	if (!pass) {
		pr_err("%s: failed execute cmd %08x (%d)\n", __func__,
			msg, rc);
		a1026_i2c_sw_reset(sw_reset);
	}
	return rc;
}

ssize_t chk_wakeup_a1026(void)
{
	int rc = 0, retry = POLLING_RETRY_CNT;

	if (a1026_suspended == 1) {
		/* Enable A1026 clock */
		if (control_a1026_clk) {
			gpio_set_value(a1026_data.pdata->gpio_a1026_clk, 1);
			mdelay(1);
		}

		gpio_set_value(a1026_data.pdata->gpio_a1026_wakeup, 0);
		msleep(50);

		do {
			rc = execute_cmdmsg(A1026_msg_Sync);
		} while ((rc < 0) && --retry);

		if (rc < 0) {
			pr_err("%s: failed (%d)\n", __func__, rc);
			acer_set_bypass_switch(0);
			goto wakeup_sync_err;
		}

		a1026_suspended = 0;
	}
wakeup_sync_err:
	gpio_set_value(a1026_data.pdata->gpio_a1026_wakeup, 1);
	return rc;
}

static ssize_t config_wakeup_gpio(void)
{
	int rc = 0;

	if (!a1026_config_wakeup_gpio) {
		/* Enable GPIO mode for wake up */
		es305_set_uart_tx(0);

		/* config a1026 wakeup GPIO */
		rc = gpio_request(a1026_data.pdata->gpio_a1026_wakeup, "a1026_wakeup");
		if (rc < 0) {
			pr_err("%s: gpio request wakeup pin failed\n", __func__);
			goto err_gpio_request;
		}

		rc = gpio_direction_output(a1026_data.pdata->gpio_a1026_wakeup, 1);
		if (rc < 0) {
			pr_err("%s: request reset wakeup direction failed\n", __func__);
			goto err_free_gpio;
		}
		a1026_config_wakeup_gpio = 1;
	}

	return rc;

err_free_gpio:
	gpio_free(a1026_data.pdata->gpio_a1026_wakeup);
err_gpio_request:
	return rc;
}

static void es305_set_uart_tx(int on)
{
	if(on) {
		if (a1026_config_wakeup_gpio) {
			a1026_config_wakeup_gpio = 0;
			gpio_free(a1026_data.pdata->gpio_a1026_wakeup);
		}
		tegra_pinmux_config_table(ES_305_UART_MODE, ARRAY_SIZE(ES_305_UART_MODE));
	} else {
		tegra_pinmux_config_table(ES_305_GPIO_MODE, ARRAY_SIZE(ES_305_GPIO_MODE));
	}
}

static int a1026_open(struct inode *inode, struct file *file)
{
	int rc = 0;

	mutex_lock(&a1026_lock);

#if DISABLE_A1026_TOOLS_PERMISSION
	if (a1026_opened) {
		pr_err("%s: busy\n", __func__);
		rc = -EBUSY;
		goto done;
	}
#endif

	a1026_opened = 1;
#if DISABLE_A1026_TOOLS_PERMISSION
done:
#endif
	mutex_unlock(&a1026_lock);
	return rc;
}

static int a1026_release(struct inode *inode, struct file *file)
{
	mutex_lock(&a1026_lock);
	a1026_opened = 0;
	mutex_unlock(&a1026_lock);

	return 0;
}

static ssize_t a1026_uart_sync_command(void)
{
	int rc = 0, retry = 3;

	do {
		rc = execute_cmdmsg(A1026_msg_Sync);
	} while ((rc < 0) && --retry);

	if (!rc)
		pr_info("%s: a1026 sync command okay \n", __func__);
	else {
		pr_err("%s: sync command error!!! \n", __func__);
		goto set_sync_err;
	}

	rc = config_wakeup_gpio();
	msleep(10);

	if (!a1026_suspend_es305())
		pr_info("%s: a1026 suspend command okay \n", __func__);
	else {
		pr_err("%s: suspend procedure error!!! \n", __func__);
		goto set_suspend_err;
	}

	acer_set_bypass_switch(0);

set_suspend_err:
set_sync_err:
	if (!rc)
		pr_info("%s: a1026 initialized!\n", __func__);
	else
		pr_err("%s: a1026 initialization failed\n", __func__);

	return rc;
}

static void a1026_download_mode(void)
{
	/* Enable UART mode for download firmware */
	es305_set_uart_tx(1);

	/* Enable A1026 clock */
	if (control_a1026_clk)
		gpio_set_value(a1026_data.pdata->gpio_a1026_clk, 1);
	mdelay(10);

	/* Reset A1026 chip */
	gpio_set_value(a1026_data.pdata->gpio_a1026_reset, 0);
	msleep(1);

	/* Take out of reset */
	gpio_set_value(a1026_data.pdata->gpio_a1026_reset, 1);
}

static ssize_t a1026_sync_test(void)
{
	int rc = 0, retry = 3;

	do {
		rc = execute_cmdmsg(A1026_msg_Sync);
	} while ((rc < 0) && --retry);

	if(!rc)
		pr_info("%s: a1026 simple test okay\n", __func__);

	return rc;
}

static ssize_t a1026_set_config(int newid)
{
	int rc = 0;

	if ((a1026_suspended) && (newid == A1026_TABLE_SUSPEND))
		return rc;

	if (a1026_current_config == newid) {
		return rc;
	}

	rc = chk_wakeup_a1026();

	if (rc < 0)
		return rc;

	switch (newid) {
		case A1026_TABLE_SUSPEND:
			rc = a1026_suspend_es305();
			return rc;
			break;
		case A1026_TABLE_VOIP_INTMIC:
			rc = execute_cmdmsg(VOIP_INTMIC);
			break;
		case A1026_TABLE_VOIP_EXTMIC:
			rc = execute_cmdmsg(VOIP_EXTMIC);
			break;
		case A1026_TABLE_30CM_CAMCORDER_INTMIC:
			rc = execute_cmdmsg(T30CM_CAMCORDER_INTMIC);
			break;
		case A1026_TABLE_30CM_CAMCORDER_INTMIC_REAR:
			rc = execute_cmdmsg(T30CM_CAMCORDER_INTMIC_REAR);
			break;
		case A1026_TABLE_CAMCORDER_EXTMIC:
			rc = execute_cmdmsg(T30CM_CAMCORDER_EXTMIC);
			break;
		case A1026_TABLE_30CM_ASR_INTMIC:
			rc = execute_cmdmsg(T30CM_ASR_INTMIC);
			break;
		case A1026_TABLE_ASR_EXTMIC:
			rc = execute_cmdmsg(ASR_EXTMIC);
			break;
		case A1026_TABLE_STEREO_CAMCORDER:
			rc = execute_cmdmsg(STEREO_CAMCORDER);
			break;
		case A1026_TABLE_3M_CAMCORDER_INTMIC:
			rc = execute_cmdmsg(T3M_CAMCORDER_INTMIC);
			break;
		case A1026_TABLE_3M_CAMCORDER_INTMIC_REAR:
			rc = execute_cmdmsg(T3M_CAMCORDER_INTMIC_REAR);
			break;
		case A1026_TABLE_3M_ASR_INTMIC:
			rc = execute_cmdmsg(T3M_ASR_INTMIC);
			break;
		case A1026_TABLE_SOUNDHOUND_INTMIC:
			rc = execute_cmdmsg(SOUNDHOUND_INTMIC);
			break;
		case A1026_TABLE_SOUNDHOUND_EXTMIC:
			rc = execute_cmdmsg(SOUNDHOUND_EXTMIC);
			break;
		case A1026_TABLE_ES305_PLAYBACK:
			if ((a1026_current_config == A1026_TABLE_VOIP_INTMIC) ||
			    (a1026_current_config == A1026_TABLE_VOIP_EXTMIC))
			    a1026_i2c_sw_reset(ES305_SW_RESET);
			rc = execute_cmdmsg(ES305_PLAYBACK);
			break;
		case A1026_TABLE_CTS:
			rc = execute_cmdmsg(CTS);
			break;
		case A1026_TABLE_HDMI_VOIP:
			rc = execute_cmdmsg(HDMI_VOIP);
			break;
		default:
			pr_err("%s: invalid cmd %d\n", __func__, newid);
			rc = -1;
			goto input_err;
			break;
	}

	a1026_current_config = newid;
	pr_info("%s: change to mode %d\n", __func__, newid);

input_err:
	return rc;
}

static ssize_t a1026_suspend_es305(void)
{
	int rc = 0;

	/* pass throgh From Port A to Port C */
	rc = execute_cmdmsg(PassThrough_A_to_C);
	if (rc) {
		pr_err("%s: pass through command error\n", __func__);
		goto set_bypass_err;
	}

	/* Put A1026 into sleep mode */
	rc = execute_cmdmsg(A1026_msg_Sleep);
	if (rc) {
		pr_err("%s: suspend error\n", __func__);
		goto set_suspend_err;
	}

	a1026_suspended = 1;
	a1026_current_config = A1026_TABLE_SUSPEND;

	msleep(120);
	/* Disable A1026 clock */
	if (control_a1026_clk)
		gpio_set_value(a1026_data.pdata->gpio_a1026_clk, 0);


set_suspend_err:
set_bypass_err:
	return rc;
}

static long a1026_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct a1026img img;
	int rc = 0;
	int pathid = 0;
	unsigned NoiseLevelValue = 0;
	unsigned WideningModeValue = 0;
	unsigned WideningGainValue = 0;

#if ENABLE_DIAG_IOCTLS
	char msg[4];
#endif

	switch (cmd) {
		case A1026_BOOTUP_INIT:
			img.buf = 0;
			img.img_size = 0;
			if (copy_from_user(&img, argp, sizeof(img)))
				return -EFAULT;

			rc = a1026_bootup_init(file, &img);
			break;
		case A1026_WAKEUP:
			rc = chk_wakeup_a1026();
			break;
		case A1026_UART_SYNC_CMD:
			rc = a1026_uart_sync_command();
			break;
		case A1026_DOWNLOAD_MODE:
			a1026_download_mode();
			break;
		case A1026_SET_CONFIG:
			if (copy_from_user(&pathid, argp, sizeof(pathid)))
				return -EFAULT;

			rc = a1026_set_config(pathid);
			if (rc < 0)
				pr_err("%s: A1026_SET_CONFIG (%d) error %d!\n",
					__func__, pathid, rc);
			break;
		case A1026_NOISE_LEVEL:
			if (copy_from_user(&NoiseLevelValue, argp, sizeof(NoiseLevelValue)))
				return -EFAULT;

			rc = execute_cmdmsg(NOISE_LEVEL_COMMAND);
			if (rc < 0)
				pr_err("%s: A1026_NOISE_LEVEL (%d) error %d!\n",
					__func__, NoiseLevelValue, rc);

			rc = execute_cmdmsg(NoiseLevelValue);
			if (rc < 0)
				pr_err("%s: A1026_NOISE_LEVEL (%d) error %d!\n",
					__func__, NoiseLevelValue, rc);
			break;
		case A1026_WIDENING_MODE:
			if (copy_from_user(&WideningModeValue, argp, sizeof(WideningModeValue)))
				return -EFAULT;

			rc = execute_cmdmsg(WIDENING_MODE_COMMAND);
			if (rc < 0)
				pr_err("%s: A1026_WIDENING_MODE (%x) error %d!\n",
					__func__, WideningModeValue, rc);

			rc = execute_cmdmsg(WideningModeValue);
			if (rc < 0)
				pr_err("%s: A1026_WIDENING_MODE (%x) error %d!\n",
					__func__, WideningModeValue, rc);
			break;
		case A1026_WIDENING_GAIN:
			if (copy_from_user(&WideningGainValue, argp, sizeof(WideningGainValue)))
				return -EFAULT;

			rc = execute_cmdmsg(WIDENING_GAIN_COMMAND);
			if (rc < 0)
				pr_err("%s: A1026_WIDENING_GAIN (%x) error %d!\n",
					__func__, WideningGainValue, rc);

			rc = execute_cmdmsg(WideningGainValue);
			if (rc < 0)
				pr_err("%s: A1026_WIDENING_GAIN (%x) error %d!\n",
					__func__, WideningGainValue, rc);
			break;
#if ENABLE_DIAG_IOCTLS
		case A1026_SYNC_CMD:
			a1026_sync_test();
			break;
		case A1026_READ_DATA:
			rc = chk_wakeup_a1026();
			if (rc < 0)
				return rc;
			rc = a1026_i2c_read(msg, 4);
			if (copy_to_user(argp, &msg, 4))
			return -EFAULT;
			break;
		case A1026_WRITE_MSG:
			rc = chk_wakeup_a1026();
			if (rc < 0)
				return rc;
			if (copy_from_user(msg, argp, sizeof(msg)))
				return -EFAULT;
			rc = a1026_i2c_write(msg, 4);
			break;
#endif
		default:
			pr_err("%s: invalid command %d\n", __func__, _IOC_NR(cmd));
			rc = -EINVAL;
			break;
	}
	return rc;
}

static const struct file_operations a1026_fops = {
	.owner				= THIS_MODULE,
	.open				= a1026_open,
	.release			= a1026_release,
	.unlocked_ioctl		= a1026_ioctl,
};

static struct miscdevice a1026_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = AUDIENCE_A1026_NAME,
	.fops = &a1026_fops,
};

static int a1026_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int rc = 0;

	a1026_data.client = client;

	if (a1026_data.pdata == NULL) {
		a1026_data.pdata = kzalloc(sizeof(*a1026_data.pdata), GFP_KERNEL);
		if (a1026_data.pdata == NULL) {
			rc = -ENOMEM;
			pr_err("%s: platform data is NULL\n", __func__);
			goto err_alloc_data_failed;
		}
	}

	a1026_data.pdata = client->dev.platform_data;

	/* config a1026 clock GPIO */
	rc = gpio_request(a1026_data.pdata->gpio_a1026_clk, "a1026_clk");
	if (rc < 0) {
		control_a1026_clk = 0;
		goto err_free_data;
	}
	control_a1026_clk = 1;

	rc = gpio_direction_output(a1026_data.pdata->gpio_a1026_clk, 0);
	if (rc < 0) {
		pr_err("%s: request clk gpio direction failed\n", __func__);
		goto err_free_gpio_clk;
	}

	/* config a1026 reset GPIO */
	rc = gpio_request(a1026_data.pdata->gpio_a1026_reset, "a1026_reset");
	if (rc < 0) {
		pr_err("%s: gpio request reset pin failed\n", __func__);
		goto err_free_gpio_clk;
	}

	rc = gpio_direction_output(a1026_data.pdata->gpio_a1026_reset, 1);
	if (rc < 0) {
		pr_err("%s: request reset gpio direction failed\n", __func__);
		goto err_free_gpio_all;
	}

	rc = misc_register(&a1026_device);
	if (rc) {
		pr_err("%s: a1026_device register failed\n", __func__);
		goto err_free_gpio_all;
	}

	a1026_current_config = A1026_TABLE_SUSPEND;

	return 0;

err_free_gpio_all:
	gpio_free(a1026_data.pdata->gpio_a1026_reset);
err_free_gpio_clk:
	if (control_a1026_clk)
		gpio_free(a1026_data.pdata->gpio_a1026_clk);
err_free_data:
	kfree(a1026_data.pdata);
err_alloc_data_failed:
	return rc;

}

static int a1026_remove(struct i2c_client *client)
{
	gpio_free(a1026_data.pdata->gpio_a1026_wakeup);
	gpio_free(a1026_data.pdata->gpio_a1026_reset);
	gpio_free(a1026_data.pdata->gpio_a1026_clk);
	kfree(a1026_data.pdata);

	return 0;
}

static int a1026_suspend(struct i2c_client *client, pm_message_t mesg)
{
	chk_wakeup_a1026();
	a1026_suspend_es305();
	return 0;
}

static const struct i2c_device_id a1026_id[] = {
	{ AUDIENCE_A1026_NAME, 0 },
	{ }
};

static int a1026_resume(struct i2c_client *client)
{
	return 0;
}

static struct i2c_driver a1026_driver = {
	.probe		= a1026_probe,
	.remove		= a1026_remove,
	.suspend	= a1026_suspend,
	.resume		= a1026_resume,
	.id_table	= a1026_id,
	.driver		= {
		.name = AUDIENCE_A1026_NAME,
	},
};

static int __init a1026_init(void)
{
	int res=0;

	mutex_init(&a1026_lock);

	res = i2c_add_driver(&a1026_driver);
	if (res){
		pr_err("[a1026]i2c_add_driver failed! \n");
		return res;
	}

	pr_info("[a1026] a1026 i2c init ok!\n");
	return 0;
}

static void __exit a1026_exit(void)
{
	i2c_del_driver(&a1026_driver);
}

module_init(a1026_init);
module_exit(a1026_exit);

MODULE_DESCRIPTION("A1026 voice processor driver");
MODULE_LICENSE("GPL");
