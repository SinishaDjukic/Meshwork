/**
 * This file is part of the Meshwork project.
 *
 * Copyright (C) 2013, Sinisha Djukic
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
#ifndef __MESHWORK_MESHWORKCONFIGURATION_H__
#define __MESHWORK_MESHWORKCONFIGURATION_H__

#include "Cosa/Types.h"
#include "Cosa/Memory.h"
#include "Cosa/RTT.hh"
#include "Cosa/Trace.hh"
#include "Cosa/Watchdog.hh"

//Possible known board selections
#define MW_BOARD_AUTO				0
#define MW_BOARD_RBBB				1
#define MW_BOARD_MEGA				2
#define MW_BOARD_UNO				3
#define MW_BOARD_LEONARDO			4

//Auto-detect the board. Currently, only Mega, Uno and Leonardo are handled
#if !defined(MW_BOARD_SELECT) || (MW_BOARD_SELECT == MW_BOARD_AUTO)
	#pragma message "[Meshwork] Board: autodetection..."
	//Avoid compiled warning
	#undef MW_BOARD_SELECT

	#if defined (COSA_BOARD_ARDUINO_MEGA_HH)
		#pragma message "[Meshwork] Board: Arduino Mega detected"
		#define MW_BOARD_SELECT		MW_BOARD_MEGA
	#elif defined (COSA_BOARD_ARDUINO_LEONARDO_HH)
		#pragma message "[Meshwork] Board: Arduino Leonardo detected"
		#define MW_BOARD_SELECT		MW_BOARD_LEONARDO
	#else
		#pragma message "[Meshwork] Board: Arduino Uno detected"
		#define MW_BOARD_SELECT		MW_BOARD_UNO
	#endif
#endif

//If full debug has not been enforced we set it depending on the board
#if !defined(MW_FULL_DEBUG)
	#define MW_FULL_DEBUG		(MW_BOARD_SELECT == MW_BOARD_MEGA)
	#if defined (FULL_DEBUG)
		#pragma message "[Meshwork] Full debugs enabled"
	#endif
#endif

//Values for known RF chips
#define MW_RF_NRF24L01P				0

//Values for known route cache options
#define MW_ROUTECACHE_NONE 			0
#define MW_ROUTECACHE_RAM 			1
#define MW_ROUTECACHE_PERSISTENT	2


#endif
