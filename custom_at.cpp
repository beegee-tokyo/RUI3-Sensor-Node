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

int rtc_command_handler(SERIAL_PORT port, char *cmd, stParam *param);

/**
 * @brief Add custom AT commands
 *
 * @return true AT commands were added
 * @return false AT commands couldn't be added
 */
bool init_rtc_at(void)
{
	bool result = false;

	result = api.system.atMode.add("RTC",
								   "Set/Get time of RTC module RAK12002 format [yyyy:mm:dd:hh:MM]",
								   "RTC", rtc_command_handler);

	return result;
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
		for (int i = 0; i < param->argc; i++)
		{
			Serial.printf("%s", param->argv[i]);
		}
		Serial.println("");

		for (int j = 0; j < param->argc; j++)
		{
			for (int i = 0; i < strlen(param->argv[j]); i++)
			{
				if (!isdigit(*(param->argv[j] + i)))
				{
					MYLOG("AT_CMD", "%d is no digit in param %d", i, j);
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
			MYLOG("AT_CMD", "Year error %d", year);
			return AT_PARAM_ERROR;
		}

		if (year < 2022)
		{
			MYLOG("AT_CMD", "Year error %d", year);
			return AT_PARAM_ERROR;
		}

		/* Check month */
		month = strtoul(param->argv[1], NULL, 0);

		if ((month < 1) || (month > 12))
		{
			MYLOG("AT_CMD", "Month error %d", month);
			return AT_PARAM_ERROR;
		}

		// Check day
		date = strtoul(param->argv[2], NULL, 0);

		if ((date < 1) || (date > 31))
		{
			MYLOG("AT_CMD", "Day error %d", date);
			return AT_PARAM_ERROR;
		}

		// Check hour
		hour = strtoul(param->argv[3], NULL, 0);

		if (hour > 24)
		{
			MYLOG("AT_CMD", "Hour error %d", hour);
			return AT_PARAM_ERROR;
		}

		// Check minute
		minute = strtoul(param->argv[4], NULL, 0);

		if (minute > 59)
		{
			MYLOG("AT_CMD", "Minute error %d", minute);
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
