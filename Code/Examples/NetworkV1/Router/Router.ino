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

//Note: comment this out to disable LED tracing
#define LED_TRACING

#ifdef LED_TRACING
	//Note: increase the delay factory multiplier to give more blink time for LEDs
	#define MW_DELAY_FACTOR	5
	//Enable NetworkV1::RadioListener in the code
	#define MW_SUPPORT_RADIO_LISTENER
#endif

#include "Config.h"

#if FULL_DEBUG != false
	#define LOG_ROUTER	true
#else
	#define LOG_ROUTER	false
#endif

#include <stdlib.h>
#include <Cosa/Trace.hh>
#include <Cosa/Types.h>
#include <Cosa/IOStream.hh>
#include <Cosa/IOStream/Driver/UART.hh>
#include <Cosa/Watchdog.hh>
#include <Cosa/RTC.hh>
#include <Cosa/Wireless.hh>

//BEGIN: Include set for initializing the network
#include <Meshwork.h>
#include <Meshwork/L3/Network.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.cpp>
#include "NetworkInit.h"
//END: Include set for initializing the network

#ifdef LED_TRACING
	OutputPin pin_send(Board::D4);
	OutputPin pin_recv(Board::D5);
	OutputPin pin_ack(Board::D6);
	#include "Utils/LEDTracing.h"
	LEDTracing ledTracing(&mesh, &pin_send, &pin_recv, &pin_ack);
#endif

static const uint16_t 	ROUTER_NWK_ID 		= 1;
static const uint8_t 	ROUTER_CHANNEL_ID 	= 0;
static const uint8_t 	ROUTER_NODE_ID 		= 200;

void setup()
{
  uart.begin(115200);
  trace.begin(&uart, PSTR("Router: started\n"));
  
  MW_LOG_DEBUG_TRACE(LOG_ROUTER) << PSTR("Network ID: ") << ROUTER_NWK_ID << endl;
  MW_LOG_DEBUG_TRACE(LOG_ROUTER) << PSTR("Channel ID: ") << ROUTER_CHANNEL_ID << endl;
  MW_LOG_DEBUG_TRACE(LOG_ROUTER) << PSTR("Node ID: ") << ROUTER_NODE_ID << endl;
  
#ifdef LED_TRACING
  mesh.set_radio_listener(&ledTracing);
#endif

  mesh.setNetworkID(ROUTER_NWK_ID);
  mesh.setChannel(ROUTER_CHANNEL_ID);
  mesh.setNodeID(ROUTER_NODE_ID);
  mesh.begin(NULL);
  
//  uint8_t mode = SLEEP_MODE_IDLE;
  Watchdog::begin(16);  
//  rf.set_sleep(mode);
  RTC::begin();
}

void run_recv() {
	uint32_t duration = (uint32_t) 60 * 1000L;
	uint8_t src, port;
	size_t dataLenMax = NetworkV1::PAYLOAD_MAX;
	uint8_t data[dataLenMax];
	MW_LOG_DEBUG_TRACE(LOG_ROUTER) << PSTR("RECV: dur=") << duration << PSTR(", dataLenMax=") << dataLenMax << PSTR("\n");
	
	uint32_t start = RTC::millis();
	while (true) {
		int result = mesh.recv(src, port, data, dataLenMax, duration, NULL);
		if ( result != -1 ) {
			MW_LOG_DEBUG_TRACE(LOG_ROUTER) << PSTR("[RECV] res=") << result << PSTR(", src=") << src << PSTR(", port=") << port;
			MW_LOG_DEBUG_TRACE(LOG_ROUTER) << PSTR(", dataLen=") << dataLenMax << PSTR(", data=\n");
			MW_LOG_DEBUG_ARRAY(LOG_ROUTER, PSTR("\t...L3 DATA RECV: "), data, dataLenMax);
			MW_LOG_DEBUG_TRACE(LOG_ROUTER) << endl;
		}
		if ( RTC::since(start) >= duration )
			break;
	} 
	
	MW_LOG_DEBUG_TRACE(LOG_ROUTER) << PSTR("RECV: done\n");
}

void loop()
{
	run_recv();
}

#endif