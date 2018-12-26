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
#ifndef __EXAMPLES_CONSOLE_H__
#define __EXAMPLES_CONSOLE_H__

//The full sketch is too big for Leonardo, so opt out some stuff here
//LOGGING
//#define MW_FULL_DEBUG true
#define MW_LOG_NETWORKV1 true
#define MW_LOG_ALL_ENABLE  true
#define MW_LOG_TIMESTAMP_ENABLE false
//#define MW_LOG_DEBUG_ENABLE  true

//FUNCTIONS
#define LOG_CONSOLE_SUPPORT_SEND true
#define LOG_CONSOLE_SUPPORT_RECV true

#include <MeshworkConfiguration.h>
//Select your RF chip. Currently, only NRF24L01P is supported
#define MW_RF_SELECT 				MW_RF_NRF24L01P

//Select Route Cache table option: MW_ROUTECACHE_NONE, MW_ROUTECACHE_RAM, MW_ROUTECACHE_PERSISTENT
#define MW_ROUTECACHE_SELECT		MW_ROUTECACHE_NONE

#include <stdlib.h>
#include <Cosa/Trace.hh>
#include <Cosa/Types.h>
#include <Cosa/IOStream.hh>
#include <Cosa/UART.hh>
#include <Cosa/Watchdog.hh>
#include <Cosa/RTT.hh>
#include <Cosa/Wireless.hh>

#include <Meshwork.h>
#include <Meshwork/L3/Network.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.cpp>

#include "Utils/LineReader.h"

#include <NRF24L01P.h>

#include "StaticACKProvider.h"
#include "StaticRouteProvider.h"


MW_DECL_NRF24L01P(rf)

StaticRouteProvider advisor;
Meshwork::L3::NetworkV1::NetworkV1 mesh(&rf, &advisor);
LineReader<64> console(&uart);
StaticACKProvider ackProvider;

void setup()
{

  Watchdog::begin();
  RTT::begin();

  uart.begin(115200);

  trace.begin(&uart, PSTR("Interactive Serial Console: started"));
  trace << PSTR("Board: ") << MW_BOARD_SELECT << endl;
}

bool equals(char s1[], char s2[]) {
    return strncmp(s1, s2, strlen(s2)) == 0;
}

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
	trace << PSTR("NOP to ") << address << PSTR(":") << port << endl;
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

#if LOG_CONSOLE_SUPPORT_SEND
void run_send(uint8_t address, uint8_t port, char * pStart, char * pEnd) {
	size_t dataLen = pEnd - pStart;
//	trace << dataLen << PSTR(" CHARS to ") << address << PSTR(":") << port;
//	trace << PSTR("\nChars: ");
	trace.printf(PSTR("%d CHARS to %d: %d\nChars:"), dataLen, address, port);

	print_message((char *)pStart, dataLen);
	trace.println();
	size_t ackLen = 5;
	uint8_t ack[ackLen];
	for ( size_t i = 0; i < ackLen; i ++ ) ack[i] = 0;
	int res = mesh.send(delivery, -1, address, port, pStart, dataLen, ack, ackLen);
	trace.printf(PSTR("Res code: %d\nAck len: %d\nAck: \t"), res, ackLen);
	print_message((char *)ack, ackLen);
	//MW_LOG_ARRAY(true, PSTR("Ack: "), ack, ackLen);
//	trace.print(ack, ackLen, IOStream::hex, NetworkV1::PAYLOAD_MAX);
	trace.println();
	
}
#endif

void run_setup(uint16_t network, uint8_t address, uint8_t channel) {
	mesh.setNetworkID(network);
	mesh.setNodeID(address);
	mesh.setChannel(channel);
}

bool running = false;

void printRFRunning() {
	trace << PSTR("\nRF on: ") << running;
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
	trace.printf(PSTR("Network: %d\nAddress: %d\nChannel: %d\n"), mesh.getNetworkID(), mesh.getNodeID(), mesh.getChannel());
//	trace << PSTR("Network: ") << mesh.getNetworkID() << PSTR("\nAddress: ") << mesh.getNodeID() << PSTR("\nChannel: ") << mesh.getChannel() << PSTR("\n");
	set_delivery(delivery);//will print out the status
	printRFRunning();
}

void run_routereset() {
	trace << PSTR("Route reset\n");
	advisor.route_reset();
}

#if LOG_CONSOLE_SUPPORT_RECV
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
	
	uint32_t start = RTT::millis();
	while (duration > 0) {
		int result = mesh.recv(src, port, data, dataLenMax, duration, &ackProvider);
		
		trace.printf(PSTR("RECV: res=%d, src=%d, port=%d, dataLen=%d, data=\n"), result, src, port, dataLenMax);
//		trace << PSTR("[RECV] res=") << result << PSTR(", src=") << src << PSTR(", port=") << port;
//		trace << PSTR(", dataLen=") << dataLenMax << PSTR(", data=\n");
		MW_LOG_ARRAY(true, PSTR("\t...L3 DATA RECV: "), data, dataLenMax);
		trace.println();
		
		duration -= RTT::since(start);
	} 
	
	trace << PSTR("RECV: done\n");
}
#endif

void run_routeadd(char* line) {
	bool ok = false;
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
	trace << PSTR("RX clear\n");
	uint32_t duration = (uint32_t) 1 * 1000L;
	uint32_t singleTimeout = (uint32_t) 100L;
	uint8_t src, port;
	size_t dataLenMax = Meshwork::L3::NetworkV1::NetworkV1::PAYLOAD_MAX;
	uint8_t data[dataLenMax];
	
	uint32_t start = RTT::millis();
	while (true) {
		int result = mesh.recv(src, port, data, dataLenMax, singleTimeout, &ackProvider);
		if ( RTT::since(start) >= duration )
			break;
	} 
	trace << PSTR("RX cleared\n");
}

void loop()
{

	uint8_t len = console.readline();
	if ( len > 0 ) {
		char* line = console.get_line();
    trace.printf(PSTR("Command: '%s'\n"), line);
		Meshwork::Debug::printFreeMemory();
		if ( equals(line, (char*) "?") ) {
			trace << PSTR("Usage:"
			              "\n ?"
			              "\n info"
			              "\n setup <network> <address> <channel>"
			              "\n begin"
			              "\n end"
			              "\n clearrx"
#if LOG_CONSOLE_SUPPORT_RECV
			              "\n recv"
#endif
			              "\n nop <dest> <port>"
#if LOG_CONSOLE_SUPPORT_SEND
			              "\n send <dest> <port> <chars>"
#endif
			              "\n deliv <flags>"
			              "\n routereset"
			              "\n routeadd <index> <src> <dst> [hop, ...]"
			              "\n...values in DEC\n");
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
#if LOG_CONSOLE_SUPPORT_RECV
		} else if ( equals(line, (char*) "recv") ) {
			run_recv();
#endif
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
#if LOG_CONSOLE_SUPPORT_SEND
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
				trace << PSTR("No data!\n");
#endif
		} else {
      trace << PSTR("Unknown cmd\n");
		}
    trace << endl << endl;
		//
		console.clear();
	} else if ( len == -1 ) {
		console.clear();
	}
	
}

#endif
