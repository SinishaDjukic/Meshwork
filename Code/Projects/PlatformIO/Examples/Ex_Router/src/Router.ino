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
#ifndef __EXAMPLES_ROUTER_H__
#define __EXAMPLES_ROUTER_H__


//First, include the zeroConfConfiguration constants file
#include "MeshworkConfiguration.h"



//All build-time zeroConfConfiguration, RF selection, EEPROM usage,
//Route Cache table, etc. is defined in a single place here
#include "Config.h"



///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: INCLUDES AND USES /////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <Cosa/EEPROM.hh>
#include <Cosa/Trace.hh>
#include <Cosa/Types.h>
#include <Cosa/IOStream.hh>
#include <Cosa/UART.hh>
#include <Cosa/Watchdog.hh>
#include <Cosa/OutputPin.hh>
#include <Cosa/RTT.hh>
#include <Cosa/Wireless.hh>

#include <Meshwork.h>
#include <Meshwork/L3/Network.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.cpp>

#if EX_LED_TRACING
	#include "Utils/LEDTracing.h"
#endif

#if ( MW_RF_SELECT == MW_RF_NRF24L01P )
	#include <NRF24L01P/NRF24L01P.hh>
	#include <NRF24L01P/NRF24L01P.cpp>
#endif

#if ( ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_RAM ) || ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT ) )
	#include <Meshwork/L3/NetworkV1/RouteCache.h>
	#include <Meshwork/L3/NetworkV1/RouteCache.cpp>
	#include <Meshwork/L3/NetworkV1/CachingRouteProvider.h>
	#if ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT )
		#include <Meshwork/L3/NetworkV1/RouteCachePersistent.h>
	#endif
#endif

using namespace Meshwork::L3::NetworkV1;


///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: MEMBER DECLARATION ////////////////////////
///////////////////////////////////////////////////////////////////////////////

//RF and Mesh
MW_DECL_NRF24L01P(rf)
#if ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT )
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_PERSISTENT(routeprovider, eeprom, EX_ROUTECACHE_TABLE_EEPROM_OFFSET)
#elif ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_RAM )
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_RAM(routeprovider)
#else // MW_ROUTECACHE_NONE
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_NONE(routeprovider)
#endif
NetworkV1 mesh(&rf, routeprovider, NetworkV1::NWKCAPS_ROUTER);

//Tracing LEDs
#if EX_LED_TRACING
	MW_DECL_LEDTRACING(ledTracing, mesh, EX_LED_TRACING_SEND, EX_LED_TRACING_RECV, EX_LED_TRACING_ACK)
#endif

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: BUSINESS LOGIC/CODE ///////////////////////
///////////////////////////////////////////////////////////////////////////////

//Setup sequence
void setup()
{
	//Basic setup
	Watchdog::begin();
	RTT::begin();
	
	uart.begin(115200);
	
	trace.begin(&uart, PSTR("Router: started\n"));

	MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("Network ID: ") << EX_NWK_ID << endl;
	MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("Channel ID: ") << EX_CHANNEL_ID << endl;
	MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("Node ID: ") << EX_NODE_ID << endl;
	
#if EX_LED_TRACING
	mesh.set_radio_listener(&ledTracing);
#endif

	mesh.setNetworkID(EX_NWK_ID);
	mesh.setChannel(EX_CHANNEL_ID);
	mesh.setNodeID(EX_NODE_ID);
	
	ASSERT(mesh.begin());
}

//Receive RF messages loop
void run_recv() {
	uint32_t duration = (uint32_t) 60 * 1000L;
	uint8_t src, port;
	size_t dataLenMax = NetworkV1::PAYLOAD_MAX;
	uint8_t data[dataLenMax];
	MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("RECV: dur=") << duration << PSTR(", dataLenMax=") << dataLenMax << PSTR("\n");
	
	uint32_t start = RTT::millis();
	while (duration > 0) {
		int result = mesh.recv(src, port, data, dataLenMax, duration, NULL);
		if ( result >= 0 ) {
			MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("[RECV] res=") << result << PSTR(", src=") << src << PSTR(", port=") << port;
			MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR(", dataLen=") << dataLenMax << PSTR(", data=\n");
			MW_LOG_DEBUG_ARRAY(EX_LOG_ROUTER, PSTR("\t...L3 DATA RECV: "), data, dataLenMax);
			MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << endl;
		} else if ( result == NetworkV1::ERROR_RECV_TIMEOUT ) {
			//do nothing
		} else if ( result == NetworkV1::ERROR_RECV_TOO_LONG ) {
			//do nothing. convertError will log an error anyway
		} else {
			MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("Error code: ") << result << endl;
		}
		duration -= RTT::since(start);
	} 
	
	MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("RECV: done\n");
}

//Main loop
void loop()
{
	run_recv();
}

#endif
