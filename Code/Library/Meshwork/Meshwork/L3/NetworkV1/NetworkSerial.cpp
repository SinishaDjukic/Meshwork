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
#include "Meshwork/L3/NetworkV1/NetworkSerial.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

#ifndef LOG_NETWORKSERIAL
#define LOG_NETWORKSERIAL true
#endif

int Meshwork::L3::NetworkV1::NetworkSerial::readByte() {
	if ( m_lastSerialMsgLen > 1 ) {
		m_lastSerialMsgLen --;
		return m_serial->getchar();
	} else {
		return -2;
	}
}

void Meshwork::L3::NetworkV1::NetworkSerial::writeMessage(uint8_t len, uint8_t* data, bool flush) {
	for ( int i = 0; i < len; i ++ )
		m_serial->putchar(((uint8_t*)data)[i]);
	if ( flush )
		m_serial->flush();
}

bool Meshwork::L3::NetworkV1::NetworkSerial::initSerial() {
	uint8_t data[] = {0, 1, NS_SUBCODE_CFGREQUEST};
	writeMessage(sizeof(data), data, true);
	return true;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::waitForBytes(uint8_t count, uint16_t millis) {
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

void Meshwork::L3::NetworkV1::NetworkSerial::readRemainingMessageBytes() {
	if ( m_lastSerialMsgLen > 0 ) {
		MW_LOG_WARNING(LOG_NETWORKSERIAL, "Remaining message bytes to be discarded=%d", m_lastSerialMsgLen);
		if ( waitForBytes(m_lastSerialMsgLen, m_timeout) ) {
			while (readByte() >= 0);
		} else {
			MW_LOG_ERROR(LOG_NETWORKSERIAL, "Timeout waiting for bytes count to be discarded: %d", m_lastSerialMsgLen);
		}
	} else {
		MW_LOG_DEBUG(LOG_NETWORKSERIAL, "No message bytes to be discarded", NULL);
	}
}

void Meshwork::L3::NetworkV1::NetworkSerial::respondWCode(serialmsg_t* msg, uint8_t code) {
	readRemainingMessageBytes();
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Code=%d", msg->seq, code);
	uint8_t data[] = {3, msg->seq, NS_CODE, code};
	writeMessage(sizeof(data), data, true);
}

void Meshwork::L3::NetworkV1::NetworkSerial::respondNOK(serialmsg_t* msg, uint8_t error) {
	readRemainingMessageBytes();
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Error=%d", msg->seq, error);
	uint8_t data[] = {4, msg->seq, NS_CODE, NS_SUBCODE_NOK, error};
	writeMessage(sizeof(data), data, true);
}

void Meshwork::L3::NetworkV1::NetworkSerial::set_address(uint8_t src) {
	//no action needed
}

void Meshwork::L3::NetworkV1::NetworkSerial::route_found(Meshwork::L3::NetworkV1::NetworkV1::route_t* route) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Src=%d, Dst=%d, hops=%d", m_currentMsg->seq, route->src, route->dst, route->hopCount);
	uint8_t hopCount = route->hopCount;	
	uint8_t data[] = {6 + hopCount, m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFROUTEFOUND, hopCount, route->src};
	writeMessage(sizeof(data), data, false);
	writeMessage(hopCount, route->hops, false);
	data[0] = route->dst;
	writeMessage(1, data, true);
}

void Meshwork::L3::NetworkV1::NetworkSerial::route_failed(Meshwork::L3::NetworkV1::NetworkV1::route_t* route) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Src=%d, Dst=%d, hops=%d", m_currentMsg->seq, route->src, route->dst, route->hopCount);
	uint8_t hopCount = route->hopCount;
	uint8_t data[] = {6 + hopCount, m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFROUTEFAILED, hopCount, route->src};
	writeMessage(sizeof(data), data, false);
	writeMessage(hopCount, route->hops, false);
	data[0] = route->dst;
	writeMessage(1, data, true);
}

uint8_t Meshwork::L3::NetworkV1::NetworkSerial::get_routeCount(uint8_t dst) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Dst=%d", m_currentMsg->seq, dst);

	uint8_t result = 0;
	uint8_t data[] = {4, m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFGETROUTECOUNT, dst};
	writeMessage(sizeof(data), data, true);
	
	if ( waitForBytes(6, m_timeout) ) {
		m_lastSerialMsgLen = readByte();
		uint8_t seq = readByte();
		if ( m_currentMsg->seq == seq ) {
			readByte();//read major code
			uint8_t code = readByte();//read sub-code
			if ( code == NS_SUBCODE_NOK ) {
				//read the error code and return NULL... shouldn't happen though
				readRemainingMessageBytes();
			} else if ( code == NS_SUBCODE_RFGETROUTECOUNTRES ) {
				result = readByte();
				readRemainingMessageBytes();
				MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Route count: %d", result);
			} else {
				MW_LOG_ERROR(LOG_NETWORKSERIAL, "Wrong response code received to RFGETROUTECOUNT: %d", code);
				respondNOK(m_currentMsg, ERROR_GENERAL);
			}
		} else {
			MW_LOG_ERROR(LOG_NETWORKSERIAL, "Sequence mismatch! Expected SERSEQ=%d, Received SEQSEQ=%d", m_currentMsg->seq, seq);
			respondNOK(m_currentMsg, ERROR_SEQUENCE_MISMATCH);
		}
	} else {
		MW_LOG_ERROR(LOG_NETWORKSERIAL, "No response to NS_SUBCODE_RFGETROUTECOUNT, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(m_currentMsg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

Meshwork::L3::NetworkV1::NetworkV1::route_t* Meshwork::L3::NetworkV1::NetworkSerial::get_route(uint8_t dst, uint8_t index) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Dst=%d, index=%d", m_currentMsg->seq, dst, index);
	Meshwork::L3::NetworkV1::NetworkV1::route_t* result = NULL;
	
	uint8_t data[] = {5, m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFGETROUTE, dst, index};
	writeMessage(sizeof(data), data, true);
	
	if ( waitForBytes(5, m_timeout) ) {
		m_lastSerialMsgLen = readByte();
			uint8_t seq = readByte();
			if ( m_currentMsg->seq == seq ) {
				readByte();//read major code
				uint8_t code = readByte();//read sub-code
				if ( code == NS_SUBCODE_NOK ) {
					//read the error code and return NULL... shouldn't happen though
					readRemainingMessageBytes();
					m_currentRoute.hopCount = 0;
				} else if ( code == NS_SUBCODE_RFGETROUTERES ) {
					uint8_t hopCount = 0;
					//some magic goes here...
					//first wait ensure we have at least 3 bytes (HOPCOUNT = 0 | SRC | DST), so that we can read the hopCount value
					//second wait ensures we have at least 3 + hopCount bytes (HOPCOUNT | SRC | <list> | DST)
					if ( waitForBytes(3, m_timeout) && 
						 waitForBytes(3 + (hopCount = readByte()) > 0 ? hopCount : 0, m_timeout)) {
						m_currentRoute.hopCount = hopCount;
						m_currentRoute.src = readByte();
						for ( int i = 0; i < hopCount; i ++ )
							m_currentRouteHops[i] = readByte();
						m_currentRoute.hops = m_currentRouteHops;
						m_currentRoute.dst = readByte();
						result = &m_currentRoute;
						readRemainingMessageBytes();
					} else {
						MW_LOG_ERROR(LOG_NETWORKSERIAL, "Not enough hops in NS_SUBCODE_RFGETROUTE, ERROR_INSUFFICIENT_DATA", NULL);
						respondNOK(m_currentMsg, ERROR_INSUFFICIENT_DATA);
					}	
				} else {
					MW_LOG_ERROR(LOG_NETWORKSERIAL, "Wrong response code received to RFGETROUTE: %d", code);
					respondNOK(m_currentMsg, ERROR_GENERAL);
				}
			} else {
				MW_LOG_ERROR(LOG_NETWORKSERIAL, "Sequence mismatch! Expected SERSEQ=%d, Received SEQSEQ=%d", m_currentMsg->seq, seq);
				respondNOK(m_currentMsg, ERROR_SEQUENCE_MISMATCH);
			}
	} else {
		MW_LOG_ERROR(LOG_NETWORKSERIAL, "No response to NS_SUBCODE_RFGETROUTE, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(m_currentMsg, ERROR_INSUFFICIENT_DATA);
	}	
	return result;
}

void Meshwork::L3::NetworkV1::NetworkSerial::respondSendACK(serialmsg_t* msg, uint8_t datalen, uint8_t* ackData) {
	MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, DataLen=%d", msg->seq, datalen);
	uint8_t data[] = {4 + datalen, msg->seq, NS_CODE, NS_SUBCODE_RFSENDACK, datalen};
	writeMessage(sizeof(data), data, false);
	writeMessage(datalen, data, true);
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processCfgBasic(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 3 ) {
		data_cfgbasic_t* cfgbasic;
		cfgbasic = (data_cfgbasic_t*) msg->data;
		cfgbasic->nwkcaps = readByte();
		cfgbasic->delivery = readByte();
		cfgbasic->retry = readByte();
		
		m_network->setNetworkCaps(cfgbasic->nwkcaps);
		m_network->setDelivery(cfgbasic->delivery);
		m_network->setRetry(cfgbasic->retry);
		
		MW_LOG_INFO(LOG_NETWORKSERIAL, "NwkCaps=%d, Delivery=%d, Retry=%d", cfgbasic->nwkcaps, cfgbasic->delivery, cfgbasic->retry);
		
		respondWCode(msg, NS_SUBCODE_OK);
		result = true;
	} else {
		MW_LOG_ERROR(LOG_NETWORKSERIAL, "Not enough data in Config Basic, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processCfgNwk(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 3 ) {
		data_cfgnwk_t* cfgnwk;
		cfgnwk = (data_cfgnwk_t*) msg->data;
		cfgnwk->channel = readByte();
		cfgnwk->nwkid = (uint16_t) readByte() << 8 | readByte();
		cfgnwk->nodeid = readByte();
		m_network->setChannel(cfgnwk->channel);
		m_network->setNetworkID(cfgnwk->nwkid);
		m_network->setNodeID(cfgnwk->nodeid);
		MW_LOG_INFO(LOG_NETWORKSERIAL, "NodeID=%d, NwkID=%d, Channel=%d", cfgnwk->nodeid, cfgnwk->nwkid, cfgnwk->channel);
		size_t keyLen = readByte();
		if ( keyLen >= 0 && keyLen <= Meshwork::L3::Network::MAX_NETWORK_KEY_LEN ) {
			m_networkKey[0] = 0;
			for (size_t i = 0; i < keyLen; i ++ )
				m_networkKey[i] = readByte();
			if ( keyLen > 0 )
				m_networkKey[keyLen + 1] = 0;
			m_network->setNetworkKey((char*) m_networkKey);
			respondWCode(msg, NS_SUBCODE_OK);
			result = true;
		} else {
			respondNOK(msg, ERROR_KEY_TOO_LONG);
			result = false;
		}
	} else {
		MW_LOG_ERROR(LOG_NETWORKSERIAL, "Not enough data in Config Network, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processRFInit(serialmsg_t* msg) {
	bool result = m_network->begin();
	MW_LOG_INFO(LOG_NETWORKSERIAL, "", NULL);
	if ( result )
		respondWCode(msg, NS_SUBCODE_OK);
	else
		respondNOK(msg, ERROR_ILLEGAL_STATE);
	result ? respondWCode(msg, NS_SUBCODE_OK) : respondNOK(msg, ERROR_GENERAL);
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processRFDeinit(serialmsg_t* msg) {
	bool result = m_network->end();
	MW_LOG_INFO(LOG_NETWORKSERIAL, "", NULL);
	if ( result )
		respondWCode(msg, NS_SUBCODE_OK);
	else
		respondNOK(msg, ERROR_ILLEGAL_STATE);
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
		uint8_t data[] = {6 + len, m_currentMsg->seq, NS_CODE, NS_SUBCODE_RFRECV, src, port, len};
		writeMessage(sizeof(data), data, false);
		writeMessage(len, (uint8_t*)buf, true);
		
		if ( waitForBytes(6, m_timeout) )  {
			m_lastSerialMsgLen = readByte();
			uint8_t seq = readByte();
			MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Response to RFRECV, SERSEQ=%d", seq);
			if ( m_currentMsg->seq == seq ) {
				readByte();//read major code
				uint8_t code = readByte();//read sub-code

				if ( code == NS_SUBCODE_RFRECVACK ) {
					uint8_t reslen = readByte();
					MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Response to RFRECV, reslen=%d", reslen);
					if ( reslen >=0 && reslen <= lenACK && waitForBytes(reslen, m_timeout) ) {
						for ( int i = 0; i < reslen; i ++ )
							((char*) bufACK)[i] = readByte();
						//The other side does not expect a reply here, so response must not be sent
						//respondWCode(m_currentMsg, NS_SUBCODE_OK);
						bytes = reslen;
					} else {
						MW_LOG_ERROR(LOG_NETWORKSERIAL, "Response to RFRECV, ERROR_TOO_LONG_DATA", NULL);
						//The other side does not expect a reply here, so response must not be sent
						//respondNOK(m_currentMsg, ERROR_TOO_LONG_DATA);
					}
				} else {
					MW_LOG_ERROR(LOG_NETWORKSERIAL, "Wrong response code received to RFRECV: %d", code);
					//The other side does not expect a reply here, so response must not be sent
					//respondNOK(m_currentMsg, ERROR_GENERAL);
				}
			} else {
				MW_LOG_ERROR(LOG_NETWORKSERIAL, "Response to RFRECV, serial seq WRONG", NULL);
				//The other side does not expect a reply here, so response must not be sent
				//respondNOK(m_currentMsg, ERROR_SEQUENCE_MISMATCH);
			}
			//Since the other side does not expect any replies in the above sequnces
			//we read out the message fully at the end here
			readRemainingMessageBytes();
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
								(uint32_t) readByte() << 24 |
								(uint32_t) readByte() << 16 |
								(uint32_t) readByte() << 8 |
								(uint32_t) readByte();
		MW_LOG_INFO(LOG_NETWORKSERIAL, "Receiving with timeout=%l", timeout);
//not needed here		m_currentMsg = msg;

		int res = m_network->recv(m_lastMsgSrc, m_lastMsgPort, &m_lastMsgData, m_lastMsgLen, timeout, this);
		MW_LOG_INFO(LOG_NETWORKSERIAL, "Receive done. SerResult=%d, Src=%d, Port=%d", res, m_lastMsgSrc, m_lastMsgPort);
//not needed here		m_currentMsg = NULL;
		if ( res == Meshwork::L3::Network::OK ) {
			respondWCode(msg, NS_SUBCODE_OK);
		} else if ( res == Meshwork::L3::Network::OK_MESSAGE_INTERNAL ) {
			respondWCode(msg, NS_SUBCODE_INTERNAL);
		} else if ( res == Meshwork::L3::Network::OK_MESSAGE_IGNORED ) {
			respondWCode(msg, NS_SUBCODE_INTERNAL);
		} else {
			respondNOK(msg, res);
		}
		result = true;
	} else {
		MW_LOG_ERROR(LOG_NETWORKSERIAL, "Not enough data in Start Recv, ERROR_INSUFFICIENT_DATA", NULL);
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
		uint8_t dst = rfsend->dst = readByte();
		uint8_t port = rfsend->port = readByte();
		uint8_t datalen = rfsend->datalen = readByte();
		MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Dst=%d, Port=%d, DataLen=%d", msg->seq, dst, port, datalen);
//		if ( waitForBytes(datalen, m_timeout) ) {
		MW_LOG_DEBUG(LOG_NETWORKSERIAL, "Reading data to send...", NULL);
			for ( int i = 0; i < datalen; i ++ )//ok, this can be optimized
				rfsend->data[i] = readByte();

			//make sure we read out the rest here, since we will have nested messages during the send
			readRemainingMessageBytes();

			size_t maxACKLen = NetworkV1::ACK_PAYLOAD_MAX;

			MW_LOG_DEBUG_ARRAY(LOG_NETWORKSERIAL, PSTR("Sending data: "), rfsend->data, datalen);

			int res = m_network->send(dst, port, rfsend->data, datalen, m_lastAckData, maxACKLen);
			if ( res == Meshwork::L3::Network::OK ) {
				MW_LOG_INFO(LOG_NETWORKSERIAL, "Send done. SERSEQ=%d, Result=%d, ACK Len=%d", msg->seq, res, maxACKLen);
				//TODO ??? check why m_lastAckData always has zeros
				MW_LOG_DEBUG_ARRAY(LOG_NETWORKSERIAL, PSTR("Response data: "), m_lastAckData, maxACKLen);
				uint8_t data[] = {4 + maxACKLen, msg->seq, NS_CODE, NS_SUBCODE_RFSENDACK, maxACKLen};
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
		MW_LOG_ERROR(LOG_NETWORKSERIAL, "Not enough data in Send, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processRFBroadcast(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 2 ) {//minimal msg len
		data_rfbcast_t* rfbcast;
		rfbcast = (data_rfbcast_t*) msg->data;
		uint8_t port = rfbcast->port = readByte();
		uint8_t datalen = rfbcast->datalen = readByte();
		MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Port=%d, DataLen=%d", msg->seq, port, datalen);
		if ( m_serial->available() >= datalen ) {
			for ( int i = 0; i < datalen; i ++ )//ok, this can be optimized
				rfbcast->data[i] = readByte();
			int res = m_network->broadcast(port, rfbcast->data, datalen);
			if ( res == Meshwork::L3::Network::OK ) {
				respondWCode(msg, NS_SUBCODE_OK);
				result = true;
			} else {
				respondNOK(msg, ERROR_BCAST);
			}
		} else {
			MW_LOG_ERROR(LOG_NETWORKSERIAL, "Not enough data to send in Broadcast, ERROR_INSUFFICIENT_DATA", NULL);
			respondNOK(msg, ERROR_INSUFFICIENT_DATA);
		}
	} else {
		MW_LOG_ERROR(LOG_NETWORKSERIAL, "Not enough data in Broadcast, ERROR_INSUFFICIENT_DATA", NULL);
		respondNOK(msg, ERROR_INSUFFICIENT_DATA);
	}
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkSerial::processOneMessageEx(serialmsg_t* msg) {
	return false;
}


bool Meshwork::L3::NetworkV1::NetworkSerial::processOneMessage(serialmsg_t* msg) {
	bool result = true;
	
	if ( m_serial->available() >= 4 ) {//minimal msg len
		int len = msg->len = m_serial->getchar();//len
		m_lastSerialMsgLen = len;
		if ( len > 0 ) {
			msg->seq = readByte();//seq
			msg->code = readByte();//read major code
			//needed to make sure we have enough data arrived in the buffer for the entire command
			msg->subcode = readByte();//read sub-code
			m_currentMsg = msg;
			MW_LOG_DEBUG_TRACE(LOG_NETWORKSERIAL) << endl << endl << endl;
			MW_LOG_INFO(LOG_NETWORKSERIAL, "SERSEQ=%d, Len=%d, Code=%d, SubCode=$d", msg->seq, len, msg->code, msg->subcode);
			if ( waitForBytes(len - 3, m_timeout) ) {
				switch ( msg->subcode ) {
					case NS_SUBCODE_CFGBASIC: processCfgBasic(msg); break;
					case NS_SUBCODE_CFGNWK: processCfgNwk(msg); break;
					case NS_SUBCODE_RFINIT: processRFInit(msg); break;
					case NS_SUBCODE_RFDEINIT: processRFDeinit(msg); break;
					case NS_SUBCODE_RFSTARTRECV: processRFStartRecv(msg); break;
					case NS_SUBCODE_RFSEND: processRFSend(msg); break;
					case NS_SUBCODE_RFBCAST: processRFBroadcast(msg); break;
					default:
						result = processOneMessageEx(msg);
						if (!result)
							respondWCode(msg, NS_SUBCODE_UNKNOWN);
						m_currentMsg = NULL;
				}
				MW_LOG_DEBUG_TRACE(LOG_NETWORKSERIAL) << endl << endl;
			} else {
				MW_LOG_ERROR(LOG_NETWORKSERIAL, "INSUFFICIENT_DATA", NULL);
				respondNOK(msg, ERROR_INSUFFICIENT_DATA);
				result = false;
			}
		} else {
			MW_LOG_ERROR(LOG_NETWORKSERIAL, "Invalid message: Len=%d", len);
			respondNOK(msg, ERROR_GENERAL);
			result = false;
		}
	} else {
		result = false;
	}
	return result;
}
#endif
