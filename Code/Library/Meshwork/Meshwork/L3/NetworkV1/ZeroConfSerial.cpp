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
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Power.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/ZeroConfSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

#ifndef LOG_ZEROCONFSERIAL
#define LOG_ZEROCONFSERIAL true
#endif

void Meshwork::L3::NetworkV1::ZeroConfSerial::writeMessage(uint8_t len, uint8_t* data, bool flush) {
	for ( int i = 0; i < len; i ++ )
		m_serial->putchar(((uint8_t*)data)[i]);
	if ( flush )
		m_serial->flush();
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::waitForBytes(uint8_t count, uint16_t millis) {
	bool result = false;
	uint32_t start = RTC::millis();
	while (true) {
		if ( m_serial->available() < count )//minimum response size
			Watchdog::delay(16);
		else
			result = true;
		if ( result || RTC::since(start) >= millis )
			break;
	}
	return result;
}

void Meshwork::L3::NetworkV1::ZeroConfSerial::respondWCode(serialmsg_t* msg, uint8_t code) {
	uint8_t data[] = {msg->seq, 1, code};
	writeMessage(sizeof(data), data, true);
}

void Meshwork::L3::NetworkV1::ZeroConfSerial::respondNOK(serialmsg_t* msg, uint8_t error) {
	uint8_t data[] = {msg->seq, 2, MSGCODE_NOK, error};
	writeMessage(sizeof(data), data, true);
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCInit(serialmsg_t* msg) {
	m_initmode = true;
	respondWCode(msg, MSGCODE_OK);
	return true;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCDenit(serialmsg_t* msg) {
	m_initmode = false;
	respondWCode(msg, MSGCODE_OK);
	return true;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCID(serialmsg_t* msg) {
	uint8_t data[] = {m_currentMsg->seq, 4, MSGCODE_ZCIDRES, m_network->getNetworkCaps(), m_network->getDelivery(), m_sernumlen};
	writeMessage(sizeof(data), data, false);
	writeMessage(m_sernumlen, m_sernum, true);
	return true;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCNwkID(serialmsg_t* msg) {
	uint16_t nwkid = m_network->getNetworkID();
	uint8_t data[] = {m_currentMsg->seq, 5, MSGCODE_ZCNWKIDRES, m_network->getChannelID(), nwkid << 8, nwkid && 0xFF, m_network->getNodeID()};
	writeMessage(sizeof(data), data, true);
	return true;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCCfgNwk(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 5 ) {
		data_zccfgnwk_t* zccfgnwk;
		cfgnwk = (data_cfgnwk_t*) msg->data;
		zccfgnwk->channel = m_serial->getchar();
		zccfgnwk->nwkid = (uint16_t) m_serial->getchar() << 8 | m_serial->getchar();
		zccfgnwk->nodeid = m_serial->getchar();
		m_network->setChannel(cfgnwk->channel);
		m_network->setNetworkID(cfgnwk->nwkid);
		m_network->setNodeID(cfgnwk->nodeid);
		MW_LOG_INFO(LOG_ZEROCONFSERIAL, "Configuring node: NodeID=%d, NwkID=%d, Channel=%d", cfgnwk->nodeid, cfgnwk->nwkid, cfgnwk->channel);
		size_t keyLen = m_serial->getchar();
		if ( keyLen >= 0 && keyLen <= Meshwork::L3::Network::MAX_NETWORK_KEY_LEN ) {
			m_networkKey[0] = 0;
			for (size_t i = 0; i < keyLen; i ++ )
				m_networkKey[i] = m_serial->getchar();
			if ( keyLen > 0 )
				m_networkKey[keyLen + 1] = 0;
			m_network->setNetworkKey((char*) m_networkKey);
			
			if ( m_listener != NULL )
				m_listener->network_updated();
			
			respondWCode(msg, MSGCODE_OK);
			result = true;
		} else {
			respondWCode(msg, ERROR_KEY_TOO_LONG);
			result = false;
		}
	} else {
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCCfgRep(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 2 ) {
		reporting_t reporting;
		reporting.targetnodeid = m_serial->getchar();
		reporting.repflags = m_serial->getchar();
		MW_LOG_INFO(LOG_ZEROCONFSERIAL, "Update reporting cfg: TargetNodeID=%d, RepFlags=%d", reporting.targetnodeid, reporting.repflags);
		
		if ( m_listener != NULL )
			m_listener->reporting_updated(&reporting);
		
		respondWCode(msg, MSGCODE_OK);
		result = true;
	} else {
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}


bool Meshwork::L3::NetworkV1::ZeroConfSerial::processOneMessageEx(serialmsg_t* msg) {
	return false;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processOneMessage(serialmsg_t* msg) {
	bool result = true;
	
	if ( m_serial->available() >= 3 ) {//minimal msg len
		msg->seq = m_serial->getchar();//seq
		int len = msg->len = m_serial->getchar();//len
		if ( len >= 0 ) {
			//needed to make sure we have enough data arrived in the buffer for the entire command
			int msgcode = msg->code = m_serial->getchar();//msgcode
			m_currentMsg = msg;
			MW_LOG_DEBUG_TRACE(LOG_ZEROCONFSERIAL) << endl << endl << endl;
			MW_LOG_INFO(LOG_ZEROCONFSERIAL, "PROCESSING SERIAL MESSAGE: Seq=%d, Len=%d, Code=%d", msg->seq, len, msgcode);
			if ( waitForBytes(len - 1, m_timeout) ) {
				switch ( msgcode ) {
					case MSGCODE_ZCINIT: processZCInit(msg); break;
					case MSGCODE_ZCDEINIT: processZCDeinit(msg); break;
					case MSGCODE_ZCID: processZCID(msg); break;
					case MSGCODE_ZCNWKID: processZCNwkID(msg); break;
					case MSGCODE_ZCCFGNWK: processZCCfgNwk(msg); break;
					case MSGCODE_ZCCFGREP: processZCCfgRep(msg); break;
					default:
						result = processOneMessageEx(msg);
						if (!result)
							respondWCode(msg, MSGCODE_UNKNOWN);
						m_currentMsg = NULL;
				}
				MW_LOG_DEBUG_TRACE(LOG_ZEROCONFSERIAL) << endl << endl;
			} else {
				MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "INSUFFICIENT_DATA", NULL);
				respondWCode(msg, ERROR_INSUFFICIENT_DATA);
				result = false;
			}
		} else {
			MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "MSGCODE_UNKNOWN: Seq=%d, Len=%d", msg->seq, len);
			respondWCode(msg, MSGCODE_UNKNOWN);
			result = false;
		}
	} else {
		result = false;
	}
	return result;
}