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

//Select Route Cache table option: MW_ROUTECACHE_NONE, MW_ROUTECACHE_RAM, MW_ROUTECACHE_PERSISTENT
#define MW_ROUTECACHE_SELECT		MW_ROUTECACHE_RAM

//Enable/disable LED tracing for RF messages
#define EX_LED_TRACING		true

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

//Uncomment to enforce true/false. Otherwise it will be automatically
//set to true for more powerful boards, like the Mega
//#define MW_FULL_DEBUG	true

//Our sketch's own debug
#define EX_LOG		MW_FULL_DEBUG

//Timeout for arrival of new serial messages
#define EX_SERIAL_NEXT_MSG_TIMEOUT	5000

//Defines a delay (slowness) factor if LED tracing is enabled
#if EX_LED_TRACING
	//Note: increase the delay factory multiplier to give more blink time for LEDs
	#define MW_DELAY_FACTOR	5
	//Enable NetworkV1::RadioListener in the code
	#define MW_SUPPORT_RADIO_LISTENER	true
#endif

#endif
