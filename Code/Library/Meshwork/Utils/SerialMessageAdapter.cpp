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
#ifndef __MESHWORK_UTILS_SERIALMESSAGEADAPTER_CPP__
#define __MESHWORK_UTILS_SERIALMESSAGEADAPTER_CPP__

#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Utils/SerialMessageAdapter.h"


#ifndef MW_LOG_SERIALMESSAGEADAPTER
	#define MW_LOG_SERIALMESSAGEADAPTER		MW_FULL_DEBUG
#endif


int16_t SerialMessageAdapter::readByte() {
	if ( m_lastSerialMsgLen > 0 ) {
		m_lastSerialMsgLen --;
		return m_serial->getchar();
	} else {
		return -2;
	}
}

void SerialMessageAdapter::writeMessage(uint8_t len, uint8_t* data, bool flush) {
	for ( int i = 0; i < len; i ++ )
		m_serial->putchar(((uint8_t*)data)[i]);
	if ( flush )
		m_serial->flush();
}

bool SerialMessageAdapter::waitForBytes(uint8_t count, uint16_t millis) {
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

void SerialMessageAdapter::readRemainingMessageBytes() {
  MW_LOG_DEBUG(MW_LOG_SERIALMESSAGEADAPTER, "Bytes to discard=%d", m_lastSerialMsgLen);
	if ( m_lastSerialMsgLen > 0 ) {
		if ( waitForBytes(m_lastSerialMsgLen, m_timeout) ) {
			while (readByte() >= 0);
		} else {
			MW_LOG_ERROR(MW_LOG_SERIALMESSAGEADAPTER, "Timeout while discarding bytes: %d", m_lastSerialMsgLen);
		}
	}
}

void SerialMessageAdapter::respondWCode(serialmsg_t* msg, uint8_t subcode) {
	readRemainingMessageBytes();
	uint8_t code = msg->code;
	MW_LOG_INFO(MW_LOG_SERIALMESSAGEADAPTER, "SERSEQ=%d, Code=%d, SubCode=%d", msg->seq, code, subcode);
	uint8_t data[] = {3, (uint8_t) msg->seq, code, subcode};
	writeMessage(sizeof(data), data, true);
}

void SerialMessageAdapter::respondNOK(serialmsg_t* msg, uint8_t errorValue) {
	readRemainingMessageBytes();
	uint8_t code = msg->code;
	MW_LOG_INFO(MW_LOG_SERIALMESSAGEADAPTER, "SERSEQ=%d, Code=%d, SubCode=%d, ErrorValue=%d", msg->seq, code, SM_SUBCODE_NOK, errorValue);
	uint8_t data[] = {4, (uint8_t) msg->seq, code, SM_SUBCODE_NOK, errorValue};
	writeMessage(sizeof(data), data, true);
}

uint8_t SerialMessageAdapter::processOneMessageEx(serialmsg_t* msg) {
	UNUSED(msg);
	return SM_MESSAGE_UNKNOWN;
}

uint8_t SerialMessageAdapter::processOneMessageHeader(serialmsg_t* msg) {
	uint8_t result = SM_MESSAGE_UNKNOWN;

	if ( m_serial->available() >= 4 ) {//minimal msg len
		int len = msg->len = m_serial->getchar();//len
		m_lastSerialMsgLen = len;
		if ( len >= 3 ) {
			msg->seq = readByte();//seq
			msg->code = readByte();//read major code
			//needed to make sure we have enough data arrived in the buffer for the entire command
			msg->subcode = readByte();//read sub-code
			m_currentMsg = msg;
			result = SM_MESSAGE_PROCESSED;
		} else {
			MW_LOG_ERROR(MW_LOG_SERIALMESSAGEADAPTER, "Invalid message: Len=%d", len);
			respondNOK(msg, SM_NOK_GENERAL);
			result = SM_MESSAGE_ERROR;
		}
	} else {
		result = SM_MESSAGE_NONE;
	}
	return result;
}

uint8_t SerialMessageAdapter::processOneMessage(serialmsg_t* msg) {
	uint8_t result = SM_MESSAGE_NONE;
	uint8_t resultCode = processOneMessageHeader(msg);
	
	if ( resultCode == SM_MESSAGE_PROCESSED ) {
		MW_LOG_DEBUG_TRACE(MW_LOG_SERIALMESSAGEADAPTER) << endl << endl << endl;
		MW_LOG_INFO(MW_LOG_SERIALMESSAGEADAPTER, "SERSEQ=%d, Len=%d, Code=%d, SubCode=%d", msg->seq, msg->len, msg->code, msg->subcode);
		if ( waitForBytes(msg->len - 3, m_timeout) ) {
			SerialMessageListener* listener;

			for ( int i = 0; i < (int) membersof(m_listeners); i ++ ) {
				listener = m_listeners[i];
				result = listener->processOneMessage(m_currentMsg);

				MW_LOG_DEBUG(MW_LOG_SERIALMESSAGEADAPTER, "Listner [%d] result=%d", i, result);

				if ( result != SM_MESSAGE_UNKNOWN ) {
					//all done
					break;
				}
			}
			if ( result == SM_MESSAGE_UNKNOWN ) {
				result = processOneMessageEx(msg);
				if ( result == SM_MESSAGE_UNKNOWN )
					respondWCode(msg, SM_SUBCODE_UNKNOWN);
			}
			//this was set in processOneMessageHeader, so null it here
			m_currentMsg = NULL;
			MW_LOG_INFO(MW_LOG_SERIALMESSAGEADAPTER, "SERSEQ=%d, Processed=%d", msg->seq, result);
			MW_LOG_DEBUG_TRACE(MW_LOG_SERIALMESSAGEADAPTER) << endl << endl;
		} else {
			MW_LOG_ERROR(MW_LOG_SERIALMESSAGEADAPTER, "INSUFFICIENT_DATA", NULL);
			respondNOK(msg, SM_NOK_INSUFFICIENT_DATA);
			result = SM_MESSAGE_ERROR;
		}
	} else {
		result = resultCode;
	}
	return result;
}
#endif
