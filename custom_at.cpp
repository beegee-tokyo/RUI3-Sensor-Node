/**
 * @file custom_at.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief
 * @version 0.1
 * @date 2022-05-12
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "main.h"

// Forward declarations
int send_interval_handler(SERIAL_PORT port, char *cmd, stParam *param);
int rtc_command_handler(SERIAL_PORT port, char *cmd, stParam *param);
int gnss_format_handler(SERIAL_PORT port, char *cmd, stParam *param);
int status_handler(SERIAL_PORT port, char *cmd, stParam *param);

uint32_t g_send_interval_time = 0;

/**
 * @brief GNSS format and precision
 * 0 = 4 digit standard Cayenne LPP format
 * 1 = 6 digit extended Cayenne LPP format
 * 2 = Helium Mapper data format
 */
uint8_t gnss_format = 0;

/**
 * @brief Add send interval AT command
 *
 * @return true if success
 * @return false if failed
 */
bool init_frequency_at(void)
{
	return api.system.atMode.add((char *)"SENDINT",
								 (char *)"Set/Get the send interval time values in seconds 0 = off, max 2,147,483 seconds",
								 (char *)"SENDINT", send_interval_handler);
}

/**
 * @brief Handler for send interval AT commands
 *
 * @param port Serial port used
 * @param cmd char array with the received AT command
 * @param param char array with the received AT command parameters
 * @return int result of command parsing
 * 			AT_OK AT command & parameters valid
 * 			AT_PARAM_ERROR command or parameters invalid
 */
int send_interval_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	if (param->argc == 1 && !strcmp(param->argv[0], "?"))
	{
		Serial.print(cmd);
		Serial.printf("=%lds\r\n", g_send_interval_time / 1000);
	}
	else if (param->argc == 1)
	{
		MYLOG("AT_CMD", "param->argv[0] >> %s", param->argv[0]);
		for (int i = 0; i < strlen(param->argv[0]); i++)
		{
			if (!isdigit(*(param->argv[0] + i)))
			{
				MYLOG("AT_CMD", "%d is no digit", i);
				return AT_PARAM_ERROR;
			}
		}

		uint32_t new_send_interval = strtoul(param->argv[0], NULL, 10);

		MYLOG("AT_CMD", "Requested interval %ld", new_send_interval);

		g_send_interval_time = new_send_interval * 1000;

		MYLOG("AT_CMD", "New interval %ld", g_send_interval_time);
		// Stop the timer
		api.system.timer.stop(RAK_TIMER_0);
		if (g_send_interval_time != 0)
		{
			// Restart the timer
			api.system.timer.start(RAK_TIMER_0, g_send_interval_time, NULL);
		}
		// Save custom settings
		save_at_setting(SEND_INTERVAL_OFFSET);
	}
	else
	{
		return AT_PARAM_ERROR;
	}

	return AT_OK;
}

/**
 * @brief Add custom RTC AT commands
 *
 * @return true AT commands were added
 * @return false AT commands couldn't be added
 */
bool init_rtc_at(void)
{
	return api.system.atMode.add((char *)"RTC",
								 (char *)"Set/Get time of RTC module RAK12002 format [yyyy:mm:dd:hh:MM]",
								 (char *)"RTC", rtc_command_handler);
}

/**
 * @brief Handler for custom AT command for the RTC module
 *
 * @param port Serial port used
 * @param cmd char array with the received AT command
 * @param param char array with the received AT command parameters
 * @return int result of command parsing
 * 			AT_OK AT command & parameters valid
 * 			AT_PARAM_ERROR command or parameters invalid
 */
int rtc_command_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	MYLOG("AT_CMD", "Param size %d", param->argc);
	if (param->argc == 1 && !strcmp(param->argv[0], "?"))
	{
		read_rak12002();
		Serial.print(cmd);
		Serial.print("=");
		Serial.printf("%d.%02d.%02d %d:%02d:%02d\n", g_date_time.year, g_date_time.month,
					  g_date_time.date, g_date_time.hour,
					  g_date_time.minute, g_date_time.second);
	}
	else if (param->argc == 5)
	{
		for (int j = 0; j < param->argc; j++)
		{
			for (int i = 0; i < strlen(param->argv[j]); i++)
			{
				if (!isdigit(*(param->argv[j] + i)))
				{
					// MYLOG("AT_CMD", "%d is no digit in param %d", i, j);
					return AT_PARAM_ERROR;
				}
			}
		}
		uint32_t year;
		uint32_t month;
		uint32_t date;
		uint32_t hour;
		uint32_t minute;

		/* Check year */
		year = strtoul(param->argv[0], NULL, 0);

		if (year > 3000)
		{
			// MYLOG("AT_CMD", "Year error %d", year);
			return AT_PARAM_ERROR;
		}

		if (year < 2022)
		{
			// MYLOG("AT_CMD", "Year error %d", year);
			return AT_PARAM_ERROR;
		}

		/* Check month */
		month = strtoul(param->argv[1], NULL, 0);

		if ((month < 1) || (month > 12))
		{
			// MYLOG("AT_CMD", "Month error %d", month);
			return AT_PARAM_ERROR;
		}

		// Check day
		date = strtoul(param->argv[2], NULL, 0);

		if ((date < 1) || (date > 31))
		{
			// MYLOG("AT_CMD", "Day error %d", date);
			return AT_PARAM_ERROR;
		}

		// Check hour
		hour = strtoul(param->argv[3], NULL, 0);

		if (hour > 24)
		{
			// MYLOG("AT_CMD", "Hour error %d", hour);
			return AT_PARAM_ERROR;
		}

		// Check minute
		minute = strtoul(param->argv[4], NULL, 0);

		if (minute > 59)
		{
			// MYLOG("AT_CMD", "Minute error %d", minute);
			return AT_PARAM_ERROR;
		}

		set_rak12002((uint16_t)year, (uint8_t)month, (uint8_t)date, (uint8_t)hour, (uint8_t)minute);

		return AT_OK;
	}
	else
	{
		return AT_PARAM_ERROR;
	}

	return AT_OK;
}

/**
 * @brief Add custom GNSS AT commands
 *
 * @return true AT commands were added
 * @return false AT commands couldn't be added
 */
bool init_gnss_at(void)
{
	bool result = false;

	result = api.system.atMode.add((char *)"GNSS",
								   (char *)"Change GNSS precision and payload format. 0 = 4digit prec., 1 = 6digit prec, 2 = Helium Mapper format, 3 = Field Tester format",
								   (char *)"GNSS", gnss_format_handler);

	if (!get_at_setting(GNSS_OFFSET))
	{
		MYLOG("AT_CMD", "Could not get default GNSS settings");
		result = false;
	}
	switch (gnss_format)
	{
	case LPP_4_DIGIT:
		MYLOG("AT_CMD", "GNSS 4 digit Cayenne LPP");
		break;
	case LPP_6_DIGIT:
		MYLOG("AT_CMD", "GNSS 6 digit extended Cayenne LPP");
		break;
	case HELIUM_MAPPER:
		MYLOG("AT_CMD", "GNSS Helium Mapper data format");
		break;
	case FIELD_TESTER:
		MYLOG("AT_CMD", "GNSS Field Tester data format");
		break;
	}

	return result;
}

/**
 * @brief Handler for custom AT command for GNSS settings
 *
 * @param port Serial port used
 * @param cmd char array with the received AT command
 * @param param char array with the received AT command parameters
 * @return int result of command parsing
 * 			AT_OK AT command & parameters valid
 * 			AT_PARAM_ERROR command or parameters invalid
 */
int gnss_format_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	if (param->argc == 1 && !strcmp(param->argv[0], "?"))
	{
		Serial.print(cmd);
		Serial.print("=");
		switch (gnss_format)
		{
		case LPP_4_DIGIT:
			Serial.println("4 digit Cayenne LPP");
			break;
		case LPP_6_DIGIT:
			Serial.println("6 digit extended Cayenne LPP");
			break;
		case HELIUM_MAPPER:
			Serial.println("Helium Mapper data format");
			break;
		case FIELD_TESTER:
			Serial.println("Field Tester data format");
			break;
		}
	}
	else if (param->argc == 1)
	{
		for (int i = 0; i < strlen(param->argv[0]); i++)
		{
			if (!isdigit(*(param->argv[0] + i)))
			{
				MYLOG("AT_CMD", "%d is no digit", i);
				return AT_PARAM_ERROR;
			}
		}

		uint32_t gnss_format_new = strtoul(param->argv[0], NULL, 10);

		MYLOG("AT_CMD", "Requested format %ld", gnss_format_new);

		if (gnss_format_new > FIELD_TESTER)
		{
			return AT_PARAM_ERROR;
		}

		MYLOG("AT_CMD", "Set format to %d", gnss_format_new);
		gnss_format = gnss_format_new;
		if (!save_at_setting(GNSS_OFFSET))
		{
			MYLOG("AT_CMD", "Save failed");
			return AT_PARAM_ERROR;
		}
	}
	else
	{
		return AT_PARAM_ERROR;
	}

	return AT_OK;
}

/**
 * @brief Add custom Status AT commands
 *
 * @return true AT commands were added
 * @return false AT commands couldn't be added
 */
bool init_status_at(void)
{
	return api.system.atMode.add((char *)"STATUS",
								 (char *)"Get device information",
								 (char *)"STATUS", status_handler);
}

/** Regions as text array */
char *regions_list[] = {"EU433", "CN470", "RU864", "IN865", "EU868", "US915", "AU915", "KR920", "AS923", "AS923-2", "AS923-3", "AS923-4"};
/** Network modes as text array*/
char *nwm_list[] = {"P2P", "LoRaWAN", "FSK"};

/**
 * @brief Print device status over Serial
 *
 * @param port Serial port used
 * @param cmd char array with the received AT command
 * @param param char array with the received AT command parameters
 * @return int result of command parsing
 * 			AT_OK AT command & parameters valid
 * 			AT_PARAM_ERROR command or parameters invalid
 */
int status_handler(SERIAL_PORT port, char *cmd, stParam *param)
{
	String value_str = "";
	int nw_mode = 0;
	int region_set = 0;
	uint8_t key_eui[16] = {0}; // efadff29c77b4829acf71e1a6e76f713

	if (param->argc == 1 && !strcmp(param->argv[0], "?"))
	{
		Serial.println("Device Status:");
		value_str = api.system.hwModel.get();
		value_str.toUpperCase();
		Serial.printf("Module: %s\r\n", value_str.c_str());
		Serial.printf("Version: %s\r\n", api.system.firmwareVer.get().c_str());
		Serial.printf("Send time: %d s\r\n", g_send_interval_time / 1000);
		nw_mode = api.lorawan.nwm.get();
		Serial.printf("Network mode %s\r\n", nwm_list[nw_mode]);
		if (nw_mode == 1)
		{
			Serial.printf("Network %s\r\n", api.lorawan.njs.get() ? "joined" : "not joined");
			region_set = api.lorawan.band.get();
			Serial.printf("Region: %d\r\n", region_set);
			Serial.printf("Region: %s\r\n", regions_list[region_set]);
			if (api.lorawan.njm.get())
			{
				Serial.println("OTAA mode");
				api.lorawan.deui.get(key_eui, 8);
				Serial.printf("DevEUI = %02X%02X%02X%02X%02X%02X%02X%02X\r\n",
							  key_eui[0], key_eui[1], key_eui[2], key_eui[3],
							  key_eui[4], key_eui[5], key_eui[6], key_eui[7]);
				api.lorawan.appeui.get(key_eui, 8);
				Serial.printf("AppEUI = %02X%02X%02X%02X%02X%02X%02X%02X\r\n",
							  key_eui[0], key_eui[1], key_eui[2], key_eui[3],
							  key_eui[4], key_eui[5], key_eui[6], key_eui[7]);
				api.lorawan.appkey.get(key_eui, 16);
				Serial.printf("AppKey = %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
							  key_eui[0], key_eui[1], key_eui[2], key_eui[3],
							  key_eui[4], key_eui[5], key_eui[6], key_eui[7],
							  key_eui[8], key_eui[9], key_eui[10], key_eui[11],
							  key_eui[12], key_eui[13], key_eui[14], key_eui[15]);
			}
			else
			{
				Serial.println("ABP mode");
				api.lorawan.appskey.get(key_eui, 16);
				Serial.printf("AppsKey = %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
							  key_eui[0], key_eui[1], key_eui[2], key_eui[3],
							  key_eui[4], key_eui[5], key_eui[6], key_eui[7],
							  key_eui[8], key_eui[9], key_eui[10], key_eui[11],
							  key_eui[12], key_eui[13], key_eui[14], key_eui[15]);
				api.lorawan.nwkskey.get(key_eui, 16);
				Serial.printf("NwsKey = %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
							  key_eui[0], key_eui[1], key_eui[2], key_eui[3],
							  key_eui[4], key_eui[5], key_eui[6], key_eui[7],
							  key_eui[8], key_eui[9], key_eui[10], key_eui[11],
							  key_eui[12], key_eui[13], key_eui[14], key_eui[15]);
				api.lorawan.daddr.set(key_eui, 4);
				Serial.printf("DevAddr = %02X%02X%02X%02X\r\n",
							  key_eui[0], key_eui[1], key_eui[2], key_eui[3]);
			}
		}
		else if (nw_mode == 0)
		{
			Serial.printf("Frequency = %d\r\n", api.lora.pfreq.get());
			Serial.printf("SF = %d\r\n", api.lora.psf.get());
			Serial.printf("BW = %d\r\n", api.lora.pbw.get());
			Serial.printf("CR = %d\r\n", api.lora.pcr.get());
			Serial.printf("Preamble length = %d\r\n", api.lora.ppl.get());
			Serial.printf("TX power = %d\r\n", api.lora.ptp.get());
		}
		else
		{
			Serial.printf("Frequency = %d\r\n", api.lora.pfreq.get());
			Serial.printf("Bitrate = %d\r\n", api.lora.pbr.get());
			Serial.printf("Deviaton = %d\r\n", api.lora.pfdev.get());
		}
		announce_modules();
	}
	else
	{
		return AT_PARAM_ERROR;
	}
	return AT_OK;
}

/**
 * @brief Get setting from flash
 *
 * @param setting_type type of setting, valid values
 * 			GNSS_OFFSET for GNSS precision and data format
 * @return true read from flash was successful
 * @return false read from flash failed or invalid settings type
 */
bool get_at_setting(uint32_t setting_type)
{
	uint8_t flash_value[16];
	switch (setting_type)
	{
	case GNSS_OFFSET:
		if (!api.system.flash.get(GNSS_OFFSET, flash_value, 2))
		{
			MYLOG("AT_CMD", "Failed to read GNSS value from Flash");
			return false;
		}
		if (flash_value[1] != 0xAA)
		{
			MYLOG("AT_CMD", "Invalid GNSS settings, using default");
			gnss_format = 0;
			save_at_setting(GNSS_OFFSET);
		}
		if (flash_value[0] > FIELD_TESTER)
		{
			MYLOG("AT_CMD", "No valid GNSS value found, set to default, read 0X%0X", flash_value[0]);
			gnss_format = 0;
			return false;
		}
		gnss_format = flash_value[0];
		MYLOG("AT_CMD", "Found GNSS format to %d", flash_value[0]);
		return true;
		break;
	case SEND_INTERVAL_OFFSET:
		if (!api.system.flash.get(SEND_INTERVAL_OFFSET, flash_value, 5))
		{
			MYLOG("AT_CMD", "Failed to read send interval from Flash");
			return false;
		}
		if (flash_value[4] != 0xAA)
		{
			MYLOG("AT_CMD", "No valid send interval found, set to default, read 0X%02X 0X%02X 0X%02X 0X%02X",
				  flash_value[0], flash_value[1],
				  flash_value[2], flash_value[3]);
			g_send_interval_time = 0;
			save_at_setting(SEND_INTERVAL_OFFSET);
			return false;
		}
		MYLOG("AT_CMD", "Read send interval 0X%02X 0X%02X 0X%02X 0X%02X",
			  flash_value[0], flash_value[1],
			  flash_value[2], flash_value[3]);
		g_send_interval_time = 0;
		g_send_interval_time |= flash_value[0] << 0;
		g_send_interval_time |= flash_value[1] << 8;
		g_send_interval_time |= flash_value[2] << 16;
		g_send_interval_time |= flash_value[3] << 24;
		MYLOG("AT_CMD", "send interval found %ld", g_send_interval_time);
		return true;
		break;
	default:
		return false;
	}
}

/**
 * @brief Save setting to flash
 *
 * @param setting_type type of setting, valid values
 * 			GNSS_OFFSET for GNSS precision and data format
 * @return true write to flash was successful
 * @return false write to flash failed or invalid settings type
 */
bool save_at_setting(uint32_t setting_type)
{
	uint8_t flash_value[16] = {0};
	bool wr_result = false;
	switch (setting_type)
	{
	case GNSS_OFFSET:
		flash_value[0] = gnss_format;
		flash_value[1] = 0xAA;
		return api.system.flash.set(GNSS_OFFSET, flash_value, 2);
		break;
	case SEND_INTERVAL_OFFSET:
		flash_value[0] = (uint8_t)(g_send_interval_time >> 0);
		flash_value[1] = (uint8_t)(g_send_interval_time >> 8);
		flash_value[2] = (uint8_t)(g_send_interval_time >> 16);
		flash_value[3] = (uint8_t)(g_send_interval_time >> 24);
		flash_value[4] = 0xAA;
		MYLOG("AT_CMD", "Writing send interval 0X%02X 0X%02X 0X%02X 0X%02X ",
			  flash_value[0], flash_value[1],
			  flash_value[2], flash_value[3]);
		wr_result = api.system.flash.set(SEND_INTERVAL_OFFSET, flash_value, 5);
		// MYLOG("AT_CMD", "Writing %s", wr_result ? "Success" : "Fail");
		wr_result = true;
		return wr_result;
		break;
	default:
		return false;
		break;
	}
	return false;
}

