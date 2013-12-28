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
#include "Meshwork/L3/Network.h"
#include "Meshwork/L4/NodeBase.h"

uint8_t Meshwork::L4::NodeBase::getNodeID() {
	return m_network->getNodeID();
}

void Meshwork::L4::NodeBase::setNodeID(uint8_t nodeID) {
	m_network->end();
	m_network->setNodeID(nodeID);
	m_network->begin();
}

void Meshwork::L4::NodeBase::resetNode() {
	m_network->end();
	m_mode = MODE_NORMAL;
	m_network->reset();
	m_network->begin();
}

uint8_t Meshwork::L4::NodeBase::getModeRequest() {
	return m_mode;
}

int Meshwork::L4::NodeBase::setModeRequest(uint8_t mode, uint32_t timeout) {
	if ( mode != MODE_NORMAL && mode != MODE_ADDING && mode != MODE_REMOVING )
		return ERROR_UNKNOWN_MODE;
	uint8_t nodeID = m_network->getNodeID();
	if ( mode == MODE_ADDING && nodeID != 0 ||
			mode == MODE_REMOVING && nodeID == 0 )
		return ERROR_ILLEGAL_MODE;
	return setModeRequestImpl(mode, timeout);
}
