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

//Early include of Cosa's files, so that board definitions and identifiers are loaded
#include "Cosa/Trace.hh"

#define EXAMPLE_BOARD_AUTO				0
#define EXAMPLE_BOARD_RBBB				1
#define EXAMPLE_BOARD_MEGA				2
#define EXAMPLE_BOARD_UNO				3

//Choose board flavor
#if !defined(EXAMPLE_BOARD)
//Auto-detect Mega and Uno, by default
#define EXAMPLE_BOARD EXAMPLE_BOARD_AUTO
//Otherwise, uncomment to use real bare-bone board based on ATmega328
//#define EXAMPLE_BOARD EXAMPLE_BOARD_RBBB
#endif

#if defined (BOARD_ATMEGA2560)
#warning BOARD_ATMEGA2560
#elif defined (__ARDUINO_MEGA__)
#warning __ARDUINO_MEGA__
#elif defined (ARDUINO_MEGA2560)
#warning ARDUINO_MEGA2560
#endif

#define FULL_DEBUG	false

#if EXAMPLE_BOARD == EXAMPLE_BOARD_AUTO
	#if defined (BOARD_ATMEGA2560) || defined (__ARDUINO_MEGA__) || defined (ARDUINO_MEGA2560)
		#warning "Mega selected"
		#define EXAMPLE_BOARD	EXAMPLE_BOARD_MEGA
		#define FULL_DEBUG		true
	#else
		#define EXAMPLE_BOARD	EXAMPLE_BOARD_UNO
	#endif
#endif

//One-stop-shop for debug flags. Just uncomment if you don't want debugs on the Mega
//#define FULL_DEBUG	false

#if defined (FULL_DEBUG)
	#warning "Full debugs enabled"
#endif

#define MW_LOG_ALL_ENABLE 	FULL_DEBUG
#define MW_LOG_DEBUG_ENABLE	FULL_DEBUG
#define LOG_NETWORKV1		FULL_DEBUG
#define LOG_NETWORKSERIAL	FULL_DEBUG
#define LOG_NETWORKINIT		FULL_DEBUG

#define SERIAL_NEXT_MSG_TIMEOUT	5000

#endif

