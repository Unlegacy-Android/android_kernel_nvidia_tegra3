/*
 * at24.h - platform_data for the at24 (generic eeprom) driver
 * (C) Copyright 2008 by Pengutronix
 * (C) Copyright 2012 by Wolfram Sang
 * same license as the driver
 */

#ifndef _LINUX_AT24_H
#define _LINUX_AT24_H

#include <linux/types.h>
#include <linux/memory.h>

/**
 * struct at24_platform_data - data to set up at24 (generic eeprom) driver
 * @byte_len: size of eeprom in byte
 * @page_size: number of byte which can be written in one go
 * @flags: tunable options, check AT24_FLAG_* defines
 * @setup: an optional callback invoked after eeprom is probed; enables kernel
	code to access eeprom via memory_accessor, see example
 * @context: optional parameter passed to setup()
 *
 * If you set up a custom eeprom type, please double-check the parameters.
 * Especially page_size needs extra care, as you risk data loss if your value
 * is bigger than what the chip actually supports!
 *
 * An example in pseudo code for a setup() callback:
 *
 * void get_mac_addr(struct memory_accessor *mem_acc, void *context)
 * {
 * 	u8 *mac_addr = ethernet_pdata->mac_addr;
 *	off_t offset = context;
 *
 *	// Read MAC addr from EEPROM
 *	if (mem_acc->read(mem_acc, mac_addr, offset, ETH_ALEN) == ETH_ALEN)
 *		pr_info("Read MAC addr from EEPROM: %pM\n", mac_addr);
 * }
 *
 * This function pointer and context can now be set up in at24_platform_data.
 */

struct at24_platform_data {
	u32		byte_len;		/* size (sum of all addr) */
	u16		page_size;		/* for writes */
	u8		flags;
#define AT24_FLAG_ADDR16	0x80	/* address pointer is 16 bit */
#define AT24_FLAG_READONLY	0x40	/* sysfs-entry will be read-only */
#define AT24_FLAG_IRUGO		0x20	/* sysfs-entry will be world-readable */
#define AT24_FLAG_TAKE8ADDR	0x10	/* take always 8 addresses (24c00) */

	void		(*setup)(struct memory_accessor *, void *context);
	void		*context;
};

/* EEPROM DATA LENGTH */
#define LENGTH_UUID			16
#define LENGTH_BT_MAC			6
#define LENGTH_SN			14
#define LENGTH_SNWB			22
#define LENGTH_MANUFACTURE_DATE	4
#define LENGTH_IMEI			8
#define LENGTH_BOARD_ID		1
#define LENGTH_PRODUCT_SKU		2
#define LENGTH_L_SENSOR		10
#define LENGTH_G_SENSOR		75
#define LENGTH_WV_Device_ID		32

/* EEPROM DATA TABLE */
struct data_table {
	char uuid[LENGTH_UUID];
	char bt_mac[LENGTH_BT_MAC];
	char sn[LENGTH_SN];
	char sn_without_barcode[LENGTH_SNWB];
	char manufacture_date[LENGTH_MANUFACTURE_DATE];
	char IMEI[LENGTH_IMEI];
	char build_id[LENGTH_BOARD_ID];
	char product_sku[LENGTH_PRODUCT_SKU];
	char light_sensor_calibration[LENGTH_L_SENSOR];
	char g_sensor_calibration[LENGTH_G_SENSOR];
	char wv_device_id[LENGTH_WV_Device_ID];
} __attribute__((__packed__));

/* EEPROM EXTERN API */
extern int Get_UUID(char * buf);
extern int Get_BT_MAC(char * buf);
extern int Get_Serial_Number(char * buf);
extern int Get_Board_ID(char * buf);
extern int Get_Product_SKU(char * buf);
extern int Get_SerialNumberwithoutBarcode(char * buf);
extern int Get_IMEIwithBarcode(char * buf);
extern int Get_Manufacture_Date(char * buf);
extern int Get_G_Sensor(char * buf);
extern int Get_Light_Sensor(char * buf);
extern int Get_WV_Device_ID(char * buf);

extern int Set_UUID(char * buf);
extern int Set_BT_MAC(char * buf);
extern int Set_Serial_Number(char * buf);
extern int Set_Board_ID(char * buf);
extern int Set_Product_SKU(char * buf);
extern int Set_SerialNumberwithoutBarcode(char * buf);
extern int Set_IMEIwithBarcode(char * buf);
extern int Set_Manufacture_Date(char * buf);
extern int Set_G_Sensor(char * buf);
extern int Set_Light_Sensor(char * buf);
extern int Set_WV_Device_ID(char * buf);

#define ADDR_UUID 		(offsetof(struct data_table, uuid))
#define ADDR_BT_MAC 		(offsetof(struct data_table, bt_mac))
#define ADDR_SN 		(offsetof(struct data_table, sn))
#define ADDR_SNWB 		(offsetof(struct data_table, sn_without_barcode))
#define ADDR_MANUFACTURE_DATE	(offsetof(struct data_table, manufacture_date))
#define ADDR_IMEI 		(offsetof(struct data_table, IMEI))
#define ADDR_BOARD_ID 		(offsetof(struct data_table, build_id))
#define ADDR_PRODUCT_SKU 	(offsetof(struct data_table, product_sku))
#define ADDR_L_SENSOR 		(offsetof(struct data_table, light_sensor_calibration))
#define ADDR_G_SENSOR 		(offsetof(struct data_table, g_sensor_calibration))
#define ADDR_WV_Device_ID 	(offsetof(struct data_table, wv_device_id))

#define EEPROM_IO    0xA2

#define AT24_IOCTL_WRITE_UUID                _IOW(EEPROM_IO, 0x01, char*)
#define AT24_IOCTL_WRITE_BT_MAC              _IOW(EEPROM_IO, 0x02, char*)
#define AT24_IOCTL_WRITE_SN                  _IOW(EEPROM_IO, 0x03, char*)
#define AT24_IOCTL_WRITE_SNWB                _IOW(EEPROM_IO, 0x04, char*)
#define AT24_IOCTL_WRITE_MANUFACTURE_DATE    _IOW(EEPROM_IO, 0x05, char*)
#define AT24_IOCTL_WRITE_IMEI                _IOW(EEPROM_IO, 0x06, char*)
#define AT24_IOCTL_WRITE_BOARD_ID            _IOW(EEPROM_IO, 0x07, char*)
#define AT24_IOCTL_WRITE_PRODUCT_SKU         _IOW(EEPROM_IO, 0x08, char*)
#define AT24_IOCTL_WRITE_L_SENSOR            _IOW(EEPROM_IO, 0x09, char*)
#define AT24_IOCTL_WRITE_G_SENSOR            _IOW(EEPROM_IO, 0x0a, char*)
#define AT24_IOCTL_WRITE_WV_Device_ID        _IOW(EEPROM_IO, 0x0b, char*)

#endif /* _LINUX_AT24_H */
