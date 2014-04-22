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
#ifndef __MESHWORK_L3_NETWORK_H__
#define __MESHWORK_L3_NETWORK_H__

#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"

//#define SUPPORT_DELIVERY_ROUTED_DISABLE
//#define SUPPORT_DELIVERY_FLOOD_DISABLE
//#define SUPPORT_REROUTING_DISABLE

#ifndef SUPPORT_DELIVERY_ROUTED_DISABLE
#define SUPPORT_DELIVERY_ROUTED
#endif

#ifndef SUPPORT_DELIVERY_FLOOD_DISABLE
#define SUPPORT_DELIVERY_FLOOD
#endif

#ifndef SUPPORT_REROUTING_DISABLE
#define SUPPORT_REROUTING
#endif

namespace Meshwork {

	namespace L3 {
  
	  class Network {
	  public:

		class ACKProvider {
		public:
		  //returns 0 for no payload or number of bytes written in the payload buffer
		  virtual int returnACKPayload(uint8_t src, uint8_t port, void* buf, uint8_t len, void* bufACK, size_t lenACK) = 0;
		};//end of Meshwork::L3::Network::ACKProvider
		
		// Define networking capabilities.

		// Bits: 0, 1
		/** If set: sleeping node. Not set: always on*/
		static const uint8_t NWKCAPS_SLEEPING 			= 1;
		/** If set: periodically wakes up. Not set: always sleeping. Relevant only if NWKCAPS_SLEEPING is set. */
		static const uint8_t NWKCAPS_PERIODIC_WAKEUP 	= 2;

		// Bits: 2, 3
		/** If set: supports re-routing. Not set: doesn't re-route (typically an edge node). */
		static const uint8_t NWKCAPS_ROUTER				= 4;
		/** If set: acts as gateway to other networks.*/
		static const uint8_t NWKCAPS_GATEWAY 			= 8;

		// Bits: 4
		/** If set: can include new nodes in the network (aka a controller).*/
		static const uint8_t NWKCAPS_CONTROLLER			= 16;

		//define delivery methods
		/** Defines direct delivery. */
		static const uint8_t DELIVERY_DIRECT = 0x01;
		/** Defines routed message delivery. */
	#ifdef SUPPORT_DELIVERY_ROUTED
		static const uint8_t DELIVERY_ROUTED = 0x02;
	#ifdef SUPPORT_DELIVERY_FLOOD
		/** Defines flood routing message delivery. */
		static const uint8_t DELIVERY_FLOOD = 0x04;
	#endif
	#endif
		/** Defines exhaustive delivery approach. */
		static const uint8_t DELIVERY_EXHAUSTIVE =
								DELIVERY_DIRECT
	#ifdef SUPPORT_DELIVERY_ROUTED
								| DELIVERY_ROUTED
	#ifdef SUPPORT_DELIVERY_FLOOD
								| DELIVERY_FLOOD
	#endif
	#endif
								;

		//Positive values define success
		//Negative values define errors
		//Zero value undefined?
		
		//OK code group
		/** Message sent/received correctly. */
		static const int8_t OK = 1;
		/** Internal message received and can be ignored. */
		static const int8_t OK_MESSAGE_INTERNAL = 2;
		/** Message irrelevant for this node has been received and can be ignored. */
		static const int8_t OK_MESSAGE_IGNORED = 3;
		/** Warning, ACK message too long. */
		static const int8_t OK_WARNING_ACK_TOO_LONG = 4;
		
		//Wrong parameters code group
		/** Unknown delivery invalid or incompatible with destination. */
		static const int8_t ERROR_DELIVERY_METHOD_INVALID = -10;
		/** Payload to long for delivery. */
		static const int8_t ERROR_PAYLOAD_TOO_LONG = -11;
		/** Retry count is invalid. */
		static const int8_t ERROR_INVALID_RETRY_COUNT = -12;
		
		//Internal network errors code group
		/** Message ignored due to max hops reached. */
		static const int8_t ERROR_MESSAGE_IGNORED_MAX_HOPS_REACHED = -20;
		
		//ACK errors code group
		/** No acknowledge received. */
		static const int8_t ERROR_ACK_NOT_RECEIVED = -30;
		/** Sending an ACK has failed. */
		static const int8_t ERROR_ACK_SEND_FAILED = -31;
		
		//Routing errors code group
		/** No known routes to the destination. */
		static const int8_t ERROR_NO_KNOWN_ROUTES = -40;
		/** Rerouting a message has failed. */
		static const int8_t ERROR_REROUTE_FAILED = -41;

		//Send errors code group
		/** Driver send has failed. */
		static const int8_t ERROR_DRIVER_SEND_FAILED = -51;
		/** Send aborted by the app. */
		static const int8_t ERROR_DRIVER_SEND_ABORTED = -52;
		
		/** First possible node ID. */
		static const uint8_t MIN_NODE_ID 	= 1;
		/** Last possible node ID. */
		static const uint8_t MAX_NODE_ID 	= 254;
		/** Maximum node count in the network. */
		static const uint8_t MAX_NODE_COUNT = MAX_NODE_ID - MIN_NODE_ID + 1;
		
		/** Maximum length of a network key. */
		static const uint8_t MAX_NETWORK_KEY_LEN 	= 8;
		
		/** Worst QoS level */
		static const int8_t QOS_LEVEL_MIN 		= -100;
		/** Best QoS level */
		static const int8_t QOS_LEVEL_AVERAGE	=  0;
		/** Best QoS level */
		static const int8_t QOS_LEVEL_MAX 		=  100;
		/** Unknown QoS level */
		static const int8_t QOS_LEVEL_UNKNOWN 	=  -127;
		
		/** Calculate worst QoS level */
		static const int8_t QOS_CALCULATE_WORST 	= -1;
		/** Calculate average QoS level */
		static const int8_t QOS_CALCULATE_AVERAGE 	=  0;
		/** Calculate best QoS level */
		static const int8_t QOS_CALCULATE_BEST 		=  1;
		
		//protected fields
		protected:
			Wireless::Driver* m_driver;
			uint8_t m_nwkcaps;
			uint8_t m_delivery;
			uint8_t m_retry;
			uint8_t m_channel;
			char* m_networkKey;
			uint8_t m_networkKeyLen;
			bool m_sendAbort;

		//public constructor and functions
		public:

			Network(Wireless::Driver* driver,
						uint8_t nwkcaps = NWKCAPS_ROUTER,
							uint8_t delivery = DELIVERY_EXHAUSTIVE,
									uint8_t retry = 2) :
			  m_driver(driver),
			  m_nwkcaps(nwkcaps),
			  m_delivery(delivery),
			  m_retry(retry),
			  m_channel(0),
			  m_sendAbort(false)
			  {}

			Wireless::Driver* get_driver() {
			  return m_driver;
			}

			uint8_t getChannel() {
				return m_channel;//TODO update Cosa and change to m_driver->get_channel();
			}
			
			void setChannel(uint8_t channel) {
				m_channel = channel;//TODO update Cosa and remove the field entirely
//				m_driver->set_channel(channel);
			}
			
			uint8_t getNetworkCaps() {
				return m_nwkcaps;
			}
			
			void setNetworkCaps(uint8_t nwkcaps) {
				m_nwkcaps = nwkcaps;
			}
			
			uint8_t getDelivery() {
				return m_delivery;
			}
			
			void setDelivery(uint8_t delivery) {
				m_delivery = delivery;
			}
			
			uint8_t getRetry() {
				return m_retry;
			}
			
			void setRetry(uint8_t retry) {
				m_retry = retry;
			}
			
			int16_t getNetworkID() {
				return m_driver->get_network_address();
			}
			
			void setNetworkID(int16_t networkID) {
				m_driver->set_address(networkID, m_driver->get_device_address());
			}
			
			uint8_t getNodeID() {
				return m_driver->get_device_address();
			}
			
			void setNodeID(uint8_t networkID) {
				m_driver->set_address(m_driver->get_network_address(), networkID);
			}
						
			char* getNetworkKey() {
				return m_networkKey;
			}
			
			void setNetworkKey(char* networkKey) {
				m_networkKey = networkKey;
			}
			
			uint8_t getNetworkKeyLen() {
				return m_networkKeyLen;
			}
			
			void setNetworkKeyLen(uint8_t networkKeyLen) {
				m_networkKeyLen = networkKeyLen;
			}
			
			virtual void reset() {
				setNodeID(0);
				setNetworkID(0x0000);
				setNetworkKey(NULL);
				setChannel(0);
			}
			
			virtual bool begin(const void* config = NULL) {
				return m_driver == NULL ? false : m_driver->begin();
			}

			virtual bool end() {
				return m_driver == NULL ? false : m_driver->end();
			}

			void sendAbort() {
				m_sendAbort = true;
			}
			
			//main send method
			virtual int send(uint8_t delivery, uint8_t retry,
								uint8_t dest, uint8_t port,
								const void* buf, size_t len,
								void* bufACK, size_t& lenACK);

			//convenience send method
			int send(uint8_t dest, uint8_t port,
						const void* buf, size_t len,
						void* bufACK, size_t& maxACKLen) {
				return send(m_delivery, m_retry, dest, port, buf, len, bufACK, maxACKLen);
			}
			
			//convenience broadcast method
			virtual int broadcast(uint8_t port, const void* buf, size_t len) {
				size_t none = 0;
				return send(Wireless::Driver::BROADCAST, port, buf, len, NULL, none);
			}

			//main recv method
			virtual int recv(uint8_t& src, uint8_t& port, void* data, size_t& dataLenMax,
					uint32_t ms, Meshwork::L3::Network::ACKProvider* ackProvider);
		
		};//end of Meshwork::L3::Network
		
	};//end of Meshwork::L3
	
};//end of Meshwork
#endif