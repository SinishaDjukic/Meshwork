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

#include "Meshwork.h"
#include "Cosa/Pin.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/UART.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include <Utils/SerialMessageAdapter.h>

#ifndef MW_LOG_NETWORKSERIAL
	#define MW_LOG_NETWORKSERIAL	MW_FULL_DEBUG
#endif

using namespace Meshwork::L3::NetworkV1;

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
										 Meshwork::L3::NetworkV1::NetworkV1::RouteProvider,
										 SerialMessageAdapter::SerialMessageListener {

			public:
				
				//Network Serial Code ID
				static const uint8_t NS_CODE 						= 0;

        //32-255: Code specific
				static const uint8_t NS_SUBCODE_INTERNAL 			= 32;
				static const uint8_t NS_SUBCODE_CFGBASIC 			= 41;//0x
				static const uint8_t NS_SUBCODE_CFGNWK 				= 42;//0x
				static const uint8_t NS_SUBCODE_RFINIT 				= 43;//0x
				static const uint8_t NS_SUBCODE_RFDEINIT 			= 44;//0x
				static const uint8_t NS_SUBCODE_RFRECV 				= 45;//0x
				static const uint8_t NS_SUBCODE_RFRECVACK 			= 46;//0x
				static const uint8_t NS_SUBCODE_RFSTARTRECV 		= 47;//0x
				static const uint8_t NS_SUBCODE_RFSEND 				= 48;//0x
				static const uint8_t NS_SUBCODE_RFSENDACK 			= 49;//0x
				static const uint8_t NS_SUBCODE_RFBCAST 			= 50;//0x
				static const uint8_t NS_SUBCODE_CFGREQUEST			= 51;//0x
				static const uint8_t NS_SUBCODE_RFROUTEFOUND		= 52;//0x //called from NS_SUBCODE_RFRECV
				static const uint8_t NS_SUBCODE_RFROUTEFAILED		= 53;//0x //called from NS_SUBCODE_RFSEND
				static const uint8_t NS_SUBCODE_RFGETROUTECOUNT		= 54;//0x //called from NS_SUBCODE_RFSEND
				static const uint8_t NS_SUBCODE_RFGETROUTECOUNTRES	= 55;//0x //response to NS_SUBCODE_RFGETROUTECOUNT
				static const uint8_t NS_SUBCODE_RFGETROUTE			= 56;//0x //called from NS_SUBCODE_RFSEND
				static const uint8_t NS_SUBCODE_RFGETROUTERES		= 57;//0x //response to NS_SUBCODE_RFGETROUTE
				
        //32-255: Code/sub-code specific
				static const uint8_t NS_NOK_RECV 				= 32;
				static const uint8_t NS_NOK_SEND 				= 33;
				static const uint8_t NS_NOK_BCAST 				= 34;
				static const uint8_t NS_NOK_KEY_TOO_LONG 		= 35;

			protected:
				Meshwork::L3::Network* m_network;
				SerialMessageAdapter* m_adapter;

				NetworkV1::route_t m_currentRoute;
				uint8_t m_currentRouteHops[NetworkV1::MAX_ROUTING_HOPS];
				char m_networkKey[Meshwork::L3::Network::MAX_NETWORK_KEY_LEN + 1];//+1 for NULL
				uint8_t m_lastSerialMsgLen;
				uint8_t m_lastMsgSrc;
				uint8_t m_lastMsgPort;
				uint8_t m_lastMsgData[NetworkV1::PAYLOAD_MAX];
				uint16_t m_lastMsgLen;
				uint8_t m_lastAckData[NetworkV1::ACK_PAYLOAD_MAX];
				SerialMessageAdapter::serialmsg_t* m_currentMsg;
		
				//needed?
				void respondSendACK(SerialMessageAdapter::serialmsg_t* msg, uint8_t datalen, uint8_t* ackData);
				
				uint8_t processCfgBasic(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processCfgNwk(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processRFInit(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processRFDeinit(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processRFStartRecv(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processRFSend(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processRFBroadcast(SerialMessageAdapter::serialmsg_t* msg);

			public:
				NetworkSerial(Meshwork::L3::Network* network, SerialMessageAdapter* adapter):
					m_network(network),
					m_adapter(adapter),
					m_lastSerialMsgLen(0),
					m_lastMsgLen(NetworkV1::PAYLOAD_MAX)
				{
					m_networkKey[0] = 0;
					//TODO Pull into Network class API
					((Meshwork::L3::NetworkV1::NetworkV1*)network)->set_route_advisor(this);
				}
				
				uint8_t initSerial();
				
    			uint8_t processOneMessage(SerialMessageAdapter::serialmsg_t* msg);

				SerialMessageAdapter* getSerialMessageAdapter() {
					return m_adapter;
				}

				void setSerialMessageAdapter(SerialMessageAdapter* adapter) {
					m_adapter = adapter;
				}
				
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
