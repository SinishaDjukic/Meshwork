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
#ifndef __MESHWORK_UTILS_SERIALMESSAGEADAPTER_H__
#define __MESHWORK_UTILS_SERIALMESSAGEADAPTER_H__

#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/IOStream/Driver/UART.hh"

#include "Meshwork.h"


class SerialMessageAdapter {

	public:

		static const uint8_t MAX_SERIALMSG_LEN 			= 32;

		//0-31: Reserved general
		//32-255: Code specific
		static const uint8_t SM_MESSAGE_UNKNOWN			= 0;
		static const uint8_t SM_MESSAGE_PROCESSED		= 1;
		static const uint8_t SM_MESSAGE_NONE			= 2;
		static const uint8_t SM_MESSAGE_ERROR			= 3;

		//0-31: Reserved general
		//32-255: Code specific
		static const uint8_t SM_SUBCODE_OK 				= 0;
		static const uint8_t SM_SUBCODE_NOK 			= 1;
		static const uint8_t SM_SUBCODE_UNKNOWN 		= 2;

		//0-31: Reserved general
		//32-255: Code/sub-code specific
		static const uint8_t SM_NOK_GENERAL 			= 0;
		static const uint8_t SM_NOK_INSUFFICIENT_DATA 	= 1;
		static const uint8_t SM_NOK_TOO_LONG_DATA 		= 2;
		static const uint8_t SM_NOK_ILLEGAL_STATE 		= 3;
		static const uint8_t SM_NOK_SEQUENCE_MISMATCH	= 4;

		//Default message response timeout (ms)
		static const uint16_t TIMEOUT_RESPONSE 			= 5000;

		typedef struct {
			//these are actually uint8_t
			//but we need two bytes to handle
			//negative EOF and buffer len overrun codes
			uint16_t len;
			uint16_t code;
			uint16_t subcode;
			uint16_t seq;
		} serialmsg_t;

    	class SerialMessageListener {
    		public:
    			virtual uint8_t processOneMessage(serialmsg_t* msg) = 0;
    	};



	protected:
		UART* m_serial;
		SerialMessageListener** m_listeners;
		serialmsg_t* m_currentMsg;
		uint8_t m_lastSerialMsgLen;
		uint8_t m_timeout;

	public:
		SerialMessageAdapter(UART* serial, uint16_t timeout = TIMEOUT_RESPONSE):
			m_serial(serial),
			m_listeners(NULL),
			m_currentMsg(NULL),
			m_lastSerialMsgLen(0),
			m_timeout(timeout)
		{
		};

		void setListeners(SerialMessageListener* listeners[]) {
			m_listeners = listeners;
		}

		int16_t readByte();
		void readRemainingMessageBytes();

		//this saves us ~500 bytes against repetitive putchar calls
		void writeMessage(uint8_t len, uint8_t* data, bool flush);

		void respondWCode(serialmsg_t* msg, uint8_t subcode);
		void respondNOK(serialmsg_t* msg, uint8_t errorValue);

		bool waitForBytes(uint8_t count, uint16_t millis);

		uint8_t processOneMessageHeader(serialmsg_t* msg);
		uint8_t processOneMessage(serialmsg_t* msg);
		uint8_t processOneMessageEx(serialmsg_t* msg);

};

#endif
