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
	LEN | SEQ | MSG
	MSG = MSGCODE | MSGSUBCODE | MSGDATA
	ERRORCODE  = <list of possible errors TBD>
	
	Incoming
	========
	ZCINIT      = <empty>
	ZCDEINIT    = <empty>
	ZCDEVREQ    = <empty>
	ZCDEVCFG    = NKWCAPS | DELIVERY
	ZCNWKREQ    = <empty>
	ZCNWKCFG    = CHANNEL | NWK ID | NODE ID | NWKKEYLEN | NWKKEY
	ZCREPREQ    = <empty>
	ZCREPCFG    = REPORTING NODE ID | REPFLAGS
	ZCSERIALREQ = <empty>
	ZCSERIALCFG = SERNUMLEN | SERNUM
	REPFLAGS   = <list of reporting flags TBD>
		b0: Report after add/remove to/from the network
		b1: Report values upon discrete change
		b2: Report values upon threshold change
		b3-b7: Reserved
	
	Outgoing
	========
	ZCDEVRES    = NKWCAPS | DELIVERY
	ZCNWKRES    = CHANNEL | NWK ID | NODE ID | NWKKEYLEN | NWKKEY
	ZCREPRES    = REPORTING NODE ID | REPFLAGS
	ZCSERIALRES = SERNUMLEN | SERNUM
	OK, NOK, ERROR_ILLEGAL_STATE, ERROR_KEY_TOO_LONG, ERROR_SERIAL_TOO_LONG, ERROR_INSUFFICIENT_DATA, ZC_SUBCODE_UNKNOWN
*/
namespace Meshwork {

	namespace L3 {
	
		namespace NetworkV1 {
		
			class ZeroConfSerial {

			public:
			
				class ZeroConfListener {
					public:
						//called after device configuration flags have been updated
						virtual void devconfig_updated() = 0;
						//called after the network configuration has been updated
						virtual void network_updated() = 0;
						//called after the serial number has been updated
						virtual void serial_updated() = 0;
						//called after the new reporting flags have been updated
						virtual void reporting_updated() = 0;
				};
				
				static const uint8_t MAX_SERIALMSG_LEN 			= 32;
				static const uint8_t MAX_SERIAL_LEN 			= 16;
				
				struct serialmsg_t {
					//these are actually uint8_t
					//but we need two bytes to handle
					//negative EOF and buffer len overrun codes
					uint16_t len;
					uint16_t code;
					uint16_t subcode;
					uint16_t seq;
				};

				struct zctype_sernum_t {
					uint8_t sernumlen;
					char sernum[MAX_SERIAL_LEN];
				};

				struct zctype_devconfig_t {
					//dev config flags
					uint8_t m_nwkcaps;
					uint8_t m_delivery;
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
					char nwkkey[Network::MAX_NETWORK_KEY_LEN];
				};

			public:
				
				//ZeroConf Serial Code ID
				static const uint8_t ZC_CODE 					= 1;

				//ZeroConf Serial SubCode IDs
				static const uint8_t ZC_SUBCODE_OK 				= 0;
				static const uint8_t ZC_SUBCODE_NOK 			= 1;
				static const uint8_t ZC_SUBCODE_UNKNOWN 		= 2;
				
				static const uint8_t ZC_SUBCODE_ZCINIT 			= 48;
				static const uint8_t ZC_SUBCODE_ZCDEINIT 		= 49;
				static const uint8_t ZC_SUBCODE_ZCDEVREQ        = 50;
				static const uint8_t ZC_SUBCODE_ZCDEVRES        = 51;
				static const uint8_t ZC_SUBCODE_ZCDEVCFG        = 52;
				static const uint8_t ZC_SUBCODE_ZCNWKREQ        = 53;
				static const uint8_t ZC_SUBCODE_ZCNWKRES        = 54;
				static const uint8_t ZC_SUBCODE_ZCNWKCFG        = 55;
				static const uint8_t ZC_SUBCODE_ZCREPREQ        = 56;
				static const uint8_t ZC_SUBCODE_ZCREPRES 		= 57;
				static const uint8_t ZC_SUBCODE_ZCREPCFG 		= 58;
				static const uint8_t ZC_SUBCODE_ZCSERIALREQ     = 59;
				static const uint8_t ZC_SUBCODE_ZCSERIALRES		= 60;
				static const uint8_t ZC_SUBCODE_ZCSERIALCFG		= 61;
				//TODO add messages for reading device vendor and model, used RF chip and frequency, extra metadata
				
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
				uint8_t m_lastSerialMsgLen;
				
				zctype_sernum_t* m_sernum;
				zctype_reporting_t* m_reporting;
				zctype_nwkconfig_t* m_nwkconfig;
				zctype_devconfig_t* m_devconfig;
				
				ZeroConfListener* m_listener;
				uint16_t m_timeout;
				bool m_initmode;
				
				serialmsg_t* m_currentMsg;
				
				uint16_t readByte();
				void readRemainingMessageBytes();

				//this saves us ~500 bytes against repetitive putchar calls
				void writeMessage(uint8_t len, uint8_t* data, bool flush);
				
				void respondWCode(serialmsg_t* msg, uint8_t code);
				void respondNOK(serialmsg_t* msg, uint8_t error);
				void respondSendACK(serialmsg_t* msg, uint8_t datalen, uint8_t* ackData);
				
				bool processZCInit(serialmsg_t* msg);
				bool processZCDeinit(serialmsg_t* msg);

				bool processZCDevReq(serialmsg_t* msg);
				bool processZCDevCfg(serialmsg_t* msg);

				bool processZCNwkReq(serialmsg_t* msg);
				bool processZCNwkCfg(serialmsg_t* msg);

				bool processZCSerialReq(serialmsg_t* msg);
				bool processZCSerialCfg(serialmsg_t* msg);

				bool processZCRepReq(serialmsg_t* msg);
				bool processZCRepCfg(serialmsg_t* msg);

			public:
				ZeroConfSerial(Meshwork::L3::Network* network, UART* serial,
								zctype_sernum_t* sernum, zctype_reporting_t* reporting,
									zctype_nwkconfig_t* nwkconfig, zctype_devconfig_t* devconfig,
										ZeroConfListener* listener, uint16_t timeout = TIMEOUT_RESPONSE):
					m_network(network),
					m_serial(serial),
					m_lastSerialMsgLen(0),
					m_sernum(sernum),
					m_reporting(reporting),
					m_nwkconfig(nwkconfig),
					m_devconfig(devconfig),
					m_listener(listener),
					m_timeout(timeout),
					m_initmode(false)
				{
				}
				
				bool waitForBytes(uint8_t count, uint16_t millis);
				
				bool processOneMessage(serialmsg_t* msg);
				bool processOneMessageEx(serialmsg_t* msg);
				
				bool checkZCInit(serialmsg_t* msg);

				zctype_sernum_t* getSernum() {
					return m_sernum;
				}
				
				zctype_reporting_t* getReporting() {
					return m_reporting;
				}
				
				zctype_nwkconfig_t* getNwkConfig() {
					return m_nwkconfig;
				}
				
				zctype_devconfig_t* getDevConfig() {
					return m_devconfig;
				}
			  };
		};
	};
};
#endif
