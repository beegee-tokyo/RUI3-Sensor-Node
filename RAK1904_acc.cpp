/**
 * @file RAK1904_acc.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialize and read values from the LIS3DH sensor
 * @version 0.1
 * @date 2022-04-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "main.h"
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// Forward declarations
void int_callback_rak1904(void);

/** Sensor instance using Wire */
Adafruit_LIS3DH acc_sensor(&Wire);

/** For internal usage */
TwoWire *usedWire;

/** Interrupt pin, depends on slot */
uint8_t acc_int_pin = WB_IO3;

/** Last time a motion was detected */
time_t last_trigger = 0;

/** Flag if motion was detected */
bool motion_detected = false;

/**
 * @brief Read RAK1904 register
 *     Added here because Adafruit made that function private :-(
 *
 * @param chip_reg register address
 * @param dataToWrite data to write
 * @return true write success
 * @return false write failed
 */
bool rak1904_writeRegister(uint8_t chip_reg, uint8_t dataToWrite)
{
	// Write the byte
	usedWire->beginTransmission(LIS3DH_DEFAULT_ADDRESS);
	usedWire->write(chip_reg);
	usedWire->write(dataToWrite);
	if (usedWire->endTransmission() != 0)
	{
		return false;
	}

	return true;
}

/**
 * @brief Write RAK1904 register
 *     Added here because Adafruit made that function private :-(
 *
 * @param outputPointer
 * @param chip_reg
 * @return true read success
 * @return false read failed
 */
bool rak1904_readRegister(uint8_t *outputPointer, uint8_t chip_reg)
{
	// Return value
	uint8_t result;
	uint8_t numBytes = 1;

	usedWire->beginTransmission(LIS3DH_DEFAULT_ADDRESS);
	usedWire->write(chip_reg);
	if (usedWire->endTransmission() != 0)
	{
		return false;
	}
	usedWire->requestFrom(LIS3DH_DEFAULT_ADDRESS, numBytes);
	while (usedWire->available()) // slave may send less than requested
	{
		result = usedWire->read(); // receive a byte as a proper uint8_t
	}

	*outputPointer = result;
	return true;
}

/**
 * @brief Initialize LIS3DH 3-axis
 * acceleration sensor
 *
 * @return true If sensor was found and is initialized
 * @return false If sensor initialization failed
 */
bool init_rak1904(void)
{
	// Setup interrupt pin
	pinMode(acc_int_pin, INPUT);

	Wire.begin();
	usedWire = &Wire;

	acc_sensor.setDataRate(LIS3DH_DATARATE_10_HZ);
	acc_sensor.setRange(LIS3DH_RANGE_4_G);

	if (!acc_sensor.begin())
	{
		MYLOG("ACC", "ACC sensor initialization failed");
		return false;
	}

	// Enable interrupts
	acc_sensor.enableDRDY(true, 1);
	acc_sensor.enableDRDY(false, 2);

	uint8_t data_to_write = 0;
	data_to_write |= 0x20;									  // Z high
	data_to_write |= 0x08;									  // Y high
	data_to_write |= 0x02;									  // X high
	rak1904_writeRegister(LIS3DH_REG_INT1CFG, data_to_write); // Enable interrupts on high tresholds for x, y and z

	// Set interrupt trigger range
	data_to_write = 0;
	// Check if Helium Mapper is enabled
	if (gnss_format = 2)
	{
		data_to_write |= 0x3; // A lower threshold for mapping purposes
	}
	else
	{
		data_to_write |= 0x10; // 1/8 range
	}
	rak1904_writeRegister(LIS3DH_REG_INT1THS, data_to_write); // 1/8th range

	// Set interrupt signal length
	data_to_write = 0;
	data_to_write |= 0x01; // 1 * 1/50 s = 20ms
	rak1904_writeRegister(LIS3DH_REG_INT1DUR, data_to_write);

	rak1904_readRegister(&data_to_write, LIS3DH_REG_CTRL5);
	data_to_write &= 0xF3;									// Clear bits of interest
	data_to_write |= 0x08;									// Latch interrupt (Cleared by reading int1_src)
	rak1904_writeRegister(LIS3DH_REG_CTRL5, data_to_write); // Set interrupt to latching

	// Select interrupt pin 1
	data_to_write = 0;
	data_to_write |= 0x40; // AOI1 event (Generator 1 interrupt on pin 1)
	data_to_write |= 0x20; // AOI2 event ()
	rak1904_writeRegister(LIS3DH_REG_CTRL3, data_to_write);

	// No interrupt on pin 2
	rak1904_writeRegister(LIS3DH_REG_CTRL6, 0x00);

	// Enable high pass filter
	rak1904_writeRegister(LIS3DH_REG_CTRL2, 0x01);

	// Set low power mode
	data_to_write = 0;
	rak1904_readRegister(&data_to_write, LIS3DH_REG_CTRL1);
	data_to_write |= 0x08;
	rak1904_writeRegister(LIS3DH_REG_CTRL1, data_to_write);
	delay(100);
	data_to_write = 0;
	rak1904_readRegister(&data_to_write, 0x1E);
	data_to_write |= 0x90;
	rak1904_writeRegister(0x1E, data_to_write);
	delay(100);

	clear_int_rak1904();

	// Set the interrupt callback function
	MYLOG("ACC", "Int pin %s", acc_int_pin == WB_IO3 ? "WB_IO3" : "WB_IO5");
	attachInterrupt(acc_int_pin, int_callback_rak1904, RISING);

	last_trigger = millis();

	return true;
}

/**
 * @brief Reads the values from the LIS3DH sensor
 *
 */
void read_rak1904(void)
{
	sensors_event_t event;
	acc_sensor.getEvent(&event);
	MYLOG("ACC", "Acceleration in g (x,y,z): %f %f %f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
}

/**
 * @brief Assign/reassing interrupt pin
 *
 * @param new_irq_pin new GPIO to assign to interrupt
 */
void int_assign_rak1904(uint8_t new_irq_pin)
{
	detachInterrupt(acc_int_pin);
	acc_int_pin = new_irq_pin;
	attachInterrupt(acc_int_pin, int_callback_rak1904, RISING);
}

/**
 * @brief ACC interrupt handler
 * @note gives semaphore to wake up main loop
 *
 */
void int_callback_rak1904(void)
{
	MYLOG("ACC", "Interrupt triggered");
	// detachInterrupt(acc_int_pin);
	if ((millis() - last_trigger) > (g_lorawan_settings.send_repeat_time / 2) && !gnss_active)
	{
		motion_detected = true;
		last_trigger = millis();
		// Read the sensors and trigger a packet
		sensor_handler(NULL);
		// Stop a unified C timer in C language. This API is defined in udrv_timer.h. It will be replaced by api.system.timer.stop() after story #1195 is done.
		udrv_timer_stop(TIMER_0);
		// Start a unified C timer in C language. This API is defined in udrv_timer.h. It will be replaced by api.system.timer.start() after story #1195 is done.
		udrv_timer_start(TIMER_0, g_lorawan_settings.send_repeat_time, NULL);
	}
	else
	{
		MYLOG("ACC", "GNSS still active or too less time since last trigger");
		motion_detected = false;
	}
	attachInterrupt(acc_int_pin, int_callback_rak1904, RISING);
	clear_int_rak1904();
	// attachInterrupt(acc_int_pin, int_callback_rak1904, RISING);
}

/**
 * @brief Clear ACC interrupt register to enable next wakeup
 *
 */
void clear_int_rak1904(void)
{
	acc_sensor.readAndClearInterrupt();
	attachInterrupt(acc_int_pin, int_callback_rak1904, RISING);
}

/***********************************************************/

// #include <rak1904.h>

// // Forward declarations
// void int_callback_rak1904(void);

// /** Sensor instance using Wire */
// rak1904 myIMU();

// /** Interrupt pin, depends on slot RAK19007 => WB_IO3  RAK19003 => WB_IO5*/
// uint8_t acc_int_pin = WB_IO5;

// /** Last time a motion was detected */
// time_t last_trigger = 0;

// /** Flag if motion was detected */
// bool motion_detected = false;

// /**
//  * @brief Initialize LIS3DH 3-axis
//  * acceleration sensor
//  *
//  * @return true If sensor was found and is initialized
//  * @return false If sensor initialization failed
//  */
// bool init_rak1904(void)
// {
// 	Wire.begin();

// 	rak1904::settings.adcEnabled = 1;
// 	rak1904::settings.tempEnabled = 0;
// 	rak1904::settings.accelSampleRate = 10;
// 	rak1904::settings.accelRange = 4;
// 	rak1904::settings.xAccelEnabled = 1;
// 	rak1904::settings.yAccelEnabled = 1;
// 	rak1904::settings.zAccelEnabled = 1;

// 	myIMU.applySettings();

// 	// Enable interrupts
// 	uint8_t data_to_write = 0;
// 	data_to_write |= 0x20;								 // Z high
// 	data_to_write |= 0x08;								 // Y high
// 	data_to_write |= 0x02;								 // X high
// 	myIMU.writeRegister(RAK1904_INT1_CFG, data_to_write); // Enable interrupts on high tresholds for x, y and z

// 	// Set interrupt trigger range
// 	data_to_write = 0;
// 	data_to_write |= 0x10;								 // 1/8 range
// 	myIMU.writeRegister(RAK1904_INT1_THS, data_to_write); // 1/8th range

// 	// Set interrupt signal length
// 	data_to_write = 0;
// 	data_to_write |= 0x01; // 1 * 1/50 s = 20ms
// 	myIMU.writeRegister(RAK1904_INT1_DURATION, data_to_write);

// 	myIMU.readRegister(&data_to_write, LIS3DH_CTRL_REG5);
// 	data_to_write &= 0xF3;								  // Clear bits of interest
// 	data_to_write |= 0x08;								  // Latch interrupt (Cleared by reading int1_src)
// 	myIMU.writeRegister(RAK1904_CTRL_REG5, data_to_write); // Set interrupt to latching

// 	// Select interrupt pin 1
// 	data_to_write = 0;
// 	data_to_write |= 0x40; // AOI1 event (Generator 1 interrupt on pin 1)
// 	data_to_write |= 0x20; // AOI2 event ()
// 	myIMU.writeRegister(RAK1904_CTRL_REG3, data_to_write);

// 	// No interrupt on pin 2
// 	myIMU.writeRegister(RAK1904_CTRL_REG6, 0x00);

// 	// Enable high pass filter
// 	myIMU.writeRegister(RAK1904_CTRL_REG2, 0x01);

// 	// Set low power mode
// 	data_to_write = 0;
// 	myIMU.readRegister(&data_to_write, RAK1904_CTRL_REG1);
// 	data_to_write |= 0x08;
// 	myIMU.writeRegister(RAK1904_CTRL_REG1, data_to_write);
// 	delay(100);
// 	data_to_write = 0;
// 	myIMU.readRegister(&data_to_write, 0x1E);
// 	data_to_write |= 0x90;
// 	myIMU.writeRegister(0x1E, data_to_write);
// 	delay(100);

// 	clear_int_rak1904();

// 	// Set the interrupt callback function
// 	pinMode(acc_int_pin, INPUT_PULLDOWN);
// 	attachInterrupt(acc_int_pin, int_callback_rak1904, RISING);

// 	clear_int_rak1904();

// 	return true;
// }

// /**
//  * @brief Reads the values from the LIS3DH sensor
//  *
//  */
// void read_rak1904(void)
// {
// 	myIMU.update()
// 	MYLOG("ACC", "Acceleration in g (x,y,z): %f %f %f", myIMU.x(), myIMU.y(), myIMU.z());
// }

// /**
//  * @brief Assign/reassing interrupt pin
//  *
//  * @param new_irq_pin new GPIO to assign to interrupt
//  */
// void int_assign_rak1904(uint8_t new_irq_pin)
// {
// 	detachInterrupt(acc_int_pin);
// 	acc_int_pin = new_irq_pin;
// 	attachInterrupt(acc_int_pin, int_callback_rak1904, RISING);
// }

// /**
//  * @brief ACC interrupt handler
//  * @note gives semaphore to wake up main loop
//  *
//  */
// void int_callback_rak1904(void)
// {
// 	// if ((millis() - last_trigger) > 15000)
// 	{
// 		// MYLOG("ACC", "Interrupt triggered");
// 		last_trigger = millis();
// 		// Read the sensors and trigger a packet
// 		sensor_handler(NULL);
// 		// Stop a unified C timer in C language. This API is defined in udrv_timer.h. It will be replaced by api.system.timer.stop() after story #1195 is done.
// 		udrv_timer_stop(TIMER_0);
// 		// Start a unified C timer in C language. This API is defined in udrv_timer.h. It will be replaced by api.system.timer.start() after story #1195 is done.
// 		udrv_timer_start(TIMER_0, g_lorawan_settings.send_repeat_time, NULL);
// 		motion_detected = true;
// 	}
// 	clear_int_rak1904();
// }

// /**
//  * @brief Clear ACC interrupt register to enable next wakeup
//  *
//  */
// void clear_int_rak1904(void)
// {
// 	uint8_t dataRead;
// 	myIMU.readRegister(&dataRead, RAK1904_INT1_SRC); // cleared by reading
// }

/***********************************************************/

// #include <SparkFunLIS3DH.h>

// // Forward declarations
// void int_callback_rak1904(void);

// /** Sensor instance using Wire */
// LIS3DH myIMU(I2C_MODE, 0x18);

// /** Interrupt pin, depends on slot RAK19007 => WB_IO3  RAK19003 => WB_IO5*/
// uint8_t acc_int_pin = WB_IO5;

// /** Last time a motion was detected */
// time_t last_trigger = 0;

// /** Flag if motion was detected */
// bool motion_detected = false;

// /**
//  * @brief Initialize LIS3DH 3-axis
//  * acceleration sensor
//  *
//  * @return true If sensor was found and is initialized
//  * @return false If sensor initialization failed
//  */
// bool init_rak1904(void)
// {
// 	Wire.begin();

// 	myIMU.settings.accelSampleRate = 10;
// 	myIMU.settings.accelRange = 4;

// 	myIMU.settings.xAccelEnabled = 1;
// 	myIMU.settings.yAccelEnabled = 1;
// 	myIMU.settings.zAccelEnabled = 1;

// 	myIMU.begin();

// 	// Enable interrupts
// 	uint8_t data_to_write = 0;
// 	data_to_write |= 0x20;								 // Z high
// 	data_to_write |= 0x08;								 // Y high
// 	data_to_write |= 0x02;								 // X high
// 	myIMU.writeRegister(LIS3DH_INT1_CFG, data_to_write); // Enable interrupts on high tresholds for x, y and z

// 	// Set interrupt trigger range
// 	data_to_write = 0;
// 	data_to_write |= 0x10;								 // 1/8 range
// 	myIMU.writeRegister(LIS3DH_INT1_THS, data_to_write); // 1/8th range

// 	// Set interrupt signal length
// 	data_to_write = 0;
// 	data_to_write |= 0x01; // 1 * 1/50 s = 20ms
// 	myIMU.writeRegister(LIS3DH_INT1_DURATION, data_to_write);

// 	myIMU.readRegister(&data_to_write, LIS3DH_CTRL_REG5);
// 	data_to_write &= 0xF3;								  // Clear bits of interest
// 	data_to_write |= 0x08;								  // Latch interrupt (Cleared by reading int1_src)
// 	myIMU.writeRegister(LIS3DH_CTRL_REG5, data_to_write); // Set interrupt to latching

// 	// Select interrupt pin 1
// 	data_to_write = 0;
// 	data_to_write |= 0x40; // AOI1 event (Generator 1 interrupt on pin 1)
// 	data_to_write |= 0x20; // AOI2 event ()
// 	myIMU.writeRegister(LIS3DH_CTRL_REG3, data_to_write);

// 	// No interrupt on pin 2
// 	myIMU.writeRegister(LIS3DH_CTRL_REG6, 0x00);

// 	// Enable high pass filter
// 	myIMU.writeRegister(LIS3DH_CTRL_REG2, 0x01);

// 	// Set low power mode
// 	data_to_write = 0;
// 	myIMU.readRegister(&data_to_write, LIS3DH_CTRL_REG1);
// 	data_to_write |= 0x08;
// 	myIMU.writeRegister(LIS3DH_CTRL_REG1, data_to_write);
// 	delay(100);
// 	data_to_write = 0;
// 	myIMU.readRegister(&data_to_write, 0x1E);
// 	data_to_write |= 0x90;
// 	myIMU.writeRegister(0x1E, data_to_write);
// 	delay(100);

// 	clear_int_rak1904();

// 	// Set the interrupt callback function
// 	pinMode(acc_int_pin, INPUT_PULLDOWN);
// 	attachInterrupt(acc_int_pin, int_callback_rak1904, RISING);

// 	clear_int_rak1904();

// 	return true;
// }

// /**
//  * @brief Reads the values from the LIS3DH sensor
//  *
//  */
// void read_rak1904(void)
// {
// 	MYLOG("ACC", "Acceleration in g (x,y,z): %f %f %f", myIMU.readFloatAccelX(), myIMU.readFloatAccelY(), myIMU.readFloatAccelZ());
// }

// /**
//  * @brief Assign/reassing interrupt pin
//  *
//  * @param new_irq_pin new GPIO to assign to interrupt
//  */
// void int_assign_rak1904(uint8_t new_irq_pin)
// {
// 	detachInterrupt(acc_int_pin);
// 	acc_int_pin = new_irq_pin;
// 	attachInterrupt(acc_int_pin, int_callback_rak1904, RISING);
// }

// /**
//  * @brief ACC interrupt handler
//  * @note gives semaphore to wake up main loop
//  *
//  */
// void int_callback_rak1904(void)
// {
// 	// if ((millis() - last_trigger) > 15000)
// 	{
// 		// MYLOG("ACC", "Interrupt triggered");
// 		last_trigger = millis();
// 		// Read the sensors and trigger a packet
// 		sensor_handler(NULL);
// 		// Stop a unified C timer in C language. This API is defined in udrv_timer.h. It will be replaced by api.system.timer.stop() after story #1195 is done.
// 		udrv_timer_stop(TIMER_0);
// 		// Start a unified C timer in C language. This API is defined in udrv_timer.h. It will be replaced by api.system.timer.start() after story #1195 is done.
// 		udrv_timer_start(TIMER_0, g_lorawan_settings.send_repeat_time, NULL);
// 		motion_detected = true;
// 	}
// 	clear_int_rak1904();
// }

// /**
//  * @brief Clear ACC interrupt register to enable next wakeup
//  *
//  */
// void clear_int_rak1904(void)
// {
// 	uint8_t dataRead;
// 	myIMU.readRegister(&dataRead, LIS3DH_INT1_SRC); // cleared by reading
// }
