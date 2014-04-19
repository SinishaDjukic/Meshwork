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
#include <stdlib.h>
#include "Cosa/Trace.hh"
#include "Cosa/Types.h"
#include "Cosa/IOStream.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Wireless.hh"
#include <Meshwork.h>
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.cpp"
#include "Meshwork/L3/NetworkV1/NetworkSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkSerial.cpp"
#include "Utils/LineReader.h"

#if defined (__ARDUINO_MEGA__)
#include "Cosa/IOBuffer.hh"
#endif

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
NRF24L01P rf(0x0001, 0x01,
	    Board::DigitalPin(Board::D53), 
	    Board::DigitalPin(Board::D48), 
	    Board::ExternalInterruptPin(Board::EXT2));

//Mini Pro or Nano
#else
#define BOARD_VARIANT	3
NRF24L01P rf(0x0001, 0x01,
	    Board::DigitalPin(Board::D10), 
	    Board::DigitalPin(Board::D3), 
	    Board::ExternalInterruptPin(Board::EXT0));
#endif

#endif

static const uint16_t 	BEACON_NWK_ID 		= 1;
static const uint8_t 	BEACON_CHANNEL_ID 	= 0;
static const uint8_t 	BEACON_NODE_ID 		= 100;
static const uint8_t 	BEACON_BCAST_PORT 	= 0;
static const char 		BEACON_BCAST_MSG[] 	= "*BEACON*";
static const uint8_t	BEACON_BCAST_MSG_LEN = sizeof(BEACON_BCAST_MSG) - 1;//without null termination

Meshwork::L3::NetworkV1::NetworkV1 mesh(&rf, NULL);

void setup()
{
  uart.begin(115200);
  trace.begin(&uart, PSTR("Beacon: started"));
  trace << endl;
  
  trace << PSTR("Network ID: ") << BEACON_NWK_ID << endl;
  trace << PSTR("Channel ID: ") << BEACON_CHANNEL_ID << endl;
  trace << PSTR("Node ID: ") << BEACON_NODE_ID << endl;
  trace << PSTR("Bcast port: ") << BEACON_BCAST_PORT << endl;
  trace << PSTR("Bcast msg len: ") << BEACON_BCAST_MSG_LEN << endl;
  
  mesh.setNetworkID(BEACON_NWK_ID);
  mesh.setChannel(BEACON_CHANNEL_ID);
  mesh.setNodeID(BEACON_NODE_ID);
  
  uint8_t mode = SLEEP_MODE_IDLE;
  Watchdog::begin(16, mode);  
  rf.set_sleep(mode);
  RTC::begin();
}

void loop()
{
	trace << PSTR("Broadcasting...") << endl;
	mesh.broadcast(BEACON_BCAST_PORT, BEACON_BCAST_MSG, BEACON_BCAST_MSG_LEN);
	SLEEP(1);
}

