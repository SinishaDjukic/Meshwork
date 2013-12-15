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
#include <stdio.h>
#include <stdlib.h>
#include "Cosa/Trace.hh"
#include "Cosa/Types.h"
#include "Cosa/IOStream.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Wireless.hh"
#include "Mesh.h"
#include "Mesh/Network/MeshV1.h"
#include "Mesh/Network/MeshV1.cpp"
#include "LineReader.h"

uint8_t network = 1;
uint8_t address = 1;

// Select Wireless device driver
// #include "Cosa/Wireless/Driver/CC1101.hh"
// CC1101 rf(0xC05A, 0x01);

#include "Cosa/Wireless/Driver/NRF24L01P.hh"
//NRF24L01P rf(0xC05A, 0x01);

//#define RBBB

//RBBB
#if defined (RBBB)
#define BOARD_VARIANT	1
NRF24L01P rf(0x0001, 0x01,
	    Board::DigitalPin(Board::D7), 
	    Board::DigitalPin(Board::D8), 
	    Board::ExternalInterruptPin(Board::EXT0));
#else

//Mega
#if defined (__ARDUINO_MEGA__)
#define BOARD_VARIANT	2
NRF24L01P rf(0x0001, 0x01);

//Mini Pro or Nano
#else
#define BOARD_VARIANT	3
NRF24L01P rf(0x0001, 0x01,
	    Board::DigitalPin(Board::D10), 
	    Board::DigitalPin(Board::D3), 
	    Board::ExternalInterruptPin(Board::EXT0));
#endif

#endif

MeshV1 mesh(&rf, NULL);

void setup()
{  
  uart.begin(9600);
  Watchdog::begin();
  RTC::begin();
  
  trace.begin(&uart, PSTR("MeshV1 Receiver: started"));
  trace << PSTR("Board: ") << BOARD_VARIANT << PSTR("\n");
  trace << PSTR("Set up network:address: ") << network << ":" << address << PSTR("\n");
 
  mesh.get_driver()->set_address(network, address);
  bool running = mesh.begin();
  
  trace << PSTR("RF Running: ") << running;
}

void loop()
{
	uint8_t src, port;
	uint32_t duration = 60 * 1000L;
	size_t dataLenMax = MeshV1::PAYLOAD_MAX;
	uint8_t data[dataLenMax];
	int result = mesh.recv(src, port, &data, dataLenMax, duration, NULL);
	trace << RTC::millis();
	trace << PSTR(": [RECEIVED] result=") << result << PSTR(", src=") << src << PSTR(", port=") << port;
	trace << PSTR(", dataLen=") << dataLenMax << PSTR(", data=") << PSTR("\n\t\t\t");
	if ( result > 0 )
	  trace.print(data, result, IOStream::hex, result);
	trace.println();
}

