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
	#define LOG_SERIALROUTER	true
#else
	#define LOG_SERIALROUTER	false
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
#include <Utils/SerialMessageAdapter.h>
#include <Utils/SerialMessageAdapter.cpp>
#include "NetworkInit.h"
//END: Include set for initializing the network

#ifdef LED_TRACING
	OutputPin pin_send(Board::D4);
	OutputPin pin_recv(Board::D5);
	OutputPin pin_ack(Board::D6);
	#include "Utils/LEDTracing.h"
	LEDTracing ledTracing(&mesh, &pin_send, &pin_recv, &pin_ack);
#endif

#include "Meshwork/L3/NetworkV1/NetworkSerial/NetworkSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkSerial/NetworkSerial.cpp"

//Setup extra UART on Mega
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
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

NetworkSerial networkSerial(&mesh, &serialMessageAdapter);

SerialMessageAdapter::SerialMessageListener* serialMessageListeners[1];


void setup()
{
	//Basic setup
	Watchdog::begin();
	RTC::begin();

	serialMessageListeners[0] = (SerialMessageAdapter::SerialMessageListener*) &serialMessageAdapter;
	serialMessageAdapter.setListeners(serialMessageListeners);

	uart.begin(115200);

//Trace debugs only supported on Mega, since it has extra UARTs
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
  trace.begin(&uart, NULL);
  trace << PSTR("Serial Router: started") << endl;
  uartHC.begin(115200);
#else
  trace.begin(&null_device, NULL);
#endif

#ifdef LED_TRACING
  mesh.set_radio_listener(&ledTracing);
#endif

  networkSerial.initSerial();
}

SerialMessageAdapter::serialmsg_t msg;
uint8_t last_message_processed;
uint32_t last_message_timestamp = 0;

void loop()
{
	last_message_processed = networkSerial.processOneMessage(&msg);
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
	if ( last_message_processed != SerialMessageAdapter::SM_MESSAGE_NONE )
		last_message_timestamp = RTC::millis();
	else if ( RTC::since(last_message_timestamp) > SERIAL_NEXT_MSG_TIMEOUT ) {
		last_message_timestamp = RTC::millis();
		MW_LOG_WARNING(LOG_SERIALROUTER, "No serial messages processed for %d ms", SERIAL_NEXT_MSG_TIMEOUT);
	}
#endif
}

#endif
