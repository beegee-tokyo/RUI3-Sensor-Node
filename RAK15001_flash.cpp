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
#include "SparkFun_SPI_SerialFlash.h" //Click here to get the library: http://librarymanager/All#SparkFun_SPI_SerialFlash

/** Flash hal instance */
SFE_SPI_FLASH g_flash; // Sparkfun SPI Flash

/** Flash specific definition */
// SPIFlash_Device_t g_RAK15001{
// 	.total_size = (1UL << 21),
// 	.start_up_time_us = 5000,
// 	.manufacturer_id = 0xc8,
// 	.memory_type = 0x40,
// 	.capacity = 0x15,
// 	.max_clock_speed_mhz = 15,
// 	.quad_enable_bit_mask = 0x00,
// 	.has_sector_protection = false,
// 	.supports_fast_read = true,
// 	.supports_qspi = false,
// 	.supports_qspi_writes = false,
// 	.write_status_register_split = false,
// 	.single_status_byte = true,
// }; // Flash definition structure for GD25Q16C Flash

bool init_rak15001(void)
{
	g_flash.enableDebugging();
	if (!g_flash.begin(WB_SPI_CS)) // Start access to the flash
	{
		MYLOG("FLASH", "E: Flash access failed, check the settings");
		return false;
	}

	sfe_flash_manufacturer_e mfgID = g_flash.getManufacturerID();
	if (mfgID != SFE_FLASH_MFG_UNKNOWN)
	{
		MYLOG("FLASH","Manufacturer ID: 0x%02X %s", g_flash.getManufacturerID(),g_flash.manufacturerIDString(mfgID));
	}
	else
	{
		MYLOG("FLASH","Unknown manufacturer ID: 0x%02X", g_flash.getManufacturerID());
	}

	MYLOG("FLASH","Device ID: 0x%02X",g_flash.getDeviceID());
	return true;
}

bool read_rak15001(uint16_t address, uint8_t *buffer, uint16_t size)
{
	// Read the bytes
	uint16_t bytes_check = g_flash.readBlock(address, buffer, size);
	// Check if number of bytes is same as requester
	if (bytes_check != size)
	{
		MYLOG("FLASH", "E: Bytes read %d, requested %d", bytes_check, size);
		return false;
	}
	return true;
}

bool write_rak15001(uint16_t address, uint8_t *buffer, uint16_t size)
{
	uint8_t check_buff[size];
	// Write the bytes
	uint16_t bytes_check = g_flash.writeBlock(address, buffer, size);
	g_flash.blockingBusyWait();
	// Check if number of bytes is same as requester
	if (bytes_check != size)
	{
		MYLOG("FLASH", "E: Bytes written %d, requested %d", bytes_check, size);
		return false;
	}
	// Read back the data
	bytes_check = g_flash.readBlock(address, check_buff, size);
	// Check if number of bytes is same as requested
	if (bytes_check != size)
	{
		MYLOG("FLASH", "E: Bytes read back %d, requested %d", bytes_check, size);
		return false;
	}
	// Check if read data is same as requested data
	if (memcmp(check_buff, buffer, size) != 0)
	{
		MYLOG("FLASH", "E: Bytes read back are not the same as written");
		return false;
	}
	return true;
}

// #include "SdFat.h"
// #include "Adafruit_SPIFlash.h" //Click here to get the library: http://librarymanager/All#Adafruit_SPIFlash

// /** Flash access instance */
// Adafruit_FlashTransport_SPI g_flashTransport(SS, SPI); // Adafruit_FlashTransport_SPI flashTransport;

// /** Flash hal instance */
// Adafruit_SPIFlash g_flash(&g_flashTransport); // Adafruit SPI Flash

// /** Flash specific definition */
// SPIFlash_Device_t g_RAK15001{
// 	.total_size = (1UL << 21),
// 	.start_up_time_us = 5000,
// 	.manufacturer_id = 0xc8,
// 	.memory_type = 0x40,
// 	.capacity = 0x15,
// 	.max_clock_speed_mhz = 15,
// 	.quad_enable_bit_mask = 0x00,
// 	.has_sector_protection = false,
// 	.supports_fast_read = true,
// 	.supports_qspi = false,
// 	.supports_qspi_writes = false,
// 	.write_status_register_split = false,
// 	.single_status_byte = true,
// }; // Flash definition structure for GD25Q16C Flash

// bool init_rak15001(void)
// {
// 	if (!g_flash.begin(&g_RAK15001)) // Start access to the flash
// 	{
// 		MYLOG("FLASH", "E: Flash access failed, check the settings");
// 		return false;
// 	}

// 	g_flash.waitUntilReady(); // Wait for Flash to be ready
// 	return true;
// }

// bool read_rak15001(uint16_t address, uint8_t *buffer, uint16_t size)
// {
// 	// Read the bytes
// 	uint16_t bytes_check = g_flash.readBuffer(address, buffer, size);
// 	// Check if number of bytes is same as requester
// 	if (bytes_check != size)
// 	{
// 		MYLOG("FLASH", "E: Bytes read %d, requested %d", bytes_check, size);
// 		return false;
// 	}
// 	return true;
// }

// bool write_rak15001(uint16_t address, uint8_t *buffer, uint16_t size)
// {
// 	uint8_t check_buff[size];
// 	// Write the bytes
// 	uint16_t bytes_check = g_flash.writeBuffer(address, buffer, size);
// 	g_flash.waitUntilReady();
// 	// Check if number of bytes is same as requester
// 	if (bytes_check != size)
// 	{
// 		MYLOG("FLASH", "E: Bytes written %d, requested %d", bytes_check, size);
// 		return false;
// 	}
// 	// Read back the data
// 	bytes_check = g_flash.readBuffer(address, check_buff, size);
// 	// Check if number of bytes is same as requested
// 	if (bytes_check != size)
// 	{
// 		MYLOG("FLASH", "E: Bytes read back %d, requested %d", bytes_check, size);
// 		return false;
// 	}
// 	// Check if read data is same as requested data
// 	if (memcmp(check_buff, buffer, size) != 0)
// 	{
// 		MYLOG("FLASH", "E: Bytes read back are not the same as written");
// 		return false;
// 	}
// 	return true;
// }