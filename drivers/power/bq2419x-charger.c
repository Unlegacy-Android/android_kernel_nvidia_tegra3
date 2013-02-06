/*
 * bq2419x-charger.c - Battery charger driver
 *
 * Copyright (c) 2012, NVIDIA Corporation.
 *
 * Author: Laxman Dewangan <ldewangan@nvidia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mfd/bq2419x.h>
#include <linux/regmap.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

/* input current limit */
static const unsigned int iinlim[] = {
	100, 150, 500, 900, 1200, 1500, 2000, 3000,
};

struct bq2419x_charger {
	struct device		*dev;
	struct bq2419x_chip	*chip;
	struct power_supply	ac;
	struct power_supply	usb;
	int			ac_online;
	int			usb_online;
	int			gpio_status;
	int			gpio_interrupt;
	int			usb_in_current_limit;
	int			ac_in_current_limit;
	unsigned		use_mains:1;
	unsigned		use_usb:1;
	int status;
	void (*update_status)(int, int);
	struct regulator_dev	*rdev;
	struct regulator_desc	reg_desc;
	struct regulator_init_data	reg_init_data;
	int			shutdown_complete;
	struct mutex		mutex;
	struct mutex		chrg_mutex;
};

struct bq2419x_charger *bq_charger;

static enum power_supply_property bq2419x_psy_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};
static int current_to_reg(const unsigned int *tbl,
			size_t size, unsigned int val)
{
	size_t i;

	for (i = 0; i < size; i++)
		if (val < tbl[i])
			break;
	return i > 0 ? i - 1 : -EINVAL;
}

static int bq2419x_charger_enable(struct bq2419x_charger *charger)
{
	int ret;
	mutex_lock(&charger->mutex);
	if (charger && charger->shutdown_complete) {
		mutex_unlock(&charger->mutex);
		return -ENODEV;
	}
	mutex_unlock(&charger->mutex);

	dev_info(charger->dev, "Charging enabled\n");
	ret = regmap_update_bits(charger->chip->regmap, BQ2419X_PWR_ON_REG,
				ENABLE_CHARGE_MASK, ENABLE_CHARGE);
	if (ret < 0)
		dev_err(charger->dev, "register update failed, err %d\n", ret);
	return ret;
}

static int bq2419x_charger_disable(struct bq2419x_charger *charger)
{
	int ret;

	mutex_lock(&charger->mutex);
	if (charger && charger->shutdown_complete) {
		mutex_unlock(&charger->mutex);
		return -ENODEV;
	}
	mutex_unlock(&charger->mutex);

	dev_info(charger->dev, "Charging disabled\n");
	ret = regmap_update_bits(charger->chip->regmap, BQ2419X_PWR_ON_REG,
				ENABLE_CHARGE_MASK, 0);
	if (ret < 0)
		dev_err(charger->dev, "register update failed, err %d\n", ret);
	return ret;
}

static int bq2419x_ac_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct bq2419x_charger *chip;

	chip = container_of(psy, struct bq2419x_charger, ac);
	if (psp == POWER_SUPPLY_PROP_ONLINE)
		val->intval = chip->ac_online;
	else
		return -EINVAL;
	return 0;
}

static int bq2419x_usb_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct bq2419x_charger *chip;

	chip = container_of(psy, struct bq2419x_charger, usb);
	if (psp == POWER_SUPPLY_PROP_ONLINE)
		val->intval = chip->usb_online;
	else
		return -EINVAL;
	return 0;
}

static int bq2419x_init(struct bq2419x_charger *charger)
{
	int val, ret = 0;

	mutex_lock(&charger->mutex);
	if (charger && charger->shutdown_complete) {
		mutex_unlock(&charger->mutex);
		return -ENODEV;
	}
	mutex_unlock(&charger->mutex);

	if (charger->usb_online) {
		val = current_to_reg(iinlim, ARRAY_SIZE(iinlim),
					charger->usb_in_current_limit);
		if (val < 0)
			return 0;

		ret = regmap_update_bits(charger->chip->regmap,
			BQ2419X_INPUT_SRC_REG, BQ2419x_CONFIG_MASK, val);
		if (ret < 0)
			dev_err(charger->dev, "error reading reg: 0x%x\n",
				BQ2419X_INPUT_SRC_REG);
	}

	if (charger->ac_online) {
		val = current_to_reg(iinlim, ARRAY_SIZE(iinlim),
					charger->ac_in_current_limit);
		if (val < 0)
			return 0;

		ret = regmap_update_bits(charger->chip->regmap,
			BQ2419X_INPUT_SRC_REG, BQ2419x_CONFIG_MASK, val);
		if (ret < 0)
			dev_err(charger->dev, "error reading reg: 0x%x\n",
				BQ2419X_INPUT_SRC_REG);
	}
	return ret;
}

static int bq2419x_enable_charger(struct regulator_dev *rdev,
					int min_uA, int max_uA)
{
	int ret = 0;
	int val;

	msleep(200);
	bq_charger->usb_online = 0;
	bq_charger->ac_online = 0;
	bq_charger->status = 0;

	mutex_lock(&bq_charger->chrg_mutex);
	if (bq_charger && bq_charger->shutdown_complete) {
		mutex_unlock(&bq_charger->chrg_mutex);
		return -ENODEV;
	}
	mutex_unlock(&bq_charger->chrg_mutex);

	ret = regmap_read(bq_charger->chip->regmap, BQ2419X_SYS_STAT_REG, &val);
	if (ret < 0)
		dev_err(bq_charger->dev, "error reading reg: 0x%x\n",
				BQ2419X_SYS_STAT_REG);

	if (max_uA == 0 && val != 0)
		return ret;

	if ((val & BQ2419x_VBUS_STAT) == BQ2419x_VBUS_UNKNOWN) {
		bq_charger->status = 0;
		bq_charger->usb_online = 0;
		bq_charger->usb_in_current_limit = 500;
		ret = bq2419x_init(bq_charger);
		if (ret < 0)
			goto error;
		if (bq_charger->update_status)
			bq_charger->update_status
				(bq_charger->status, 0);
	} else if ((val & BQ2419x_VBUS_STAT) == BQ2419x_VBUS_USB) {
		bq_charger->status = 1;
		bq_charger->usb_online = 1;
		bq_charger->usb_in_current_limit = max_uA/1000;
		ret = bq2419x_init(bq_charger);
		if (ret < 0)
			goto error;
		if (bq_charger->update_status)
			bq_charger->update_status
				(bq_charger->status, 2);
	} else if ((val & BQ2419x_VBUS_STAT) == BQ2419x_VBUS_AC) {
		bq_charger->status = 1;
		bq_charger->ac_online = 1;
		bq_charger->ac_in_current_limit = max_uA/1000;
		ret = bq2419x_init(bq_charger);
		if (ret < 0)
			goto error;
		if (bq_charger->update_status)
			bq_charger->update_status
				(bq_charger->status, 1);
	}
	if (ret == 0) {
		if (bq_charger->use_mains)
			power_supply_changed(&bq_charger->ac);
		if (bq_charger->use_usb)
			power_supply_changed(&bq_charger->usb);
	}
	return 0;
error:
	dev_err(bq_charger->dev, "Charger enable failed, err = %d\n", ret);
	return ret;
}

static struct regulator_ops bq2419x_tegra_regulator_ops = {
	.set_current_limit = bq2419x_enable_charger,
};

static void bq2419x_charger_shutdown(struct platform_device *pdev)
{
	struct bq2419x_charger *charger = platform_get_drvdata(pdev);

	mutex_lock(&charger->mutex);
	charger->shutdown_complete = 1;
	mutex_unlock(&charger->mutex);
}

static int __devinit bq2419x_charger_probe(struct platform_device *pdev)
{
	struct bq2419x_platform_data *chip_pdata;
	struct bq2419x_charger_platform_data *bcharger_pdata = NULL;
	struct bq2419x_charger *charger;
	int ret;
	u32 val;

	chip_pdata = dev_get_platdata(pdev->dev.parent);
	if (chip_pdata)
		bcharger_pdata = chip_pdata->bcharger_pdata;

	if (!bcharger_pdata) {
		dev_err(&pdev->dev, "No Platform data");
		return -EIO;
	}

	charger = devm_kzalloc(&pdev->dev, sizeof(*charger), GFP_KERNEL);
	if (!charger) {
		dev_err(&pdev->dev, "Memory alloc failed\n");
		return -ENOMEM;
	}

	charger->chip = dev_get_drvdata(pdev->dev.parent);
	charger->dev = &pdev->dev;
	charger->use_usb = bcharger_pdata->use_usb;
	charger->use_mains = bcharger_pdata->use_mains;
	charger->gpio_status = bcharger_pdata->gpio_status;
	charger->gpio_interrupt = bcharger_pdata->gpio_interrupt;
	charger->update_status = bcharger_pdata->update_status;
	charger->shutdown_complete = 0;
	bq_charger = charger;
	platform_set_drvdata(pdev, charger);
	mutex_init(&charger->mutex);
	mutex_init(&charger->chrg_mutex);

	ret = regmap_read(charger->chip->regmap, BQ2419X_REVISION_REG, &val);
	if (ret < 0) {
		dev_err(charger->dev, "error reading reg: 0x%x, err = %d\n",
					BQ2419X_REVISION_REG, ret);
		return ret;
	}

	if ((val & BQ24190_IC_VER) == BQ24190_IC_VER)
		dev_info(charger->dev, "chip type BQ24190 detected\n");
	else if ((val & BQ24192_IC_VER) == BQ24192_IC_VER)
		dev_info(charger->dev, "chip type BQ2419X/3 detected\n");
	else if ((val & BQ24192i_IC_VER) == BQ24192i_IC_VER)
		dev_info(charger->dev, "chip type BQ2419Xi detected\n");

	charger->gpio_status = bcharger_pdata->gpio_status;
	charger->gpio_interrupt = bcharger_pdata->gpio_interrupt;

	if (gpio_is_valid(charger->gpio_status)) {
		ret = gpio_request_one(charger->gpio_status,
					GPIOF_IN, "bq2419x-status");
		if (ret < 0) {
			dev_err(charger->dev,
				"status gpio request failed, err = %d\n", ret);
			return ret;
		}
	}

	/* Configure Charge Current Control to 3A*/
	ret = regmap_write(charger->chip->regmap, BQ2419X_CHRG_CTRL_REG, 0xC0);
	if (ret < 0) {
		dev_err(charger->dev, "error writing to reg: 0x%x\n",
				BQ2419X_CHRG_CTRL_REG);
		return ret;
	}

	/* Configure Input voltage limit reset to OTP value */
	ret = regmap_write(charger->chip->regmap, BQ2419X_INPUT_SRC_REG, 0x32);
	if (ret < 0) {
		dev_err(charger->dev, "error writing to reg: 0x%x\n",
				BQ2419X_CHRG_CTRL_REG);
		return ret;
	}

	charger->reg_desc.name  = "vbus_charger";
	charger->reg_desc.ops   = &bq2419x_tegra_regulator_ops;
	charger->reg_desc.type  = REGULATOR_CURRENT;
	charger->reg_desc.id    = bcharger_pdata->regulator_id;
	charger->reg_desc.type  = REGULATOR_CURRENT;
	charger->reg_desc.owner = THIS_MODULE;

	charger->reg_init_data.supply_regulator		= NULL;
	charger->reg_init_data.num_consumer_supplies	=
				bcharger_pdata->num_consumer_supplies;

	charger->reg_init_data.regulator_init		= NULL;
	charger->reg_init_data.consumer_supplies	=
					bcharger_pdata->consumer_supplies;
	charger->reg_init_data.driver_data		= charger;
	charger->reg_init_data.constraints.name		= "vbus_charger";
	charger->reg_init_data.constraints.min_uA	= 0;
	charger->reg_init_data.constraints.max_uA	=
				bcharger_pdata->max_charge_current_mA * 1000;

	charger->reg_init_data.constraints.valid_modes_mask =
					REGULATOR_MODE_NORMAL |
					REGULATOR_MODE_STANDBY;

	charger->reg_init_data.constraints.valid_ops_mask =
					REGULATOR_CHANGE_MODE |
					REGULATOR_CHANGE_STATUS |
					REGULATOR_CHANGE_CURRENT;

	charger->rdev = regulator_register(&charger->reg_desc, charger->dev,
					&charger->reg_init_data, charger, NULL);

	if (IS_ERR(charger->rdev)) {
		dev_err(&pdev->dev, "failed to register %s\n",
				charger->reg_desc.name);
		ret = PTR_ERR(charger->rdev);
		goto free_gpio_status;
	}

	charger->ac_online = 0;
	charger->usb_online = 0;
	charger->status = 0;
	if (charger->use_mains) {
		charger->ac.name		= "bq2419x-ac";
		charger->ac.type		= POWER_SUPPLY_TYPE_MAINS;
		charger->ac.get_property	= bq2419x_ac_get_property;
		charger->ac.properties		= bq2419x_psy_props;
		charger->ac.num_properties	= ARRAY_SIZE(bq2419x_psy_props);
		ret = power_supply_register(&pdev->dev, &charger->ac);
		if (ret) {
			dev_err(&pdev->dev, "failed: power supply register\n");
			return ret;
		}
	}

	if (charger->use_usb) {
		charger->usb.name		= "bq2419x-usb";
		charger->usb.type		= POWER_SUPPLY_TYPE_USB;
		charger->usb.get_property	= bq2419x_usb_get_property;
		charger->usb.properties		= bq2419x_psy_props;
		charger->usb.num_properties	= ARRAY_SIZE(bq2419x_psy_props);
		ret = power_supply_register(&pdev->dev, &charger->usb);
		if (ret) {
			dev_err(&pdev->dev, "failed: power supply register\n");
			goto psy_error;
		}
	}

	/* enable charging */
	ret = bq2419x_charger_enable(charger);
	if (ret < 0)
		goto psy_error1;
	return 0;

psy_error1:
	power_supply_unregister(&charger->usb);
psy_error:
	power_supply_unregister(&charger->ac);
free_gpio_status:
	if (gpio_is_valid(charger->gpio_status))
		gpio_free(charger->gpio_status);
	mutex_destroy(&charger->mutex);
	mutex_destroy(&charger->chrg_mutex);
	return ret;
}

static int __devexit bq2419x_charger_remove(struct platform_device *pdev)
{
	struct bq2419x_charger *charger = platform_get_drvdata(pdev);

	if (charger->use_mains)
		power_supply_unregister(&charger->usb);
	if (charger->use_usb)
		power_supply_unregister(&charger->ac);
	if (gpio_is_valid(charger->gpio_status))
		gpio_free(charger->gpio_status);
	mutex_destroy(&charger->mutex);
	mutex_destroy(&charger->chrg_mutex);
	return 0;
}

static struct platform_driver bq2419x_charger_driver = {
	.driver = {
		.name = "bq2419x-battery-charger",
		.owner = THIS_MODULE,
	},
	.probe = bq2419x_charger_probe,
	.shutdown = bq2419x_charger_shutdown,
	.remove = __devexit_p(bq2419x_charger_probe),
};

static int __init bq2419x_charger_init(void)
{
	return platform_driver_register(&bq2419x_charger_driver);
}
subsys_initcall(bq2419x_charger_init);

static void __exit bq2419x_charger_cleanup(void)
{
	platform_driver_unregister(&bq2419x_charger_driver);
}
module_exit(bq2419x_charger_cleanup);

MODULE_DESCRIPTION("bq2419x battery charger driver");
MODULE_AUTHOR("Laxman Dewangan <ldewangan@nvidia.com>");
MODULE_LICENSE("GPL v2");
