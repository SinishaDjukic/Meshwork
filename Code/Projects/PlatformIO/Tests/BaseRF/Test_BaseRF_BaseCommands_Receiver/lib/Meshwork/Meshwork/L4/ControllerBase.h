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
#ifndef __MESHWORK_L4_CONTROLLERBASE_H__
#define __MESHWORK_L4_CONTROLLERBASE_H__

#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/BitSet.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L4/NodeBase.h"

namespace Meshwork {

	namespace L4 {
	
		class ControllerBase: NodeBase {
		protected:
			BitSet<Meshwork::L3::Network::MAX_NODE_COUNT> nodeList;
			virtual int setModeRequestImpl(uint8_t mode, uint32_t timeout);
		
		public:
			static const int ERROR_MAX_NODES_REACHED		= -64;//(add) MAX_NODE_COUNT reached
			static const int ERROR_NODE_INVALID				= -65;//(remove) given node does not exist
			static const int ERROR_NODE_INVALID_CONTROLLER	= -66;//(remove) cannot remove self

			virtual int addNode(uint8_t nodeID);//return new node ID given a desired ID (255 for any) or <0 for errors
			virtual int removeNode(uint8_t nodeID);//returns node ID 0 for success, <0 for errors
			virtual int setModeAnnounce(uint8_t mode, uint32_t timeout);//returns the added node ID or <0 for timeout
			
			virtual void getNodeList(BitSet<Meshwork::L3::Network::MAX_NODE_COUNT>* destList);//fill in the bitmask for up to maxNodes and return node count
			virtual uint8_t getNodeCount();
			virtual void getNextNode(uint8_t &start); //start inclusive

			virtual void resetNode();//override
			
		};//end of Meshwork::L4::ControllerBase
		
	}//end of Meshwork:L4
	
};//end of Meshwork
#endif
