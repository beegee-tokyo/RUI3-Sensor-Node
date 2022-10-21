/**
 * @file main.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Globals and Includes
 * @version 0.1
 * @date 2022-04-10
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <Arduino.h>

#ifdef _VARIANT_RAK3172_
#error This code is too large for RAk3172, check the separate examples for RAK3172
#endif
// Debug
// Debug output set to 0 to disable app debug output
#ifndef MY_DEBUG
#define MY_DEBUG 0
#endif

#if MY_DEBUG > 0
#define MYLOG(tag, ...)                  \
	do                                   \
	{                                    \
		if (tag)                         \
			Serial.printf("[%s] ", tag); \
		Serial.printf(__VA_ARGS__);      \
		Serial.printf("\n");             \
	} while (0);                         \
	delay(100)
#else
#define MYLOG(...)
#endif

#ifndef RAK_REGION_AS923_2
#define RAK_REGION_AS923_2 9
#endif
#ifndef RAK_REGION_AS923_3
#define RAK_REGION_AS923_3 10
#endif
#ifndef RAK_REGION_AS923_4
#define RAK_REGION_AS923_4 11
#endif

// Globals
extern char g_dev_name[];
extern bool g_has_rak15001;
extern uint32_t g_send_repeat_time;

/** Module stuff */
#include "module_handler.h"
