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
#ifndef __EXAMPLES_NETWORKINIT_H__
#define __EXAMPLES_NETWORKINIT_H__

using namespace Meshwork::L3::NetworkV1;

//Values for cache variants
#define EXAMPLE_ROUTECACHE_NONE 		0
#define EXAMPLE_ROUTECACHE_RAM 			1
#define EXAMPLE_ROUTECACHE_PERSISTENT	2

#define EXAMPLE_BOARD_AUTO				0
#define EXAMPLE_BOARD_RBBB				1
#define EXAMPLE_BOARD_MEGA				2
#define EXAMPLE_BOARD_UNO				3

//////////////////////////////////////////////////////////////////////////////
//////////////////////// BEGIN: CONFIGURATION SECTION ////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifndef LOG_NETWORKINIT
#define LOG_NETWORKINIT  true
#endif

//Define the used cache variant. Uncomment your combination
#if !defined(EXAMPLE_ROUTECACHE)
#define EXAMPLE_ROUTECACHE EXAMPLE_ROUTECACHE_NONE
//#define EXAMPLE_ROUTECACHE EXAMPLE_ROUTECACHE_RAM
//#define EXAMPLE_ROUTECACHE EXAMPLE_ROUTECACHE_PERSISTENT
#endif

//Define network capabilities. Uncomment your combination or define another one
#define EXAMPLE_NWKCAPS NetworkV1::NWKCAPS_SLEEPING
//#define EXAMPLE_NWKCAPS NetworkV1::NWKCAPS_ROUTER

//Choose board flavor
#if !defined(EXAMPLE_BOARD)
//Uncomment to use real bare-bone board (atmega328)
//#define EXAMPLE_BOARD EXAMPLE_BOARD_RBBB
//Otherwise auto-detect Mega and Uno
#define EXAMPLE_BOARD EXAMPLE_BOARD_AUTO
#endif

//////////////////////////////////////////////////////////////////////////////
////////////////////////   END: CONFIGURATION SECTION ////////////////////////
//////////////////////////////////////////////////////////////////////////////



////////////////////////  IMPL: INITIALIZE ROUTE CACHE ///////////////////////
#if EXAMPLE_ROUTECACHE != EXAMPLE_ROUTECACHE_NONE
	#include <Meshwork/L3/NetworkV1/RouteCache.h>
	#include <Meshwork/L3/NetworkV1/RouteCache.cpp>
	#include <Meshwork/L3/NetworkV1/CachingRouteProvider.h>
	
	//RAM-only
	#if EXAMPLE_ROUTECACHE == EXAMPLE_ROUTECACHE_RAM
		RouteCache m_route_cache(NULL);
	
	//RAM and EEPROM
	#elif EXAMPLE_ROUTECACHE == EXAMPLE_ROUTECACHE_PERSISTENT
		#include <Cosa/EEPROM.hh>
		#include <Meshwork/L3/NetworkV1/RouteCachePersistent.h>
		//Offset for storing persistent cache in the EEPROM
		#define EXAMPLE_ROUTECACHE_PERSISTENT_EEPROM_OFFSET	128
		//Final address might be useful for the app
		#define EXAMPLE_ROUTECACHE_PERSISTENT_EEPROM_END	EXAMPLE_ROUTECACHE_PERSISTENT_EEPROM_OFFSET + \
													RouteCachePersistent::ROUTE_SIZE_TABLE
		EEPROM eeprom;
		RouteCachePersistent m_route_cache(&eeprom, EXAMPLE_ROUTECACHE_PERSISTENT_EEPROM_OFFSET);

	#endif
		
		//Initialize the CachingRouteProvider accordingly
		CachingRouteProvider m_route_provider(&m_route_cache,
								CachingRouteProvider::UPDATE_REMOVE_ON_QOS_MIN |
								CachingRouteProvider::UPDATE_REPLACE_ON_QOS_WORST);

		NetworkV1::RouteProvider* m_route_provider_ptr = &m_route_provider;

#else
	//No RouteCache
	NetworkV1::RouteProvider* m_route_provider_ptr = NULL;
#endif


////////////////////////  IMPL: WIRELESS IMPLEMENTATION //////////////////////

#include "Cosa/Wireless/Driver/NRF24L01P.hh"

#if EXAMPLE_BOARD == EXAMPLE_BOARD_AUTO
	#if defined (BOARD_ATMEGA2560) || defined (__ARDUINO_MEGA__)
	#define EXAMPLE_BOARD	EXAMPLE_BOARD_MEGA
	#else
	#define EXAMPLE_BOARD	EXAMPLE_BOARD_UNO
	#endif
#endif

#if EXAMPLE_BOARD == EXAMPLE_BOARD_RBBB
NRF24L01P rf(0x0001, 0x01,
	    Board::DigitalPin(Board::D7), 
	    Board::DigitalPin(Board::D8), 
	    Board::ExternalInterruptPin(Board::EXT0));
#elif EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
NRF24L01P rf(0x0001, 0x01,
	    Board::DigitalPin(Board::D53), 
	    Board::DigitalPin(Board::D48), 
	    Board::ExternalInterruptPin(Board::EXT2));

#elif EXAMPLE_BOARD == EXAMPLE_BOARD_UNO
NRF24L01P rf(0x0001, 0x01,
	    Board::DigitalPin(Board::D10), 
	    Board::DigitalPin(Board::D3), 
	    Board::ExternalInterruptPin(Board::EXT0));
#endif


////////////////////////  IMPL: MESH NETWORK IMPLEMENTATION //////////////////
NetworkV1 mesh(&rf, m_route_provider_ptr, EXAMPLE_NWKCAPS);

#endif

