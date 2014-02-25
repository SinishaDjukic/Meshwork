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
#include "Utils/LineReader.h"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.cpp"
#include "Meshwork/L3/NetworkV1/NetworkSerial.cpp"

#include "Meshwork/L4/NodeBase.h"
#include "Meshwork/L4/NodeBase.cpp"
#include "Meshwork/L4/NodeBaseSerial.h"

#include "Meshwork/L4/ControllerBase.h"
#include "Meshwork/L4/ControllerBase.cpp"
#include "Meshwork/L4/ControllerBaseSerial.h"
#include "Meshwork/L4/NetworkV1/ControllerBaseSerial.cpp"

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

NetworkV1 mesh(&rf, NULL);
Meshwork::L3::NetworkSerial networkSerial(&mesh, &uart);

void setup()
{  
  uart.begin(9600);
  Watchdog::begin();
  RTC::begin();
  
}

void loop()
{
	static uint8_t databuf[Meshwork::L3::NetworkSerial::MAX_SERIALMSG_LEN];
	static Meshwork::L3::NetworkSerial::serialmsg_t msg;
	msg.data = databuf;
	networkSerial.processOneMessage(&msg);
}

