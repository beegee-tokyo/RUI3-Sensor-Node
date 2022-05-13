/**
 * @file RAK15001_flash.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialization and handling of RAK15001 Flash module
 * @version 0.1
 * @date 2022-04-12
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "main.h"
#include "RAK_FLASH_SPI.h" //Click here to get the library: http://librarymanager/All#RAK_Storage

// SPI Flash Interface instance
RAK_FlashInterface_SPI g_flashTransport(SS, SPI);

/** SPI Flash instance */
RAK_FLASH_SPI g_flash(&g_flashTransport);

/** Flash definition structure for GD25Q16C Flash */
SPIFlash_Device_t g_RAK15001{
	.total_size = (1UL << 21),
	.start_up_time_us = 5000,
	.manufacturer_id = 0xc8,
	.memory_type = 0x40,
	.capacity = 0x15,
	.max_clock_speed_mhz = 15,
	.quad_enable_bit_mask = 0x00,
	.has_sector_protection = false,
	.supports_fast_read = true,
	.supports_qspi = false,
	.supports_qspi_writes = false,
	.write_status_register_split = false,
	.single_status_byte = true,
};

/** Flag if RAK15001 was found */
bool g_has_rak15001 = false;

/**
 * @brief Initialize the RAK15001 module
 *
 * @return true if module found and init ok
 * @return false if module not found or init failed
 */
bool init_rak15001(void)
{
	// MYLOG("FLASH", "SS = %d", SS);

	if (!g_flash.begin(&g_RAK15001)) // Start access to the flash
	{
		MYLOG("FLASH", "Flash access failed, check the settings");
		return false;
	}
	if (!g_flash.waitUntilReady(5000))
	{
		MYLOG("FLASH", "Busy timeout");
		return false;
	}

	// MYLOG("FLASH", "Device ID: 0x%02X", g_flash.getJEDECID());
	// MYLOG("FLASH", "Size: %ld", g_flash.size());
	// MYLOG("FLASH", "Pages: %d", g_flash.numPages());
	// MYLOG("FLASH", "Page Size: %d", g_flash.pageSize());

	g_has_rak15001 = true;
	return true;
}

/**
 * @brief Read data from a sector of the RAK15001
 *
 * @param sector Flash sector, valid 0 to 511
 * @param buffer Buffer to read the data to
 * @param size Number of bytes to read
 * @return true If no error
 * @return false If read failed
 */
bool read_rak15001(uint16_t sector, uint8_t *buffer, uint16_t size)
{
	if (sector > 511)
	{
		MYLOG("FLASH", "Invalid sector");
		return false;
	}

	// Read the bytes
	if (!g_flash.readBuffer(sector * 4096, buffer, size))
	{
		MYLOG("FLASH", "Read failed");
		return false;
	}
	return true;
}

/**
 * @brief Write data to a sector of the RAK15001
 *
 * @param sector Flash sector, valid 0 to 511
 * @param buffer Buffer with the data to be written
 * @param size Number of bytes to write
 * @return true If write succeeded
 * @return false If write failed or readback data is not the same
 */
bool write_rak15001(uint16_t sector, uint8_t *buffer, uint16_t size)
{
	if (sector > 511)
	{
		MYLOG("FLASH", "Invalid sector");
		return false;
	}

	// Format the sector
	if (!g_flash.eraseSector(sector))
	{
		MYLOG("FLASH", "Erase failed");
	}

	uint8_t check_buff[size];
	// Write the bytes
	if (!g_flash.writeBuffer(sector * 4096, buffer, size))
	{
		MYLOG("FLASH", "Write failed");
		return false;
	}

	if (!g_flash.waitUntilReady(5000))
	{
		MYLOG("FLASH", "Busy timeout");
		return false;
	}

	// Read back the data
	if (!g_flash.readBuffer(sector * 4096, check_buff, size))
	{
		MYLOG("FLASH", "Read back failed");
		return false;
	}

	// Check if read data is same as requested data
	if (memcmp(check_buff, buffer, size) != 0)
	{
		MYLOG("FLASH", "Bytes read back are not the same as written");
		Serial.println("Adr  Write buffer     Read back");
		for (int idx = 0; idx < size; idx++)
		{
			Serial.printf("%03d  %02X   %02X\r\n", idx, buffer[idx], check_buff[idx]);
		}
		Serial.println(" ");
		return false;
	}
	return true;
}

/**
 * @brief Save the device configuration to the RAK15001
 * 
 * @return true if save was successful
 * @return false if save failed
 */
bool save_config(void)
{
	if (write_rak15001(0, (uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings)))
	{
		MYLOG("FLASH", "Wrote default data set");
	}
	else
	{
		MYLOG("FLASH", "Write failed. Something is wrong");
		return false;
	}
	return true;
}

/**
 * @brief Check if a RAK15001 is available
 * and if a configuration is saved in the Flash
 *
 * @return true if configuration was found
 * @return false if either RAK15001 was not found or the configuration is invalid
 */
bool read_config(void)
{
	uint8_t check_buff[2];
	if (read_rak15001(0, check_buff, 2))
	{
		// Check if it is LPWAN settings
		if ((check_buff[0] != 0xAA) || (check_buff[1] != LORAWAN_DATA_MARKER))
		{
			// Data is not valid, reset to defaults
			MYLOG("FLASH", "Invalid data set");
			if (write_rak15001(0, (uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings)))
			{
				MYLOG("FLASH", "Wrote default data set");
			}
			else
			{
				MYLOG("FLASH", "Write failed. Something is wrong");
				return false;
			}
		}
		else
		{
			MYLOG("FLASH", "Found valid dataset");
		}
		if (read_rak15001(0, (uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings)))
		{
			MYLOG("FLASH", "Read saved data set");
		}
		log_settings();
		return true;
	}
	else
	{
		MYLOG("FLASH", "Reading from RAK15001 failed");
		g_has_rak15001 = false;
		return false;
	}
}

char *region_names[] = {(char *)"EU433", (char *)"CN470", (char *)"RU864", (char *)"IN865",
						(char *)"EU868", (char *)"US915", (char *)"AU915", (char *)"KR920",
						(char *)"AS923", (char *)"AS923-2", (char *)"AS923-3", (char *)"AS923-4"};

/**
 * @brief Printout of all settings
 *
 */
void log_settings(void)
{
	MYLOG("FLASH", "Saved settings:");
	MYLOG("FLASH", "000 Marks: %02X %02X", g_lorawan_settings.valid_mark_1, g_lorawan_settings.valid_mark_2);
	MYLOG("FLASH", "002 Dev EUI %02X%02X%02X%02X%02X%02X%02X%02X", g_lorawan_settings.node_device_eui[0], g_lorawan_settings.node_device_eui[1],
		  g_lorawan_settings.node_device_eui[2], g_lorawan_settings.node_device_eui[3],
		  g_lorawan_settings.node_device_eui[4], g_lorawan_settings.node_device_eui[5],
		  g_lorawan_settings.node_device_eui[6], g_lorawan_settings.node_device_eui[7]);
	MYLOG("FLASH", "010 App EUI %02X%02X%02X%02X%02X%02X%02X%02X", g_lorawan_settings.node_app_eui[0], g_lorawan_settings.node_app_eui[1],
		  g_lorawan_settings.node_app_eui[2], g_lorawan_settings.node_app_eui[3],
		  g_lorawan_settings.node_app_eui[4], g_lorawan_settings.node_app_eui[5],
		  g_lorawan_settings.node_app_eui[6], g_lorawan_settings.node_app_eui[7]);
	MYLOG("FLASH", "018 App Key %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		  g_lorawan_settings.node_app_key[0], g_lorawan_settings.node_app_key[1],
		  g_lorawan_settings.node_app_key[2], g_lorawan_settings.node_app_key[3],
		  g_lorawan_settings.node_app_key[4], g_lorawan_settings.node_app_key[5],
		  g_lorawan_settings.node_app_key[6], g_lorawan_settings.node_app_key[7],
		  g_lorawan_settings.node_app_key[8], g_lorawan_settings.node_app_key[9],
		  g_lorawan_settings.node_app_key[10], g_lorawan_settings.node_app_key[11],
		  g_lorawan_settings.node_app_key[12], g_lorawan_settings.node_app_key[13],
		  g_lorawan_settings.node_app_key[14], g_lorawan_settings.node_app_key[15]);
	MYLOG("FLASH", "034 Dev Addr %08lX", g_lorawan_settings.node_dev_addr);
	MYLOG("FLASH", "038 NWS Key %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		  g_lorawan_settings.node_nws_key[0], g_lorawan_settings.node_nws_key[1],
		  g_lorawan_settings.node_nws_key[2], g_lorawan_settings.node_nws_key[3],
		  g_lorawan_settings.node_nws_key[4], g_lorawan_settings.node_nws_key[5],
		  g_lorawan_settings.node_nws_key[6], g_lorawan_settings.node_nws_key[7],
		  g_lorawan_settings.node_nws_key[8], g_lorawan_settings.node_nws_key[9],
		  g_lorawan_settings.node_nws_key[10], g_lorawan_settings.node_nws_key[11],
		  g_lorawan_settings.node_nws_key[12], g_lorawan_settings.node_nws_key[13],
		  g_lorawan_settings.node_nws_key[14], g_lorawan_settings.node_nws_key[15]);
	MYLOG("FLASH", "054 Apps Key %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		  g_lorawan_settings.node_apps_key[0], g_lorawan_settings.node_apps_key[1],
		  g_lorawan_settings.node_apps_key[2], g_lorawan_settings.node_apps_key[3],
		  g_lorawan_settings.node_apps_key[4], g_lorawan_settings.node_apps_key[5],
		  g_lorawan_settings.node_apps_key[6], g_lorawan_settings.node_apps_key[7],
		  g_lorawan_settings.node_apps_key[8], g_lorawan_settings.node_apps_key[9],
		  g_lorawan_settings.node_apps_key[10], g_lorawan_settings.node_apps_key[11],
		  g_lorawan_settings.node_apps_key[12], g_lorawan_settings.node_apps_key[13],
		  g_lorawan_settings.node_apps_key[14], g_lorawan_settings.node_apps_key[15]);
	MYLOG("FLASH", "070 OTAA %s", g_lorawan_settings.otaa_enabled ? "enabled" : "disabled");
	MYLOG("FLASH", "071 ADR %s", g_lorawan_settings.adr_enabled ? "enabled" : "disabled");
	MYLOG("FLASH", "072 %s Network", g_lorawan_settings.public_network ? "Public" : "Private");
	MYLOG("FLASH", "073 Dutycycle %s", g_lorawan_settings.duty_cycle_enabled ? "enabled" : "disabled");
	MYLOG("FLASH", "074 Repeat time %ld", g_lorawan_settings.send_repeat_time);
	MYLOG("FLASH", "078 Join trials %d", g_lorawan_settings.join_trials);
	MYLOG("FLASH", "079 TX Power %d", g_lorawan_settings.tx_power);
	MYLOG("FLASH", "080 DR %d", g_lorawan_settings.data_rate);
	MYLOG("FLASH", "081 Class %d", g_lorawan_settings.lora_class);
	MYLOG("FLASH", "082 Subband %d", g_lorawan_settings.subband_channels);
	MYLOG("FLASH", "083 Auto join %s", g_lorawan_settings.auto_join ? "enabled" : "disabled");
	MYLOG("FLASH", "084 Fport %d", g_lorawan_settings.app_port);
	MYLOG("FLASH", "085 %s Message", g_lorawan_settings.confirmed_msg_enabled ? "Confirmed" : "Unconfirmed");
	MYLOG("FLASH", "086 Region %s", region_names[g_lorawan_settings.lora_region]);
	MYLOG("FLASH", "087 Mode %s", g_lorawan_settings.lorawan_enable ? "LPWAN" : "P2P");
	MYLOG("FLASH", "088 P2P frequency %ld", g_lorawan_settings.p2p_frequency);
	MYLOG("FLASH", "092 P2P TX Power %d", g_lorawan_settings.p2p_tx_power);
	MYLOG("FLASH", "093 P2P BW %d", g_lorawan_settings.p2p_bandwidth);
	MYLOG("FLASH", "094 P2P SF %d", g_lorawan_settings.p2p_sf);
	MYLOG("FLASH", "095 P2P CR %d", g_lorawan_settings.p2p_cr);
	MYLOG("FLASH", "096 P2P Preamble length %d", g_lorawan_settings.p2p_preamble_len);
	MYLOG("FLASH", "097 P2P Symbol Timeout %d", g_lorawan_settings.p2p_symbol_timeout);
}
