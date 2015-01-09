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
#ifndef __MESHWORK_L3_NETWORKV1_ZEROCONFSERIAL_CPP__
#define __MESHWORK_L3_NETWORKV1_ZEROCONFSERIAL_CPP__

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

int Meshwork::L3::NetworkV1::ZeroConfSerial::readByte() {
	if ( m_lastSerialMsgLen > 0 ) {
		m_lastSerialMsgLen --;
		return m_serial->getchar();
	} else {
		return -2;
	}
}

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
			Meshwork::Time::delay(16);
		else
			result = true;
		if ( result || Meshwork::Time::passed(RTC::since(start), millis) )
			break;
	}
	return result;
}

void Meshwork::L3::NetworkV1::ZeroConfSerial::readRemainingMessageBytes() {
	if ( m_lastSerialMsgLen > 0 ) {
		MW_LOG_WARNING(LOG_ZEROCONFSERIAL, "Remaining message bytes to be discarded=%d", m_lastSerialMsgLen);
		if ( waitForBytes(m_lastSerialMsgLen, m_timeout) ) {
			while (readByte() >= 0);
		} else {
			MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Timeout waiting for bytes count to be discarded: %d", m_lastSerialMsgLen);
		}
	} else {
		MW_LOG_DEBUG(LOG_ZEROCONFSERIAL, "No message bytes to be discarded", NULL);
	}
}

void Meshwork::L3::NetworkV1::ZeroConfSerial::respondWCode(serialmsg_t* msg, uint8_t code) {
	readRemainingMessageBytes();
	MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SERSEQ=%d, Code=%d", msg->seq, code);
	uint8_t data[] = {3, msg->seq, ZW_CODE, code};
	writeMessage(sizeof(data), data, true);
}

void Meshwork::L3::NetworkV1::ZeroConfSerial::respondNOK(serialmsg_t* msg, uint8_t error) {
	readRemainingMessageBytes();
	MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SERSEQ=%d, Error=%d", msg->seq, error);
	uint8_t data[] = {4, msg->seq, ZW_CODE, ZC_SUBCODE_NOK, error};
	writeMessage(sizeof(data), data, true);
}



bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCInit(serialmsg_t* msg) {
	m_initmode = true;
	MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SERSEQ=%d", msg->seq);
	bool result = m_network == NULL ? false : m_network->end();
	respondWCode(msg, result ? ZC_SUBCODE_OK : ZC_SUBCODE_NOK);
	return result;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCDeinit(serialmsg_t* msg) {
	m_initmode = false;
	MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SERSEQ=%d", msg->seq);
	bool result = m_network == NULL ? false : m_network->begin();
	respondWCode(msg, result ? ZC_SUBCODE_OK : ZC_SUBCODE_NOK);
	return result;
}



bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCDevReq(serialmsg_t* msg) {
	readRemainingMessageBytes();
	MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SERSEQ=%d, NwkCaps=%d, Delivery=%d", msg->seq, m_network->getNetworkCaps(), m_network->getDelivery());
	uint8_t data[] = {5, m_currentMsg->seq, ZW_CODE, ZC_SUBCODE_ZCDEVRES, m_network->getNetworkCaps(), m_network->getDelivery()};
	writeMessage(sizeof(data), data, true);
	return true;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCDevCfg(serialmsg_t* msg) {
	if ( !m_initmode ) {
		MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Must be in ZCInit mode!", NULL);
		respondNOK(msg, ERROR_ILLEGAL_STATE);
		return false;
	}
	bool result = false;
	if ( m_serial->available() >= 2 ) {
		m_network->setNetworkCaps(m_serial->getchar());
		m_network->setDelivery(m_serial->getchar());
		MW_LOG_INFO(LOG_ZEROCONFSERIAL, "NwkCaps=%d, Delivery=%d", msg->seq, m_network->getNetworkCaps(), m_network->getDelivery());

		if ( m_listener != NULL )
			m_listener->devconfig_updated();
		result = true;
		respondWCode(msg, ZC_SUBCODE_OK);
	} else {
		MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Not enough data in ZCDevCfg, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return true;
}



bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCNwkReq(serialmsg_t* msg) {
	MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SERSEQ=%d, Channel=%d, NwkID=%d, NodeID=%d", msg->seq, m_nwkconfig->channel, m_nwkconfig->nwkid, m_nwkconfig->nodeid);
	uint8_t data[] = {8, m_currentMsg->seq, ZW_CODE, ZC_SUBCODE_ZCNWKRES, m_nwkconfig->channel, ( m_nwkconfig->nwkid >> 8 ) && 0xFF, m_nwkconfig->nwkid && 0xFF, m_nwkconfig->nodeid, m_nwkconfig->nwkkeylen};
#ifndef ZEROCONF_NWKKEY_REPORT_ENABLE
	writeMessage(sizeof(data), data, false);
	writeMessage(m_nwkconfig->nwkkeylen, (uint8_t*) m_nwkconfig->nwkkey, true);
#else
	//forbid reporting the network key for security reasons
	data[7] = 0;
	writeMessage(sizeof(data), data, true);
#endif
	return true;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCNwkCfg(serialmsg_t* msg) {
	if ( !m_initmode ) {
		MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Must be in ZCInit mode!", NULL);
		respondNOK(msg, ERROR_ILLEGAL_STATE);
		return false;
	}
	bool result = false;
	if ( m_serial->available() >= 5 ) {
		m_nwkconfig->channel = m_serial->getchar();
		m_nwkconfig->nwkid = (uint16_t) m_serial->getchar() << 8 | m_serial->getchar();
		m_nwkconfig->nodeid = m_serial->getchar();
		
		m_network->setChannel(m_nwkconfig->channel);
		m_network->setNetworkID(m_nwkconfig->nwkid);
		m_network->setNodeID(m_nwkconfig->nodeid);
		
		MW_LOG_INFO(LOG_ZEROCONFSERIAL, "Channel=%d, NwkID=%d, NodeID=%d", m_nwkconfig->channel, m_nwkconfig->nwkid, m_nwkconfig->nodeid);
		
		m_nwkconfig->nwkkey[0] = 0;
		m_nwkconfig->nwkkeylen = m_serial->getchar();
		if ( m_nwkconfig->nwkkeylen <= Meshwork::L3::Network::MAX_NETWORK_KEY_LEN ) {
			m_nwkconfig->nwkkey[0] = 0;
			for (size_t i = 0; i < m_nwkconfig->nwkkeylen; i ++ )
				m_nwkconfig->nwkkey[i] = m_serial->getchar();
			if ( m_nwkconfig->nwkkeylen > 0 )
				m_nwkconfig->nwkkey[m_nwkconfig->nwkkeylen + 1] = 0;//zero-terminate
			result = true;
		} else {
			m_nwkconfig->nwkkeylen = 0;
		}
		m_network->setNetworkKey((char*) m_nwkconfig->nwkkey);
		if ( m_listener != NULL )
			m_listener->network_updated();
		//the only possible fail point, so far
		respondWCode(msg, result ? ZC_SUBCODE_OK : ERROR_KEY_TOO_LONG);
	} else {
		MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Not enough data in ZCCfgNwk, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}



bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCSerialReq(serialmsg_t* msg) {
	MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SERSEQ=%d, SerNumLen=%d", msg->seq, m_sernum->sernumlen);
	uint8_t data[] = {4, m_currentMsg->seq, ZW_CODE, ZC_SUBCODE_ZCSERIALRES, m_sernum->sernumlen};
	writeMessage(sizeof(data), data, false);
	writeMessage(m_sernum->sernumlen, (uint8_t*) m_sernum->sernum, true);
	return true;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCSerialCfg(serialmsg_t* msg) {
	if ( !m_initmode ) {
		MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Must be in ZCInit mode!", NULL);
		respondNOK(msg, ERROR_ILLEGAL_STATE);
		return false;
	}
#ifndef ZEROCONF_SERIAL_CHANGE_ENABLE
	if ( m_sernum->sernumlen > 0 ) {
		MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Serial number change disabled!", NULL);
		respondNOK(msg, ERROR_ILLEGAL_STATE);
		return false;
	}
#endif

	bool result = false;
	if ( m_serial->available() >= 1 ) {
		m_sernum->sernumlen = m_serial->getchar();
		m_sernum->sernum[0] = 0;
		
		if ( m_sernum->sernumlen <= MAX_SERIAL_LEN ) {
			for (size_t i = 0; i < m_sernum->sernumlen; i ++ )
				m_sernum->sernum[i] = m_serial->getchar();
			if ( m_sernum->sernumlen > 0 )
				m_sernum->sernum[m_sernum->sernumlen + 1] = 0;//zero-terminate
			result = true;
		} else {
			m_sernum->sernumlen = 0;
		}
		MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SerNumLen=%d, SerNum=%s", m_sernum->sernumlen, m_sernum->sernum);
		if ( m_listener != NULL )
			m_listener->serial_updated();
		//the only possible fail point, so far
		respondWCode(msg, result ? ZC_SUBCODE_OK : ERROR_SERIAL_TOO_LONG);
	} else {
		MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Not enough data in ZCCfgSerial, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}



bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCRepReq(serialmsg_t* msg) {
	MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SERSEQ=%d, TargetNodeID=%d, RepFlags=%d", m_reporting->targetnodeid, m_reporting->repflags);
	uint8_t data[] = {5, m_currentMsg->seq, ZW_CODE, ZC_SUBCODE_ZCREPRES, m_reporting->targetnodeid, m_reporting->repflags};
	writeMessage(sizeof(data), data, true);
	return true;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processZCRepCfg(serialmsg_t* msg) {
	if ( !m_initmode ) {
		MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Must be in ZCInit mode!", NULL);
		respondNOK(msg, ERROR_ILLEGAL_STATE);
		return false;
	}
	bool result = false;
	if ( m_serial->available() >= 2 ) {
		m_reporting->targetnodeid = m_serial->getchar();
		m_reporting->repflags = m_serial->getchar();
		MW_LOG_INFO(LOG_ZEROCONFSERIAL, "TargetNodeID=%d, RepFlags=%d", m_reporting->targetnodeid, m_reporting->repflags);
		
		if ( m_listener != NULL )
			m_listener->reporting_updated();
		
		respondWCode(msg, ZC_SUBCODE_OK);
		result = true;
	} else {
		MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Not enough data in ZCCfgRep, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processOneMessageEx(serialmsg_t* msg) {
	UNUSED(msg);
	return false;
}

bool Meshwork::L3::NetworkV1::ZeroConfSerial::processOneMessage(serialmsg_t* msg) {
	bool result = true;
	
	if ( m_serial->available() >= 3 ) {//minimal msg len
		int len = msg->len = readByte();//len
		m_lastSerialMsgLen = len;
		if ( len > 0 ) {
			msg->seq = readByte();//seq
			readByte();//read major code
			//needed to make sure we have enough data arrived in the buffer for the entire command
			int msgcode = msg->code = readByte();//read sub-code
			m_currentMsg = msg;
			MW_LOG_DEBUG_TRACE(LOG_ZEROCONFSERIAL) << endl << endl << endl;
			MW_LOG_INFO(LOG_ZEROCONFSERIAL, "SERSEQ=%d, Len=%d, Code=%d", msg->seq, len, msgcode);
			if ( waitForBytes(len - 1, m_timeout) ) {
				switch ( msgcode ) {
					case ZC_SUBCODE_ZCINIT: processZCInit(msg); break;
					case ZC_SUBCODE_ZCDEINIT: processZCDeinit(msg); break;

					case ZC_SUBCODE_ZCDEVREQ: processZCDevReq(msg); break;
					case ZC_SUBCODE_ZCDEVCFG: processZCDevCfg(msg); break;

					case ZC_SUBCODE_ZCNWKREQ: processZCNwkReq(msg); break;
					case ZC_SUBCODE_ZCNWKCFG: processZCNwkCfg(msg); break;

					case ZC_SUBCODE_ZCREPREQ: processZCRepReq(msg); break;
					case ZC_SUBCODE_ZCREPCFG: processZCRepCfg(msg); break;

					case ZC_SUBCODE_ZCSERIALREQ: processZCSerialReq(msg); break;
					case ZC_SUBCODE_ZCSERIALCFG: processZCSerialCfg(msg); break;

					default:
						result = processOneMessageEx(msg);
						if (!result)
							respondWCode(msg, ZC_SUBCODE_UNKNOWN);
						m_currentMsg = NULL;
				}
				MW_LOG_DEBUG_TRACE(LOG_ZEROCONFSERIAL) << endl << endl;
			} else {
				MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "INSUFFICIENT_DATA", NULL);
				respondWCode(msg, ERROR_INSUFFICIENT_DATA);
				result = false;
			}
		} else {
			MW_LOG_ERROR(LOG_ZEROCONFSERIAL, "Invalid message: Len=%d", len);
			respondWCode(msg, ERROR_GENERAL);
			result = false;
		}
	} else {
		result = false;
	}
	return result;
}
#endif
