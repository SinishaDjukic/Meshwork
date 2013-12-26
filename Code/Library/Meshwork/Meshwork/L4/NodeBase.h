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
#ifndef __MESHWORK_L4_NODEBASE_H__
#define __MESHWORK_L4_NODEBASE_H__

#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/BitSet.hh"
#include "Meshwork/L3/Network.h"

//Meshwork::L4::NodeBase
namespace Meshwork {

	namespace L4 {

		struct NetworkInfo {
			uint16_t networkID;
			char* key;
		};
	
		class NodeBase {
		protected:
			NetworkInfo networkInfo;
			uint8_t nodeID;
			uint8_t mode;
			
		public:
			static const uint8_t MODE_NORMAL	= 0;
			static const uint8_t MODE_ADDING	= 1;
			static const uint8_t MODE_REMOVING	= 2;
			
			static const int ERROR_UNKNOWN	= -1;
			
			virtual void getNetworkInfo(NetworkInfo* &nwkInfo) = 0;
			virtual int setNetworkInfo(NetworkInfo* nwkInfo) = 0;
			
			virtual void resetNode() = 0; //factory reset
			
			virtual uint8_t getNodeID() = 0;
			virtual void setNodeID(uint8_t nodeID) = 0;
			
			virtual uint8_t getModeRequest() = 0;
			virtual int setModeRequest(uint8_t mode, uint32_t timeout) = 0;
			
		};//end of Meshwork::L4::NodeBase

		class SlaveBase: NodeBase {
		public:


		};//end of Meshwork::L4::SlaveBase

		class ControllerBase: NodeBase {
		protected:
			BitSet<Meshwork::L3::Network::MAX_NODE_COUNT> nodeList;
		
		public:
			static const int ERROR_MAX_NODES_REACHED		= -64;//(add) MAX_NODE_COUNT reached
			static const int ERROR_NODE_INVALID				= -65;//(remove) given node does not exist
			static const int ERROR_NODE_INVALID_CONTROLLER	= -66;//(remove) cannot remove self

			virtual int addNode(uint8_t nodeID) = 0;//return new node ID given a desired ID (255 for any) or <0 for errors
			virtual int removeNode(uint8_t nodeID) = 0;//return removes node ID <0 for errors
			
			virtual void getNodeList(BitSet<Meshwork::L3::Network::MAX_NODE_COUNT> &nodeList) = 0;//fill in the bitmask for up to maxNodes and return node count
			virtual uint8_t getNodeCount() = 0;
			virtual int getNextNode(uint8_t &start) = 0; //start inclusive

		};//end of Meshwork::L4::ControllerBase
		
	}//end of Meshwork:L4
	
};//end of Meshwork
#endif