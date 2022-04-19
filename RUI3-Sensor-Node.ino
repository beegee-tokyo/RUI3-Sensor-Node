/**
 * @file WisBlock-Sensor-Node.ino
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief RUI3 based code for easy testing of WisBlock I2C modules
 * @version 0.1
 * @date 2022-04-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "main.h"
#include "udrv_timer.h"

/** Initialization results */
bool ret;

/** LoRaWAN packet */
WisCayenne g_solution_data(255);

/** Set the device name, max length is 10 characters */
char g_dev_name[64] = "RUI3 Sensor";

/** Device settings */
s_lorawan_settings g_lorawan_settings;

/** OTAA Device EUI MSB */
uint8_t node_device_eui[8] = {0}; // ac1f09fff8683172
/** OTAA Application EUI MSB */
uint8_t node_app_eui[8] = {0}; // ac1f09fff8683172
/** OTAA Application Key MSB */
uint8_t node_app_key[16] = {0}; // efadff29c77b4829acf71e1a6e76f713

/**
 * @brief Callback after TX is finished
 *
 * @param status TX status
 */
void sendCallback(int32_t status)
{
	MYLOG("TX-CB", "Packet sent finished, status %d", status);
	digitalWrite(LED_BLUE, LOW);
}

/**
 * @brief Callback after join request cycle
 *
 * @param status Join result
 */
void joinCallback(int32_t status)
{
	MYLOG("JOIN-CB", "Join result %d", status);
	if (status != 0)
	{
		if (!(ret = api.lorawan.join()))
		{
			MYLOG("JOIN-CB", "LoRaWan OTAA - join fail! \r\n");
			rak1921_add_line("Join NW failed");
		}
	}
	else
	{
		MYLOG("JOIN-CB", "Set the data rate  %s", api.lorawan.dr.set(g_lorawan_settings.data_rate) ? "Success" : "Fail");
		MYLOG("JOIN-CB", "Disable ADR  %s", api.lorawan.adr.set(g_lorawan_settings.adr_enabled ? 1 : 0) ? "Success" : "Fail");

		digitalWrite(LED_BLUE, LOW);

		if (found_sensors[OLED_ID].found_sensor)
		{
			rak1921_add_line("Joined NW");
		}
	}
}

/**
 * @brief Arduino setup, called once after reboot/power-up
 *
 */
void setup()
{
	pinMode(LED_GREEN, OUTPUT);
	digitalWrite(LED_GREEN, HIGH);
	pinMode(LED_BLUE, OUTPUT);
	digitalWrite(LED_BLUE, HIGH);

	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);

	Serial.begin(115200, RAK_CUSTOM_MODE);
	// Serial.begin(115200, RAK_AT_MODE);

	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial.available())
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
		}
		else
		{
			break;
		}
	}

	// Find WisBlock I2C modules
	find_modules();

	MYLOG("SETUP", "RAKwireless %s Node", g_dev_name);
	MYLOG("SETUP", "------------------------------------------------------");

	// Check if credentials are in the Flash module
	if (g_has_rak15001)
	{
		s_lorawan_settings check_settings;
		if (read_rak15001(0, (uint8_t *)&check_settings, sizeof(s_lorawan_settings)))
		{
			// Check if it is LPWAN settings
			if ((check_settings.valid_mark_1 != 0xAA) || (check_settings.valid_mark_2 != LORAWAN_DATA_MARKER))
			{
				// Data is not valid, reset to defaults
				MYLOG("SETUP", "Invalid data set");
				if (write_rak15001(0, (uint8_t *)&g_lorawan_settings, sizeof(s_lorawan_settings)))
				{
					MYLOG("SETUP", "Wrote default data set");
				}
				else
				{
					MYLOG("SETUP", "Set default dataset");
					log_settings();
				}
			}
			else
			{
				MYLOG("SETUP", "Found valid dataset");
				log_settings();
			}
			if (!api.lorawan.deui.set(g_lorawan_settings.node_device_eui, 8))
			{
				MYLOG("SETUP", "LoRaWan OTAA - set device EUI failed!");
				return;
			}
			if (!api.lorawan.appeui.set(g_lorawan_settings.node_app_eui, 8))
			{
				MYLOG("SETUP", "LoRaWan OTAA - set app EUI failed!");
				return;
			}
			if (!api.lorawan.appkey.set(g_lorawan_settings.node_app_key, 16))
			{
				MYLOG("SETUP", "LoRaWan OTAA - set app key failed!");
				return;
			}

		}
		else
		{
			MYLOG("SETUP", "Reading from RAK15001 failed");
			g_has_rak15001 = false;
		}
	}
	else
	{
		bool creds_ok = true;
		if (api.lorawan.appeui.get(node_app_eui, 8))
		{
			MYLOG("SETUP", "Got AppEUI %02X%02X%02X%02X%02X%02X%02X%02X",
				  node_app_eui[0], node_app_eui[1], node_app_eui[2], node_app_eui[3],
				  node_app_eui[4], node_app_eui[5], node_app_eui[6], node_app_eui[7]);
			if (node_app_eui[0] == 0)
			{
				creds_ok = false;
			}
			else
			{
				creds_ok = true;
			}
		}
		// else
		if (!creds_ok)
		{
			MYLOG("SETUP", "LoRaWan OTAA - set application EUI!"); //
			node_app_eui[0] = 0xac;
			node_app_eui[1] = 0x1f;
			node_app_eui[2] = 0x09;
			node_app_eui[3] = 0xff;
			node_app_eui[4] = 0xf8;
			node_app_eui[5] = 0x68;
			node_app_eui[6] = 0x31;
			node_app_eui[7] = 0x72;

			if (!api.lorawan.appeui.set(node_app_eui, 8))
			{
				MYLOG("SETUP", "LoRaWan OTAA - set app EUI failed!");
				return;
			}
		}
		if (api.lorawan.appeui.get(node_app_eui, 8))
		{
			MYLOG("SETUP", "Got AppEUI %02X%02X%02X%02X%02X%02X%02X%02X",
				  node_app_eui[0], node_app_eui[1], node_app_eui[2], node_app_eui[3],
				  node_app_eui[4], node_app_eui[5], node_app_eui[6], node_app_eui[7]);
		}

		if (api.lorawan.appkey.get(node_app_key, 16))
		{
			MYLOG("SETUP", "Got AppKEY %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
				  node_app_key[0], node_app_key[1], node_app_key[2], node_app_key[3],
				  node_app_key[4], node_app_key[5], node_app_key[6], node_app_key[7],
				  node_app_key[8], node_app_key[9], node_app_key[10], node_app_key[11],
				  node_app_key[12], node_app_key[13], node_app_key[14], node_app_key[15]);
			if (node_app_key[0] == 0)
			{
				creds_ok = false;
			}
			else
			{
				creds_ok = true;
			}
		}
		// else
		if (!creds_ok)
		{
			MYLOG("SETUP", "LoRaWan OTAA - set application key!"); //
			node_app_key[0] = 0xef;
			node_app_key[1] = 0xad;
			node_app_key[2] = 0xff;
			node_app_key[3] = 0x29;
			node_app_key[4] = 0xc7;
			node_app_key[5] = 0x7b;
			node_app_key[6] = 0x48;
			node_app_key[7] = 0x29;
			node_app_key[8] = 0xac;
			node_app_key[9] = 0xf7;
			node_app_key[10] = 0x1e;
			node_app_key[11] = 0x1a;
			node_app_key[12] = 0x6e;
			node_app_key[13] = 0x76;
			node_app_key[14] = 0xf7;
			node_app_key[15] = 0x13;
			if (!api.lorawan.appkey.set(node_app_key, 16))
			{
				MYLOG("SETUP", "LoRaWan OTAA - set application key failed!");
				return;
			}
		}
		if (api.lorawan.appkey.get(node_app_key, 16))
		{
			MYLOG("SETUP", "Got AppKEY %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
				  node_app_key[0], node_app_key[1], node_app_key[2], node_app_key[3],
				  node_app_key[4], node_app_key[5], node_app_key[6], node_app_key[7],
				  node_app_key[8], node_app_key[9], node_app_key[10], node_app_key[11],
				  node_app_key[12], node_app_key[13], node_app_key[14], node_app_key[15]);
		}

		if (api.lorawan.deui.get(node_device_eui, 8))
		{
			MYLOG("SETUP", "Got DevEUI %02X%02X%02X%02X%02X%02X%02X%02X",
				  node_device_eui[0], node_device_eui[1], node_device_eui[2], node_device_eui[3],
				  node_device_eui[4], node_device_eui[5], node_device_eui[6], node_device_eui[7]);
			if (node_device_eui[0] == 0)
			{
				creds_ok = false;
			}
			else
			{
				creds_ok = true;
			}
		}
		// else
		if (!creds_ok)
		{
			MYLOG("SETUP", "LoRaWan OTAA - set device EUI!"); //
			node_device_eui[0] = 0xac;
			node_device_eui[1] = 0x1f;
			node_device_eui[2] = 0x09;
			node_device_eui[3] = 0xff;
			node_device_eui[4] = 0xf8;
			node_device_eui[5] = 0x68;
			node_device_eui[6] = 0x31;
			node_device_eui[7] = 0x72;
			if (!api.lorawan.deui.set(node_device_eui, 8))
			{
				MYLOG("SETUP", "LoRaWan OTAA - set device EUI failed! \r\n");
				return;
			}
		}
		if (api.lorawan.deui.get(node_device_eui, 8))
		{
			MYLOG("SETUP", "Got DevEUI %02X%02X%02X%02X%02X%02X%02X%02X",
				  node_device_eui[0], node_device_eui[1], node_device_eui[2], node_device_eui[3],
				  node_device_eui[4], node_device_eui[5], node_device_eui[6], node_device_eui[7]);
		}
	}

	/*************************************

	   LoRaWAN band setting:
		 EU433: 0
		 CN470: 1
		 RU864: 2
		 IN865: 3
		 EU868: 4
		 US915: 5
		 AU915: 6
		 KR920: 7
		 AS923: 8

	 * ************************************/

	MYLOG("SETUP", "Set the transmit power %s", api.lorawan.txp.set(g_lorawan_settings.tx_power) ? "Success" : "Fail");

	MYLOG("SETUP", "Set the region %s", api.lorawan.band.set(g_lorawan_settings.lora_region) ? "Success" : "Fail");

	uint16_t maskBuff = 0x0001; // << (g_lorawan_settings.subband_channels - 1);
	MYLOG("SETUP", "Set the channel mask %s", api.lorawan.mask.set(&maskBuff) ? "Success" : "Fail");
	maskBuff = 0x0000;
	api.lorawan.mask.get(&maskBuff);
	MYLOG("SETUP", "Channel mask 0x%04X", maskBuff);

	MYLOG("SETUP", "Set the network join mode %s", api.lorawan.njm.set(g_lorawan_settings.otaa_enabled) ? "Success" : "Fail");

	if (!(ret = api.lorawan.join()))
	{
		MYLOG("SETUP", "LoRaWan OTAA - join fail! \r\n");
		return;
	}
	digitalWrite(LED_GREEN, LOW);

	MYLOG("SETUP", "Set the data rate  %s", api.lorawan.dr.set(g_lorawan_settings.data_rate) ? "Success" : "Fail");

	MYLOG("SETUP", "Disable duty cycle  %s", api.lorawan.dcs.set(g_lorawan_settings.duty_cycle_enabled ? 1 : 0) ? "Success" : "Fail");

	MYLOG("SETUP", "Enable confirmed packets  %s", api.lorawan.cfm.set(g_lorawan_settings.confirmed_msg_enabled) ? "Success" : "Fail");

	api.lorawan.registerSendCallback(sendCallback);
	api.lorawan.registerJoinCallback(joinCallback);

	// Create a unified timer in C language. This API is defined in udrv_timer.h. It will be replaced by api.system.timer.create() after story #1195 is done.
	udrv_timer_create(TIMER_0, sensor_handler, HTMR_PERIODIC);
	// Start a unified C timer in C language. This API is defined in udrv_timer.h. It will be replaced by api.system.timer.start() after story #1195 is done.
	udrv_timer_start(TIMER_0, RAK_SENSOR_PERIOD, NULL);

	MYLOG("SETUP", "Waiting for Lorawan join...");
	// wait for Join success
	while (api.lorawan.njs.get() == 0)
	{
		api.lorawan.join();
		delay(5000);
	}

	// Show found modules
	announce_modules();
	digitalWrite(LED_BLUE, LOW);
}

void sensor_handler(void *)
{
	MYLOG("SENS", "Start");
	digitalWrite(LED_BLUE, HIGH);

	if (api.lorawan.deui.get(node_device_eui, 8))
	{
		MYLOG("SETUP", "Got DevEUI %02X%02X%02X%02X%02X%02X%02X%02X",
			  node_device_eui[0], node_device_eui[1], node_device_eui[2], node_device_eui[3],
			  node_device_eui[4], node_device_eui[5], node_device_eui[6], node_device_eui[7]);
	}

	// Reset trigger time
	last_trigger = millis();

	// Check if the node has joined the network
	if (!api.lorawan.njs.get())
	{
		MYLOG("UPLINK", "Not joined, skip sending");
		return;
	}

	// Clear payload
	g_solution_data.reset();

	// Read sensor data
	get_sensor_values();

	// Send packet
	g_solution_data.addVoltage(LPP_CHANNEL_BATT, api.system.bat.get());

	MYLOG("UPLINK", "Send packet at %ld", millis() / 1000);

	if (found_sensors[OLED_ID].found_sensor)
	{
		char disp_line[254];
		sprintf(disp_line, "Send packet %d bytes", g_solution_data.getSize());
		rak1921_add_line(disp_line);
		sprintf(disp_line, "Seconds since boot %ld", millis() / 1000);
		rak1921_add_line(disp_line);
	}

	if (api.lorawan.send(g_solution_data.getSize(), g_solution_data.getBuffer(), 2, true, 1))
	{
		MYLOG("UPLINK", "Packet enqueued");
	}
	else
	{
		MYLOG("UPLINK", "Send fail");
	}
}

void loop()
{
	api.system.sleep.all();
	// if (Serial.available())
	// {
	// 	char ser_buff[256];
	// 	size_t ser_rcvd = 0;
	// 	Serial.readBytesUntil('\n', ser_buff, ser_rcvd);
	// 	Serial.printf("Ser RX: %s", ser_buff);
	// }
}