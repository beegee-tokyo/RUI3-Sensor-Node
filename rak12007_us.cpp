/**
 * @file rak12007_us.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialization and reading of RAK12007 US sensor
 * @version 0.1
 * @date 2023-12-18
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "main.h"

#define TIME_OUT 24125 // max measure distance is 4m,the velocity of sound is 331.6m/s in 0℃,TIME_OUT=4*2/331.6*1000000=24215us

float ratio = 346.6 / 1000 / 2; // velocity of sound =331.6+0.6*25℃(m/s),(Indoor temperature about 25℃)

bool has_rak12007 = false;

bool init_rak12007(void)
{
	// Initialize GPIO
	pinMode(ECHO, INPUT);  // Echo Pin of Ultrasonic Sensor is an Input
	pinMode(TRIG, OUTPUT); // Trigger Pin of Ultrasonic Sensor is an Output
	pinMode(PD, OUTPUT);   // power done control pin is an Output
	digitalWrite(TRIG, LOW);
	digitalWrite(PD, LOW); // Power up the sensor
	delay(500);
	has_rak12007 = read_rak12007(false);
	if (!has_rak12007)
	{
		MYLOG("US", "Timeout on first measurement, assuming no sensor");
	}
	digitalWrite(PD, HIGH); // Power down the sensor

	return has_rak12007;
}

bool read_rak12007(bool add_payload)
{
	uint32_t respond_time;
	uint64_t measure_time = 0;
	uint32_t distance = 0;
	uint16_t valid_measures = 0;

	digitalWrite(PD, LOW); // Power up the sensor
	digitalWrite(TRIG, LOW);
	delay(500);

	// Do 10 measurements to get an average
	for (int reading = 0; reading < 10; reading++)
	{
		digitalWrite(TRIG, HIGH);
		delayMicroseconds(20); // pull high time need over 10us
		digitalWrite(TRIG, LOW);
		respond_time = pulseInLong(ECHO, HIGH, 2000000L); // microseconds
		delay(33);
		if ((respond_time > 0) && (respond_time < TIME_OUT)) // ECHO pin max timeout is 33000us according it's datasheet
		{
#ifdef _VARIANT_RAK4630_
			measure_time += respond_time * 0.7726; // Time calibration factor is 0.7726,to get real time microseconds for 4631 board
#else
			measure_time += respond_time; // No time calibration
#endif
			valid_measures++;
		}
		else
		{
			MYLOG("US", "Timeout");
		}
		MYLOG("US", "Respond time is %d valid measures %d", respond_time, valid_measures);
		delay(500);
	}
	measure_time = measure_time / valid_measures;

	if ((measure_time > 0) && (measure_time < TIME_OUT)) // ECHO pin max timeout is 33000us according it's datasheet
	{
		// Calculate measured distance
		distance = measure_time * ratio; // Test distance = (high level time × velocity of sound (340M/S) / 2

		MYLOG("US", "Distance is %ld mm", distance);

		if (add_payload)
		{
			// Add level to the payload (in cm !)
			g_solution_data.addAnalogInput(LPP_CHANNEL_WLEVEL, (float)(distance / 1.0));
		}
		digitalWrite(PD, HIGH); // Power down the sensor
		digitalWrite(TRIG, HIGH);
		return true;
	}
	digitalWrite(PD, HIGH); // Power down the sensor
	digitalWrite(TRIG, HIGH);
	return false;
}
