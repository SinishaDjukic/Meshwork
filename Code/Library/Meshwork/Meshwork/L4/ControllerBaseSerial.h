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
#ifndef __MESHWORK_L4_CONTROLLERBASESERIAL_H__
#define __MESHWORK_L4_CONTROLLERBASESERIAL_H__

#include "Cosa/Pins.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkSerial.h"

namespace Meshwork {

	namespace L4 {

		class ControllerBaseSerial : public Meshwork::L3::NetworkSerial {

		public:
			static const uint8_t MSGCODE_SET_MODE_ANN			= 96;
			static const uint8_t MSGCODE_GET_NODE_LIST			= 97;
			static const uint8_t MSGCODE_ADD_NODE				= 98;
			static const uint8_t MSGCODE_REMOVE_NODE			= 99;
				
			struct data_setmodeann_t {
				uint8_t mode;
				uint32_t* timeout;
			};
			
		protected:
			
			virtual bool processSetModeAnnounce(serialmsg_t* msg);
			virtual bool processGetNodeList(serialmsg_t* msg);
			virtual bool processAddNode(serialmsg_t* msg);
			virtual bool processRemoveNode(serialmsg_t* msg);

		public:
			ControllerBaseSerial(Meshwork::L3::Network* network, UART* serial):
				NetworkSerial(network, serial)
			{ }
			
			virtual bool processOneMessage(serialmsg_t* msg);
			
		};
	};
};
#endif