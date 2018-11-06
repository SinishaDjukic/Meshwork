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

namespace Meshwork {

	namespace L4 {

		class NodeBase {
		protected:
			Meshwork::L3::Network* m_network;
			uint8_t m_mode;
			
			virtual int setModeRequestImpl(uint8_t mode, uint32_t timeout) = 0;
			
		public:
			static const uint8_t MODE_NORMAL	= 0;
			static const uint8_t MODE_ADDING	= 1;
			static const uint8_t MODE_REMOVING	= 2;
			
			static const int ERROR_UNKNOWN		= -1;
			static const int ERROR_UNKNOWN_MODE	= -2;
			static const int ERROR_ILLEGAL_MODE	= -3;
			
			NodeBase(Meshwork::L3::Network* network):
				m_network(network),
				m_mode(MODE_NORMAL)
			{}
			
			virtual void resetNode(); //factory reset
			
			virtual uint8_t getNodeID();
			virtual void setNodeID(uint8_t nodeID);
			
			virtual uint8_t getModeRequest();
			virtual int setModeRequest(uint8_t mode, uint32_t timeout);
			
		};//end of Meshwork::L4::NodeBase

	}//end of Meshwork:L4
	
};//end of Meshwork
#endif