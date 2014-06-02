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

#include "Meshwork/L3/NetworkV1/NetworkSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkSerial.cpp"
#include "Utils/LineReader.h"

//Setup extra UART on Mega
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
	#include "Cosa/IOBuffer.hh"
	// Create buffer for HC UART
	static IOBuffer<UART::BUFFER_MAX> ibuf;
	static IOBuffer<UART::BUFFER_MAX> obuf;
	// HC UART will be used for Host-Controller communication
	UART uartHC(3, &ibuf, &obuf);
	NetworkSerial networkSerial(&mesh, &uartHC);
#else
	NetworkSerial networkSerial(&mesh, &uart);
#endif

void setup()
{
  uart.begin(115200);

//Trace debugs only supported on Mega, since it has extra UARTs
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
  trace.begin(&uart, NULL);
  trace << PSTR("Serial Console: started");
  uartHC.begin(115200);
#endif

//  uint8_t mode = SLEEP_MODE_IDLE;
  Watchdog::begin(16);
  RTC::begin();
  
  networkSerial.initSerial();
}

uint32_t last_message_timestamp = 0;

void loop()
{
	static uint8_t databuf[NetworkSerial::MAX_SERIALMSG_LEN];
	static NetworkSerial::serialmsg_t msg;
	*msg.data = *databuf;
	bool processed = networkSerial.processOneMessage(&msg);
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
	if ( processed )
		last_message_timestamp = RTC::millis();
	else if ( RTC::since(last_message_timestamp) > 5000 ) {
		last_message_timestamp = RTC::millis();
		MW_LOG_WARNING("No serial messages processed for 5000 ms", NULL);
	}
#endif
}

#endif