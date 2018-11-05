/**
 * This file is part of the Meshwork project.
 *
 * Copyright (C) 2014, Sinisha Djukic
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */
#ifndef __EXAMPLES_CONFIG_H__
#define __EXAMPLES_CONFIG_H__

///////////////////////////////////////////////////////////////////////////////
/////////////////////// SECTION: BUILD-TIME CONFIGURATION /////////////////////
////////////////// ~feel free to edit and adapt to your needs~ ////////////////
///////////////////////////////////////////////////////////////////////////////

//Select your RF chip. Currently, only NRF24L01P is supported
#define MW_RF_SELECT 				MW_RF_NRF24L01P

//0-127 used by apps, 128-255 reserved for system
//#ifndef EX_PORT
//	#define EX_PORT					127
//#endif

#ifndef EX_UART_BAUD
	#define EX_UART_BAUD	9600
#endif

#ifndef EX_TEMP_DISABLE
	//Pin connections of DS18B20. Default:
	//  D4: GND
	//  D5: DATA <---------|
	//  D6: VCC --- 4.7K --|
	#ifndef EX_TEMP_GND
		#define EX_TEMP_GND		Board::D4
	#endif

	#ifndef EX_TEMP_VCC
		#define EX_TEMP_VCC		Board::D6
	#endif

	#ifndef EX_TEMP_DATA
		#define EX_TEMP_DATA	Board::D5
	#endif
#endif

#ifndef EX_BINARY_PININT
	#define EX_BINARY_PININT		Board::D15 //328p/Uno: D15 = A1 = PC1/ADC1. Pin change interrupt only
#endif

//Beacon message interval (ms)
#ifndef EX_BEACON_INTERVAL
	#define EX_BEACON_INTERVAL			60000
#endif
//Beacon watchdog granularity interval (ms)
#ifndef EX_BEACON_WATCHDOG_INTERVAL
//	#define EX_BEACON_WATCHDOG_INTERVAL		1024*8
	#define EX_BEACON_WATCHDOG_INTERVAL		128
#endif

//Select Route Cache table option: MW_ROUTECACHE_NONE, MW_ROUTECACHE_RAM, MW_ROUTECACHE_PERSISTENT
#ifndef MW_ROUTECACHE_SELECT
	#define MW_ROUTECACHE_SELECT		MW_ROUTECACHE_RAM
#endif

#define EX_LED_BOOTUP		Board::LED

//Enable/disable LED tracing for RF messages
#define EX_LED_TRACING		false

#if EX_LED_TRACING
	#define EX_LED_TRACING_SEND	Board::D4
	#define EX_LED_TRACING_RECV	Board::D5
	#define EX_LED_TRACING_ACK	Board::D6

	//Defines a delay (slowness) factor if LED tracing is enabled
	//Increase the delay factory multiplier to give more blink time for LEDs
	#define MW_DELAY_FACTOR				5
	//Enable NetworkV1::RadioListener in the code
	#define MW_SUPPORT_RADIO_LISTENER	true
#endif

//Allows us to change the serial number at any time, opposed to only once
#define ZEROCONF_SERIAL_CHANGE_ENABLE	true

//Offset for storing ZC device configuration in the EEPROM
#define EX_ZC_CONFIGURATION_EEPROM_OFFSET  64

//TODO Add support for raw sketch-specific settings (byte[]) to be configured via serial

#if MW_ROUTECACHE_SELECT != MW_ROUTECACHE_RAM
	//Offset for storing Route Cache table in the EEPROM
	#define EX_ROUTECACHE_TABLE_EEPROM_OFFSET 128
#endif

//Uncomment to enforce true/false. Otherwise it will be automatically
//set to true for more powerful boards, like the Mega
#ifndef MW_FULL_DEBUG
	#define MW_FULL_DEBUG	false
#endif
#ifndef MW_LOG_ALL_ENABLE
	#define MW_LOG_ALL_ENABLE	true
#endif
#ifndef MW_LOG_DEBUG_ENABLE
	#define MW_LOG_DEBUG_ENABLE true
#endif

//Our sketch's own debug
#ifndef EX_LOG
	#define EX_LOG	true//MW_FULL_DEBUG
#endif

//Enables boot-time debug info on non-Mega boards
#ifndef EX_BOOT_DEBUG
	#define EX_BOOT_DEBUG	false
#endif


//Timeout for arrival of new serial messages
#define EX_SERIAL_NEXT_MSG_TIMEOUT	5000

//Incoming connection timeout for serial autoconfig init
#ifndef EX_STARTUP_AUTOCONFIG_INIT_TIMEOUT
	#define EX_STARTUP_AUTOCONFIG_INIT_TIMEOUT		(MW_FULL_DEBUG ? 10000 : 3000)
#endif

//Timeout after last incoming serial autoconfig message before autodeinit
#define EX_STARTUP_AUTOCONFIG_DEINIT_TIMEOUT	(MW_FULL_DEBUG ? 60000 : 10000)

#endif
