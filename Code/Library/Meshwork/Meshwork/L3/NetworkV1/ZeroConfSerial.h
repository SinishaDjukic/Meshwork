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
#include <Utils/SerialMessageAdapter.h>

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
		
			class ZeroConfSerial: public SerialMessageAdapter::SerialMessageListener {

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
				
				static const uint8_t MAX_SERIAL_LEN 			= 16;
				
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

        //32-255: Code specific
				static const uint8_t ZC_SUBCODE_ZCINIT 			= 41;
				static const uint8_t ZC_SUBCODE_ZCDEINIT 		= 42;
				static const uint8_t ZC_SUBCODE_ZCDEVREQ        = 43;
				static const uint8_t ZC_SUBCODE_ZCDEVRES        = 44;
				static const uint8_t ZC_SUBCODE_ZCDEVCFG        = 45;
				static const uint8_t ZC_SUBCODE_ZCNWKREQ        = 46;
				static const uint8_t ZC_SUBCODE_ZCNWKRES        = 47;
				static const uint8_t ZC_SUBCODE_ZCNWKCFG        = 48;
				static const uint8_t ZC_SUBCODE_ZCREPREQ        = 49;
				static const uint8_t ZC_SUBCODE_ZCREPRES 		= 50;
				static const uint8_t ZC_SUBCODE_ZCREPCFG 		= 51;
				static const uint8_t ZC_SUBCODE_ZCSERIALREQ     = 52;
				static const uint8_t ZC_SUBCODE_ZCSERIALRES		= 53;
				static const uint8_t ZC_SUBCODE_ZCSERIALCFG		= 54;
				//TODO add messages for reading device vendor and model, used RF chip and frequency, extra metadata
				
        //32-255: Code/sub-code specific
				static const uint8_t ZC_NOK_KEY_TOO_LONG 		= 32;
				static const uint8_t ZC_NOK_SERIAL_TOO_LONG 	= 33;
				
				//flags for configuring reporting
				static const uint8_t MASK_REPORT_NWK_ADD_REMOVE		= 1 << 0;
				static const uint8_t MASK_REPORT_DISCRETE_CHANGE	= 1 << 1;
				static const uint8_t MASK_REPORT_THRESHOLD_CHANGE	= 1 << 2;
				
			protected:
				Meshwork::L3::Network* m_network;
				SerialMessageAdapter* m_adapter;
				
				zctype_sernum_t* m_sernum;
				zctype_reporting_t* m_reporting;
				zctype_nwkconfig_t* m_nwkconfig;
				zctype_devconfig_t* m_devconfig;
				
				ZeroConfListener* m_listener;
				bool m_initmode;
				
				bool checkZCInit(SerialMessageAdapter::serialmsg_t* msg);

				uint8_t processZCInit(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processZCDeinit(SerialMessageAdapter::serialmsg_t* msg);

				uint8_t processZCDevReq(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processZCDevCfg(SerialMessageAdapter::serialmsg_t* msg);

				uint8_t processZCNwkReq(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processZCNwkCfg(SerialMessageAdapter::serialmsg_t* msg);

				uint8_t processZCSerialReq(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processZCSerialCfg(SerialMessageAdapter::serialmsg_t* msg);

				uint8_t processZCRepReq(SerialMessageAdapter::serialmsg_t* msg);
				uint8_t processZCRepCfg(SerialMessageAdapter::serialmsg_t* msg);

			public:
				ZeroConfSerial(Meshwork::L3::Network* network, SerialMessageAdapter* adapter,
								zctype_sernum_t* sernum, zctype_reporting_t* reporting,
									zctype_nwkconfig_t* nwkconfig, zctype_devconfig_t* devconfig,
										ZeroConfListener* listener):
					m_network(network),
					m_adapter(adapter),
					m_sernum(sernum),
					m_reporting(reporting),
					m_nwkconfig(nwkconfig),
					m_devconfig(devconfig),
					m_listener(listener),
					m_initmode(false)
				{
				}

    			uint8_t processOneMessage(SerialMessageAdapter::serialmsg_t* msg);

				SerialMessageAdapter* getSerialMessageAdapter() {
					return m_adapter;
				}

				void setSerialMessageAdapter(SerialMessageAdapter* adapter) {
					m_adapter = adapter;
				}
				
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
