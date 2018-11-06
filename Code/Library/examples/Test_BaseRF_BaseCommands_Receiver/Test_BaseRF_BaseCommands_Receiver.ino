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
#ifndef __TEST_BASERF_BASECOMMANDS_RECEIVERDEVICE_H__
#define __TEST_BASERF_BASECOMMANDS_RECEIVERDEVICE_H__


//First, include the configuration constants file
#include "MeshworkConfiguration.h"


#include "CommonConfig.h"

//All build-time configuration, RF selection, EEPROM usage,
//Route Cache table, etc. is defined in a single place here
#include "ReceiverConfig.h"



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

#include <Meshwork/L7/Endpoints/BinaryPinSensor.h>
#include <Meshwork/L7/Endpoints/BinaryPinSwitch.h>
#include <Meshwork/L7/Endpoints/BinarySensor.h>
#include <Meshwork/L7/Endpoints/BinarySwitch.h>
#include <Meshwork/L7/Endpoints/MultilevelPinSensor.h>
#include <Meshwork/L7/Endpoints/MultilevelSensor.h>
#include <Meshwork/L7/Endpoints/MultilevelSwitch.h>

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
using namespace Meshwork::L7::Endpoints;

#include "TestUtils.h"


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

bool receiver_notify_report(univmsg_l7_any_t* msg) {
	UNUSED(msg);
	//TODO
	return false;
}

BinaryPinSwitch 	ep_0_0(NULL, NULL, NULL);
BinaryPinSwitch 	ep_0_1(NULL, NULL, NULL);
BinaryPinSwitch 	ep_0_2(NULL, NULL, NULL);
Endpoint* ep_0_endpoints[TEST_RECEIVER_CLUSTER_0_ENDPOINT_COUNT] = {&ep_0_0, &ep_0_1, &ep_0_2};
Cluster cluster_0(desc_receiver_0.type, desc_receiver_0.subtype, desc_receiver_0.endpoint_count, (Endpoint**) &ep_0_endpoints);

BinaryPinSensor 	ep_1_0(NULL, NULL, NULL);
MultilevelPinSensor ep_1_1(NULL, NULL, 0, 0, NULL);
Endpoint* ep_1_endpoints[TEST_RECEIVER_CLUSTER_1_ENDPOINT_COUNT] = {&ep_1_0, &ep_1_1};
Cluster cluster_1(desc_receiver_1.type, desc_receiver_1.subtype, desc_receiver_1.endpoint_count, (Endpoint**) &ep_1_endpoints);

Cluster* receiver_clusters[TEST_RECEIVER_CLUSTER_COUNT] = {&cluster_0, &cluster_1};
Device receiver_device(desc_receiver_device.type, desc_receiver_device.subtype, desc_receiver_device.cluster_count, (Cluster**) &receiver_clusters);
ReportListener receiver_listener(&receiver_notify_report);
BaseRFApplication receiver_application(&mesh, &receiver_device, &receiver_listener);

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
	
	trace.begin(&uart, PSTR("L7 Test: BaseRF Base Commands Receiver: started\n"));

	mesh.setNetworkID(TEST_NODE_NWK_ID);
	mesh.setChannel(TEST_NODE_CHANNEL_ID);
	mesh.setNodeID(TEST_RECEIVER_NODE_ID);

#if EX_LED_TRACING
	mesh.set_radio_listener(&ledTracing);
#endif

	mesh.print(trace);

	MW_LOG_DEBUG_TRACE(TEST_LOG_RECEIVER) << PSTR("L7 BaseRF Port: ") << Meshwork::L7::BASERF_MESSAGE_PORT << endl;
  
	ASSERT(mesh.begin());
}

//Main loop
void loop()
{
	receiver_application.pollMessage();
}

#endif
