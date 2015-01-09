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

#include "Cosa/Pin.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

/*
	LEN | SEQ | MSG
	MSG = MSGCODE | MSGSUBCODE | MSGDATA

    ERRORCODE = <list of possible errors TBD>
    CFGBASIC = NKWCAPS | DELIVERY | RETRY
    CFGNWK = CHANNEL ID | NWK ID | NODE ID | NWKKEYLEN | NWKKEY
    NWKKEY = <char seq>
    RFRECV = SRC | PORT | DATALEN | DATA
    RFRECVACK = DATALEN | DATA
    RFSTARTRECV = TIMEOUT
    RFSEND = DST | PORT | DATALEN | DATA
    RFSENDACK = DATALEN | DATA
    RFBCAST = PORT | DATALEN | DATA
    RFROUTEFOUND = HOPCOUNT | SRC | <list of HOPs> | DST
    RFROUTEFAILED = HOPCOUNT | SRC | <list of HOPs> | DST
    RFGETROUTECOUNT = DST
    RFGETROUTECOUNTRES = ROUTECOUNT
    RFGETROUTE = DST | ROUTEINDEX
    RFGETROUTERES = HOPCOUNT | SRC | <list of HOPs> | DST
*/
namespace Meshwork {

	namespace L3 {
	
		namespace NetworkV1 {
		
			class NetworkSerial : public Meshwork::L3::Network::ACKProvider,
										 Meshwork::L3::NetworkV1::NetworkV1::RouteProvider {

			public:
				static const uint8_t MAX_SERIALMSG_LEN 				= 32;
				
				//Network Serial Code ID
				static const uint8_t NS_CODE 						= 0;

				//Network Serial SubCode IDs
				static const uint8_t NS_SUBCODE_OK 					= 0;
				static const uint8_t NS_SUBCODE_NOK 				= 1;
				static const uint8_t NS_SUBCODE_UNKNOWN 			= 2;
				static const uint8_t NS_SUBCODE_INTERNAL 			= 3;
				static const uint8_t NS_SUBCODE_CFGBASIC 			= 10;//0x0A
				static const uint8_t NS_SUBCODE_CFGNWK 				= 11;//0x0B
				static const uint8_t NS_SUBCODE_RFINIT 				= 20;//0x14
				static const uint8_t NS_SUBCODE_RFDEINIT 			= 21;//0x15
				static const uint8_t NS_SUBCODE_RFRECV 				= 22;//0x16
				static const uint8_t NS_SUBCODE_RFRECVACK 			= 23;//0x17
				static const uint8_t NS_SUBCODE_RFSTARTRECV 		= 24;//0x18
				static const uint8_t NS_SUBCODE_RFSEND 				= 25;//0x19
				static const uint8_t NS_SUBCODE_RFSENDACK 			= 26;//0x1A
				static const uint8_t NS_SUBCODE_RFBCAST 			= 27;//0x1B
				static const uint8_t NS_SUBCODE_CFGREQUEST			= 28;//0x1C
				static const uint8_t NS_SUBCODE_RFROUTEFOUND		= 29;//0x1D //called from NS_SUBCODE_RFRECV
				static const uint8_t NS_SUBCODE_RFROUTEFAILED		= 30;//0x1E //called from NS_SUBCODE_RFSEND
				static const uint8_t NS_SUBCODE_RFGETROUTECOUNT		= 31;//0x1F //called from NS_SUBCODE_RFSEND
				static const uint8_t NS_SUBCODE_RFGETROUTECOUNTRES	= 32;//0x20 //response to NS_SUBCODE_RFGETROUTECOUNT
				static const uint8_t NS_SUBCODE_RFGETROUTE			= 33;//0x21 //called from NS_SUBCODE_RFSEND
				static const uint8_t NS_SUBCODE_RFGETROUTERES		= 34;//0x22 //response to NS_SUBCODE_RFGETROUTE
				
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
				
				static const uint16_t TIMEOUT_RESPONSE 			= 5000;
				
			public:
				struct serialmsg_t {
					uint8_t seq;
					uint8_t len;
					uint8_t code;
					uint8_t data[MAX_SERIALMSG_LEN];
				};

				struct data_cfgbasic_t {
					uint8_t nwkcaps;
					uint8_t delivery;
					uint8_t retry;
				};

				struct data_cfgnwk_t {
					uint8_t channel;
					uint16_t nwkid;
					uint8_t nodeid;
				};

				struct data_rfrecv_t {
					uint8_t src;
					uint8_t port;
					uint8_t datalen;
					uint8_t data[NetworkV1::PAYLOAD_MAX];
				};

				struct data_rfrecvack_t {
					uint8_t datalen;
					uint8_t data[NetworkV1::ACK_PAYLOAD_MAX];
				};

				struct data_rfstartrecv_t {
					uint32_t timeout;
				};

				struct data_rfsend_t {
					uint8_t dst;
					uint8_t port;
					uint8_t datalen;
					uint8_t data[NetworkV1::PAYLOAD_MAX];
				};

				struct data_rfbcast_t {
					uint8_t port;
					uint8_t datalen;
					uint8_t data[NetworkV1::PAYLOAD_MAX];
				};

			protected:
				Meshwork::L3::Network* m_network;
				UART* m_serial;
				uint8_t m_lastSerialMsgLen;
				uint16_t m_timeout;
				serialmsg_t* m_currentMsg;
				NetworkV1::route_t m_currentRoute;
				uint8_t m_currentRouteHops[NetworkV1::MAX_ROUTING_HOPS];
				char m_networkKey[Meshwork::L3::Network::MAX_NETWORK_KEY_LEN + 1];//+1 for NULL
				uint8_t m_lastMsgSrc;
				uint8_t m_lastMsgPort;
				uint8_t m_lastMsgData[NetworkV1::PAYLOAD_MAX];
				uint16_t m_lastMsgLen;
				uint8_t m_lastAckData[NetworkV1::ACK_PAYLOAD_MAX];
		
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
				NetworkSerial(Meshwork::L3::Network* network, UART* serial, uint16_t timeout = TIMEOUT_RESPONSE):
					m_network(network),
					m_serial(serial),
					m_lastSerialMsgLen(0),
					m_timeout(timeout),
					m_lastMsgLen(NetworkV1::PAYLOAD_MAX)
				{
					m_networkKey[0] = 0;
					//TODO Pull into Network class API
					((Meshwork::L3::NetworkV1::NetworkV1*)network)->set_route_advisor(this);
				}
				
				bool initSerial();
				bool waitForBytes(uint8_t count, uint16_t millis);
				int readByte();
				void readRemainingMessageBytes();
				
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
