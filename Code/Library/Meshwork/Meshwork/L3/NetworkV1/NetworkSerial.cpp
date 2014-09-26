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
#include "Meshwork/L3/NetworkV1/NetworkSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

#ifndef LOG_NETWORKSERIAL
#define LOG_NETWORKSERIAL true
#endif

void Meshwork::L3::NetworkV1::NetworkSerial::writeMessage(uint8_t len, uint8_t* data, bool flush) {
	for ( int i = 0; i < len; i ++ )
		m_serial->putchar(((uint8_t*)data)[i]);
	if ( flush )
		m_serial->flush();
}

bool Meshwork::L3::NetworkV1::NetworkSerial::initSerial() {
	uint8_t data[] = {0, 1, MSGCODE_CFGREQUEST};
	writeMessage(sizeof(data), data, true);
	return true;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::waitForBytes(uint8_t count, uint16_t millis) {
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

void Meshwork::L3::NetworkV1::NetworkSerial::respondWCode(serialmsg_t* msg, uint8_t code) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Code=%d", msg->seq, code);
	uint8_t data[] = {msg->seq, 1, code};
	writeMessage(sizeof(data), data, true);
}

void Meshwork::L3::NetworkV1::NetworkSerial::respondNOK(serialmsg_t* msg, uint8_t error) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Error=%d", msg->seq, error);
	uint8_t data[] = {msg->seq, 2, MSGCODE_NOK, error};
	writeMessage(sizeof(data), data, true);
}

void Meshwork::L3::NetworkV1::NetworkSerial::set_address(uint8_t src) {
	//no action needed
}

void Meshwork::L3::NetworkV1::NetworkSerial::route_found(Meshwork::L3::NetworkV1::NetworkV1::route_t* route) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Src=%d, Dst=%d, hops=%d", m_currentMsg->seq, route->src, route->dst, route->hopCount);
	uint8_t hopCount = route->hopCount;	
	uint8_t data[] = {m_currentMsg->seq, 4 + hopCount, MSGCODE_RFROUTEFOUND, hopCount, route->src};
	writeMessage(sizeof(data), data, false);
	writeMessage(hopCount, route->hops, false);
	data[0] = route->dst;
	writeMessage(1, data, true);
}

void Meshwork::L3::NetworkV1::NetworkSerial::route_failed(Meshwork::L3::NetworkV1::NetworkV1::route_t* route) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Src=%d, Dst=%d, hops=%d", m_currentMsg->seq, route->src, route->dst, route->hopCount);
	uint8_t hopCount = route->hopCount;
	uint8_t data[] = {m_currentMsg->seq, 4 + hopCount, MSGCODE_RFROUTEFAILED, hopCount, route->src};
	writeMessage(sizeof(data), data, false);
	writeMessage(hopCount, route->hops, false);
	data[0] = route->dst;
	writeMessage(1, data, true);
}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::get_routeCount(uint8_t dst) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Dst=%d", m_currentMsg->seq, dst);

	uint8_t result = 0;
	uint8_t data[] = {m_currentMsg->seq, 2, MSGCODE_RFGETROUTECOUNT, dst};
	writeMessage(sizeof(data), data, true);
	
	if ( waitForBytes(4, m_timeout) ) {
		uint8_t seq = m_serial->getchar();
		if ( m_currentMsg->seq == seq ) {
			uint8_t len = m_serial->getchar();
			uint8_t code = m_serial->getchar();
			if ( code == MSGCODE_NOK ) {
				//read the error code and return NULL;
				//TODO this should actually be an invalid response! so remove...
				m_serial->getchar();
			} else if ( code == MSGCODE_RFGETROUTECOUNTRES ) {
				result = m_serial->getchar();
				MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Route count: %d", result);
			} else {
				//TODO we should always read out the entire message len
				MW_LOG_ERROR(LOG_NETWORKSERIAL, "Wrong response received: %d", code);
				respondNOK(m_currentMsg, ERROR_GENERAL);
			}
		} else {
			MW_LOG_ERROR(LOG_NETWORKSERIAL, "Sequence mismatch! Expected SERSEQ=%d, Received SEQSEQ=%d", m_currentMsg->seq, seq);
			respondNOK(m_currentMsg, ERROR_SEQUENCE_MISMATCH);
		}
	} else {
		respondNOK(m_currentMsg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

Meshwork::L3::NetworkV1::NetworkV1::route_t* Meshwork::L3::NetworkV1::NetworkSerial::get_route(uint8_t dst, uint8_t index) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Dst=%d, index=%d", m_currentMsg->seq, dst, index);
	Meshwork::L3::NetworkV1::NetworkV1::route_t* result = NULL;
	
	uint8_t data[] = {m_currentMsg->seq, 3, MSGCODE_RFGETROUTE, dst, index};
	writeMessage(sizeof(data), data, true);
	
	if ( waitForBytes(3, m_timeout) ) {
			uint8_t seq = m_serial->getchar();
			if ( m_currentMsg->seq == seq ) {
				uint8_t len = m_serial->getchar();
				uint8_t code = m_serial->getchar();
				if ( code == MSGCODE_NOK ) {
					//read the error code and return NULL;
					m_serial->getchar();
					//TODO improve error handling here
					m_currentRoute.hopCount = 0;
				} else if ( code == MSGCODE_RFGETROUTERES ) {
					uint8_t hopCount = 0;
					//some magic goes here...
					//first wait ensure we have at least 3 bytes (HOPCOUNT = 0 | SRC | DST), so that we can read the hopCount value
					//second wait ensures we have at least 3 + hopCount bytes (HOPCOUNT | SRC | <list> | DST)
					if ( waitForBytes(3, m_timeout) && 
						 waitForBytes(3 + (hopCount = m_serial->getchar()) > 0 ? hopCount : 0, m_timeout)) {
						m_currentRoute.hopCount = hopCount;
						m_currentRoute.src = m_serial->getchar();
						for ( int i = 0; i < hopCount; i ++ )
							m_currentRouteHops[i] = m_serial->getchar();
						m_currentRoute.hops = m_currentRouteHops;
						m_currentRoute.dst = m_serial->getchar();
						result = &m_currentRoute;
					} else {
						respondNOK(m_currentMsg, ERROR_INSUFFICIENT_DATA);
					}	
				} else {
					//TODO we should always read out the entire message len
					respondNOK(m_currentMsg, ERROR_ILLEGAL_STATE);
				}
			} else {
				MW_LOG_ERROR(LOG_NETWORKSERIAL, "Sequence mismatch! Expected SERSEQ=%d, Received SEQSEQ=%d", m_currentMsg->seq, seq);
				respondNOK(m_currentMsg, ERROR_SEQUENCE_MISMATCH);
			}
	} else {
		respondNOK(m_currentMsg, ERROR_INSUFFICIENT_DATA);
	}	
	return result;
}

void Meshwork::L3::NetworkV1::NetworkSerial::respondSendACK(serialmsg_t* msg, uint8_t datalen, uint8_t* ackData) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, DataLen=%d", msg->seq, datalen);
	uint8_t data[] = {msg->seq, 2 + datalen, MSGCODE_RFSENDACK, datalen};
	writeMessage(sizeof(data), data, false);
	writeMessage(datalen, data, true);
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processCfgBasic(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 3 ) {
		data_cfgbasic_t* cfgbasic;
		cfgbasic = (data_cfgbasic_t*) msg->data;
		cfgbasic->nwkcaps = m_serial->getchar();
		cfgbasic->delivery = m_serial->getchar();
		cfgbasic->retry = m_serial->getchar();
		
		m_network->setNetworkCaps(cfgbasic->nwkcaps);
		m_network->setDelivery(cfgbasic->delivery);
		m_network->setRetry(cfgbasic->retry);
		
		MW_LOG_INFO(LOG_NETWORKSERIAL, "NwkCaps=%d, Delivery=%d, Retry=%d", cfgbasic->nwkcaps, cfgbasic->delivery, cfgbasic->retry);
		
		respondWCode(msg, MSGCODE_OK);
		result = true;
	} else {
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processCfgNwk(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 3 ) {
		data_cfgnwk_t* cfgnwk;
		cfgnwk = (data_cfgnwk_t*) msg->data;
		cfgnwk->channel = m_serial->getchar();
		cfgnwk->nwkid = (uint16_t) m_serial->getchar() << 8 | m_serial->getchar();
		cfgnwk->nodeid = m_serial->getchar();
		m_network->setChannel(cfgnwk->channel);
		m_network->setNetworkID(cfgnwk->nwkid);
		m_network->setNodeID(cfgnwk->nodeid);
		MW_LOG_INFO(LOG_NETWORKSERIAL, "NodeID=%d, NwkID=%d, Channel=%d", cfgnwk->nodeid, cfgnwk->nwkid, cfgnwk->channel);
		size_t keyLen = m_serial->getchar();
		if ( keyLen >= 0 && keyLen <= Meshwork::L3::Network::MAX_NETWORK_KEY_LEN ) {
			m_networkKey[0] = 0;
			for (size_t i = 0; i < keyLen; i ++ )
				m_networkKey[i] = m_serial->getchar();
			if ( keyLen > 0 )
				m_networkKey[keyLen + 1] = 0;
			m_network->setNetworkKey((char*) m_networkKey);
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

bool Meshwork::L3::NetworkV1::NetworkSerial::processRFInit(serialmsg_t* msg) {
	bool result = m_network->begin();
	MW_LOG_INFO(LOG_NETWORKSERIAL, "", NULL);
	result ? respondWCode(msg, MSGCODE_OK) : respondNOK(msg, ERROR_GENERAL);
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processRFDeinit(serialmsg_t* msg) {
	bool result = m_network->end();
	MW_LOG_INFO(LOG_NETWORKSERIAL, "", NULL);
	result ? respondWCode(msg, MSGCODE_OK) : respondNOK(msg, ERROR_GENERAL);
	return result;
}

int Meshwork::L3::NetworkV1::NetworkSerial::returnACKPayload(uint8_t src, uint8_t port,
													void* buf, uint8_t len,
														void* bufACK, size_t lenACK) {
	int bytes = 0;
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Src=%d, Port=%d, SerLen=%d", m_currentMsg->seq, src, port, len);
	if ( m_currentMsg != NULL ) {//must be the case, but need a sanity check
		MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Sending RFRECV", NULL);
		//first, send RFRECV
		uint8_t data[] = {m_currentMsg->seq, 4 + len, MSGCODE_RFRECV, src, port, len};
		writeMessage(sizeof(data), data, false);
		writeMessage(len, (uint8_t*)buf, true);
		
		if ( waitForBytes(4, m_timeout) )  {
			uint8_t seq = m_serial->getchar();
			MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Response to RFRECV, SERSEQ=%d", seq);
			if ( m_currentMsg->seq == seq ) {
				uint8_t len = m_serial->getchar();
				uint8_t code = m_serial->getchar();
				//TODO check if code == MSGCODE_RFRECVACK

				uint8_t reslen = m_serial->getchar();
				MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Response to RFRECV, reslen=%d", reslen);
				if ( reslen >=0 && reslen <= lenACK && waitForBytes(reslen, m_timeout) ) {
					for ( int i = 0; i < reslen; i ++ )
						((char*) bufACK)[i] = m_serial->getchar();
//					respondWCode(m_currentMsg, MSGCODE_OK);
					bytes = reslen;
				} else {
					MW_LOG_ERROR(LOG_NETWORKSERIAL, "Response to RFRECV, ERROR_TOO_LONG_DATA", NULL);
//					respondNOK(m_currentMsg, ERROR_TOO_LONG_DATA);
				}
			} else {
				MW_LOG_ERROR(LOG_NETWORKSERIAL, "Response to RFRECV, serial seq WRONG", NULL);
//				respondNOK(m_currentMsg, ERROR_SEQUENCE_MISMATCH);
			}
		} else {
			MW_LOG_ERROR(LOG_NETWORKSERIAL, "Response to RFRECV, ERROR_INSUFFICIENT_DATA", NULL);
			respondNOK(m_currentMsg, ERROR_INSUFFICIENT_DATA);
		}
	} else {
		respondNOK(m_currentMsg, ERROR_ILLEGAL_STATE);
	}
	return bytes;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processRFStartRecv(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 4 ) {
		data_rfstartrecv_t* rfstartrecv;
		rfstartrecv = (data_rfstartrecv_t*) msg->data;
		uint32_t timeout = rfstartrecv->timeout =
								(uint32_t) m_serial->getchar() << 24 |
								(uint32_t) m_serial->getchar() << 16 |
								(uint32_t) m_serial->getchar() << 8 |
								(uint32_t) m_serial->getchar();
		MW_LOG_INFO(LOG_NETWORKSERIAL, "Receiving with timeout=%l", timeout);
//not needed here		m_currentMsg = msg;

		int res = m_network->recv(m_lastMsgSrc, m_lastMsgPort, &m_lastMsgData, m_lastMsgLen, timeout, this);
		MW_LOG_INFO(LOG_NETWORKSERIAL, "Receive done. SerResult=%d, Src=%d, Port=%d", res, m_lastMsgSrc, m_lastMsgPort);
//not needed here		m_currentMsg = NULL;
		if ( res == Meshwork::L3::Network::OK ) {
			respondWCode(msg, MSGCODE_OK);
		} else if ( res == Meshwork::L3::Network::OK_MESSAGE_INTERNAL ) {
			respondWCode(msg, MSGCODE_INTERNAL);
		} else if ( res == Meshwork::L3::Network::OK_MESSAGE_IGNORED ) {
			respondWCode(msg, MSGCODE_INTERNAL);
		} else {
			respondNOK(msg, res);
		}
		result = true;
	} else {
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processRFSend(serialmsg_t* msg) {
	bool result = false;
	MW_LOG_INFO(LOG_NETWORKSERIAL, "Entered", NULL);
	if ( m_serial->available() >= 3 ) {//minimal msg len
		data_rfsend_t* rfsend;
		rfsend = (data_rfsend_t*) msg->data;
		uint8_t dst = rfsend->dst = m_serial->getchar();
		uint8_t port = rfsend->port = m_serial->getchar();
		uint8_t datalen = rfsend->datalen = m_serial->getchar();
		MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Dst=%d, Port=%d, DataLen=%d", msg->seq, dst, port, datalen);
//		if ( waitForBytes(datalen, m_timeout) ) {
		MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Reading data to send...", NULL);
			for ( int i = 0; i < datalen; i ++ )//ok, this can be optimized
				rfsend->data[i] = m_serial->getchar();
			//TODO debug print the array
			size_t maxACKLen = NetworkV1::ACK_PAYLOAD_MAX;
			MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Sending data...", NULL);
			int res = m_network->send(dst, port, rfsend->data, datalen, m_lastAckData, maxACKLen);
			if ( res == Meshwork::L3::Network::OK ) {
				MW_LOG_INFO(LOG_NETWORKSERIAL, "Send done. SERSEQ=%d, Result=%d, ACK Len=%d", msg->seq, res, maxACKLen);
				//TODO check why m_lastAckData always has zeros
				MW_LOG_DEBUG_ARRAY(LOG_NETWORKSERIAL, PSTR("Response data: "), m_lastAckData, maxACKLen);
				uint8_t data[] = {msg->seq, 2 + maxACKLen, MSGCODE_RFSENDACK, maxACKLen};
				writeMessage(sizeof(data), data, false);
				writeMessage(maxACKLen, m_lastAckData, true);
				result = true;
			} else {
				MW_LOG_INFO(LOG_NETWORKSERIAL, "Send error. Code=%d", res);
				respondNOK(msg, ERROR_SEND);
			}
//		} else {
//			respondNOK(msg, ERROR_INSUFFICIENT_DATA);
//		}
	} else {
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processRFBroadcast(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 2 ) {//minimal msg len
		data_rfbcast_t* rfbcast;
		rfbcast = (data_rfbcast_t*) msg->data;
		uint8_t port = rfbcast->port = m_serial->getchar();
		uint8_t datalen = rfbcast->datalen = m_serial->getchar();
		MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Port=%d, DataLen=%d", msg->seq, port, datalen);
		if ( m_serial->available() >= datalen ) {
			for ( int i = 0; i < datalen; i ++ )//ok, this can be optimized
				rfbcast->data[i] = m_serial->getchar();
			int res = m_network->broadcast(port, rfbcast->data, datalen);
			if ( res == Meshwork::L3::Network::OK ) {
				respondWCode(msg, MSGCODE_OK);
				result = true;
			} else {
				respondNOK(msg, ERROR_BCAST);
			}
		} else {
			respondNOK(msg, ERROR_INSUFFICIENT_DATA);
		}
	} else {
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processOneMessageEx(serialmsg_t* msg) {
	return false;
}


bool Meshwork::L3::NetworkV1::NetworkSerial::processOneMessage(serialmsg_t* msg) {
	bool result = true;
	
	if ( m_serial->available() >= 3 ) {//minimal msg len
		msg->seq = m_serial->getchar();//seq
		int len = msg->len = m_serial->getchar();//len
		if ( len >= 0 ) {
			//needed to make sure we have enough data arrived in the buffer for the entire command
			int msgcode = msg->code = m_serial->getchar();//msgcode
			m_currentMsg = msg;
			MW_LOG_DEBUG_TRACE(LOG_NETWORKSERIAL) << endl << endl << endl;
			MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Len=%d, Code=%d", msg->seq, len, msgcode);
			if ( waitForBytes(len - 1, m_timeout) ) {
				switch ( msgcode ) {
					case MSGCODE_CFGBASIC: processCfgBasic(msg); break;
					case MSGCODE_CFGNWK: processCfgNwk(msg); break;
					case MSGCODE_RFINIT: processRFInit(msg); break;
					case MSGCODE_RFDEINIT: processRFDeinit(msg); break;
					case MSGCODE_RFSTARTRECV: processRFStartRecv(msg); break;
					case MSGCODE_RFSEND: processRFSend(msg); break;
					case MSGCODE_RFBCAST: processRFBroadcast(msg); break;
					default:
						result = processOneMessageEx(msg);
						if (!result)
							respondWCode(msg, MSGCODE_UNKNOWN);
						m_currentMsg = NULL;
				}
				MW_LOG_DEBUG_TRACE(LOG_NETWORKSERIAL) << endl << endl;
			} else {
				MW_LOG_ERROR(LOG_NETWORKSERIAL, "INSUFFICIENT_DATA", NULL);
				respondWCode(msg, ERROR_INSUFFICIENT_DATA);
				result = false;
			}
		} else {
			MW_LOG_ERROR(LOG_NETWORKSERIAL, "MSGCODE_UNKNOWN: SERSEQ=%d, Len=%d", msg->seq, len);
			respondWCode(msg, MSGCODE_UNKNOWN);
			result = false;
		}
	} else {
		result = false;
	}
	return result;
}
