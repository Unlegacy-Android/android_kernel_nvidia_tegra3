#ifndef __ACER_AUDIO_COMMON_H__
#define __ACER_AUDIO_COMMON_H__

#include <mach/tegra_asoc_pdata.h>
#include <linux/delay.h>
#include <sound/wm8903.h>

extern bool is_hp_plugged(void);
extern bool is_debug_on(void);
extern void wm8903_event_printf(const char* func, int event);
extern bool handset_mic_detect(struct snd_soc_codec *codec);

typedef enum {
	DEFAULT             = 0,
	MIC                 = 1,
	VOICE_UPLINK        = 2,
	VOICE_DOWNLINK      = 3,
	VOICE_CALL          = 4,
	CAMCORDER           = 5,
	VOICE_RECOGNITION   = 6,
	VOICE_COMMUNICATION = 7,

	AUDIO_SOURCE_CNT,
	AUDIO_SOURCE_MAX    = AUDIO_SOURCE_CNT - 1,
} audio_source_t;

struct acer_table_data {
	int input;
	int output;
};

struct acer_gpio_data {
	int spkr_en;
	int hp_det;
	int int_mic_en;
	int bypass_en;
	int debug_en;
};

struct acer_state_data {
	bool int_mic;
	bool ext_mic;
	int old;
};

struct acer_mode_data {
	int control;
	int input_source;
	int ap_control;
};

struct acer_audio_data {
	struct snd_soc_codec* codec;
	const char *pin;
	bool AP_Lock;
	struct acer_table_data table;
	struct acer_gpio_data gpio;
	struct acer_state_data state;
	struct acer_mode_data mode;
};

#endif
