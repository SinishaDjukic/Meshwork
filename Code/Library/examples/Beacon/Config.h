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

//Network configuration is hardcoded here
#ifndef EX_NWK_ID
	#define EX_NWK_ID				1
#endif

#ifndef EX_CHANNEL_ID
	#define EX_CHANNEL_ID			0
#endif

#ifndef EX_NODE_ID
	#define EX_NODE_ID					200
#endif

//0-127 used by apps, 128-255 reserved for system
#ifndef EX_PORT
	#define EX_PORT					127
#endif

#ifndef EX_TEMP_DISABLE
	//Pin connections of DS18B20. Default:
	//  D4: GND
	//  D5: DATA <---------|
	//  D6: VCC --- 4.7K --|
	#ifndef EX_TEMP_GND
		#define EX_TEMP_GND		Board::D14//Leonardo A0/D14
	#endif

	#ifndef EX_TEMP_VCC
		#define EX_TEMP_VCC		Board::D16//Leonardo A2/D16
	#endif

	#ifndef EX_TEMP_DATA
		#define EX_TEMP_DATA		Board::D15//Leonardo A1/D15
	#endif
#endif

//Beacon message interval (ms)
#ifndef EX_BEACON_INTERVAL
	#define EX_BEACON_INTERVAL			60000
#endif
//Beacon watchdog granularity interval (ms)
#ifndef EX_BEACON_WATCHDOG_INTERVAL
	#define EX_BEACON_WATCHDOG_INTERVAL		1024*8
#endif

//Select Route Cache table option: MW_ROUTECACHE_NONE, MW_ROUTECACHE_RAM, MW_ROUTECACHE_PERSISTENT
#ifndef MW_ROUTECACHE_SELECT
	#define MW_ROUTECACHE_SELECT		MW_ROUTECACHE_RAM
#endif

//Enable/disable LED tracing for RF messages
#define EX_LED_TRACING		false

#if EX_LED_TRACING
	#define EX_LED_TRACING_SEND	Board::D4
	#define EX_LED_TRACING_RECV	Board::D5
	#define EX_LED_TRACING_ACK	Board::D6
	#define EX_LED_BOOTUP		Board::LED

	//Defines a delay (slowness) factor if LED tracing is enabled
	//Increase the delay factory multiplier to give more blink time for LEDs
	#define MW_DELAY_FACTOR	5
	//Enable NetworkV1::RadioListener in the code
	#define MW_SUPPORT_RADIO_LISTENER	true
#endif

#if MW_ROUTECACHE_SELECT != MW_ROUTECACHE_RAM
	//Offset for storing Route Cache table in the EEPROM
	#define EX_ROUTECACHE_TABLE_EEPROM_OFFSET 128
#endif

//Uncomment to enforce true/false. Otherwise it will be automatically
//set to true for more powerful boards, like the Mega
#define MW_FULL_DEBUG	(MW_BOARD_SELECT == MW_BOARD_MEGA)
#define MW_LOG_DEBUG_ENABLE true

//No need for rerouting here
#define MW_SUPPORT_REROUTING	false

//Our sketch's own debug
#define EX_LOG	true

#endif
