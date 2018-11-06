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
#include "Cosa/RTT.hh"
#include "Cosa/Power.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Cosa/UART.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

using namespace Meshwork::L3::NetworkV1;

uint8_t ZeroConfSerial::processZCInit(SerialMessageAdapter::serialmsg_t* msg) {
	m_initmode = true;
	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "SERSEQ=%d", msg->seq);
	bool result = m_network == NULL ? false : true;//m_network->end();//temporarily commented out
	if ( result )
		m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
	else
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_ILLEGAL_STATE);
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}

uint8_t ZeroConfSerial::processZCDeinit(SerialMessageAdapter::serialmsg_t* msg) {
	m_initmode = false;
	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "SERSEQ=%d", msg->seq);
	bool result = m_network == NULL ? false : true;//m_network->begin();//temporarily commented out
	if ( result )
		m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
	else
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_ILLEGAL_STATE);
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}

bool ZeroConfSerial::checkZCInit(SerialMessageAdapter::serialmsg_t* msg) {
	if ( !m_initmode ) {
		MW_LOG_ERROR(MW_LOG_ZEROCONFSERIAL, "Must be in ZCInit mode!", NULL);
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_ILLEGAL_STATE);
	}
	return m_initmode;
}

uint8_t ZeroConfSerial::processZCDevReq(SerialMessageAdapter::serialmsg_t* msg) {
	if (!checkZCInit(msg) )
		return SerialMessageAdapter::SM_MESSAGE_ERROR;
	m_adapter->readRemainingMessageBytes();
	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "SERSEQ=%d, NwkCaps=%d, Delivery=%d", msg->seq, m_network->getNetworkCaps(), m_network->getDelivery());
	uint8_t data[] = {5, (uint8_t) msg->seq, ZC_CODE, ZC_SUBCODE_ZCDEVRES, m_network->getNetworkCaps(), m_network->getDelivery()};
	m_adapter->writeMessage(sizeof(data), data, true);
	return SerialMessageAdapter::SM_MESSAGE_PROCESSED;
}

uint8_t ZeroConfSerial::processZCDevCfg(SerialMessageAdapter::serialmsg_t* msg) {
	if (!checkZCInit(msg) )
		return SerialMessageAdapter::SM_MESSAGE_ERROR;
	m_devconfig->m_nwkcaps = m_adapter->readByte();
	m_devconfig->m_delivery = m_adapter->readByte();
	m_network->setNetworkCaps(m_devconfig->m_nwkcaps);
	m_network->setDelivery(m_devconfig->m_delivery);
	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "SERSEQ=%d, NwkCaps=%d, Delivery=%d", msg->seq, m_network->getNetworkCaps(), m_network->getDelivery());

	if ( m_listener != NULL )
		m_listener->devconfig_updated();
	m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
	return SerialMessageAdapter::SM_MESSAGE_PROCESSED;
}



uint8_t ZeroConfSerial::processZCNwkReq(SerialMessageAdapter::serialmsg_t* msg) {
	if (!checkZCInit(msg) )
		return SerialMessageAdapter::SM_MESSAGE_ERROR;
	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "SERSEQ=%d, Channel=%d, NwkID=%d, NodeID=%d, NwkKeyLen=%d",
			msg->seq, m_nwkconfig->channel, m_nwkconfig->nwkid, m_nwkconfig->nodeid, m_nwkconfig->nwkkeylen);
	uint8_t data[] = {(uint8_t) (8 + m_nwkconfig->nwkkeylen), (uint8_t) msg->seq, ZC_CODE, ZC_SUBCODE_ZCNWKRES, m_nwkconfig->channel,
			(uint8_t) (( m_nwkconfig->nwkid >> 8 ) & 0xFF), (uint8_t) (m_nwkconfig->nwkid & 0xFF), m_nwkconfig->nodeid, m_nwkconfig->nwkkeylen};
#ifndef ZEROCONF_NWKKEY_REPORT_ENABLE
	m_adapter->writeMessage(sizeof(data), data, false);
	m_adapter->writeMessage(m_nwkconfig->nwkkeylen, (uint8_t*) m_nwkconfig->nwkkey, true);
#else
	//forbid reporting the network key for security reasons
	data[8] = 0;
	m_adapter->writeMessage(sizeof(data), data, true);
#endif
	return SerialMessageAdapter::SM_MESSAGE_PROCESSED;
}

uint8_t ZeroConfSerial::processZCNwkCfg(SerialMessageAdapter::serialmsg_t* msg) {
	if (!checkZCInit(msg) )
		return SerialMessageAdapter::SM_MESSAGE_ERROR;
	bool result = false;
	m_nwkconfig->channel = m_adapter->readByte();
	m_nwkconfig->nwkid = (uint16_t) m_adapter->readByte() << 8 | m_adapter->readByte();
	m_nwkconfig->nodeid = m_adapter->readByte();

	m_network->setChannel(m_nwkconfig->channel);
	m_network->setNetworkID(m_nwkconfig->nwkid);
	m_network->setNodeID(m_nwkconfig->nodeid);

	m_nwkconfig->nwkkey[0] = 0;
	m_nwkconfig->nwkkeylen = m_adapter->readByte();

	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "Channel=%d, NwkID=%d, NodeID=%d, NwkKeyLen=%d",
			m_nwkconfig->channel, m_nwkconfig->nwkid, m_nwkconfig->nodeid, m_nwkconfig->nwkkeylen);

	if ( m_nwkconfig->nwkkeylen <= Meshwork::L3::Network::MAX_NETWORK_KEY_LEN ) {
		m_nwkconfig->nwkkey[0] = 0;
		for (size_t i = 0; i < m_nwkconfig->nwkkeylen; i ++ )
			m_nwkconfig->nwkkey[i] = m_adapter->readByte();
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
	if ( result )
		m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
	else
		m_adapter->respondNOK(msg, ZC_NOK_KEY_TOO_LONG);
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}



uint8_t ZeroConfSerial::processZCSerialReq(SerialMessageAdapter::serialmsg_t* msg) {
	if (!checkZCInit(msg) )
		return SerialMessageAdapter::SM_MESSAGE_ERROR;
	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "SERSEQ=%d, SerNumLen=%d", msg->seq, m_sernum->sernumlen);
	uint8_t data[] = {(uint8_t) (4 + m_sernum->sernumlen), (uint8_t) msg->seq, ZC_CODE, ZC_SUBCODE_ZCSERIALRES, m_sernum->sernumlen};
	m_adapter->writeMessage(sizeof(data), data, false);
	m_adapter->writeMessage(m_sernum->sernumlen, (uint8_t*) m_sernum->sernum, true);
	return SerialMessageAdapter::SM_MESSAGE_PROCESSED;
}

uint8_t ZeroConfSerial::processZCSerialCfg(SerialMessageAdapter::serialmsg_t* msg) {
	if (!checkZCInit(msg) )
		return false;
#if !(ZEROCONF_SERIAL_CHANGE_ENABLE)
	if ( m_sernum->sernumlen > 0 ) {
		MW_LOG_ERROR(MW_LOG_ZEROCONFSERIAL, "Serial number change disabled!", NULL);
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_ILLEGAL_STATE);
		return false;
	}
#endif

	bool result = false;
	m_sernum->sernumlen = m_adapter->readByte();
	m_sernum->sernum[0] = 0;

	if ( m_sernum->sernumlen <= MAX_SERIAL_LEN ) {
		for (size_t i = 0; i < m_sernum->sernumlen; i ++ )
			m_sernum->sernum[i] = m_adapter->readByte();
		if ( m_sernum->sernumlen > 0 )
			m_sernum->sernum[m_sernum->sernumlen + 1] = 0;//zero-terminate
		result = true;
	} else {
		m_sernum->sernumlen = 0;
	}
	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "SerNumLen=%d, SerNum=%s", m_sernum->sernumlen, m_sernum->sernum);
	if ( m_listener != NULL )
		m_listener->serial_updated();
	//the only possible fail point, so far
	if ( result )
		m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
	else
		m_adapter->respondNOK(msg, ZC_NOK_SERIAL_TOO_LONG);
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}



uint8_t ZeroConfSerial::processZCRepReq(SerialMessageAdapter::serialmsg_t* msg) {
	if (!checkZCInit(msg) )
		return SerialMessageAdapter::SM_MESSAGE_ERROR;
	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "SERSEQ=%d, TargetNodeID=%d, RepFlags=%d", msg->seq, m_reporting->targetnodeid, m_reporting->repflags);
	uint8_t data[] = {5, (uint8_t) msg->seq, ZC_CODE, ZC_SUBCODE_ZCREPRES, m_reporting->targetnodeid, m_reporting->repflags};
	m_adapter->writeMessage(sizeof(data), data, true);
	return SerialMessageAdapter::SM_MESSAGE_PROCESSED;
}

uint8_t ZeroConfSerial::processZCRepCfg(SerialMessageAdapter::serialmsg_t* msg) {
	if (!checkZCInit(msg) )
		return SerialMessageAdapter::SM_MESSAGE_ERROR;
	m_reporting->targetnodeid = m_adapter->readByte();
	m_reporting->repflags = m_adapter->readByte();
	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "TargetNodeID=%d, RepFlags=%d", m_reporting->targetnodeid, m_reporting->repflags);

	if ( m_listener != NULL )
		m_listener->reporting_updated();

	m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);

	return SerialMessageAdapter::SM_MESSAGE_PROCESSED;
}

uint8_t ZeroConfSerial::processZCFactoryReset(SerialMessageAdapter::serialmsg_t* msg) {
	if (!checkZCInit(msg) )
		return SerialMessageAdapter::SM_MESSAGE_ERROR;

	if ( m_listener != NULL ) {
		MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "Factory reset", NULL);

		m_listener->factory_reset();
	} else {
		MW_LOG_WARNING(MW_LOG_ZEROCONFSERIAL, "No listener for Factory reset", NULL);
	}
	m_adapter->respondWCode(msg, m_listener != NULL ? SerialMessageAdapter::SM_SUBCODE_OK : SerialMessageAdapter::SM_SUBCODE_NOK);

	return SerialMessageAdapter::SM_MESSAGE_PROCESSED;
}

uint8_t ZeroConfSerial::processOneMessage(SerialMessageAdapter::serialmsg_t* msg) {
	uint8_t result = SerialMessageAdapter::SM_MESSAGE_UNKNOWN;

	MW_LOG_INFO(MW_LOG_ZEROCONFSERIAL, "[ZeroConfSerial] Code=%d, SubCode=%d", msg->code, msg->subcode);

	if ( msg->code == ZC_CODE ) {
		switch ( msg->subcode ) {
			case ZeroConfSerial::ZC_SUBCODE_ZCINIT: result = processZCInit(msg); break;
			case ZeroConfSerial::ZC_SUBCODE_ZCDEINIT: result = processZCDeinit(msg); break;

			case ZeroConfSerial::ZC_SUBCODE_ZCDEVREQ: result = processZCDevReq(msg); break;
			case ZeroConfSerial::ZC_SUBCODE_ZCDEVCFG: result = processZCDevCfg(msg); break;

			case ZeroConfSerial::ZC_SUBCODE_ZCNWKREQ: result = processZCNwkReq(msg); break;
			case ZeroConfSerial::ZC_SUBCODE_ZCNWKCFG: result = processZCNwkCfg(msg); break;

			case ZeroConfSerial::ZC_SUBCODE_ZCREPREQ: result = processZCRepReq(msg); break;
			case ZeroConfSerial::ZC_SUBCODE_ZCREPCFG: result = processZCRepCfg(msg); break;

			case ZeroConfSerial::ZC_SUBCODE_ZCSERIALREQ: result = processZCSerialReq(msg); break;
			case ZeroConfSerial::ZC_SUBCODE_ZCSERIALCFG: result = processZCSerialCfg(msg); break;

			case ZeroConfSerial::ZC_SUBCODE_ZCSFACTORYRESET: result = processZCFactoryReset(msg); break;
		}
	}
	return result;
}

bool ZeroConfSerial::processConfigSequence(uint16_t initTimeout, uint16_t deinitTimeout, uint16_t nextMsgTimeout) {
	UNUSED(nextMsgTimeout);
	uint32_t start = RTT::millis();
	uint8_t state = 0;
	uint32_t lastMessage = start;
	uint8_t lastProcessCode;
	bool connected = false;
	SerialMessageAdapter::serialmsg_t msg;
	while ( ( !connected && (RTT::since(start)       < initTimeout  ) ) ||
			(  connected && (RTT::since(lastMessage) < deinitTimeout) ) ) {
		//the state flow must be 0 -> ZC_SUBCODE_ZCINIT -> ZC_SUBCODE_ZCDEINIT
		lastProcessCode = m_adapter->processOneMessage(&msg);
		if ( lastProcessCode != SerialMessageAdapter::SM_MESSAGE_NONE) {
			connected = true;
			if ( msg.subcode == ZeroConfSerial::ZC_SUBCODE_ZCINIT ) {
				state = ZeroConfSerial::ZC_SUBCODE_ZCINIT;
			} else if ( msg.subcode == ZeroConfSerial::ZC_SUBCODE_ZCDEINIT ) {
				state = ZeroConfSerial::ZC_SUBCODE_ZCDEINIT;
				break;
			}
			lastMessage = RTT::millis();
		}
	}
	return state == ZeroConfSerial::ZC_SUBCODE_ZCDEINIT;
}

#endif
