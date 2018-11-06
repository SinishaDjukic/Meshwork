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
#ifndef __TEST_BASERF_BASECOMMANDS_H__
#define __TEST_BASERF_BASECOMMANDS_H__

//First, include the configuration constants file
#include "MeshworkConfiguration.h"

#include "CommonConfig.h"

#include <stdlib.h>
#include <Cosa/Trace.hh>
#include <Cosa/Types.h>
#include <Cosa/IOStream.hh>
#include <Cosa/UART.hh>
#include <Cosa/Watchdog.hh>
#include <Cosa/RTT.hh>

#include <Meshwork.h>
#include <Meshwork/L3/Network.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.cpp>

#include <Meshwork/L7/Device.h>
#include <Meshwork/L7/Cluster.h>
#include <Meshwork/L7/Endpoint.h>
#include <Meshwork/L7/BaseRFApplication.h>

#include <Meshwork/L7/Device.cpp>
#include <Meshwork/L7/Cluster.cpp>
#include <Meshwork/L7/BaseRFApplication.cpp>

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


///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: IMPLEMENTATION ////////////////////////////
///////////////////////////////////////////////////////////////////////////////


bool sender_notify_report(univmsg_l7_any_t* msg) {
	UNUSED(msg);
	//TODO
	return false;
}

Device sender_device(TEST_SENDER_DEVICE_TYPE, TEST_SENDER_DEVICE_SUBTYPE, TEST_SENDER_CLUSTER_COUNT, NULL);
ReportListener sender_listener(&sender_notify_report);
BaseRFApplication sender_application(&mesh, &sender_device, &sender_listener);

bool verifyDescriptor(bool print, test_device_t* device, bool v_device, bool v_clusters, bool v_endpoints) {
	bool result = true;

	if ( v_device ) {
		result &= device->verified;
		if ( print )
			trace << endl << PSTR("[verifyDescriptor] Device verified: ") << device->verified << endl;
	}

	if ( v_clusters || v_endpoints ) {
		for ( uint8_t i = 0; i < device->cluster_count; i ++ ) {
			test_cluster_t* cluster = device->clusters[i];
			if ( v_clusters ) {
				result &= cluster->verified;
				if ( print )
					trace << PSTR("[verifyDescriptor] Cluster ") << i << PSTR(" verified: ") << cluster->verified << endl;
			}
			if ( v_endpoints ) {
				for ( uint8_t j = 0; j < cluster->endpoint_count; j ++ ) {
					test_endpoint_t* endpoint = cluster->endpoints[j];
						result &= endpoint->verified;
						if ( print )
							trace << PSTR("[verifyDescriptor] Endpoint ") << i << PSTR(":") << j << PSTR(" verified: ") << endpoint->verified << endl;
				}
			}
		}
	}

	if ( print ) {
		trace << PSTR("[verifyDescriptor] Result: ") << ( result ? PSTR("SUCCESS") : PSTR("FAILED") ) << endl << endl;
	}

	return result;
}

void invalidateDescriptor(test_device_t* device) {
	device->verified = false;
	for ( uint8_t i = 0; i < device->cluster_count; i ++ ) {
		test_cluster_t* cluster = device->clusters[i];
		cluster->verified = false;
		for ( uint8_t j = 0; j < cluster->endpoint_count; j ++ ) {
			test_endpoint_t* endpoint = cluster->endpoints[j];
			endpoint->verified = false;
		}
	}
}

void printDelimiter1() {
	trace << PSTR("***************************") << endl;
}

void printDelimiter2() {
	trace << PSTR("---------------------------") << endl;
}

void printDelimiter3() {
	trace << PSTR("...........................") << endl;
}

static uint8_t nextID = 0;

void prepareMessage(univmsg_l7_any_t* msg, size_t dataLen, uint8_t* data) {
	msg->msg_header.seq = nextID ++;
	msg->msg_header.cmd_mc = false;
	msg->msg_header.src = TEST_RECEIVER_NODE_ID;
	msg->msg_header.port = BASERF_MESSAGE_PORT;
	msg->msg_header.seq_meta = true;
	msg->msg_header.dataLen = dataLen;
	msg->data = data;
}

//Tests: listening on correct message port (sending a mandatory command)
bool testMessagePort() {
	printDelimiter2();
	trace << PSTR("[testMessagePort] Started") << endl;

	bool result = true;

//	univmsg_l7_any_t msg;
//	msg.msg_header.cmd_id = BASERF_CMD_META_DEVICE_GET;
//	const uint8_t msg_data_len = 1;
//	uint8_t msg_data[msg_data_len] = {0};
//	prepareMessage(&msg, msg_data_len, msg_data);
//
//	size_t bufLen = BASERF_MESSAGE_PAYLOAD_MAXLEN;
//	uint8_t bufACK[bufLen];
//	msg_l7_ack_status_t ack_status = sender_application.sendMessageWithACK(&msg, (void*) bufACK, bufLen);

	trace << PSTR("[testMessagePort] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl << endl;
	printDelimiter2();
	return result;
}

bool testStart() {
	printDelimiter1();
	trace << PSTR("[TestSuite][Test_BaseRF_BaseCommands] Started") << endl;
	uint32_t time = RTT::millis();
	bool result = true;

	///////////////////// BEGIN /////////////////////
	//Allocate all structures in memory
	//TODO

	//Test 1: listening on correct message port (sending a mandatory command)
	result &= testMessagePort();

	//Test 2: handling of an unsupported command
//	result &= testUnsupportedCommand();

	//Test 3: handling of a supported command: returns correct seq number, returns ACK_PROCESSED || ACK_SCHEDULED
//	result &= testSeqAck();

	//Test 4: ignores unsolicited ACK
//	result &= testIgnoreAck();


	//Test 5: responds with PROPERTY_REP to a PROPERTY_GET
//	result &= testPropertyGet();

	//Test 6: after receiving PROPERTY_SET sends PROPERTY_REP to the reporting node (on the correct port)
//	result &= testPropertyGetReportingNode();

	//Test 7: PROPERTY_SET followed by PROPERTY_GET yields a PROPERTY_REP with the correct last value
//	result &= testPropertyGetAfterPropertySet();


	//Test 8: responds to a META_ENDPOINT_GET; case: single endpoint (flags 0); MCFLAG tested
//	result &= testMetaEndpointGetSingleEndpoint();

	//Test 9: responds to a META_ENDPOINT_GET; case: all endpoints (flags 7b); MCFLAG tested
//	result &= testMetaEndpointGetAllEndpoints();


	//Test 10: responds to a META_CLUSTER_GET; case: single cluster (flags 0); MCFLAG tested
//	result &= testMetaClusterGetSingleCluster();

	//Test 11: responds a META_CLUSTER_GET; case: all clusters (flags 7b); MCFLAG tested
//	result &= testMetaClustertGetAllClusters();

	//Test 12: responds to a META_CLUSTER_GET; case: single cluster, all endpoints (flags 6b); MCFLAG tested
//	result &= testMetaClusterGetSingleClusterAllEndpoints();

	//Test 13: responds a META_CLUSTER_GET; case: all clusters, all endpoints (flags 6b + 7b); MCFLAG tested
//	result &= testMetaClusterGetAllClustersAllEndpoints();


	//Test 14: responds to a META_DEVICE_GET; case: device-only (flags 0); MCFLAG tested
//	result &= testMetaDeviceGet();

	//Test 15: responds to a META_DEVICE_GET; case: all clusters (flags 6b); MCFLAG tested
//	result &= testMetaDeviceGetAllClusters();

	//Test 16: responds to a META_DEVICE_GET; case: clusters and endpoints (flags 6b + 7b); MCFLAG tested
//	result &= testMetaDeviceGetAllClustersAllEndpoints();


	////////////////////// END //////////////////////

	time = RTT::millis() - time;
	trace << PSTR("Run time (ms): ") << time << PSTR("\r\n");
	trace << PSTR("[TestSuite][Test_BaseRF_BaseCommands] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl;
	printDelimiter1();
	return result;
}

//void testSenderStart() {
//	trace << PSTR("[TestSuite][Test_BaseRF_BaseCommands] Sender thread started") << endl;
//	testStart();
//	trace << PSTR("[TestSuite][Test_BaseRF_BaseCommands] Sender thread ended") << endl;
//	Watchdog::delay(600000);
//}
//

//Executor sender_executor(&testSenderStart);

void setup()
{
	uart.begin(115200);
	Watchdog::begin();
	RTT::begin();
	trace.begin(&uart, PSTR(__FILE__));

	testStart();

}

void loop()
{
	Watchdog::delay(60000);
}

#endif
