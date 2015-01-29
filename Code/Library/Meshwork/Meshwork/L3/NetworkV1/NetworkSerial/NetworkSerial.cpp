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
#ifndef __MESHWORK_L3_NETWORKV1_NETWORKSERIAL_CPP__
#define __MESHWORK_L3_NETWORKV1_NETWORKSERIAL_CPP__

#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Power.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkSerial/NetworkSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::initSerial() {
	uint8_t data[] = {2, 1, NS_SUBCODE_CFGREQUEST};
	m_adapter->writeMessage(sizeof(data), data, true);
	return SerialMessageAdapter::SM_MESSAGE_PROCESSED;
}

void Meshwork::L3::NetworkV1::NetworkSerial::set_address(uint8_t src) {
	UNUSED(src);
	//no action needed
}

void Meshwork::L3::NetworkV1::NetworkSerial::route_found(Meshwork::L3::NetworkV1::NetworkV1::route_t* route) {
	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "SERSEQ=%d, Src=%d, Dst=%d, hops=%d", m_currentMsg->seq, route->src, route->dst, route->hopCount);
	uint8_t hopCount = route->hopCount;	
	uint8_t data[] = {(uint8_t) (6 + hopCount), (uint8_t) m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFROUTEFOUND, hopCount, route->src};
	m_adapter->writeMessage(sizeof(data), data, false);
	m_adapter->writeMessage(hopCount, route->hops, false);
	data[0] = route->dst;
	m_adapter->writeMessage(1, data, true);
}

void Meshwork::L3::NetworkV1::NetworkSerial::route_failed(Meshwork::L3::NetworkV1::NetworkV1::route_t* route) {
	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "SERSEQ=%d, Src=%d, Dst=%d, hops=%d", m_currentMsg->seq, route->src, route->dst, route->hopCount);
	uint8_t hopCount = route->hopCount;
	uint8_t data[] = {(uint8_t) (6 + hopCount), (uint8_t) m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFROUTEFAILED, hopCount, route->src};
	m_adapter->writeMessage(sizeof(data), data, false);
	m_adapter->writeMessage(hopCount, route->hops, false);
	data[0] = route->dst;
	m_adapter->writeMessage(1, data, true);
}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::get_routeCount(uint8_t dst) {
	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "SERSEQ=%d, Dst=%d", m_currentMsg->seq, dst);

	uint8_t result = 0;
	uint8_t data[] = {4, (uint8_t) m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFGETROUTECOUNT, dst};
	m_adapter->writeMessage(sizeof(data), data, true);
	
	if ( m_adapter->waitForBytes(6, SerialMessageAdapter::TIMEOUT_RESPONSE) ) {
		uint8_t seq = m_currentMsg->seq;
		uint8_t processCode = m_adapter->processOneMessageHeader(m_currentMsg);
		if ( processCode == SerialMessageAdapter::SM_MESSAGE_PROCESSED ) {
			if ( m_currentMsg->seq == seq ) {
				if ( m_currentMsg->code == SerialMessageAdapter::SM_SUBCODE_NOK ) {
					//read the error code and return NULL... shouldn't happen though
					m_adapter->readRemainingMessageBytes();
				} else if ( m_currentMsg->code == NS_SUBCODE_RFGETROUTECOUNTRES ) {
					result = m_adapter->readByte();
					m_adapter->readRemainingMessageBytes();
					MW_LOG_DEBUG(MW_LOG_NETWORKSERIAL, "Route count: %d", result);
				} else {
					MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Wrong response code received to RFGETROUTECOUNT: %d", m_currentMsg->code);
					m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_GENERAL);
				}
			} else {
				MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Sequence mismatch! Expected SERSEQ=%d, Received SERSEQ=%d", m_currentMsg->seq, seq);
				m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_SEQUENCE_MISMATCH);
			}
		} else {
			MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Invalid response to NS_SUBCODE_RFGETROUTECOUNT, Process Code=%d", processCode);
			m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_SEQUENCE_MISMATCH);
		}
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "No response to NS_SUBCODE_RFGETROUTECOUNT", NULL);
		m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA);
	}
	return result;
}

Meshwork::L3::NetworkV1::NetworkV1::route_t* Meshwork::L3::NetworkV1::NetworkSerial::get_route(uint8_t dst, uint8_t index) {
	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "SERSEQ=%d, Dst=%d, index=%d", m_currentMsg->seq, dst, index);
	Meshwork::L3::NetworkV1::NetworkV1::route_t* result = NULL;
	
	uint8_t data[] = {5, (uint8_t) m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFGETROUTE, dst, index};
	m_adapter->writeMessage(sizeof(data), data, true);
	
	if ( m_adapter->waitForBytes(5, SerialMessageAdapter::TIMEOUT_RESPONSE) ) {
		m_lastSerialMsgLen = m_adapter->readByte();
			uint8_t seq = m_adapter->readByte();
			if ( m_currentMsg->seq == seq ) {
				m_adapter->readByte();//read major code
				uint8_t code = m_adapter->readByte();//read sub-code
				if ( code == SerialMessageAdapter::SM_SUBCODE_NOK ) {
					//read the error code and return NULL... shouldn't happen though
					m_adapter->readRemainingMessageBytes();
					m_currentRoute.hopCount = 0;
				} else if ( code == NS_SUBCODE_RFGETROUTERES ) {
					uint8_t hopCount = 0;
					//some magic goes here...
					//first wait ensure we have at least 3 bytes (HOPCOUNT = 0 | SRC | DST), so that we can read the hopCount value
					//second wait ensures we have at least 3 + hopCount bytes (HOPCOUNT | SRC | <list> | DST)
					if ( m_adapter->waitForBytes(3, SerialMessageAdapter::TIMEOUT_RESPONSE) &&
						 m_adapter->waitForBytes(3 + (hopCount = m_adapter->readByte()) > 0 ? hopCount : 0, SerialMessageAdapter::TIMEOUT_RESPONSE)) {
						m_currentRoute.hopCount = hopCount;
						m_currentRoute.src = m_adapter->readByte();
						for ( int i = 0; i < hopCount; i ++ )
							m_currentRouteHops[i] = m_adapter->readByte();
						m_currentRoute.hops = m_currentRouteHops;
						m_currentRoute.dst = m_adapter->readByte();
						result = &m_currentRoute;
						m_adapter->readRemainingMessageBytes();
					} else {
						MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Not enough hops in NS_SUBCODE_RFGETROUTE, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA", NULL);
						m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA);
					}	
				} else {
					MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Wrong response code received to RFGETROUTE: %d", code);
					m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_GENERAL);
				}
			} else {
				MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Sequence mismatch! Expected SERSEQ=%d, Received SEQSEQ=%d", m_currentMsg->seq, seq);
				m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_SEQUENCE_MISMATCH);
			}
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "No response to NS_SUBCODE_RFGETROUTE, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA", NULL);
		m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA);
	}	
	return result;
}

//void Meshwork::L3::NetworkV1::NetworkSerial::respondSendACK(serialmsg_t* msg, uint8_t datalen, uint8_t* ackData) {
//	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "SERSEQ=%d, DataLen=%d", msg->seq, datalen);
//	uint8_t data[] = {4 + datalen, msg->seq, NS_CODE, NS_SUBCODE_RFSENDACK, datalen};
//	m_adapter->writeMessage(sizeof(data), data, false);
//	m_adapter->writeMessage(datalen, data, true);
//}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::processCfgBasic(SerialMessageAdapter::serialmsg_t* msg) {
	bool result = false;
	if ( m_adapter->waitForBytes(3, SerialMessageAdapter::TIMEOUT_RESPONSE) ) {
		uint16_t nwkcaps = m_adapter->readByte();
		uint16_t delivery = m_adapter->readByte();
		uint16_t retry = m_adapter->readByte();
		
		m_network->setNetworkCaps(nwkcaps);
		m_network->setDelivery(delivery);
		m_network->setRetry(retry);
		
		MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "NwkCaps=%d, Delivery=%d, Retry=%d", nwkcaps, delivery, retry);
		
		m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
		result = true;
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Not enough data in Config Basic, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA", NULL);
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA);
	}
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::processCfgNwk(SerialMessageAdapter::serialmsg_t* msg) {
	bool result = false;
	if ( m_adapter->waitForBytes(3, SerialMessageAdapter::TIMEOUT_RESPONSE) ) {
		uint16_t channel = m_adapter->readByte();
		uint16_t nwkid = (uint16_t) m_adapter->readByte() << 8 | m_adapter->readByte();
		uint16_t nodeid = m_adapter->readByte();

		m_network->setChannel(channel);
		m_network->setNetworkID(nwkid);
		m_network->setNodeID(nodeid);

		MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "NodeID=%d, NwkID=%d, Channel=%d", nodeid, nwkid, channel);

		int16_t keyLen = m_adapter->readByte();
		if ( keyLen >= 0 && keyLen <= Meshwork::L3::Network::MAX_NETWORK_KEY_LEN ) {
			m_networkKey[0] = 0;
			for (int16_t i = 0; i < keyLen; i ++ )
				m_networkKey[i] = m_adapter->readByte();
			if ( keyLen > 0 )
				m_networkKey[keyLen + 1] = 0;
			m_network->setNetworkKey((char*) m_networkKey);
			m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
			result = true;
		} else {
			m_adapter->respondNOK(msg, NetworkSerial::NS_NOK_KEY_TOO_LONG);
			result = false;
		}
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Not enough data in Config Network, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA", NULL);
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA);
	}
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::processRFInit(SerialMessageAdapter::serialmsg_t* msg) {
	bool result = m_network->begin();
	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "", NULL);
	if ( result )
		m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
	else
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_ILLEGAL_STATE);
	result ? m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK) : m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_GENERAL);
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::processRFDeinit(SerialMessageAdapter::serialmsg_t* msg) {
	bool result = m_network->end();
	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "", NULL);
	if ( result )
		m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
	else
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_ILLEGAL_STATE);
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}

int Meshwork::L3::NetworkV1::NetworkSerial::returnACKPayload(uint8_t src, uint8_t port,
													void* buf, uint8_t len,
														void* bufACK, size_t lenACK) {
	int bytes = 0;
	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "SERSEQ=%d, Src=%d, Port=%d, SerLen=%d", m_currentMsg->seq, src, port, len);
	if ( m_currentMsg != NULL ) {//must be the case, but need a sanity check
		MW_LOG_DEBUG(MW_LOG_NETWORKSERIAL, "Sending RFRECV", NULL);
		//first, send RFRECV
		uint8_t data[] = {(uint8_t) (6 + len), (uint8_t) m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFRECV, src, port, len};
		m_adapter->writeMessage(sizeof(data), data, false);
		m_adapter->writeMessage(len, (uint8_t*)buf, true);
		
		if ( m_adapter->waitForBytes(6, SerialMessageAdapter::TIMEOUT_RESPONSE) )  {
			m_lastSerialMsgLen = m_adapter->readByte();
			uint8_t seq = m_adapter->readByte();
			MW_LOG_DEBUG(MW_LOG_NETWORKSERIAL, "Response to RFRECV, SERSEQ=%d", seq);
			if ( m_currentMsg->seq == seq ) {
				m_adapter->readByte();//read major code
				uint8_t code = m_adapter->readByte();//read sub-code

				if ( code == NS_SUBCODE_RFRECVACK ) {
					int16_t reslen = m_adapter->readByte();
					MW_LOG_DEBUG(MW_LOG_NETWORKSERIAL, "Response to RFRECV, reslen=%d", reslen);
					if ( reslen >=0 && ((size_t) reslen <= lenACK) && m_adapter->waitForBytes(reslen, SerialMessageAdapter::TIMEOUT_RESPONSE) ) {
						for ( int16_t i = 0; i < reslen; i ++ )
							((char*) bufACK)[i] = m_adapter->readByte();
						//The other side does not expect a reply here, so response must not be sent
						//m_adapter->respondWCode(m_currentMsg, SerialMessageAdapter::SM_SUBCODE_OK);
						bytes = reslen;
					} else {
						MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Response to RFRECV, ERROR_TOO_LONG_DATA", NULL);
						//The other side does not expect a reply here, so response must not be sent
						//m_adapter->respondNOK(m_currentMsg, ERROR_TOO_LONG_DATA);
					}
				} else {
					MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Wrong response code received to RFRECV: %d", code);
					//The other side does not expect a reply here, so response must not be sent
					//m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_GENERAL);
				}
			} else {
				MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Response to RFRECV, serial seq WRONG", NULL);
				//The other side does not expect a reply here, so response must not be sent
				//m_adapter->respondNOK(m_currentMsg, ERROR_SEQUENCE_MISMATCH);
			}
			//Since the other side does not expect any replies in the above sequnces
			//we read out the message fully at the end here
			m_adapter->readRemainingMessageBytes();
		} else {
			MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Response to RFRECV, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA", NULL);
			m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA);
		}
	} else {
		m_adapter->respondNOK(m_currentMsg, SerialMessageAdapter::SM_NOK_ILLEGAL_STATE);
	}
	return bytes;
}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::processRFStartRecv(SerialMessageAdapter::serialmsg_t* msg) {
	bool result = false;
	if ( m_adapter->waitForBytes(4, SerialMessageAdapter::TIMEOUT_RESPONSE) ) {
		uint32_t timeout = (uint32_t) m_adapter->readByte() << 24 |
						   (uint32_t) m_adapter->readByte() << 16 |
						   (uint32_t) m_adapter->readByte() << 8 |
						   (uint32_t) m_adapter->readByte();
		MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "Receiving with timeout=%l", timeout);

		//not needed here:
		//m_currentMsg = msg;

		int res = m_network->recv(m_lastMsgSrc, m_lastMsgPort, &m_lastMsgData, m_lastMsgLen, timeout, this);

		MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "Receive done. SerResult=%d, Src=%d, Port=%d", res, m_lastMsgSrc, m_lastMsgPort);

		//not needed here:
		//m_currentMsg = NULL;
		if ( res == Meshwork::L3::Network::OK ) {
			m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
			result = true;
		} else if ( res == Meshwork::L3::Network::OK_MESSAGE_INTERNAL ) {
			m_adapter->respondWCode(msg, NS_SUBCODE_INTERNAL);
			result = true;
		} else if ( res == Meshwork::L3::Network::OK_MESSAGE_IGNORED ) {
			m_adapter->respondWCode(msg, NS_SUBCODE_INTERNAL);
			result = true;
		} else {
			m_adapter->respondNOK(msg, res);
		}
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Not enough data in Start Recv", NULL);
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA);
	}
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::processRFSend(SerialMessageAdapter::serialmsg_t* msg) {
	bool result = false;
	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "Entered", NULL);
	uint16_t dst = m_adapter->readByte();
	uint16_t port = m_adapter->readByte();
	uint16_t datalen = m_adapter->readByte();
	uint8_t indata[datalen];

	if ( m_adapter->waitForBytes(datalen, SerialMessageAdapter::TIMEOUT_RESPONSE) ) {//minimal msg len
		MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "SERSEQ=%d, Dst=%d, Port=%d, DataLen=%d", msg->seq, dst, port, datalen);

		MW_LOG_DEBUG(MW_LOG_NETWORKSERIAL, "Reading data to send...", NULL);
		for ( uint8_t i = 0; i < datalen; i ++ )//ok, this can be optimized
			indata[i] = (uint8_t) m_adapter->readByte();

		//make sure we read out the rest here, since we will have nested messages during the send
		m_adapter->readRemainingMessageBytes();

		size_t maxACKLen = NetworkV1::ACK_PAYLOAD_MAX;

		MW_LOG_DEBUG_ARRAY(MW_LOG_NETWORKSERIAL, PSTR("Sending data: "), indata, datalen);

		int res = m_network->send(dst, port, indata, datalen, m_lastAckData, maxACKLen);

		if ( res == Meshwork::L3::Network::OK ) {
			MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "Send done. SERSEQ=%d, Result=%d, ACK Len=%d", msg->seq, res, maxACKLen);
			//TODO ??? check why m_lastAckData always has zeros
			MW_LOG_DEBUG_ARRAY(MW_LOG_NETWORKSERIAL, PSTR("Response data: "), m_lastAckData, maxACKLen);

			uint8_t data[] = {(uint8_t) (4 + maxACKLen), (uint8_t) msg->seq, NS_CODE, NS_SUBCODE_RFSENDACK, (uint8_t) maxACKLen};
			m_adapter->writeMessage(sizeof(data), data, false);
			m_adapter->writeMessage(maxACKLen, m_lastAckData, true);
			result = true;
		} else {
			MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "Send error. Code=%d", res);
			m_adapter->respondNOK(msg, NetworkSerial::NS_NOK_SEND);
		}
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Not enough data in Send", NULL);
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA);
	}
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::processRFBroadcast(SerialMessageAdapter::serialmsg_t* msg) {
	bool result = false;
	uint16_t port = m_adapter->readByte();
	uint16_t datalen = m_adapter->readByte();
	uint8_t indata[datalen];

	MW_LOG_INFO(MW_LOG_NETWORKSERIAL, "SERSEQ=%d, Port=%d, DataLen=%d", msg->seq, port, datalen);

	if ( m_adapter->waitForBytes(datalen, SerialMessageAdapter::TIMEOUT_RESPONSE) ) {
		for ( uint16_t i = 0; i < datalen; i ++ )//ok, this can be optimized
			indata[i] = m_adapter->readByte();

		int res = m_network->broadcast(port, indata, datalen);

		if ( res == Meshwork::L3::Network::OK ) {
			m_adapter->respondWCode(msg, SerialMessageAdapter::SM_SUBCODE_OK);
			result = true;
		} else {
			m_adapter->respondNOK(msg, NS_NOK_BCAST);
		}
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKSERIAL, "Not enough data to send in Broadcast", NULL);
		m_adapter->respondNOK(msg, SerialMessageAdapter::SM_NOK_INSUFFICIENT_DATA);
	}
	return result ? SerialMessageAdapter::SM_MESSAGE_PROCESSED : SerialMessageAdapter::SM_MESSAGE_ERROR;
}

uint8_t NetworkSerial::processOneMessage(SerialMessageAdapter::serialmsg_t* msg) {
	uint8_t result = SerialMessageAdapter::SM_MESSAGE_UNKNOWN;
	m_currentMsg = msg;

	if ( msg->code == NS_CODE ) {
		switch ( msg->subcode ) {
			case NetworkSerial::NS_SUBCODE_CFGBASIC: result = processCfgBasic(msg); break;
			case NetworkSerial::NS_SUBCODE_CFGNWK: result = processCfgNwk(msg); break;
			case NetworkSerial::NS_SUBCODE_RFINIT: result = processRFInit(msg); break;
			case NetworkSerial::NS_SUBCODE_RFDEINIT: result = processRFDeinit(msg); break;
			case NetworkSerial::NS_SUBCODE_RFSTARTRECV: result = processRFStartRecv(msg); break;
			case NetworkSerial::NS_SUBCODE_RFSEND: result = processRFSend(msg); break;
			case NetworkSerial::NS_SUBCODE_RFBCAST: result = processRFBroadcast(msg); break;
		}
	}
	m_currentMsg = NULL;
	return result;
}
#endif
