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
#ifndef __MESHWORK_L4_CONTROLLERBASE_CPP__
#define __MESHWORK_L4_CONTROLLERBASE_CPP__

#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Power.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L4/NodeBase.h"
#include "Meshwork/L4/ControllerBase.h"

int Meshwork::L4::ControllerBase::addNode(uint8_t nodeID) {
	if ( nodeList.members() == Meshwork::L3::Network::MAX_NODE_COUNT )
		return ERROR_MAX_NODES_REACHED;
	else if ( nodeID < Meshwork::L3::Network::MIN_NODE_ID || nodeID > Meshwork::L3::Network::MAX_NODE_ID )
		return ERROR_NODE_INVALID;
	uint8_t controllerID = m_network->getNodeID();
	if ( nodeID == controllerID )
		return ERROR_NODE_INVALID_CONTROLLER;
	if ( nodeList[nodeID] ) {
		for ( uint8_t i = Meshwork::L3::Network::MIN_NODE_ID; i < Meshwork::L3::Network::MAX_NODE_ID; i ++ )
			if ( i != controllerID && !nodeList[i] ) {
				nodeList += i;
				nodeID = i;
				break;
			}
	} else {
		nodeList += nodeID;
	}
	return nodeID;
}

int Meshwork::L4::ControllerBase::removeNode(uint8_t nodeID) {
	if ( nodeID < Meshwork::L3::Network::MIN_NODE_ID || nodeID > Meshwork::L3::Network::MAX_NODE_ID )
		return ERROR_NODE_INVALID;
	uint8_t controllerID = m_network->getNodeID();
	if ( nodeID == controllerID )
		return ERROR_NODE_INVALID_CONTROLLER;
	if ( nodeList[nodeID] ) {
		nodeList -= nodeID;
	} else {
		nodeID = ERROR_NODE_INVALID;
	}
	return nodeID;
}

void Meshwork::L4::ControllerBase::getNodeList(BitSet<Meshwork::L3::Network::MAX_NODE_COUNT>* destList) {
	*destList = nodeList;
}

uint8_t Meshwork::L4::ControllerBase::getNodeCount() {
	return (uint8_t) nodeList.members();
}

void Meshwork::L4::ControllerBase::getNextNode(uint8_t &start) {
	for ( int i = start; i <= Meshwork::L3::Network::MAX_NODE_ID; i ++ ) {
		if ( nodeList[i] ) {
			start = i;
			break;
		}
	}
	start = Meshwork::L3::Network::MAX_NODE_ID + 1;
}

void Meshwork::L4::ControllerBase::resetNode() {
	Meshwork::L4::NodeBase::resetNode();
	nodeList.empty();
}

int Meshwork::L4::ControllerBase::setModeRequestImpl(uint8_t mode, uint32_t timeout) {
	//TODO implement
	//TODO Once added to a network make sure we set a bit in the nodeList for our own nodeID
	return 0;
}

int Meshwork::L4::ControllerBase::setModeAnnounce(uint8_t mode, uint32_t timeout) {
	//TODO implement
	return -1;
}
#endif
