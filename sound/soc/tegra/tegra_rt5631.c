/*
 * tegra_rt5631.c - Tegra machine ASoC driver for boards using RT5631 codec.
 */

#include <asm/mach-types.h>

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#include <sound/core.h>
#include <sound/jack.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "tegra_pcm.h"
#include "tegra_asoc_utils.h"
#include <mach/board-asus-t30-misc.h>
#include <mach/tegra_asoc_pdata.h>

#include "../drivers/input/asusec/asusdec.h"
#include "../gpio-names.h"

#define DRV_NAME "tegra-snd-codec"

#define DAI_LINK_HIFI		0
#define DAI_LINK_SPDIF		1
#define NUM_DAI_LINKS		2

struct tegra_rt5631 {
	struct tegra_asoc_utils_data util_data;
	struct tegra_asoc_platform_data *pdata;
	struct regulator *spk_reg;
	struct regulator *dmic_reg;
	int gpio_requested;
};

extern void audio_dock_init(void);

static int tegra_rt5631_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_card *card = codec->card;
	struct tegra_rt5631 *machine = snd_soc_card_get_drvdata(card);

	int srate, mclk, i2s_daifmt;
	int err;

	srate = params_rate(params);
	switch (srate) {
	case 64000:
	case 88200:
	case 96000:
		mclk = 128 * srate;
		break;
	default:
		mclk = 384 * srate;
		break;
	}
	/* FIXME: Codec only requires >= 3MHz if OSR==0 */
	while (mclk < 6000000)
		mclk *= 2;

	err = tegra_asoc_utils_set_rate(&machine->util_data, srate, mclk);
	if (err < 0) {
		dev_err(card->dev, "Can't configure clocks\n");
		return err;
	}

	i2s_daifmt = SND_SOC_DAIFMT_NB_NF |
		     SND_SOC_DAIFMT_CBS_CFS;

	i2s_daifmt |= SND_SOC_DAIFMT_I2S;

	err = snd_soc_dai_set_fmt(codec_dai, i2s_daifmt);
	if (err < 0) {
		dev_err(card->dev, "codec_dai fmt not set\n");
		return err;
	}

	err = snd_soc_dai_set_fmt(cpu_dai, i2s_daifmt);
	if (err < 0) {
		dev_err(card->dev, "cpu_dai fmt not set\n");
		return err;
	}

	err = snd_soc_dai_set_sysclk(codec_dai, 0, mclk,
					SND_SOC_CLOCK_IN);
	if (err < 0) {
		dev_err(card->dev, "codec_dai clock not set\n");
		return err;
	}

	return 0;
}

static struct snd_soc_ops tegra_rt5631_ops = {
	.hw_params = tegra_rt5631_hw_params,
};

static struct snd_soc_ops tegra_spdif_ops;

static const struct snd_soc_dapm_widget tegra_rt5631_dapm_widgets[] = {
	SND_SOC_DAPM_SPK("Int Spk", NULL),
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_MIC("Mic Jack", NULL),
	SND_SOC_DAPM_MIC("Int Mic", NULL),
	SND_SOC_DAPM_SPK("AUX", NULL),
};

static const struct snd_soc_dapm_route tegra_rt5631_audio_map[] = {
	{"Headphone Jack", NULL, "HPOL"},
	{"Headphone Jack", NULL, "HPOR"},
	{"Int Spk", NULL, "SPOL"},
	{"Int Spk", NULL, "SPOR"},
	{"MIC1", NULL, "Mic Bias1"},
	{"Mic Bias1", NULL, "Mic Jack"},
	{"DMIC", NULL, "Int Mic"},
	{"AUX", NULL, "AUXO2"},
};

static const struct snd_kcontrol_new tegra_rt5631_controls[] = {
	SOC_DAPM_PIN_SWITCH("Int Spk"),
	SOC_DAPM_PIN_SWITCH("Headphone Jack"),
	SOC_DAPM_PIN_SWITCH("Mic Jack"),
	SOC_DAPM_PIN_SWITCH("Int Mic"),
	SOC_DAPM_PIN_SWITCH("AUX"),
};

static int tegra_rt5631_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	snd_soc_dapm_nc_pin(dapm, "MIC2");
	snd_soc_dapm_nc_pin(dapm, "AXIL");
	snd_soc_dapm_nc_pin(dapm, "AXIR");
	snd_soc_dapm_nc_pin(dapm, "MONOIN_RXN");
	snd_soc_dapm_nc_pin(dapm, "MONOIN_RXP");
	snd_soc_dapm_nc_pin(dapm, "MONO");
	snd_soc_dapm_disable_pin(dapm, "Int Mic");
	snd_soc_dapm_disable_pin(dapm, "Mic Jack");
	snd_soc_dapm_disable_pin(dapm, "Headphone Jack");
	snd_soc_dapm_disable_pin(dapm, "Int Spk");
	snd_soc_dapm_disable_pin(dapm, "AUX");
	snd_soc_dapm_sync(dapm);

	return 0;
}

static struct snd_soc_dai_link tegra_rt5631_dai[NUM_DAI_LINKS] = {
	[DAI_LINK_HIFI] = {
		.name = "RT5631",
		.stream_name = "RT5631 PCM",
		.codec_name = "rt5631.4-001a",
		.platform_name = "tegra-pcm-audio",
		.cpu_dai_name = "tegra30-i2s.1",
		.codec_dai_name = "rt5631-hifi",
		.init = tegra_rt5631_init,
		.ops = &tegra_rt5631_ops,
	},
	[DAI_LINK_SPDIF] = {
		.name = "SPDIF",
		.stream_name = "SPDIF PCM",
		.codec_name = "spdif-dit.0",
		.platform_name = "tegra-pcm-audio",
		.cpu_dai_name = "tegra30-spdif",
		.codec_dai_name = "dit-hifi",
		.ops = &tegra_spdif_ops,
	}
};

static struct snd_soc_card snd_soc_tegra_rt5631 = {
	.name = "tegra-codec",
	.owner = THIS_MODULE,
	.dai_link = tegra_rt5631_dai,
	.num_links = ARRAY_SIZE(tegra_rt5631_dai),
	.controls = tegra_rt5631_controls,
	.num_controls = ARRAY_SIZE(tegra_rt5631_controls),
	.dapm_widgets = tegra_rt5631_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(tegra_rt5631_dapm_widgets),
	.dapm_routes = tegra_rt5631_audio_map,
	.num_dapm_routes = ARRAY_SIZE(tegra_rt5631_audio_map),
};

static __devinit int tegra_rt5631_driver_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_tegra_rt5631;
	struct tegra_rt5631 *machine;
	struct tegra_asoc_platform_data *pdata;

	int ret;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "No platform data supplied\n");
		return -EINVAL;
	}

	if (pdata->codec_name)
		card->dai_link->codec_name = pdata->codec_name;

	if (pdata->codec_dai_name)
		card->dai_link->codec_dai_name = pdata->codec_dai_name;

	machine = kzalloc(sizeof(struct tegra_rt5631), GFP_KERNEL);
	if (!machine) {
		dev_err(&pdev->dev, "Can't allocate tegra_rt5631 struct\n");
		return -ENOMEM;
	}

	machine->pdata = pdata;

	ret = tegra_asoc_utils_init(&machine->util_data, &pdev->dev, card);
	if (ret)
		goto err_free_machine;

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, machine);

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n",
			ret);
		goto err_fini_utils;
	}

	if (!card->instantiated) {
		ret = -ENODEV;
		dev_err(&pdev->dev, "sound card not instantiated (%d)\n",
			ret);
		goto err_unregister_card;
	}

#ifndef CONFIG_ARCH_TEGRA_2x_SOC
	ret = tegra_asoc_utils_set_parent(&machine->util_data,
				pdata->i2s_param[HIFI_CODEC].is_i2s_master);
	if (ret) {
		dev_err(&pdev->dev, "tegra_asoc_utils_set_parent failed (%d)\n",
			ret);
		goto err_unregister_card;
	}
#endif

	return 0;

err_unregister_card:
	snd_soc_unregister_card(card);
err_fini_utils:
	tegra_asoc_utils_fini(&machine->util_data);
err_free_machine:
	kfree(machine);
	return ret;
}

static int __devexit tegra_rt5631_driver_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct tegra_rt5631 *machine = snd_soc_card_get_drvdata(card);

	snd_soc_unregister_card(card);

	tegra_asoc_utils_fini(&machine->util_data);

	kfree(machine);

	return 0;
}

static struct platform_driver tegra_rt5631_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
	},
	.probe = tegra_rt5631_driver_probe,
	.remove = __devexit_p(tegra_rt5631_driver_remove),
};

static int __init tegra_rt5631_modinit(void)
{
	if(tegra3_get_project_id() != TEGRA3_PROJECT_TF300T) {
		printk("tegra_rt5631: codec is supported\n");
		audio_dock_init();
		return platform_driver_register(&tegra_rt5631_driver);
	} else {
		return 0;
	}
}
module_init(tegra_rt5631_modinit);

static void __exit tegra_rt5631_modexit(void)
{
	platform_driver_unregister(&tegra_rt5631_driver);
}
module_exit(tegra_rt5631_modexit);

MODULE_AUTHOR("Stephen Warren <swarren@nvidia.com>");
MODULE_DESCRIPTION("Tegra+RT5631 machine ASoC driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
