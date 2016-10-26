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

#include "Cosa/Pin.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/UART.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkSerial/NetworkSerial.h"
#include "Utils/SerialMessageAdapter.h"

namespace Meshwork {

	namespace L4 {

		class ControllerBaseSerial : public Meshwork::L3::NetworkV1::NetworkSerial {

		public:
			static const uint8_t CS_CODE							= 2;

			static const uint8_t CS_SUBCODE_OK 						= 0;
			static const uint8_t CS_SUBCODE_NOK 					= 1;
			static const uint8_t CS_SUBCODE_UNKNOWN 				= 2;

			static const uint8_t CS_SUBCODE_SET_MODE_ANN			= 96;
			static const uint8_t CS_SUBCODE_SET_MODE_ANN_RES		= 97;
			static const uint8_t CS_SUBCODE_GET_NODE_LIST			= 98;
			static const uint8_t CS_SUBCODE_GET_NODE_LIST_RES		= 99;
			static const uint8_t CS_SUBCODE_ADD_NODE				= 100;
			static const uint8_t CS_SUBCODE_ADD_NODE_RES			= 101;
			static const uint8_t CS_SUBCODE_REMOVE_NODE				= 102;
				
			//64-95
			static const uint8_t ERROR_MAX_NODES_REACHED 		= 64;
			static const uint8_t ERROR_NODE_INVALID 			= 65;
			static const uint8_t ERROR_NODE_INVALID_CONTROLLER 	= 66;
			
			struct data_setmodeann_t {
				uint8_t mode;
				uint32_t timeout;
			};
			
		protected:
			SerialMessageAdapter* m_adapter;
			ControllerBase* m_controllerBase;
			
			virtual bool processSetModeAnnounce(SerialMessageAdapter::serialmsg_t* msg) = 0;
			virtual bool processGetNodeList(SerialMessageAdapter::serialmsg_t* msg) = 0;
			virtual bool processAddNode(SerialMessageAdapter::serialmsg_t* msg) = 0;
			virtual bool processRemoveNode(SerialMessageAdapter::serialmsg_t* msg) = 0;

		public:
			ControllerBaseSerial(Meshwork::L3::Network* network, SerialMessageAdapter* adapter, ControllerBase* controllerBase):
				NetworkSerial(network, adapter),
				m_adapter(adapter),
				m_controllerBase(controllerBase)
			{ }
			
			virtual bool processOneMessageEx(SerialMessageAdapter::serialmsg_t* msg) = 0;
			
		};
	};
};
#endif
