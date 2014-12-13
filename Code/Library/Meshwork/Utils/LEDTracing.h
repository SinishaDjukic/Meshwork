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
#ifndef __MESHWORK_UTILS_LEDTRACING_H__
#define __MESHWORK_UTILS_LEDTRACING_H__

#include "Cosa/OutputPin.hh"
#include "Cosa/Wireless.hh"
#include "Meshwork.h"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

using namespace Meshwork::L3::NetworkV1;

/**
 * This class will blink one of the three LEDs when:
 * 1) We are sending a message (whether BCAST, SCAST or ACK)
 * 2) We have received an ACK
 * 3) We have received a message
 *
 * The simple 3xLED user interface doesn't allow us to differentiate between
 * failed sends, BCAST vs SCAST, sending message vs sending ACK, although
 * this is all possible with the NetworkV1::RadioListener interface.
 *
 * On the other hand, the below implementation should nicely visualize
 * how a message travels throughout the network, and how ACKs are returned.
 */
class LEDTracing: public NetworkV1::RadioListener {

	protected:
		NetworkV1* m_mesh;
		OutputPin* m_pin_send;
		OutputPin* m_pin_recv;
		OutputPin* m_pin_ack;

	public:
		LEDTracing(NetworkV1* mesh, OutputPin* pin_send, OutputPin* pin_recv, OutputPin* pin_ack):
			m_mesh(mesh),
			m_pin_send(pin_send),
			m_pin_recv(pin_recv),
			m_pin_ack(pin_ack)
		{
			m_pin_send->off();
			m_pin_recv->off();
			m_pin_ack->off();
		};

		void notify_send_begin(uint8_t origin, uint8_t next, uint8_t port, NetworkV1::univmsg_t* msg) {
			//Here, a real send starts (whether BCAST, SCAST or ACK)
			m_pin_send->on();
		}

		void notify_send_end(uint8_t origin, uint8_t next, uint8_t port, NetworkV1::univmsg_t* msg, bool sent) {
			Meshwork::Time::delay(200);//will be multiplied by MW_DELAY_FACTOR
			m_pin_send->off();
		}

		void notify_recv_ack_begin() {
			//Do nothing here
			//This may be called multiple times until we actually get a message,
			//so we ignore it for our LEDs. See notify_recv_end
		}

		void notify_recv_ack_end(NetworkV1::univmsg_t* msg, int result) {
			m_pin_ack->on();\
			Meshwork::Time::delay(200);//will be multiplied by MW_DELAY_FACTOR
			m_pin_ack->off();
		}

		void notify_recv_begin() {
			//Do nothing here
			//This may be called multiple times until we actually get a message,
			//so we ignore it for our LEDs. See notify_recv_end
		}

		void notify_recv_end(bool broadcast, uint8_t src, uint8_t port, NetworkV1::univmsg_t* msg) {
			m_pin_recv->on();
			Meshwork::Time::delay(200);//will be multiplied by MW_DELAY_FACTOR
			m_pin_recv->off();
		}
};

#endif
