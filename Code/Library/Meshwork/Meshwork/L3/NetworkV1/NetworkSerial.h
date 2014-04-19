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
#ifndef __MESHWORK_L3_NETWORKV1_NETWORKSERIAL_H__
#define __MESHWORK_L3_NETWORKV1_NETWORKSERIAL_H__

#include "Cosa/Pins.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

//SEQ | LEN | MSG
//MSG = MSGCODE | MSGDATA
//CFGBASIC = NKWCAPS | DELIVERY | RETRY
//CFGNWK = NWK ID | NODE ID | CHANNEL
//RFRECV = SRC | PORT | DATALEN | DATA
//RFRECVACK = DATALEN | DATA
//RFSTARTRECV = TIMEOUT
//RFSEND = DST | PORT | DATALEN | DATA
//RFSENDACK = DATALEN | DATA
//RFBCAST = PORT | DATALEN | DATA
namespace Meshwork {

	namespace L3 {
	
		namespace NetworkV1 {
		
			class NetworkSerial : public Meshwork::L3::Network::ACKProvider,
										 Meshwork::L3::NetworkV1::NetworkV1::RouteProvider {

			public:
			  struct serialmsg_t {
				uint8_t seq;
				uint8_t len;
				uint8_t code;
				uint8_t data[];
			  };
			  
			  struct data_cfgbasic_t {
				uint8_t nwkcaps;
				uint8_t delivery;
				uint8_t retry;
			  };
			  
			  struct data_cfgnwk_t {
				uint16_t nwkid;
				uint8_t nodeid;
				uint8_t channel;
			  };
			  
			  struct data_rfrecv_t {
				uint8_t src;
				uint8_t port;
				uint8_t datalen;
				uint8_t data[];
			  };
			  
			  struct data_rfrecvack_t {
				uint8_t datalen;
				uint8_t data[];
			  };
			  
			  struct data_rfstartrecv_t {
				uint32_t timeout;
			  };
			  
			  struct data_rfsend_t {
				uint8_t dst;
				uint8_t port;
				uint8_t datalen;
				uint8_t data[];
			  };
			  
			  struct data_rfbcast_t {
				uint8_t port;
				uint8_t datalen;
				uint8_t data[];
			  };  
			  
			  //TODO sync structures with below constants and remove unused ones
			  
			public:
				static const uint8_t MAX_SERIALMSG_LEN 			= 64;//TODO calculate the right size!
				
				static const uint8_t MSGCODE_OK 				= 0;
				static const uint8_t MSGCODE_NOK 				= 1;
				static const uint8_t MSGCODE_UNKNOWN 			= 2;
				static const uint8_t MSGCODE_INTERNAL 			= 3;
				static const uint8_t MSGCODE_CFGBASIC 			= 10;
				static const uint8_t MSGCODE_CFGNWK 			= 11;
				static const uint8_t MSGCODE_RFINIT 			= 20;
				static const uint8_t MSGCODE_RFDEINIT 			= 21;
				static const uint8_t MSGCODE_RFRECV 			= 22;
				static const uint8_t MSGCODE_RFRECVACK 			= 23;
				static const uint8_t MSGCODE_RFSTARTRECV 		= 24;
				static const uint8_t MSGCODE_RFSEND 			= 25;
				static const uint8_t MSGCODE_RFSENDACK 			= 26;
				static const uint8_t MSGCODE_RFBCAST 			= 27;
				static const uint8_t MSGCODE_CFGREQUEST			= 28;
				static const uint8_t MSGCODE_RFROUTEFOUND		= 29;//called from MSGCODE_RFRECV
				static const uint8_t MSGCODE_RFROUTEFAILED		= 30;//called from MSGCODE_RFSEND
				static const uint8_t MSGCODE_RFGETROUTECOUNT	= 31;//called from MSGCODE_RFSEND
				static const uint8_t MSGCODE_RFGETROUTECOUNTRES	= 32;//response to MSGCODE_RFGETROUTECOUNT
				static const uint8_t MSGCODE_RFGETROUTE			= 33;//called from MSGCODE_RFSEND
				static const uint8_t MSGCODE_RFGETROUTERES		= 34;//response to MSGCODE_RFGETROUTE
				
				//0-63
				static const uint8_t ERROR_GENERAL 				= 0;
				static const uint8_t ERROR_INSUFFICIENT_DATA 	= 1;
				static const uint8_t ERROR_TOO_LONG_DATA 		= 2;
				static const uint8_t ERROR_ILLEGAL_STATE 		= 3;
				static const uint8_t ERROR_RECV 				= 4;
				static const uint8_t ERROR_SEND 				= 5;
				static const uint8_t ERROR_BCAST 				= 6;
				static const uint8_t ERROR_KEY_TOO_LONG 		= 7;
				static const uint8_t ERROR_SEQUENCE_MISMATCH	= 8;
				
				static const uint16_t TIMEOUT_RESPONSE 			= 300;
				
			protected:
				Meshwork::L3::Network* m_network;
				UART* m_serial;
				serialmsg_t* m_currentMsg;
				NetworkV1::route_t m_currentRoute;
				uint8_t m_currentRouteHops[NetworkV1::MAX_ROUTING_HOPS];
				char m_networkKey[Meshwork::L3::Network::MAX_NETWORK_KEY_LEN + 1];//+1 for NULL
				
				//this saves us ~500 bytes against repetitive putchar calls
				void writeMessage(uint8_t len, uint8_t* data, bool flush);
				
				void respondWCode(serialmsg_t* msg, uint8_t code);
				void respondNOK(serialmsg_t* msg, uint8_t error);
				void respondSendACK(serialmsg_t* msg, uint8_t datalen, uint8_t* ackData);
				
				bool processCfgBasic(serialmsg_t* msg);
				bool processCfgNwk(serialmsg_t* msg);
				bool processRFInit(serialmsg_t* msg);
				bool processRFDeinit(serialmsg_t* msg);

				bool processRFStartRecv(serialmsg_t* msg);
				bool processRFSend(serialmsg_t* msg);
				bool processRFBroadcast(serialmsg_t* msg);

			public:
				NetworkSerial(Meshwork::L3::Network* network, UART* serial):
					m_network(network),
					m_serial(serial)
				{
					m_networkKey[0] = 0;
					//TODO should pull into Network class API
					((Meshwork::L3::NetworkV1::NetworkV1*)network)->set_route_advisor(this);
				}
				
				bool initSerial();
				bool waitForBytes(uint8_t count, uint16_t millis);
				
				bool processOneMessage(serialmsg_t* msg);
				bool processOneMessageEx(serialmsg_t* msg);
				
				int returnACKPayload(uint8_t src, uint8_t port, void* buf, uint8_t len, void* bufACK, size_t lenACK);
				
				void set_address(uint8_t src);
				uint8_t get_routeCount(uint8_t dst);
				NetworkV1::route_t* get_route(uint8_t dst, uint8_t index);
				void route_found(NetworkV1::route_t* route);
				void route_failed(NetworkV1::route_t* route);
			  };
		};
	};
};
#endif