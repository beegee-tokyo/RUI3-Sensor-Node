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

/** Initialization results */
bool ret;

/** LoRaWAN packet */
WisCayenne g_solution_data(255);

/** Set the device name, max length is 10 characters */
char g_dev_name[64] = "RUI3 Sensor Node                                              ";

/** Packet is confirmed/unconfirmed (Set with AT commands) */
bool g_confirmed_mode = false;
/** If confirmed packet, number or retries (Set with AT commands)*/
uint8_t g_confirmed_retry = 0;
uint8_t g_data_rate = 3;

/** fPort to send packages */
uint8_t set_fPort = 2;

/** Counter for GNSS readings */
uint16_t check_gnss_counter = 0;
/** Max number of GNSS readings before giving up */
uint16_t check_gnss_max_try = 0;

/** Flag for GNSS readings active */
bool gnss_active = false;

/**
 * @brief Callback after packet was received
 *
 * @param data Structure with the received data
 */
void receiveCallback(SERVICE_LORA_RECEIVE_T *data)
{
	MYLOG("RX-CB", "RX, port %d, DR %d, RSSI %d, SNR %d", data->Port, data->RxDatarate, data->Rssi, data->Snr);
	for (int i = 0; i < data->BufferSize; i++)
	{
		Serial.printf("%02X", data->Buffer[i]);
	}
	Serial.print("\r\n");
	if ((gnss_format == FIELD_TESTER) && (data->Port == 2))
	{
		int16_t min_rssi = data->Buffer[1] - 200;
		int16_t max_rssi = data->Buffer[2] - 200;
		int16_t min_distance = data->Buffer[3] * 250;
		int16_t max_distance = data->Buffer[4] * 250;
		int8_t num_gateways = data->Buffer[5];
		Serial.printf("+EVT:FieldTester %d gateways\n", num_gateways);
		Serial.printf("+EVT:RSSI min %d max %d\n", min_rssi, max_rssi);
		Serial.printf("+EVT:Distance min %d max %d\n", min_distance, max_distance);
	}
}

/**
 * @brief Callback after TX is finished
 *
 * @param status TX status
 */
void sendCallback(int32_t status)
{
	gnss_active = false;
	MYLOG("TX-CB", "TX status %d", status);
	digitalWrite(LED_BLUE, LOW);
}

/**
 * @brief Callback after join request cycle
 *
 * @param status Join result
 */
void joinCallback(int32_t status)
{
	// MYLOG("JOIN-CB", "Join result %d", status);
	if (status != 0)
	{
		if (!(ret = api.lorawan.join()))
		{
			MYLOG("JOIN-CB", "LoRaWan OTAA - join fail! \r\n");
			if (found_sensors[OLED_ID].found_sensor)
			{
				rak1921_add_line((char *)"Join NW failed");
			}
		}
	}
	else
	{
		bool result_set_dr = api.lorawan.dr.set(g_data_rate);
		MYLOG("JOIN-CB", "Set the data rate  %s", result_set_dr ? "Success" : "Fail");
		MYLOG("JOIN-CB", "LoRaWan OTAA - joined! \r\n");
		digitalWrite(LED_BLUE, LOW);

		if (found_sensors[OLED_ID].found_sensor)
		{
			rak1921_add_line((char *)"Joined NW");
		}
	}
}

/**
 * @brief Arduino setup, called once after reboot/power-up
 *
 */
void setup()
{
	g_confirmed_mode = api.lorawan.cfm.get();

	g_confirmed_retry = api.lorawan.rety.get();

	g_data_rate = api.lorawan.dr.get();

	// Setup the callbacks for joined and send finished
	api.lorawan.registerRecvCallback(receiveCallback);
	api.lorawan.registerSendCallback(sendCallback);
	api.lorawan.registerJoinCallback(joinCallback);

	pinMode(LED_GREEN, OUTPUT);
	digitalWrite(LED_GREEN, HIGH);
	pinMode(LED_BLUE, OUTPUT);
	digitalWrite(LED_BLUE, HIGH);

	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);

	// Use RAK_CUSTOM_MODE supresses AT command and default responses from RUI3
	// Serial.begin(115200, RAK_CUSTOM_MODE);
	// Use "normal" mode to have AT commands available
	Serial.begin(115200);

#ifdef _VARIANT_RAK4630_
	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial.available())
	// while (!Serial)
	{
		if ((millis() - serial_timeout) < 15000)
		{
			delay(100);
			digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
		}
		else
		{
			break;
		}
	}
#else
	// For RAK3172 just wait a little bit for the USB to be ready
	delay(5000);
#endif

	// USE BELOW WHEN CREDENTIALS ARE NOT SET THROUGH AT COMMANDS

	// uint8_t node_device_eui[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x88};

	// uint8_t node_app_eui[8] = {0x0E, 0x0D, 0x0D, 0x01, 0x0E, 0x01, 0x02, 0x0E};

	// uint8_t node_app_key[16] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3E};


	// api.lorawan.deui.get(node_device_eui, 8);
	// api.lorawan.appeui.get(node_app_eui, 8);
	// api.lorawan.appkey.get(node_app_key, 16);

	// Serial.print("DevEUI: ");
	// for (int idx=0; idx < 8; idx++)
	// {
	// 	Serial.printf("%02X", node_device_eui[idx]);
	// }
	// Serial.println();

	// Serial.print("AppEUI: ");
	// for (int idx = 0; idx < 8; idx++)
	// {
	// 	Serial.printf("%02X", node_app_eui[idx]);
	// }
	// Serial.println();

	// Serial.print("AppKey: ");
	// for (int idx = 0; idx < 16; idx++)
	// {
	// 	Serial.printf("%02X", node_app_key[idx]);
	// }
	// Serial.println();

	// if (api.lorawan.appeui.set(node_app_eui, 8) == true)
	// 	Serial.println("LoRaWan AppEUI set success");
	// else
	// 	Serial.println("LoRaWan AppEUI set fail");

	// if (api.lorawan.appkey.set(node_app_key, 16) == true)
	// 	Serial.println("LoRaWan AppKey set success");
	// else
	// 	Serial.println("LoRaWan AppKey set fail");

	// if (api.lorawan.deui.set(node_device_eui, 8) == true)
	// 	Serial.println("LoRaWan device EUI set success");
	// else
	// 	Serial.println("LoRaWan device EUI set fail");


	// Find WisBlock I2C modules
	find_modules();

	Serial.printf("RAKwireless %s Node\n", g_dev_name);
	Serial.println("------------------------------------------------------");
	Serial.println("Setup the device with WisToolBox or AT commands before using it");
	Serial.println("------------------------------------------------------");

	// Register the custom AT command to get device status
	if (!init_status_at())
	{
		MYLOG("SETUP", "Add custom AT command STATUS fail");
	}
	digitalWrite(LED_GREEN, LOW);

	// Register the custom AT command to set the send interval
	if (!init_frequency_at())
	{
		MYLOG("SETUP", "Add custom AT command Send Interval fail");
	}
	// Get saved sending frequency from flash
	get_at_setting(SEND_INTERVAL_OFFSET);

	// Create a timer.
	api.system.timer.create(RAK_TIMER_0, sensor_handler, RAK_TIMER_PERIODIC);
	if (g_send_interval_time != 0)
	{
		// Start a timer.
		api.system.timer.start(RAK_TIMER_0, g_send_interval_time, NULL);
	}

	// If a GNSS module was found, setup a timer for the GNSS aqcuisions
	if (found_sensors[GNSS_ID].found_sensor)
	{
		MYLOG("SETUP", "Create timer for GNSS polling");
		// Create a timer.
		api.system.timer.create(RAK_TIMER_1, gnss_handler, RAK_TIMER_PERIODIC); // HTMR_ONESHOT);
	}

	// MYLOG("SETUP", "Waiting for Lorawan join...");
	// // wait for Join success
	// while (api.lorawan.njs.get() == 0)
	// {
	// 	api.lorawan.join();
	// 	delay(10000);
	// }

	// Show found modules
	announce_modules();
	digitalWrite(LED_BLUE, LOW);
}

/**
 * @brief GNSS location aqcuisition
 * Called every 2.5 seconds by timer 1
 * Gives up after 1/2 of send interval
 * or when location was aquired
 *
 */
void gnss_handler(void *)
{
	digitalWrite(LED_GREEN, HIGH);
	if (poll_gnss())
	{
		// Power down the module
		digitalWrite(WB_IO2, LOW);
		delay(100);
		MYLOG("GNSS", "Got location");
		api.system.timer.stop(RAK_TIMER_1);
		send_packet();
	}
	else
	{
		if (check_gnss_counter >= check_gnss_max_try)
		{
			// Power down the module
			digitalWrite(WB_IO2, LOW);
			delay(100);
			MYLOG("GNSS", "Location timeout");
			api.system.timer.stop(RAK_TIMER_1);
			if (gnss_format != HELIUM_MAPPER)
			{
				send_packet();
			}
		}
	}
	check_gnss_counter++;
	digitalWrite(LED_GREEN, LOW);
}

/**
 * @brief sensor_handler is a timer function called every
 * g_send_interval_time milliseconds. Default is 120000. Can be
 * changed in main.h
 *
 */
void sensor_handler(void *)
{
	// MYLOG("UPLINK", "Start");
	digitalWrite(LED_BLUE, HIGH);

	// Reset trigger time
	last_trigger = millis();

	// Check if the node has joined the network
	if (!api.lorawan.njs.get())
	{
		MYLOG("UPLINK", "Not joined, skip sending");
		return;
	}

	// Just for debug, show if the call is because of a motion detection
	if (motion_detected)
	{
		MYLOG("UPLINK", "ACC triggered IRQ");
		motion_detected = false;
		clear_int_rak1904();
		if (gnss_active)
		{
			// digitalWrite(LED_BLUE, LOW);
			// GNSS is already active, do nothing
			return;
		}
	}

	// Clear payload
	g_solution_data.reset();

	// Helium Mapper ignores sensor and sends only location data
	if ((gnss_format != HELIUM_MAPPER) && (gnss_format != FIELD_TESTER))
	{
		// Read sensor data
		get_sensor_values();

		// Add battery voltage
		g_solution_data.addVoltage(LPP_CHANNEL_BATT, api.system.bat.get());
	}

	// If it is a GNSS location tracker, start the timer to aquire the location
	if ((found_sensors[GNSS_ID].found_sensor) && !gnss_active)
	{
		// Set flag for GNSS active to avoid retrigger */
		gnss_active = true;
		// Startup GNSS module
		init_gnss();
		// Start the timer
		api.system.timer.start(RAK_TIMER_1, 2500, NULL);
		check_gnss_counter = 0;
		// Max location aquisition time is half of send interval
		check_gnss_max_try = g_send_interval_time / 2 / 2500;
	}
	else if (gnss_active)
	{
		return;
	}
	else
	{
		// No GNSS module, just send the packet with the sensor data
		send_packet();
	}
}

/**
 * @brief This example is complete timer
 * driven. The loop() does nothing than
 * sleep.
 *
 */
void loop()
{
	// api.system.sleep.all();
	api.system.scheduler.task.destroy();
}

/**
 * @brief Send the data packet that was prepared in
 * Cayenne LPP format by the different sensor and location
 * aqcuision functions
 *
 */
void send_packet(void)
{
	Serial.printf("Send packet with size %d on port %d\n", g_solution_data.getSize(), set_fPort);

	// If RAK1921 OLED is available, show some information on the display
	if (found_sensors[OLED_ID].found_sensor)
	{
		char disp_line[254];
		sprintf(disp_line, "Send packet %d bytes", g_solution_data.getSize());
		rak1921_add_line(disp_line);
		sprintf(disp_line, "Seconds since boot %ld", millis() / 1000);
		rak1921_add_line(disp_line);
	}

	// Send the packet
	if (api.lorawan.send(g_solution_data.getSize(), g_solution_data.getBuffer(), set_fPort, g_confirmed_mode, g_confirmed_retry))
	{
		MYLOG("UPLINK", "Packet enqueued");
	}
	else
	{
		MYLOG("UPLINK", "Send failed");
	}
}
