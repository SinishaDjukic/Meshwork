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
#ifndef __EXAMPLES_ZEROCONFROUTER_H__
#define __EXAMPLES_ZEROCONFROUTER_H__

//Note: comment this out to disable LED tracing
#define LED_TRACING

#ifdef LED_TRACING
	//Note: increase the delay factory multiplier to give more blink time for LEDs
	#define MW_DELAY_FACTOR	5
	//Enable NetworkV1::RadioListener in the code
	#define SUPPORT_RADIO_LISTENER
#endif

#include "Config.h"

#if FULL_DEBUG != false
	#define LOG_ZEROCONFROUTER	true
#else
	#define LOG_ZEROCONFROUTER	false
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

#include "Meshwork/L3/NetworkV1/ZeroConfSerial.h"
#include "Meshwork/L3/NetworkV1/ZeroConfSerial.cpp"

#include "ZCTypes.h"
zctype_configuration_t configuration;

#include "ZeroConfListenerImpl.h"
EEPROM eepromConf;
ZeroConfListenerImpl zeroConfListener(&eepromConf, &configuration);

//Setup extra UART on Mega
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
	#include "Cosa/IOBuffer.hh"
	// Create buffer for HC UART
	static IOBuffer<UART::BUFFER_MAX> ibuf;
	static IOBuffer<UART::BUFFER_MAX> obuf;
	// HC UART will be used for Host-Controller communication
	UART uartHC(3, &ibuf, &obuf);
	ZeroConfSerial zeroConfSerial(&mesh, &uartHC, &configuration.sernum, &configuration.reporting, &configuration.nwkconfig, &zeroConfListener);
#else
	ZeroConfSerial zeroConfSerial(&mesh, &uart, &configuration.sernum, &configuration.reporting, &configuration.nwkconfig, &zeroConfListener);
	IOStream::Device null_device;
#endif

#define EXAMPLE_LED		Board::LED

#if defined(EXAMPLE_LED)
	OutputPin ledPin(EXAMPLE_LED);
	#define LED(state)	ledPin.set(state)
	#define LED_BLINK(state, x)	\
		LED(state); \
		Watchdog::delay(x); \
		LED(!state);
#else
	#define LED(state)	(void) (state)
	#define LED_BLINK(state, x)	(void) (state)
#endif

ZeroConfSerial::serialmsg_t msg;
bool processed;
uint32_t last_message_timestamp = 0;

bool processOneMessage(ZeroConfSerial::serialmsg_t* msg, uint32_t timeout)
{
	bool result = zeroConfSerial.processOneMessage(msg);
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
	if ( result )
		last_message_timestamp = RTC::millis();
	else if ( RTC::since(last_message_timestamp) > timeout ) {
		last_message_timestamp = RTC::millis();
		MW_LOG_WARNING(LOG_ZEROCONFROUTER, "No serial messages processed for %d ms", timeout);
	}
#endif
	return result;
}

bool processConfigSequence()
{
	uint32_t start = RTC::millis();
	uint8_t state = 0;
	while ( RTC::since(start) < STARTUP_AUTOCONFIG_TIMEOUT ) {
		//the state flow must be 0 -> ZC_SUBCODE_ZCINIT -> ZC_SUBCODE_ZCDEINIT
		if ( processOneMessage(&msg, SERIAL_NEXT_MSG_TIMEOUT) ) {
			if ( msg.code == ZeroConfSerial::ZC_SUBCODE_ZCINIT ) {
				state = ZeroConfSerial::ZC_SUBCODE_ZCINIT;
			} else if ( msg.code == ZeroConfSerial::ZC_SUBCODE_ZCDEINIT ) {
				state = ZeroConfSerial::ZC_SUBCODE_ZCDEINIT;
				break;
			}
		}
	}
	return state == ZeroConfSerial::ZC_SUBCODE_ZCDEINIT;
}

void readConfig() {
	eepromConf.read((void*) &configuration, (void*) &EXAMPLE_ZC_CONFIGURATION_EEPROM_OFFSET, (size_t) (EXAMPLE_ZC_SERNUM_EEPROM_OFFSET - EXAMPLE_ZC_CONFIGURATION_EEPROM_END + 1));

	mesh.setNetworkID(configuration.nwkconfig.nwkid);
	mesh.setNodeID(configuration.nwkconfig.nodeid);
	mesh.setNetworkKeyLen(configuration.nwkconfig.nwkkeylen);
	mesh.setNetworkKey((char*)(&configuration.nwkconfig.nwkkey));
	mesh.setChannel(configuration.nwkconfig.channel);

	mesh.setNetworkCaps(configuration.m_nwkcaps);
	mesh.setDelivery(configuration.m_delivery);
}

void setup()
{
	//Basic setup
	Watchdog::begin();
	RTC::begin();
	
	readConfig();
	
	//Enable UART for the boot-up config sequence. Blink once and keep lit during config
	LED_BLINK(true, 500);
	LED(true);
	uart.begin(115200);
	
	//Trace debugs only supported on Mega, since it has extra UARTs
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
	trace.begin(&uart, NULL);
	trace << PSTR("ZeroConf Router: started") << endl;
	uartHC.begin(115200);
#else
	trace.begin(&null_device, NULL);
#endif
	
	zeroConfListener.init();
	
	trace << PSTR("Waiting for configuration...") << endl;
	//Allow some time for initial configuration
	bool reconfigured = processConfigSequence();
	UNUSED(reconfigured);
	
#ifdef LED_TRACING
  mesh.set_radio_listener(&ledTracing);
#endif

	mesh.setChannel(configuration.nwkconfig.channel);
	mesh.setNetworkID(configuration.nwkconfig.nwkid);
	mesh.setNodeID(configuration.nwkconfig.nodeid);

	trace << PSTR("Configuration done.") << endl;
	//Flush all chars before disabling UART
	uart.flush();
	//Disable UART when reconfigured. Blink differently if reconfigured
	uart.end();
	LED_BLINK(false, reconfigured ? 2000 : 500);
	LED(false);
	
	mesh.begin();
}

void run_recv() {
	uint32_t duration = (uint32_t) 60 * 1000L;
	uint8_t src, port;
	size_t dataLenMax = NetworkV1::PAYLOAD_MAX;
	uint8_t data[dataLenMax];
	MW_LOG_DEBUG_TRACE(LOG_ZEROCONFROUTER) << PSTR("RECV: dur=") << duration << PSTR(", dataLenMax=") << dataLenMax << PSTR("\n");
	
	uint32_t start = RTC::millis();
	while (true) {
		int result = mesh.recv(src, port, data, dataLenMax, duration, NULL);
		if ( result != -1 ) {
			MW_LOG_DEBUG_TRACE(LOG_ZEROCONFROUTER) << PSTR("[RECV] res=") << result << PSTR(", src=") << src << PSTR(", port=") << port;
			MW_LOG_DEBUG_TRACE(LOG_ZEROCONFROUTER) << PSTR(", dataLen=") << dataLenMax << PSTR(", data=\n");
			MW_LOG_DEBUG_ARRAY(LOG_ZEROCONFROUTER, PSTR("\t...L3 DATA RECV: "), data, dataLenMax);
			MW_LOG_DEBUG_TRACE(LOG_ZEROCONFROUTER) << endl;
		}
		if ( RTC::since(start) >= duration )
			break;
	} 
	
	MW_LOG_DEBUG_TRACE(LOG_ZEROCONFROUTER) << PSTR("RECV: done\n");
}

void loop()
{
	run_recv();
}

#endif
