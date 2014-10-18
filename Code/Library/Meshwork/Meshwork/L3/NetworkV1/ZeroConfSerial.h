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
#ifndef __MESHWORK_L3_NETWORKV1_ZEROCONFSERIAL_H__
#define __MESHWORK_L3_NETWORKV1_ZEROCONFSERIAL_H__

#include "Cosa/Pin.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

/*
	SEQ | LEN | MSG
	MSG = MSGCODE | MSGDATA
	ERRORCODE  = <list of possible errors TBD>
	
	Incoming
	========
	ZCINIT     = <empty>
	ZCDEINIT   = <empty>
	ZCID       = <empty>
	ZCNWKID    = <empty>
	ZCCFGNWK   = CHANNEL | NWK ID | NODE ID | NWKKEYLEN | NWKKEY
	ZCCFGREP   = TARGET NODE ID | REPFLAGS
	REPFLAGS   = <list of reporting flags TBD>
		b0: Report after add/remove to/from the network
		b1: Report values upon discrete change
		b2: Report values upon threshold change
		b3-b7: Reserved
	
	Outgoing
	========
	ZCIDRES    = NKWCAPS | DELIVERY | SERNUMLEN | SERNUM
	ZCNWKIDRES = CHANNEL | NWK ID | NODE ID
	OK, NOK, ERROR_ILLEGAL_STATE, ERROR_KEY_TOO_LONG, ERROR_SERIAL_TOO_LONG, ERROR_INSUFFICIENT_DATA, MSGCODE_UNKNOWN
*/
namespace Meshwork {

	namespace L3 {
	
		namespace NetworkV1 {
		
			class ZeroConfSerial {

			public:
			
				class ZeroConfListener {
					public:
						//called after the serial number has been updated
						virtual void serial_updated() = 0;
						//called after the network configuration has been updated
						virtual void network_updated() = 0;
						//called after the new reporting flags have been updated
						virtual void reporting_updated() = 0;
				};
				
				static const uint8_t MAX_SERIALMSG_LEN 			= 64;//TODO calculate the right size!
				static const uint8_t MAX_SERIAL_LEN 			= 16;
				
				struct serialmsg_t {
					uint8_t seq;
					uint8_t len;
					uint8_t code;
				};

				struct zctype_sernum_t {
					uint8_t sernumlen;
					char sernum[MAX_SERIAL_LEN];
				};

				struct zctype_reporting_t {
					uint8_t targetnodeid;
					uint8_t repflags;
				};
			
				struct zctype_nwkconfig_t {
					uint8_t channel;
					uint16_t nwkid;
					uint8_t nodeid;
					uint8_t nwkkeylen;
					uint8_t nwkkey[Network::MAX_NETWORK_KEY_LEN];
				};

			public:
				
				static const uint8_t MSGCODE_OK 				= 0;
				static const uint8_t MSGCODE_NOK 				= 1;
				static const uint8_t MSGCODE_UNKNOWN 			= 2;
				
				static const uint8_t MSGCODE_ZCINIT 			= 48;
				static const uint8_t MSGCODE_ZCDEINIT 			= 49;
				static const uint8_t MSGCODE_ZCID 				= 50;
				static const uint8_t MSGCODE_ZCIDRES 			= 51;
				static const uint8_t MSGCODE_ZCNWKID 			= 52;
				static const uint8_t MSGCODE_ZCNWKIDRES 		= 53;
				static const uint8_t MSGCODE_ZCCFGNWK 			= 54;
				static const uint8_t MSGCODE_ZCCFGREP 			= 55;
				static const uint8_t MSGCODE_ZCCFGSERIAL		= 56;
				//TODO add REQ and RES that identifies the device vendor and model, used RF chip and frequency, extra metadata
				
				//0-63
				static const uint8_t ERROR_GENERAL 				= 0;
				static const uint8_t ERROR_INSUFFICIENT_DATA 	= 1;
				static const uint8_t ERROR_TOO_LONG_DATA 		= 2;
				static const uint8_t ERROR_ILLEGAL_STATE 		= 3;
				static const uint8_t ERROR_KEY_TOO_LONG 		= 7;
				static const uint8_t ERROR_SEQUENCE_MISMATCH	= 8;
				static const uint8_t ERROR_SERIAL_TOO_LONG 		= 7;
				
				static const uint16_t TIMEOUT_RESPONSE 			= 5000;
				
				//flags for configuring reporting
				static const uint8_t MASK_REPORT_NWK_ADD_REMOVE		= 1 << 0;
				static const uint8_t MASK_REPORT_DISCRETE_CHANGE	= 1 << 1;
				static const uint8_t MASK_REPORT_THRESHOLD_CHANGE	= 1 << 2;
				
			protected:
				Meshwork::L3::Network* m_network;
				UART* m_serial;
				
				zctype_sernum_t* m_sernum;
				zctype_reporting_t* m_reporting;
				zctype_nwkconfig_t* m_nwkconfig;
				
				ZeroConfListener* m_listener;
				uint16_t m_timeout;
				bool m_initmode;
				
				serialmsg_t* m_currentMsg;
				
				//this saves us ~500 bytes against repetitive putchar calls
				void writeMessage(uint8_t len, uint8_t* data, bool flush);
				
				void respondWCode(serialmsg_t* msg, uint8_t code);
				void respondNOK(serialmsg_t* msg, uint8_t error);
				void respondSendACK(serialmsg_t* msg, uint8_t datalen, uint8_t* ackData);
				
				bool processZCInit(serialmsg_t* msg);
				bool processZCDeinit(serialmsg_t* msg);
				bool processZCID(serialmsg_t* msg);
				bool processZCNwkID(serialmsg_t* msg);
				bool processZCCfgNwk(serialmsg_t* msg);
				bool processZCCfgSerial(serialmsg_t* msg);
				bool processZCCfgRep(serialmsg_t* msg);

			public:
				ZeroConfSerial(Meshwork::L3::Network* network, UART* serial,
								zctype_sernum_t* sernum, zctype_reporting_t* reporting, zctype_nwkconfig_t* nwkconfig,
										ZeroConfListener* listener, uint16_t timeout = TIMEOUT_RESPONSE):
					m_network(network),
					m_serial(serial),
					m_sernum(sernum),
					m_reporting(reporting),
					m_nwkconfig(nwkconfig),
					m_listener(listener),
					m_timeout(timeout),
					m_initmode(false)
				{
				}
				
				bool waitForBytes(uint8_t count, uint16_t millis);
				
				bool processOneMessage(serialmsg_t* msg);
				bool processOneMessageEx(serialmsg_t* msg);
				
				zctype_sernum_t* getSernum() {
					return m_sernum;
				}
				
				zctype_reporting_t* getReporting() {
					return m_reporting;
				}
				
				zctype_nwkconfig_t* getNwkconfig() {
					return m_nwkconfig;
				}
				
			  };
		};
	};
};
#endif