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
#ifndef __EXAMPLES_SERIALROUTER_H__
#define __EXAMPLES_SERIALROUTER_H__

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
#include <Cosa/IOStream/Driver/UART.hh>
#include <Cosa/Watchdog.hh>
#include <Cosa/OutputPin.hh>
#include <Cosa/RTC.hh>
#include <Cosa/Wireless.hh>

#include <Meshwork.h>
#include <Meshwork/L3/Network.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.cpp>

#if EX_LED_TRACING
	#include "Utils/LEDTracing.h"
#endif

#if ( MW_RF_SELECT == MW_RF_NRF24L01P )
	#include <Cosa/Wireless/Driver/NRF24L01P.hh>
#endif

#include <Utils/SerialMessageAdapter.h>
#include <Utils/SerialMessageAdapter.cpp>

#include "Meshwork/L3/NetworkV1/NetworkSerial/NetworkSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkSerial/NetworkSerial.cpp"

using namespace Meshwork::L3::NetworkV1;

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: MEMBER DECLARATION ////////////////////////
///////////////////////////////////////////////////////////////////////////////

//RF and Mesh
MW_DECL_NRF24L01P(rf)

NetworkV1 mesh(&rf, NULL, NetworkV1::NWKCAPS_ROUTER | NetworkV1::NWKCAPS_GATEWAY | NetworkV1::NWKCAPS_CONTROLLER);

//Tracing LEDs
#if EX_LED_TRACING
	MW_DECL_LEDTRACING(ledTracing, mesh, EX_LED_TRACING_SEND, EX_LED_TRACING_RECV, EX_LED_TRACING_ACK)
#endif

//Setup extra UART on Mega
#if MW_BOARD_SELECT == MW_BOARD_MEGA
	#include "Cosa/IOBuffer.hh"
	// Create buffer for HC UART
	static IOBuffer<UART::BUFFER_MAX> ibuf;
	static IOBuffer<UART::BUFFER_MAX> obuf;
	// HC UART will be used for Host-Controller communication
	UART uartHC(3, &ibuf, &obuf);
	SerialMessageAdapter serialMessageAdapter(&uartHC);
#else
	SerialMessageAdapter serialMessageAdapter(&uart);
	IOStream::Device null_device;
#endif

//Network Serial support
NetworkSerial networkSerial(&mesh, &serialMessageAdapter);
SerialMessageAdapter::SerialMessageListener* serialMessageListeners[1];

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: BUSINESS LOGIC/CODE ///////////////////////
///////////////////////////////////////////////////////////////////////////////

//Setup sequence
void setup()
{
	//Basic setup
	Watchdog::begin();
	RTC::begin();

	uart.begin(115200);

//Trace debugs only supported on Mega, since it has extra UARTs
#if MW_BOARD_SELECT == MW_BOARD_MEGA
	  trace.begin(&uart, NULL);
	  trace << PSTR("Serial Router: started") << endl;
	  uartHC.begin(115200);
#else
	  trace.begin(&null_device, NULL);
#endif

#if LED_TRACING
	  mesh.set_radio_listener(&ledTracing);
#endif

	serialMessageListeners[0] = (SerialMessageAdapter::SerialMessageListener*) &serialMessageAdapter;
	serialMessageAdapter.setListeners(serialMessageListeners);

	networkSerial.initSerial();
}

SerialMessageAdapter::serialmsg_t msg;
uint8_t last_message_processed;
uint32_t last_message_timestamp = 0;

//Receive RF messages
void run_recv() {
	last_message_processed = networkSerial.processOneMessage(&msg);
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
	if ( last_message_processed != SerialMessageAdapter::SM_MESSAGE_NONE )
		last_message_timestamp = RTC::millis();
	else if ( RTC::since(last_message_timestamp) > EX_SERIAL_NEXT_MSG_TIMEOUT ) {
		last_message_timestamp = RTC::millis();
		MW_LOG_WARNING(EX_LOG_SERIALROUTER, "No serial messages processed for %d ms", EX_SERIAL_NEXT_MSG_TIMEOUT);
	}
#endif
}

//Main loop
void loop()
{
	run_recv();
}

#endif
