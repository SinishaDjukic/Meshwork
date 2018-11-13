/**
 * This file is part of the Meshwork project.
 *
 * Copyright (C) 2016, Sinisha Djukic
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
#ifndef __EXAMPLES_ZEROCONFBEACONL7_H__
#define __EXAMPLES_ZEROCONFBEACONL7_H__

//First, include the configuration constants file
#include "MeshworkConfiguration.h"



//All build-time configuration, RF selection, EEPROM usage,
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
#include <Cosa/AnalogPin.hh>
#include <Cosa/OutputPin.hh>
#include <Cosa/RTT.hh>
#include <Cosa/Wireless.hh>

#include <Cosa/IOPin.hh>
#include <Cosa/PinChangeInterrupt.hh>
#include <Cosa/ExternalInterrupt.hh>

#include <OWI.h>
#include <DS18B20.h>

#include <Meshwork.h>
#include <Meshwork/L3/Network.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.cpp>

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

#include <Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfSerial.h>
#include <Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfSerial.cpp>
#include <Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfPersistent.h>

#include <Utils/SerialMessageAdapter.h>
#include <Utils/SerialMessageAdapter.cpp>

using namespace Meshwork::L3::NetworkV1;

#include "MultiSensorL7.h"

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: MEMBER DECLARATION ////////////////////////
///////////////////////////////////////////////////////////////////////////////

//Our EEPROM instance
EEPROM eeprom;

//RF and Mesh
MW_DECL_NRF24L01P(rf)
#if ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT )
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_PERSISTENT(routeprovider, eeprom, EX_ROUTECACHE_TABLE_EEPROM_OFFSET)
#elif ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_RAM )
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_RAM(routeprovider)
#else // MW_ROUTECACHE_NONE
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_NONE(routeprovider)
#endif
NetworkV1 mesh(&rf, routeprovider, NetworkV1::NWKCAPS_NONE);

//Tracing LEDs
#if EX_LED_TRACING
	MW_DECL_LEDTRACING(ledTracing, mesh, EX_LED_TRACING_SEND, EX_LED_TRACING_RECV, EX_LED_TRACING_ACK)
#endif

MW_DECL_ZEROCONF_PERSISTENT(zeroConfPersistent, zeroConfConfiguration, eeprom, EX_ZC_CONFIGURATION_EEPROM_OFFSET)

//Setup extra UART on Mega
#if MW_BOARD_SELECT == MW_BOARD_MEGA
	#include "Cosa/IOBuffer.hh"
	// Create buffer for HC UART
	static IOBuffer<UART::RX_BUFFER_MAX> ibuf;
	static IOBuffer<UART::TX_BUFFER_MAX> obuf;
	// HC UART will be used for Host-Controller communication
	UART uartHC(3, &ibuf, &obuf);
	SerialMessageAdapter serialMessageAdapter(&uartHC);
#else
	SerialMessageAdapter serialMessageAdapter(&uart);
//	IOStream::Device null_device;//appears not needed with the latest Cosa impl
#endif

//ZeroConf Serial support
ZeroConfSerial zeroConfSerial(&mesh, &serialMessageAdapter,
								&zeroConfConfiguration.sernum, &zeroConfConfiguration.reporting,
									&zeroConfConfiguration.nwkconfig, &zeroConfConfiguration.devconfig,
										&zeroConfPersistent);
SerialMessageAdapter::SerialMessageListener* serialMessageListeners[1];

//Bootup notification LED
#if defined(EX_LED_BOOTUP)
	OutputPin ledPin(EX_LED_BOOTUP);
	#define LED(state)	ledPin.set(state)
	#define LED_BLINK(state, x)	\
		LED(state); \
		Watchdog::delay(x); \
		LED(!state);
#else
	#define LED(state)	(void) (state)
	#define LED_BLINK(state, x)	(void) (state)
#endif
	
Ex_MultisensorL7_App multisensor_rfapp(&mesh);


///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: BUSINESS LOGIC/CODE ///////////////////////
///////////////////////////////////////////////////////////////////////////////


//Read stored configuration
void readConfig() {
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("[Config] Reading ZeroConf EEPROM...") << endl;
	zeroConfPersistent.init();
	zeroConfPersistent.read_configuration();

	mesh.setNetworkID(zeroConfConfiguration.nwkconfig.nwkid);
	mesh.setNodeID(zeroConfConfiguration.nwkconfig.nodeid);
	mesh.setNetworkKeyLen(zeroConfConfiguration.nwkconfig.nwkkeylen);
	mesh.setNetworkKey(zeroConfConfiguration.nwkconfig.nwkkey);
	mesh.setChannel(zeroConfConfiguration.nwkconfig.channel);
	mesh.setNetworkCaps(zeroConfConfiguration.devconfig.m_nwkcaps);
	mesh.setDelivery(zeroConfConfiguration.devconfig.m_delivery);

#if ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT )
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("[Config] Reading RouteCache EEPROM...") << endl;
	routecache_routeprovider.init();
	routecache_routeprovider.read_routes();
#endif

	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("[Config] Done") << endl;
}

void traceEnable(bool enable) {
	trace.end();
	trace.begin(enable ? &uart : NULL, NULL);
}

void boardTraceEnable(bool enable) {
#if MW_BOARD_SELECT == MW_BOARD_MEGA
	traceEnable(enable || EX_LOG);
#else
	traceEnable(enable);
#endif
}

//Setup sequence
void setup()
{
	//Basic setup
	Watchdog::begin();
	RTT::begin();

	uart.begin(115200);
	LED_BLINK(true, 1000);
	boardTraceEnable(EX_BOOT_DEBUG);
	LED_BLINK(false, 1000);
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("ZeroConf Beacon: Started") << endl;
	
#if MW_BOARD_SELECT == MW_BOARD_MEGA
	uartHC.begin(115200);
#endif
	
	
	//Init listeners
	serialMessageListeners[0] = &zeroConfSerial;
	serialMessageAdapter.setListeners(serialMessageListeners);

	//Read current configuration from EEPROM
	readConfig();
	
	bool reconfigured = true;
	
	//Empty output buffers
	uart.flush();
	//Empty input buffers in our UART
	//Since there might also be some bytes buffered
	//in a usb-to-serial chip we read out as much as
	//we can before proceeding
	do {
		uart.empty();
		Watchdog::delay(250);
	} while (uart.available() > 0);
	
	//Wait for boot-time reconfiguration via serial
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Waiting for ZeroConfSerial connection...") << endl;
	
	boardTraceEnable(false);
	
	LED(true);
	reconfigured = zeroConfSerial.processConfigSequence(EX_STARTUP_AUTOCONFIG_INIT_TIMEOUT, EX_STARTUP_AUTOCONFIG_DEINIT_TIMEOUT, EX_SERIAL_NEXT_MSG_TIMEOUT);
	LED(false);

	boardTraceEnable(true);
	
	readConfig();

	MW_LOG_DEBUG_TRACE(EX_LOG) << (reconfigured ? PSTR("New configuration applied") : PSTR("Previous configuration used")) << endl;
	
	multisensor_rfapp.setZeroConfConfiguration(&zeroConfConfiguration);

	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("App configuration set") << endl;
	
//	RTT::end();

	//Flush all chars
	uart.flush();
	
#if !EX_LOG
	boardTraceEnable(false);
#endif

	//Blink differently if reconfigured
	LED_BLINK(true, reconfigured ? 2000 : 500);

#if EX_LED_TRACING
	mesh.set_radio_listener(&ledTracing);
#endif

	ASSERT(mesh.begin());
	Meshwork::Debug::printFreeMemory();
	
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Network ID: ") << mesh.getNetworkID() << endl;
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Channel ID: ") << mesh.getChannel() << endl;
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Node ID: ") << mesh.getNodeID() << endl;
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Reporting Node ID: ") << zeroConfConfiguration.reporting.targetnodeid << endl;
	
//	rf.powerdown();
	PinChangeInterrupt::begin();
	multisensor_rfapp.begin();
}

//Main loop
void loop()
{
	uart.flush();
	
	Watchdog::end();
	Watchdog::begin(16);

	// Turn on necessary hardware modules
	Power::all_enable();
	
	MW_LOG_INFO(EX_LOG, "*** Poll: Start ***", NULL);
	
	uint32_t time = RTT::millis();
	multisensor_rfapp.poll();
	time = RTT::since(time);

	MW_LOG_INFO(EX_LOG, "*** Poll: End *** Time spent: %d ms", time);

#if !EX_LOG
	//Deep sleep
	//Power down messes up the UART, so don't do it unless debugging
	//We change the watchdog granularity to sleep more efficiently
	//TODO verify
	Watchdog::end();
	Watchdog::begin(128);
	Power::all_disable();
	uint8_t mode = Power::set(SLEEP_MODE_PWR_DOWN);
#endif
	
	time = RTT::millis();
	while (!multisensor_rfapp.isWakeup() && (RTT::since(time) < (uint32_t) EX_BEACON_INTERVAL) )
		Watchdog::delay(120);
	MW_LOG_INFO(EX_LOG, "*** PinChangeInterrupt=%d", multisensor_rfapp.isWakeup());
	multisensor_rfapp.clearWakeup();
	
#if !EX_LOG
	Power::set(mode);
#endif
}

#endif
