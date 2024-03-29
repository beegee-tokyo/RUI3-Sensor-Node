/**
 * @file module_handler.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Find and handle WisBlock sensor modules
 * @version 0.1
 * @date 2022-04-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "main.h"
#include "module_handler.h"

/**
 * @brief List of all supported WisBlock modules
 *
 */
volatile sensors_t found_sensors[] = {
	// I2C address , I2C bus, found?
	{0x18, 0, false}, //  0 ✔ RAK1904 accelerometer
	{0x44, 0, false}, //  1 ✔ RAK1903 light sensor
	{0x42, 0, false}, //  2 ✔ RAK12500 GNSS sensor
	{0x5c, 0, false}, //  3 ✔ RAK1902 barometric pressure sensor
	{0x70, 0, false}, //  4 ✔ RAK1901 temperature & humidity sensor
	{0x76, 0, false}, //  5 ✔ RAK1906 environment sensor
	{0x20, 0, false}, //  6 RAK12035 soil moisture sensor !! address conflict with RAK13003
	{0x10, 0, false}, //  7 ✔ RAK12010 light sensor
	{0x51, 0, false}, //  8 RAK12004 MQ2 CO2 gas sensor
	{0x50, 0, false}, //  9 ✔ RAK15000 EEPROM !! conflict with RAK12008
	{0x50, 0, false}, // 10 RAK12008 MG812 CO2 gas sensor
	{0x55, 0, false}, // 11 RAK12009 MQ3 Alcohol gas sensor
	{0x29, 0, false}, // 12 ✔ RAK12014 Laser ToF sensor
	{0x52, 0, false}, // 13 ✔ RAK12002 RTC module address
	{0x04, 0, false}, // 14 RAK14003 LED bargraph module
	{0x59, 0, false}, // 15 ✔ RAK12047 VOC sensor address !! conflict with RAK13600, RAK13003
	{0x68, 0, false}, // 16 RAK12025 Gyroscope address !! conflict with RAK1905
	{0x73, 0, false}, // 17 RAK14008 Gesture sensor
	{0x3C, 0, false}, // 18 ✔ RAK1921 OLED display
	{0x53, 0, false}, // 19 RAK12019 LTR390 light sensor
	{0x28, 0, false}, // 20 RAK14002 Touch Button module
	{0x41, 0, false}, // 21 RAK16000 DC current sensor
	{0x68, 0, false}, // 22 ✔ RAK1905 MPU9250 9DOF sensor !! address conflict with RAK12025
	{0x61, 0, false}, // 23 ✔ RAK12037 CO2 sensor !! address conflict with RAK16001
	{0x3A, 0, false}, // 24 ✔ RAK12003 IR temperature sensor
	{0x68, 0, false}, // 25 ✔ RAK12040 AMG8833 temperature array sensor
	{0x57, 0, false}, // 26 RAK12012 MAX30102 heart rate sensor
	{0x54, 0, false}, // 27 RAK12016 Flex sensor
	{0x47, 0, false}, // 28 RAK13004 PWM expander module
	{0x38, 0, false}, // 29 RAK14001 RGB LED module
	{0x5F, 0, false}, // 30 RAK14004 Keypad interface
	{0x61, 0, false}, // 31 RAK16001 ADC sensor !! address conflict with RAK12037
	{0x59, 0, false}, // 32 RAK13600 NFC !! address conflict with RAK12047, RAK13600
	{0x59, 0, false}, // 33 RAK16002 Coulomb sensor !! address conflict with RAK13600, RAK12047
	{0x20, 0, false}, // 34 RAK13003 IO expander module !! address conflict with RAK12035
};

/**
 * @brief Scan both I2C bus for devices
 *
 */
void find_modules(void)
{
	// Scan the I2C interfaces for devices
	byte error;
	uint8_t num_dev = 0;

	Wire.begin();
	Wire.setClock(400000);
	for (byte address = 1; address < 127; address++)
	{
		if (address == 0x29)
		{
			// RAK12014 has extra GPIO for power control
			// On/Off control pin
			pinMode(xshut_pin, OUTPUT);
			// Sensor on
			digitalWrite(xshut_pin, HIGH);
			// Wait for sensor wake-up
			delay(150);
		}
		else
		{
			pinMode(WB_IO3, INPUT);
		}
		Wire.beginTransmission(address);
		error = Wire.endTransmission();
		if (error == 0)
		{
			MYLOG("SCAN", "Found sensor on I2C1 0x%02X\n", address);
			for (uint8_t i = 0; i < sizeof(found_sensors) / sizeof(sensors_t); i++)
			{
				if (address == found_sensors[i].i2c_addr)
				{
					found_sensors[i].i2c_num = 1;
					found_sensors[i].found_sensor = true;

					if (address == 0x52)
					{
						found_sensors[i + 1].i2c_num = 1;
						found_sensors[i + 1].found_sensor = true;
					}
					break;
				}
			}
			num_dev++;
		}
	}

	// MYLOG("SCAN", "Found %d sensors", num_dev);

	// if (num_dev == 0)
	// {
	// 	for (uint8_t i = 0; i < sizeof(found_sensors) / sizeof(sensors_t); i++)
	// 	{
	// 		if (found_sensors[i].found_sensor)
	// 		{
	// 			MYLOG("SCAN", "ID %d addr %02X", i, found_sensors[i].i2c_addr);
	// 		}
	// 	}
	// }

	// Check if RAK15001 is available
	if (init_rak15001())
	{
		// MYLOG("SCAN", "RAK15001 found");
	}

	// No devices found, no need to go through the initialization routines
	if (num_dev == 0)
	{
		for (uint8_t i = 0; i < sizeof(found_sensors) / sizeof(sensors_t); i++)
		{
			found_sensors[i].found_sensor = false;
		}
		return;
	}

	// Initialize the modules found
	if (found_sensors[EEPROM_ID].found_sensor)
	{
		// Check EEPROM first, it occupies multiple I2C addresses
		if (!init_rak15000())
		{
			found_sensors[EEPROM_ID].found_sensor = false;
		}
		else
		{
			// 0x51, 0x52 and 0x53 are occupied by EEPROM
			found_sensors[MQ2_ID].found_sensor = false;
			found_sensors[RTC_ID].found_sensor = false;
			found_sensors[UVL_ID].found_sensor = false;
		}
	}

	if (found_sensors[TEMP_ID].found_sensor)
	{
		if (!init_rak1901())
		{
			found_sensors[TEMP_ID].found_sensor = false;
		}
	}

	if (found_sensors[PRESS_ID].found_sensor)
	{
		if (!init_rak1902())
		{
			found_sensors[PRESS_ID].found_sensor = false;
		}
	}

	if (found_sensors[LIGHT_ID].found_sensor)
	{
		if (init_rak1903())
		{
			sprintf(g_dev_name, "RUI3 Weather Station");
		}
		else
		{
			found_sensors[LIGHT_ID].found_sensor = false;
		}
	}

	if (found_sensors[ACC_ID].found_sensor)
	{
		if (!init_rak1904())
		{
			found_sensors[ACC_ID].found_sensor = false;
		}
	}

	if (found_sensors[GYRO_ID].found_sensor)
	{
		// if (!init_rak12025())
		{
			found_sensors[GYRO_ID].found_sensor = false;
			if (!init_rak1905())
			{
				found_sensors[MPU_ID].found_sensor = false;
				if (!init_rak12040())
				{
					found_sensors[TEMP_ARR_ID].found_sensor = false;
				}
				else
				{
					found_sensors[TEMP_ARR_ID].found_sensor = true;
				}
			}
			else
			{
				found_sensors[MPU_ID].found_sensor = true;
			}
		}
	}

	if (found_sensors[ENV_ID].found_sensor)
	{
		if (init_rak1906())
		{
			sprintf(g_dev_name, "RUI3 Environment Sensor");
		}
		else
		{
			found_sensors[ENV_ID].found_sensor = false;
		}
	}

	if (found_sensors[OLED_ID].found_sensor)
	{
		if (init_rak1921())
		{
			rak1921_write_header((char *)"WisBlock Node");
		}
		else
		{
			found_sensors[OLED_ID].found_sensor = false;
		}
	}

	if (found_sensors[RTC_ID].found_sensor)
	{
		if (!init_rak12002())
		{
			found_sensors[RTC_ID].found_sensor = false;
		}
		else
		{
			init_rtc_at();
		}
	}

	if (found_sensors[FIR_ID].found_sensor)
	{
		if (!init_rak12003())
		{
			found_sensors[FIR_ID].found_sensor = false;
		}
	}

	if (found_sensors[LIGHT2_ID].found_sensor)
	{
		if (init_rak12010())
		{
			sprintf(g_dev_name, "RUI3 Weather Station");
		}
		else
		{
			found_sensors[LIGHT2_ID].found_sensor = false;
		}
	}

	if (found_sensors[TOF_ID].found_sensor)
	{
		// Try TOF sensor first
		if (!init_rak12014())
		{
			found_sensors[TOF_ID].found_sensor = false;
		}
	}

	if (found_sensors[UVL_ID].found_sensor)
	{
		if (!init_rak12019())
		{
			found_sensors[UVL_ID].found_sensor = false;
		}
	}

	if (found_sensors[CO2_ID].found_sensor)
	{
		if (init_rak12037())
		{
			sprintf(g_dev_name, "RUI3 Environment Sensor");
		}
		else
		{
			found_sensors[CO2_ID].found_sensor = false;
		}
	}

	if (found_sensors[VOC_ID].found_sensor)
	{
		if (init_rak12047())
		{
			sprintf(g_dev_name, "RUI3 VOC Sensor");
		}
		else
		{
			found_sensors[VOC_ID].found_sensor = false;
		}
	}

	if (found_sensors[GNSS_ID].found_sensor)
	{
		if (init_gnss())
		{
			sprintf(g_dev_name, "RUI3 Location Tracker");
			init_gnss_at();
		}
		else
		{
			found_sensors[GNSS_ID].found_sensor = false;
		}
	}
}

/**
 * @brief AT command feedback about found modules
 *
 */
void announce_modules(void)
{
	if (found_sensors[TEMP_ID].found_sensor)
	{
		Serial.println("+EVT:RAK1901 OK");
		// Reading sensor data
		read_rak1901();
	}

	if (found_sensors[PRESS_ID].found_sensor)
	{
		Serial.println("+EVT:RAK1902 OK");
		// Reading sensor data
		read_rak1902();
	}

	if (found_sensors[LIGHT_ID].found_sensor)
	{
		Serial.println("+EVT:RAK1903 OK");
		// Reading sensor data
		read_rak1903();
	}

	if (found_sensors[ACC_ID].found_sensor)
	{
		Serial.println("+EVT:RAK1904 OK");
		// Reading sensor data
		read_rak1904();
	}

	if (found_sensors[MPU_ID].found_sensor)
	{
		Serial.println("+EVT:RAK1905 OK");
		// Reading sensor data
		read_rak1905();
	}

	if (found_sensors[ENV_ID].found_sensor)
	{
		Serial.println("+EVT:RAK1906 OK");
		// Start reading sensor data
		start_rak1906();
		delay(100);
		// Reading sensor data
		read_rak1906();
	}

	if (found_sensors[RTC_ID].found_sensor)
	{
		Serial.println("+EVT:RAK12002 OK");
		read_rak12002();
	}

	if (found_sensors[FIR_ID].found_sensor)
	{
		Serial.println("+EVT:RAK12003 OK");
		read_rak12003();
	}

	if (found_sensors[LIGHT2_ID].found_sensor)
	{
		Serial.println("+EVT:RAK12010 OK");
		// Reading sensor data
		read_rak12010();
	}

	if (found_sensors[TOF_ID].found_sensor)
	{
		Serial.println("+EVT:RAK12014 OK");
		read_rak12014();
	}

	if (found_sensors[UVL_ID].found_sensor)
	{
		Serial.println("+EVT:RAK12019 OK");
		read_rak12019();
	}

	if (found_sensors[CO2_ID].found_sensor)
	{
		Serial.println("+EVT:RAK12037 OK");
		// Reading sensor data
		read_rak12037();
	}

	if (found_sensors[TEMP_ARR_ID].found_sensor)
	{
		Serial.println("+EVT:RAK12040 OK");
		read_rak12040();
	}

	if (found_sensors[VOC_ID].found_sensor)
	{
		Serial.println("+EVT:RAK12047 OK");
		// Sensor needs 100 readings before valid data is available.
		// Makes no sense to read it already.
	}

	if (found_sensors[GNSS_ID].found_sensor)
	{
		MYLOG("MOD", "+EVT:RAK12500 OK\n");
		// Sensor needs time to get location.
		// Makes no sense to read it already.
	}
}

/**
 * @brief Read values from the found modules
 *
 */
void get_sensor_values(void)
{
	if (found_sensors[TEMP_ID].found_sensor)
	{
		// Read sensor data
		read_rak1901();
	}

	if (found_sensors[PRESS_ID].found_sensor)
	{
		// Reading sensor data
		read_rak1902();
	}

	if (found_sensors[LIGHT_ID].found_sensor)
	{
		// Read sensor data
		read_rak1903();
	}

	if (found_sensors[ACC_ID].found_sensor)
	{
		// Read sensor data
		read_rak1904();
	}

	if (found_sensors[MPU_ID].found_sensor)
	{
		// Read sensor data
		read_rak1905();
	}

	if (found_sensors[ENV_ID].found_sensor)
	{
		// Start reading sensor data
		start_rak1906();
		delay(100);
		// Reading sensor data
		read_rak1906();
	}

	if (found_sensors[FIR_ID].found_sensor)
	{
		// Read sensor data
		read_rak12003();
	}

	if (found_sensors[LIGHT2_ID].found_sensor)
	{
		// Read sensor data
		read_rak12010();
	}

	if (found_sensors[RTC_ID].found_sensor)
	{
		// Read sensor data
		read_rak12002();
	}

	if (found_sensors[TOF_ID].found_sensor)
	{
		// Get the VL53L01 sensor values
		read_rak12014();
	}

	if (found_sensors[UVL_ID].found_sensor)
	{
		// Get the LTR390 sensor values
		read_rak12019();
	}

	if (found_sensors[CO2_ID].found_sensor)
	{
		// Read sensor data
		read_rak12037();
	}

	if (found_sensors[TEMP_ARR_ID].found_sensor)
	{
		// Get the temp array sensor values
		read_rak12040();
	}

	if (found_sensors[VOC_ID].found_sensor)
	{
		// Read sensor data
		read_rak12047();
	}
}