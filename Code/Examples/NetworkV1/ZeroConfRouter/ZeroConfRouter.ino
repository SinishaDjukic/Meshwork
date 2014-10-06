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

#include "Meshwork/L3/NetworkV1/ZeroConfSerial.h"
#include "Meshwork/L3/NetworkV1/ZeroConfSerial.cpp"

//TODO read/write from/to EEPROM
char sernum[16 + 1];

//Setup extra UART on Mega
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
	#include "Cosa/IOBuffer.hh"
	// Create buffer for HC UART
	static IOBuffer<UART::BUFFER_MAX> ibuf;
	static IOBuffer<UART::BUFFER_MAX> obuf;
	// HC UART will be used for Host-Controller communication
	UART uartHC(3, &ibuf, &obuf);
	ZeroConfSerial zeroConfSerial(&mesh, &uartHC, mesh.getNetworkKey(), sernum, NULL);
#else
	ZeroConfSerial zeroConfSerial(&mesh, &uart, mesh.getNetworkKey(), sernum, NULL);
	IOStream::Device null_device;
#endif


void setup()
{
  uart.begin(115200);

//Trace debugs only supported on Mega, since it has extra UARTs
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
  trace.begin(&uart, NULL);
  trace << PSTR("ZeroConf Router: started") << endl;
  uartHC.begin(115200);
#else
  trace.begin(&null_device, NULL);
#endif

  Watchdog::begin();
  RTC::begin();
}

ZeroConfSerial::serialmsg_t msg;
bool processed;
uint32_t last_message_timestamp = 0;

//TODO:
//1) write all configured data in EEPROM
//2) read EEPROM config upon startup
//3) enter the ZC mode via an external trigger/interrupt and notify via LED
//4) timeout from the ZC mode and notify via LED
//5) autoinit RF only if ZC'd
void loop()
{
	processed = zeroConfSerial.processOneMessage(&msg);
#if EXAMPLE_BOARD == EXAMPLE_BOARD_MEGA
	if ( processed )
		last_message_timestamp = RTC::millis();
	else if ( RTC::since(last_message_timestamp) > SERIAL_NEXT_MSG_TIMEOUT ) {
		last_message_timestamp = RTC::millis();
		MW_LOG_WARNING(LOG_ZEROCONFROUTER, "No serial messages processed for %d ms", SERIAL_NEXT_MSG_TIMEOUT);
	}
#endif
}

#endif