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
#ifndef __MESHWORK_L4_CONTROLLERBASESERIAL_CPP__
#define __MESHWORK_L4_CONTROLLERBASESERIAL_CPP__

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
#include "Meshwork/L4/ControllerBase.h"
#include "Meshwork/L4/ControllerBaseSerial.h"

#ifndef IF_CONTROLLERBASESERIAL_DEBUG
#define IF_CONTROLLERBASESERIAL_DEBUG if(false)
#endif
			
bool Meshwork::L4::ControllerBaseSerial::processSetModeAnnounce(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= sizeof(data_setmodeann_t) ) {
		data_setmodeann_t* setmodeann;
		setmodeann = (data_setmodeann_t*) msg->data;
		uint8_t mode = setmodeann->mode = m_serial->getchar();
		uint32_t timeout = setmodeann->timeout =
								(uint32_t) m_serial->getchar() << 24 |
								(uint32_t) m_serial->getchar() << 16 |
								(uint32_t) m_serial->getchar() << 8 |
								(uint32_t) m_serial->getchar();
		int mres = m_controllerBase->setModeAnnounce(mode, timeout);
		m_serial->putchar(m_currentMsg->seq);
		m_serial->putchar(3);
		m_serial->putchar(CS_SUBCODE_SET_MODE_ANN_RES);
		m_serial->putchar(0xFF & (mres >> 8));
		m_serial->putchar(0xFF & (mres >> 0));
		result = true;
	}
	return result;
}

bool Meshwork::L4::ControllerBaseSerial::processGetNodeList(serialmsg_t* msg) {
	BitSet<Meshwork::L3::Network::MAX_NODE_COUNT> nodeList;
	m_controllerBase->getNodeList(&nodeList);
	uint8_t nodeCount = 0;
	for ( uint8_t i = 0; i < Meshwork::L3::Network::MAX_NODE_COUNT; i ++ )
		nodeCount = nodeCount + nodeList[i] ? 1 : 0;
	m_serial->putchar(m_currentMsg->seq);
	m_serial->putchar(2 + nodeCount);
	m_serial->putchar(CS_SUBCODE_GET_NODE_LIST_RES);
	m_serial->putchar(nodeCount);
	for ( uint8_t i = 0; i < Meshwork::L3::Network::MAX_NODE_COUNT; i ++ )
		if ( nodeList[i] )
			m_serial->putchar(i);
	return true;
}

bool Meshwork::L4::ControllerBaseSerial::processAddNode(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 1 ) {
		uint8_t desiredID = m_serial->getchar();
		int newID = m_controllerBase->addNode(desiredID);
		if ( newID >= Meshwork::L3::Network::MIN_NODE_ID && newID <= Meshwork::L3::Network::MAX_NODE_ID ) {
			m_serial->putchar(m_currentMsg->seq);
			m_serial->putchar(2);
			m_serial->putchar(CS_SUBCODE_ADD_NODE_RES);
			m_serial->putchar(0xFF & newID);
			result = true;
		} else {
			respondNOK(msg, Meshwork::L4::ControllerBaseSerial::ERROR_GENERAL);
		}
	}
	return result;
}

bool Meshwork::L4::ControllerBaseSerial::processRemoveNode(serialmsg_t* msg) {
	bool result = false;
	if ( m_serial->available() >= 1 ) {
		uint8_t nodeID = m_serial->getchar();
		int res = m_controllerBase->removeNode(nodeID);
		if ( res == 0 ) {
			respondWCode(msg, CS_SUBCODE_OK);
		} else {
			respondNOK(msg, res == Meshwork::L4::ControllerBase::ERROR_NODE_INVALID ? Meshwork::L4::ControllerBase::ERROR_NODE_INVALID :
							(res == Meshwork::L4::ControllerBase::ERROR_NODE_INVALID_CONTROLLER ? Meshwork::L4::ControllerBase::ERROR_NODE_INVALID_CONTROLLER :
								Meshwork::L4::ControllerBaseSerial::ERROR_GENERAL)
							);
		}
	}
	return result;
}


bool Meshwork::L4::ControllerBaseSerial::processOneMessageEx(serialmsg_t* msg) {
	bool result = true;
	switch ( msg->subcode ) {
		case CS_SUBCODE_SET_MODE_ANN: processSetModeAnnounce(msg); break;
		case CS_SUBCODE_GET_NODE_LIST: processGetNodeList(msg); break;
		case CS_SUBCODE_ADD_NODE: processAddNode(msg); break;
		case CS_SUBCODE_REMOVE_NODE: processRemoveNode(msg); break;
		default: result = false;
	}
	return result;
}
#endif
