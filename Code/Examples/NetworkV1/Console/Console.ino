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
#include <ctype.h>
#include "Cosa/Trace.hh"
#include "Cosa/Types.h"
#include "Cosa/IOStream.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Memory.h"
#include <Meshwork.h>
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.cpp"
#include "Utils/LineReader.h"

// Select Wireless device driver
// #include "Cosa/Wireless/Driver/CC1101.hh"
// CC1101 rf(0xC05A, 0x01);

#include "Cosa/Wireless/Driver/NRF24L01P.hh"
//NRF24L01P rf(0xC05A, 0x01);

#include "StaticACKProvider.h"
#include "StaticRouteProvider.h"

//#define RBBB

//RBBB
#if defined (RBBB)
#define BOARD_VARIANT	PSTR("RBBB")
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
#define BOARD_VARIANT	PSTR("Mini/Nano")
NRF24L01P rf(0x0001, 0x01,
	    Board::DigitalPin(Board::D10), 
	    Board::DigitalPin(Board::D3), 
	    Board::ExternalInterruptPin(Board::EXT0));
#endif

#endif

StaticRouteProvider advisor;
Meshwork::L3::NetworkV1::NetworkV1 mesh(&rf, &advisor);
LineReader<64> console(&uart);
StaticACKProvider ackProvider;

void setup()
{

  uart.begin(9600);
  uint8_t mode = SLEEP_MODE_IDLE;
  //TODO revert back watchdog settings
  Watchdog::begin(16, mode);  
//  Watchdog::begin();
  rf.set_sleep(mode);
  RTC::begin();
  trace.begin(&uart, PSTR("NetworkV1 Serial Console: started"));
  trace << PSTR("Board: ") << BOARD_VARIANT;
  trace.println();
}

//bool equalsConst(const char s1[], const char s2[]) {
///	return equals((char*) s1, (char *) s2);
//}

bool equals(char s1[], char s2[]) {
    return strncmp(s1, s2, strlen(s2)) == 0;
/*
	if ( strlen(s1) != strlen(s2) )
		return false;
	for ( int i = strlen(s2) - 1; i >= 0; i --)
		if ( s1[i] != s2[i] )
			return false;
	return true;
	*/
}

//bool startsWithConst(int fullOffset, const char* full, const char* subs) {
//	return startsWith(fullOffset, (char *) full, (char *) subs);
//}

bool startsWith(int fullOffset, char* full, char* subs) {
    int subslen = strlen(subs);
    if ( fullOffset > ((int) strlen(full)) - subslen )
        return false;
    return strncmp( &full[fullOffset], subs, subslen ) == 0;
}

bool endsWith(char full[], char subs[]) {
    int lendiff = strlen(full) - strlen(subs);
    if ( lendiff < 0 )
        return false;
    return strcmp(&full[lendiff], subs) == 0;
}

uint8_t delivery = Meshwork::L3::Network::DELIVERY_EXHAUSTIVE;

void set_delivery(uint8_t deliv) {
	delivery = deliv;
	trace << PSTR("Deliv:\n");
	if ( delivery & Meshwork::L3::Network::DELIVERY_DIRECT )
		trace << PSTR("\tDirect\n");
	if ( delivery & Meshwork::L3::Network::DELIVERY_ROUTED )
		trace << PSTR("\tRouted\n");
	if ( delivery & Meshwork::L3::Network::DELIVERY_FLOOD )
		trace << PSTR("\tFlood\n");
}

// Check if the message is printable
bool is_ascii(char * msg, uint8_t len) {
	bool result = true;
	for (uint8_t i = 0; i < len; i++) {
	  if ((msg[i] < ' ' && msg[i] != '\n' && msg[i] != '\f') || msg[i] > 127) {
		  result = false;
		break;
	  }
	}
	return result;
}

void print_message(char * msg, uint8_t len) {
	if (is_ascii(msg, len)) {
	  trace << PSTR("\"");
	  for (uint8_t i = 0; i < len; i++)
		if (msg[i] == '\f')
		trace << PSTR("\\f");
		else if (msg[i] == '\n')
		trace << PSTR("\\n");
		else 
		trace << (char) msg[i];
	  trace << PSTR("\"") << endl;
	} else { 
	  trace.print(msg, len, IOStream::hex, len);
	}
}

void run_nop(uint8_t address, uint8_t port) {
	trace << PSTR("NOP to ") << address << PSTR(":") << port << PSTR("\n");
	size_t ackLen = 5;
	uint8_t ack[ackLen];
	for ( size_t i = 0; i < ackLen; i ++ ) ack[i] = 0;
	int res = mesh.send(delivery, 255, address, port, NULL, 0, ack, ackLen);
	trace << PSTR("Result code: ") << res;
	trace << PSTR("\nAck len: ") << ackLen;
	trace << PSTR("\nAck: \t");
//	trace.print(ack, ackLen, IOStream::hex, NetworkV1::PAYLOAD_MAX);
	if ( ack != NULL && ackLen > 0 ) {
		char * ackP = (char *) ack;
		print_message(ackP, ackLen);
	}
	trace.println();
	
}

void run_send(uint8_t address, uint8_t port, char * pStart, char * pEnd) {
	size_t dataLen = pEnd - pStart;
	trace << dataLen << PSTR(" CHARS to ") << address << PSTR(":") << port;
	trace << PSTR("\nChars: ");
	print_message((char *)pStart, dataLen);
	trace.println();
	size_t ackLen = 5;
	uint8_t ack[ackLen];
	for ( size_t i = 0; i < ackLen; i ++ ) ack[i] = 0;
	int res = mesh.send(delivery, -1, address, port, pStart, dataLen, ack, ackLen);
	trace << PSTR("Res code: ") << res;
	trace << PSTR("\nAck len: ") << ackLen;
	trace << PSTR("\nAck: \t");
	print_message((char *)ack, ackLen);
	//MW_LOG_DEBUG_ARRAY(PSTR("Ack: "), ack, ackLen);
//	trace.print(ack, ackLen, IOStream::hex, NetworkV1::PAYLOAD_MAX);
	trace.println();
	
}

void run_setup(uint16_t network, uint8_t address, uint8_t channel) {
	mesh.setNetworkID(network);
	mesh.setNodeID(address);
	mesh.setChannel(channel);
}

bool running = false;

void printRFRunning() {
	trace << PSTR("\nRF Running: ") << running;
}

void run_begin() {
	running = mesh.begin();
	printRFRunning();
}

void run_end() {
	running = !mesh.end();
	printRFRunning();
}

void run_info() {
	trace << PSTR("Network: ") << mesh.getNetworkID() << PSTR("\nAddress: ") << mesh.getNodeID() << PSTR("\nChannel: ") << mesh.getChannel() << PSTR("\n");
	set_delivery(delivery);//will print out the status
	printRFRunning();
}

void run_routereset() {
	trace << PSTR("Route reset\n");
	advisor.route_reset();
}

void run_recv() {
	if ( !running ) {
		printRFRunning();
		return;
	}
	uint32_t duration = (uint32_t) 60 * 1000L;
	uint8_t src, port;
	size_t dataLenMax = Meshwork::L3::NetworkV1::NetworkV1::PAYLOAD_MAX;
	uint8_t data[dataLenMax];
	trace << PSTR("RECV: dur=") << duration << PSTR(", dataLenMax=") << dataLenMax << PSTR("\n");
	
	uint32_t start = RTC::millis();
	while (true) {
		int result = mesh.recv(src, port, data, dataLenMax, duration, &ackProvider);
		if ( result != -1 ) {
			trace << PSTR("[RECV] res=") << result << PSTR(", src=") << src << PSTR(", port=") << port;
			trace << PSTR(", dataLen=") << dataLenMax << PSTR(", data=\n");
			MW_LOG_DEBUG_ARRAY(PSTR("\t...L3 DATA RECV: "), data, dataLenMax);
			trace.println();
		}
		if ( RTC::since(start) >= duration )
			break;
	} 
	
	trace << PSTR("RECV: done\n");
}

void run_routeadd(char* line) {
	bool ok = false;
	//format: routeIndex index src dst [hop, ...]
	char * pEnd = line + 9;//cmd len plus space
	uint8_t index = strtol(pEnd, &pEnd, 10);
	uint8_t maxRoutes = advisor.get_max_routes();
	if ( index >= 0 && index < maxRoutes ) {
		uint8_t src = strtol(pEnd, &pEnd, 10);
		if ( src != 0 ) {
			uint8_t dst = strtol(pEnd, &pEnd, 10);
			if ( dst != 0 ) {
				uint8_t maxHops = advisor.get_max_route_hops();
				uint8_t hops[maxHops];
				uint8_t hop = 0, total = 0;
				for ( int i = 0; i < maxHops; i ++ ) {
					hop = strtol(pEnd, &pEnd, 10);
					if ( hop != 0 ) {
						hops[i] = hop;
						total ++;
					} else {
						break;
					}
				}
				advisor.set_route(index, src, dst, hops, total);
				trace << PSTR("Added\n");
				ok = true;
			}
		}
	}
	if ( !ok )
		trace << PSTR("Invalid params\n");
}

void run_clearrx() {
	uint32_t duration = (uint32_t) 1 * 1000L;
	uint32_t singleTimeout = (uint32_t) 100L;
	uint8_t src, port;
	size_t dataLenMax = Meshwork::L3::NetworkV1::NetworkV1::PAYLOAD_MAX;
	uint8_t data[dataLenMax];
	
	uint32_t start = RTC::millis();
	while (true) {
		int result = mesh.recv(src, port, data, dataLenMax, singleTimeout, &ackProvider);
		if ( RTC::since(start) >= duration )
			break;
	} 
	trace << PSTR("Clear RX!\n");
}

void loop()
{

//TODO RouteProvider impl

	uint8_t len = console.readline();
	if ( len > 0 ) {
		char* line = console.get_line();
		trace << PSTR("Command: ") << line;
		trace.println();
		Meshwork::Debug::printFreeMemory();
		if ( equals(line, (char*) "?") || equals(line, (char*) "help") ) {
			trace << PSTR("Usage:\ninfo\nsetup <network> <address> <channel>\nbegin\nend\nclearrx\nrecv\nnop <dest> <port>\nsend <dest> <port> <chars>\ndeliv <delivery flags>\nroutereset\nrouteadd\nclearrx\nValues in DEC\n");
		} else if ( equals(line, (char*) "info") ) {
			run_info();
		} else if ( equals(line, (char*) "setup") ) {
			char * pEnd = line + 6;//cmd len plus space
			int16_t network = strtol(pEnd, &pEnd, 10);
			uint8_t address = strtol(pEnd, &pEnd, 10);
			uint8_t channel = strtol(pEnd, &pEnd, 10);
			run_setup(network, address, channel);
		} else if ( equals(line, (char*) "begin") ) {
			run_begin();
		} else if ( equals(line, (char*) "end") ) {
			run_end();
		} else if ( equals(line, (char*) "recv") ) {
			run_recv();
		} else if ( startsWith(0, line, (char*) "deliv") ) {
			char * pEnd = line + 6;//cmd len plus space
			uint8_t temp1 = strtol(pEnd, &pEnd, 10);
			set_delivery(temp1);
		} else if ( startsWith(0, line, (char*) "routereset") ) {
			run_routereset();
		} else if ( startsWith(0, line, (char*) "routeadd") ) {
			run_routeadd(line);
		} else if ( startsWith(0, line, (char*) "clearrx") ) {
			run_clearrx();
		} else if ( startsWith(0, line, (char*) "nop") ) {
			run_clearrx();
			char * pEnd = line + 4;//cmd len plus space
			uint8_t dest = strtol(pEnd, &pEnd, 10);
			uint8_t port = strtol(pEnd, &pEnd, 10);
			run_nop(dest, port);
		} else if ( startsWith(0, line, (char*) "send") ) {
			run_clearrx();
			char * pEnd = line + 4;//cmd len plus space
			uint8_t dest = strtol(pEnd, &pEnd, 10);
			uint8_t port = strtol(pEnd, &pEnd, 10);
			pEnd ++;
			//pEnd will point to the next char in the line, pLast to the last char
			char * pLast = line + len;
			if ( pLast > pEnd )
				run_send(dest, port, pEnd, pLast);
			else
				trace << PSTR("No data!");
		} else {
			trace << PSTR("Unknown: ") << line;
		}
		trace.println();
		trace.println();
		//
		console.clear();
	} else if ( len == -1 ) {
		console.clear();
	}
	
}

