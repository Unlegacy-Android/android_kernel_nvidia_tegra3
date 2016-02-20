#define ELAN_X_MAX_370T  2112
#define ELAN_Y_MAX_370T  1280

#ifndef _LINUX_ELAN_KTF2K_H
#define _LINUX_ELAN_KTF2K_H

#define ELAN_KTF3K_NAME "elan-ktf3k"

struct elan_ktf3k_i2c_platform_data {
	uint16_t version;
	int abs_x_min;
	int abs_x_max;
	int abs_y_min;
	int abs_y_max;
	int intr_gpio;
	int rst_gpio;
};

#endif /* _LINUX_ELAN_KTF2K_H */
