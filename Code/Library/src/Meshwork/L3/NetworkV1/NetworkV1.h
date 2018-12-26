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
#ifndef __MESHWORK_L3_NETWORK_NETWORKV1_H__
#define __MESHWORK_L3_NETWORK_NETWORKV1_H__

#include "Meshwork.h"
#include "Cosa/Pin.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Meshwork/L3/Network.h"

#ifndef MW_LOG_NETWORKV1
	#define MW_LOG_NETWORKV1		MW_FULL_DEBUG
#endif

//This one is not really needed for production
#ifndef MW_SUPPORT_RADIO_LISTENER
	#define MW_SUPPORT_RADIO_LISTENER	false
#endif


 /*
 Payload structure:
 -----------------------
 { NWKID | DSTID | DSTPORT | } DATA_L2
 Not part of the passed payload and handled by the RF driver, but included here for completeness:
   NWKID = Network ID. 
   DSTID = Destination node ID
   DSTPORT = Destination port. 0-127 used by apps, 128-255 reserved for system
 
 DATA_L2 = SEQ | NWKCTRL | (ROUTE_INFO or FLOOD_INFO) | (DATA_L3)
 
 Seq = Sequence Number
 NWKCTRL = DELIVERY_DIRECT or DELIVERY_ROUTED or DELIVERY_FLOOD + ACK Flag
 ROUTE_INFO = Node Count X | Node 1 | … | Node X | breadcrumbs Field
 FLOOD_INFO = Discovered Node Count X | Src ID | … | Dst ID
 
 DATA_L3 = app-specific payload, which is optional
 
 -----------------------
 Details:
 1) DELIVERY_DIRECT: Singlecast
 NWKID | DSTID	| DSTPORT | SEQ | DELIVERY_DIRECT 			| (DataL3)
 
 2) DELIVERY_DIRECT: Broadcast
 NWKID | 0xFF	| DSTPORT | SEQ | DELIVERY_DIRECT 			| (DataL3)
 
 3) DELIVERY_DIRECT + ACK: Singlecast Only
 NWKID | DSTID	| DSTPORT | SEQ | DELIVERY_DIRECT + ACK 	| (DataL3)
 
 4) DELIVERY_ROUTED: Singlecast Only
 NWKID | DSTID	| DSTPORT | SEQ | DELIVERY_ROUTED			| ROUTE_INFO | (DataL3)
 NWKID | DSTID	| DSTPORT | SEQ | DELIVERY_ROUTED			| Node Count X | SRCID | Node 1 | … | Node X | DSTID | breadcrumbs Field | (DataL3)
 
 5) DELIVERY_ROUTED + ACK: Singlecast Only
 NWKID | DSTID	| DSTPORT | SEQ | DELIVERY_ROUTED + ACK		| ROUTE_INFO | (DataL3)
 NWKID | DSTID	| DSTPORT | SEQ | DELIVERY_ROUTED + ACK		| Node Count X | SRCID | Node 1 | … | Node X | DSTID | breadcrumbs Field | (DataL3)
 
 6) DELIVERY_FLOOD: Broadcast Only
 NWKID | 0xFF	| DSTPORT | SEQ | DELIVERY_FLOOD			| FLOOD_INFO | (DataL3)
 NWKID | 0xFF	| DSTPORT | SEQ | DELIVERY_FLOOD			| Node Count X | SRCID | Node 1 | … | Node X | DSTID | (DataL3)
 
 7) DELIVERY_FLOOD + ACK: Singlecast Only (used ONLY to enable fast fail of FLOOD sends)
 NWKID | DSTID	| DSTPORT | SEQ | DELIVERY_FLOOD + ACK		| <Empty>
*/

namespace Meshwork {

	namespace L3 {
	
		namespace NetworkV1 {
		
			class NetworkV1: public Meshwork::L3::Network {
					
			public:

				  struct nwk_ctrl_t {
					uint8_t seq;
					uint8_t delivery;
				  };
				  
#if MW_SUPPORT_DELIVERY_ROUTED
				  struct route_t {
					uint8_t hopCount;
					uint8_t src;
					uint8_t* hops;
					uint8_t dst;
				  };
				  
				  struct route_info_t {
					route_t route;
					uint8_t breadcrumbs;
				  };
				  
	#if MW_SUPPORT_DELIVERY_FLOOD
				  struct flood_info_t {
					route_t route;
				  };
	#endif
#endif
				  
				  struct msg_direct_t {
					nwk_ctrl_t nwk_ctrl;
					uint8_t dataLen;
					uint8_t* data;
				  };
				  
#if MW_SUPPORT_DELIVERY_ROUTED
				  struct msg_routed_t {
					nwk_ctrl_t nwk_ctrl;
					route_info_t route_info;
					uint8_t dataLen;
					uint8_t* data;
				  };
				  
	#if MW_SUPPORT_DELIVERY_FLOOD
				  struct msg_flood_t {
					nwk_ctrl_t nwk_ctrl;
					flood_info_t flood_info;
					uint8_t dataLen;
					uint8_t* data;
				  };
	#endif
#endif

				  union univmsg_any_t {
					nwk_ctrl_t nwk_ctrl;
					msg_direct_t msg_direct;
#if MW_SUPPORT_DELIVERY_ROUTED
					msg_routed_t msg_routed;
	#if MW_SUPPORT_DELIVERY_FLOOD
					msg_flood_t msg_flood;
	#endif
#endif
				  };
				  
				  typedef univmsg_any_t univmsg_t;
				  
#if MW_SUPPORT_DELIVERY_ROUTED
				  class RouteProvider {
				  public:
					  //may invalidate cache, if any
					  virtual void set_address(uint8_t src) = 0;
					  virtual uint8_t get_routeCount(uint8_t dst) = 0;
					  virtual route_t* get_route(uint8_t dst, uint8_t index) = 0;
					  virtual void route_found(route_t* route) = 0;
					  virtual void route_failed(route_t* route) = 0;
				  };
#endif

#if MW_SUPPORT_RADIO_LISTENER
				  class RadioListener {
				  public:
					  virtual void notify_send_begin(uint8_t origin, uint8_t next, uint8_t port, univmsg_t* msg) = 0;
					  virtual void notify_send_end(uint8_t origin, uint8_t next, uint8_t port, univmsg_t* msg, bool sent) = 0;
					  virtual void notify_recv_ack_begin() = 0;
					  virtual void notify_recv_ack_end(univmsg_t* msg, int result) = 0;
					  virtual void notify_recv_begin() = 0;
					  virtual void notify_recv_end(bool broadcast, uint8_t src, uint8_t port, univmsg_t* msg) = 0;
				  };

			#define MW_DECL_IF_SUPPORT_RADIO_LISTENER

			#define NOTIFY_SEND_BEGIN(origin, next, port, msg) \
				  do { if ( m_radio_listener != NULL ) { \
						  m_radio_listener->notify_send_begin(origin, next, port, msg); \
					  }; } while (0)

			#define NOTIFY_SEND_END(origin, next, port, msg, sent) \
				  do { if ( m_radio_listener != NULL ) { \
						  m_radio_listener->notify_send_end(origin, next, port, msg, sent); \
					  }; } while (0)

			#define NOTIFY_RECV_ACK_BEGIN() \
				  do { if ( m_radio_listener != NULL ) { \
						  m_radio_listener->notify_recv_ack_begin(); \
					  }; } while (0)

			#define NOTIFY_RECV_ACK_END(msg, result) \
				  do { if ( m_radio_listener != NULL ) { \
						  m_radio_listener->notify_recv_ack_end(msg, result); \
					  }; } while (0)

			#define NOTIFY_RECV_BEGIN() \
				  do {  if ( m_radio_listener != NULL ) { \
						  m_radio_listener->notify_recv_begin(); \
					  }; } while (0)

			#define NOTIFY_RECV_END(broadcast, src, port, msg) \
				  do { if ( m_radio_listener != NULL ) { \
						  m_radio_listener->notify_recv_end(broadcast, src, port, msg); \
					  }; } while (0)

#else

			#define MW_DECL_IF_SUPPORT_RADIO_LISTENER if (false) { };

			#define NOTIFY_SEND_BEGIN(origin, next, port, msg) \
				  do { } while (0)

			#define NOTIFY_SEND_END(origin, next, port, msg, sent) \
				  do { } while (0)

			#define NOTIFY_RECV_ACK_BEGIN() \
				  do { } while (0)

			#define NOTIFY_RECV_ACK_END(msg, result) \
				  do { } while (0)

			#define NOTIFY_RECV_BEGIN() \
				  do { } while (0)

			#define NOTIFY_RECV_END(broadcast, src, port, msg) \
				  do { } while (0)

#endif

			protected:
				
				/** Message sequence number. */
				uint8_t seq;
				
				//converts error codes from Cosa to Meshwork
				void convertError(int &error) {
					if ( error == ETIME ) {
						error = ERROR_RECV_TIMEOUT;
						MW_LOG_INFO(MW_LOG_NETWORKV1, "Receive timeout", NULL);
					} else if ( error == EMSGSIZE ) {
						error = ERROR_RECV_TOO_LONG;
						MW_LOG_INFO(MW_LOG_NETWORKV1, "Payload too long", NULL);
					}
				}

				static const uint8_t MAX_IOVEC_MSG_SIZE	= 8;
					
				///////////// DIRECT /////////////
				static iovec_t* get_iovec_msg_direct(iovec_t* vec, univmsg_t* msg) {
					iovec_t* vp = vec;
					iovec_arg(vp, &msg->msg_direct.nwk_ctrl, sizeof(msg->msg_direct.nwk_ctrl));
					iovec_arg(vp, msg->msg_direct.data, msg->msg_direct.dataLen);
					iovec_end(vp);
					return vec;
				}
				
				static univmsg_t* get_msg_direct(univmsg_t* msg, uint8_t* data, int len) {
					msg->msg_direct.nwk_ctrl.seq = data[0];
					msg->msg_direct.nwk_ctrl.delivery = data[1];
					msg->msg_direct.dataLen = len - 2;
					msg->msg_direct.data = data + 2;
					return msg;
				}
				
#if MW_SUPPORT_DELIVERY_ROUTED
				///////////// ROUTED  /////////////
				static iovec_t* get_iovec_msg_routed(iovec_t* vec, univmsg_t* msg) {
					iovec_t* vp = vec;
					iovec_arg(vp, &msg->msg_routed.nwk_ctrl, sizeof(msg->msg_routed.nwk_ctrl));
					iovec_arg(vp, &msg->msg_routed.route_info.route, sizeof(msg->msg_routed.route_info.route.hopCount)+sizeof(msg->msg_routed.route_info.route.src));
					if ( msg->msg_routed.route_info.route.hopCount > 0 )
						iovec_arg(vp, msg->msg_routed.route_info.route.hops, msg->msg_routed.route_info.route.hopCount);
					iovec_arg(vp, &msg->msg_routed.route_info.route.dst, sizeof(msg->msg_routed.route_info.route.dst));
					iovec_arg(vp, &msg->msg_routed.route_info.breadcrumbs, sizeof(msg->msg_routed.route_info.breadcrumbs));
					iovec_arg(vp, msg->msg_routed.data, msg->msg_routed.dataLen);
					iovec_end(vp);
					return vec;
				}

				static univmsg_t* get_msg_routed(univmsg_t* msg, uint8_t* data, int len) {
					uint8_t hopCount = 0;
					msg->msg_routed.nwk_ctrl.seq = data[0];
					msg->msg_routed.nwk_ctrl.delivery = data[1];
					msg->msg_routed.route_info.route.hopCount = hopCount = data[2];
					msg->msg_routed.route_info.route.src = data[3];
					msg->msg_routed.route_info.route.hops = hopCount == 0 ? NULL : data + 4;
					msg->msg_routed.route_info.route.dst = data[4 + hopCount];
					msg->msg_routed.route_info.breadcrumbs = data[5 + hopCount];	  
					msg->msg_routed.dataLen = len - 6 - hopCount;
					msg->msg_routed.data = data + 6 + hopCount;
					return msg;
				}

				static uint8_t get_msg_routed_hop_index(univmsg_t* msg, uint8_t id) {
					uint8_t result = -1;
					for ( int i = 0; i < msg->msg_routed.route_info.route.hopCount; i ++ )
						if ( msg->msg_routed.route_info.route.hops[i] == id ) {
							result = i;
							break;
						}
					return result;
				}
				
	#if MW_SUPPORT_DELIVERY_FLOOD
				///////////// FLOOD /////////////	
				static iovec_t* get_iovec_msg_flood(iovec_t* vec, univmsg_t* msg) {
					iovec_t* vp = vec;
					iovec_arg(vp, &msg->msg_flood.nwk_ctrl, sizeof(msg->msg_flood.nwk_ctrl));
					iovec_arg(vp, &msg->msg_flood.flood_info.route, sizeof(msg->msg_flood.flood_info.route.hopCount)+sizeof(msg->msg_flood.flood_info.route.src));
					if ( msg->msg_flood.flood_info.route.hopCount )
						iovec_arg(vp, msg->msg_flood.flood_info.route.hops, msg->msg_flood.flood_info.route.hopCount);
					iovec_arg(vp, &msg->msg_flood.flood_info.route.dst, sizeof(msg->msg_flood.flood_info.route.dst));
					iovec_arg(vp, msg->msg_flood.data, msg->msg_flood.dataLen);
					iovec_end(vp);
					return vec;
				}
				
				static univmsg_t* get_msg_flood(univmsg_t* msg, uint8_t* data, int len) {
					uint8_t hopCount = 0;
					msg->msg_flood.nwk_ctrl.seq = data[0];
					msg->msg_flood.nwk_ctrl.delivery = data[1];
					msg->msg_flood.flood_info.route.hopCount = hopCount = data[2];
					msg->msg_flood.flood_info.route.src = data[3];
					msg->msg_flood.flood_info.route.hops = hopCount == 0 ? NULL : data + 4;
					msg->msg_flood.flood_info.route.dst = data[4 + hopCount];
					msg->msg_flood.dataLen = len - 5 - hopCount;
					msg->msg_flood.data = data + 5 + hopCount;
					return msg;
				}
				
				static uint8_t get_msg_flood_hop_index(univmsg_t* msg, uint8_t id) {
					uint8_t result = -1;
					for ( int i = 0; i < msg->msg_flood.flood_info.route.hopCount; i ++ )
						if ( msg->msg_flood.flood_info.route.hops[i] == id ) {
							result = i;
							break;
						}
					return result;
				}
				
	#endif
#endif
				
				///////////// GENERIC /////////////
				static iovec_t* get_iovec_msg(iovec_t* vec, univmsg_t* msg) {
					if ( msg->nwk_ctrl.delivery & DELIVERY_DIRECT )
						return get_iovec_msg_direct(vec, msg);
#if MW_SUPPORT_DELIVERY_ROUTED
					else if ( msg->nwk_ctrl.delivery & DELIVERY_ROUTED )
						return get_iovec_msg_routed(vec, msg);
	#if MW_SUPPORT_DELIVERY_FLOOD
					else if ( msg->nwk_ctrl.delivery & DELIVERY_FLOOD )
						return get_iovec_msg_flood(vec, msg);
	#endif
#endif
					return NULL;
				}
				
				static univmsg_t* get_msg(univmsg_t* msg, uint8_t* data, int len) {
					if ( data[1] & DELIVERY_DIRECT )
						return get_msg_direct(msg, data, len);
#if MW_SUPPORT_DELIVERY_ROUTED
					else if ( data[1] & DELIVERY_ROUTED )
						return get_msg_routed(msg, data, len);
	#if MW_SUPPORT_DELIVERY_FLOOD
					else if ( data[1] & DELIVERY_FLOOD )
						return get_msg_flood(msg, data, len);
	#endif
#endif
					return msg;
				}
				
				static uint8_t* get_msg_payload(univmsg_t* msg) {
					if ( msg->nwk_ctrl.delivery & DELIVERY_DIRECT )
						return msg->msg_direct.data;
#if MW_SUPPORT_DELIVERY_ROUTED
					else if ( msg->nwk_ctrl.delivery & DELIVERY_ROUTED )
						return msg->msg_routed.data;
	#if MW_SUPPORT_DELIVERY_FLOOD
					else if ( msg->nwk_ctrl.delivery & DELIVERY_FLOOD )
						return msg->msg_flood.data;
	#endif
#endif
					return NULL;
				}

				static uint8_t get_msg_payload_len(univmsg_t* msg) {
					if ( msg->nwk_ctrl.delivery & DELIVERY_DIRECT )
						return msg->msg_direct.dataLen;
#if MW_SUPPORT_DELIVERY_ROUTED
					else if ( msg->nwk_ctrl.delivery & DELIVERY_ROUTED )
						return msg->msg_routed.dataLen;
	#if MW_SUPPORT_DELIVERY_FLOOD
					else if ( msg->nwk_ctrl.delivery & DELIVERY_FLOOD )
						return msg->msg_flood.dataLen;
	#endif
#endif
					return 0;
				}
				
			protected:

#if MW_SUPPORT_DELIVERY_ROUTED
				RouteProvider* m_advisor;
				uint8_t m_maxHops;
#endif

#if MW_SUPPORT_RADIO_LISTENER
				RadioListener* m_radio_listener;
#endif

				bool sendWithoutACK(uint8_t dest, uint8_t hopPort, iovec_t* vp, uint8_t attempts);
				bool sendWithoutACK(uint8_t dest, uint8_t hopPort, const void* buf, size_t len, uint8_t attempts);

				Network::msg_l3_status_t sendWithACK(uint8_t attempts, uint16_t attemptsDelay,
					uint8_t ack, uint32_t ackTimeout,
					uint8_t dest, uint8_t port,
					univmsg_t* msg,
					void* bufACK, size_t& maxACKLen
#if MW_SUPPORT_DELIVERY_ROUTED
					, route_t* returnRoute, size_t& returnRouteSize
#endif
					);

#if MW_SUPPORT_DELIVERY_ROUTED
				Network::msg_l3_status_t sendRoutedACK(Meshwork::L3::Network::ACKProvider* ackProvider,
									univmsg_t* msg, uint8_t hopSrc, uint8_t hopPort);
#endif
				Network::msg_l3_status_t sendDirectACK(Meshwork::L3::Network::ACKProvider* ackProvider,
									univmsg_t* msg, uint8_t hopSrc, uint8_t hopPort);

			public:
				/** The maximum payload length. */
				static const uint8_t PAYLOAD_MAX = 16;
				/** The maximum ACK payload length. */
				static const uint8_t ACK_PAYLOAD_MAX = 8;
				/** Default value for additional send retries. */
				static const uint8_t DEFAULT_SEND_RETRY = 2;

				/** Network Control byte's ACK flag. */
				static const uint8_t ACK = 128;
				/** Timeout for single ACK receive when sending. */
				static const uint16_t TIMEOUT_ACK_RECEIVE = (uint16_t) 500;
				/** Maximum timeout for DIRECT ACK when sending, including retries. */
				static const uint16_t TIMEOUT_ACK_DIRECT = (uint16_t) TIMEOUT_ACK_RECEIVE * (DEFAULT_SEND_RETRY + 1);
				/** Wait period before DIRECT delivery retry. */
				static const uint16_t RETRY_WAIT_DIRECT = (uint16_t) TIMEOUT_ACK_RECEIVE;
				
#if MW_SUPPORT_DELIVERY_ROUTED
				/** Maximum routing hops for this network design. */
				static const uint8_t MAX_ROUTING_HOPS = 8;
				
				/** Timeout for ACK from ROUTED delivery send. */
				static const uint32_t TIMEOUT_ACK_ROUTED = (uint16_t) TIMEOUT_ACK_DIRECT * (MAX_ROUTING_HOPS + 0); //extra 0 spare cycles
				/** Wait period before ROUTED delivery retry. */
				static const uint32_t RETRY_WAIT_ROUTED = (uint16_t) TIMEOUT_ACK_RECEIVE;
				
	#if MW_SUPPORT_DELIVERY_FLOOD
				/** Timeout for ACK from FLOOD delivery send. */
				static const uint32_t TIMEOUT_ACK_FLOOD = (uint16_t) TIMEOUT_ACK_DIRECT * (MAX_ROUTING_HOPS + 2); //extra 2 spare cycles, just in case
				/** Wait period before FLOOD delivery retry. */
				static const uint32_t RETRY_WAIT_FLOOD = (uint16_t) TIMEOUT_ACK_RECEIVE;

	#endif
#endif

				NetworkV1(Wireless::Driver* driver,
#if MW_SUPPORT_DELIVERY_ROUTED
						RouteProvider* advisor = NULL,
#endif
						uint8_t nwkcaps = NWKCAPS_ROUTER,
						uint8_t delivery = DELIVERY_EXHAUSTIVE,
#if MW_SUPPORT_DELIVERY_ROUTED
						uint8_t maxHops = MAX_ROUTING_HOPS,
#endif
						uint8_t retry = DEFAULT_SEND_RETRY):
							Meshwork::L3::Network(driver, nwkcaps, delivery, retry)
#if MW_SUPPORT_DELIVERY_ROUTED
							, m_advisor(advisor),
							m_maxHops(maxHops)
#endif

									{ seq = 0; };
				
#if MW_SUPPORT_DELIVERY_ROUTED
				RouteProvider* get_route_advisor() {
					return m_advisor;
				}
				void set_route_advisor(RouteProvider* provider) {
					m_advisor = provider;
				}
#endif
				
#if MW_SUPPORT_RADIO_LISTENER
				RadioListener* get_radio_listener() {
					return m_radio_listener;
				}
				void set_radio_listener(RadioListener* radio_listener) {
					m_radio_listener = radio_listener;
				}
#endif

				bool begin(const void* config = NULL);
				bool end();
				Network::msg_l3_status_t send(uint8_t delivery, uint8_t retry,
									uint8_t dest, uint8_t port,
									const void* buf, size_t len,
									void* bufACK, size_t& lenACK);
				
				Network::msg_l3_status_t recv(uint8_t& src, uint8_t& port, void* data, size_t& dataLenMax,
						uint32_t ms, Meshwork::L3::Network::ACKProvider* ackProvider);

			};
		};
	};
};
#endif

