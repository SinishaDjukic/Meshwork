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

#include <Meshwork/L7/BaseRFApplication.h>

#if EX_LED_TRACING
	#include "Utils/LEDTracing.h"
#endif

#if ( MW_RF_SELECT == MW_RF_NRF24L01P )
	#include <NRF24L01P.h>
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
using namespace Meshwork::L7;


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
	MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("Node ID: ") << EX_NODE_ID << endl << endl;
	
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
	MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("*** Receiving: dur=") << duration << PSTR(", dataLenMax=") << dataLenMax << PSTR("\n");
	
	uint32_t start = RTT::millis();
	while (duration > 0) {
		int result = mesh.recv(src, port, data, dataLenMax, duration, NULL);
		if ( result >= 0 ) {
			MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << endl << PSTR("****************************") << endl;
			
			MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("[RECV] res=") << result << PSTR(", src=") << src << PSTR(", port=") << port;
			MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR(", dataLen=") << dataLenMax << PSTR(", data=\n");
			MW_LOG_DEBUG_ARRAY(EX_LOG_ROUTER, PSTR("\t...L3 DATA RECV: "), data, dataLenMax);
			MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << endl;
			
			if ( port == BASERF_MESSAGE_PORT ) {
				MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("*** L7 Message Received ***") << endl;
				
				univmsg_l7_any_t msg;
				getMessageFromData(&msg, dataLenMax, data);
				
				MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("\tSequence: ") << msg.msg_header.seq <<
						PSTR(", Meta: ") << ( msg.msg_header.seq_meta ? PSTR("yes") : PSTR("no") ) << endl;
				MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("\tCommand ID: ") << msg.msg_header.cmd_id <<
						PSTR(", Multicommand: ") << ( msg.msg_header.cmd_mc ? PSTR("yes") : PSTR("no") ) << endl;
				MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("\tL7 Data Length: ") << msg.msg_header.dataLen << endl;
				
				MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("\tCommand Name: ");
				
				switch ( msg.msg_header.cmd_id ) {
					//General commands
					case BASERF_CMD_ACK:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_ACK") << endl;
						break;
					//PROPERTY commands
					case BASERF_CMD_PROPERTY_GET:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_PROPERTY_GET") << endl;
						break;
					case BASERF_CMD_PROPERTY_REP:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_PROPERTY_REP") << endl;
						break;
					case BASERF_CMD_PROPERTY_SET:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_PROPERTY_SET") << endl;
						break;
#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
					//META commands and flags
					case BASERF_CMD_META_DEVICE_GET:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_META_DEVICE_GET") << endl;
						break;
					case BASERF_CMD_META_DEVICE_REP:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_META_DEVICE_REP") << endl;
						break;
					case BASERF_CMD_META_CLUSTER_GET:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_META_CLUSTER_GET") << endl;
						break;
					case BASERF_CMD_META_CLUSTER_REP:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_META_CLUSTER_REP") << endl;
						break;
					case BASERF_CMD_META_ENDPOINT_GET:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_META_ENDPOINT_GET") << endl;
						break;
					case BASERF_CMD_META_ENDPOINT_REP:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_META_ENDPOINT_REP") << endl;
						break;
#endif
					//Unknown
					default:
						MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("UNKNOWN") << endl;
				}
//#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
//				if ( msg.msg_header.cmd_id && BASERF_CMD_METAFLAG_ALL_ENDPOINTS	)
//					MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("\tMeta: BASERF_CMD_METAFLAG_ALL_ENDPOINTS") << endl;
//				if ( msg.msg_header.cmd_id && BASERF_CMD_METAFLAG_ALL_CLUSTERS )
//					MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("\tMeta: BASERF_CMD_METAFLAG_ALL_CLUSTERS") << endl;
//#endif
				if ( msg.msg_header.cmd_id == BASERF_CMD_ACK && msg.msg_header.dataLen > 0 ) {
					msg_l7_ack_status_t status = msg.data[0];
					MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("\tBASERF_CMD_ACK Status:");
					switch (status) {
						case BASERF_CMD_ACKFLAG_ERROR:
							MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_ACKFLAG_ERROR") << endl;
							break;
						case BASERF_CMD_ACK_STATUS_PROCESSED:
							MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_ACK_STATUS_PROCESSED") << endl;
							break;
						case BASERF_CMD_ACK_STATUS_SCHEDULED:
							MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_ACK_STATUS_SCHEDULED") << endl;
							break;
						case BASERF_CMD_ACK_STATUS_INVALID:
							MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_ACK_STATUS_INVALID") << endl;
							break;
						case BASERF_CMD_ACK_STATUS_FORBIDDEN:
							MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_ACK_STATUS_FORBIDDEN") << endl;
							break;
						case BASERF_CMD_ACK_STATUS_NOT_SUPPORTED:
							MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_ACK_STATUS_NOT_SUPPORTED") << endl;
							break;
						case BASERF_CMD_ACK_STATUS_NO_ACK:
							MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("BASERF_CMD_ACK_STATUS_NO_ACK") << endl;
							break;
						default:
							MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("UNKNOWN") << endl;
					}
				}
				
				if ( msg.msg_header.cmd_id = BASERF_CMD_PROPERTY_REP && msg.msg_header.dataLen > 2 ) {
					MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << endl;
					MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("\tCluster ID: ") << msg.data[0] << endl;
					MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("\tEndpoint ID: ") << msg.data[1] << endl;
					MW_LOG_DEBUG_ARRAY(EX_LOG_ROUTER, PSTR("\tData: : "), &msg.data[2], msg.msg_header.dataLen - 2);
				}
				
				//TODO: print data
				
			}
			
		} else if ( result == NetworkV1::ERROR_RECV_TIMEOUT ) {
			//do nothing
		} else if ( result == NetworkV1::ERROR_RECV_TOO_LONG ) {
			//do nothing. convertError will log an error anyway
		} else {
			MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("Error code: ") << result << endl;
		}
		duration -= RTT::since(start);
	} 
	
	MW_LOG_DEBUG_TRACE(EX_LOG_ROUTER) << PSTR("*** Receiving: done") << endl;
}

//Main loop
void loop()
{
	run_recv();
}

#endif
