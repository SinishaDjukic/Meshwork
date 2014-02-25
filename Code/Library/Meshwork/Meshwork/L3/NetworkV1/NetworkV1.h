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

#include "Cosa/Pins.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Meshwork/L3/Network.h"

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
 
 7) DELIVERY_FLOOD + ACK: Singlecast Only
 NWKID | DSTID	| DSTPORT | SEQ | DELIVERY_FLOOD + ACK		| FLOOD_INFO | (DataL3)
 NWKID | DSTID	| DSTPORT | SEQ | DELIVERY_FLOOD + ACK		| Node Count X | SRCID | Node 1 | … | Node X | DSTID | (DataL3)
*/

//TODO fix namespaces of all extending classes
class NetworkV1: public Meshwork::L3::Network {
		
public:

	  struct nwk_ctrl_t {
		uint8_t seq;
		uint8_t delivery;
	  };
	  
#ifdef SUPPORT_DELIVERY_ROUTED
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
	  
#ifdef SUPPORT_DELIVERY_FLOOD
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
	  
#ifdef SUPPORT_DELIVERY_ROUTED
	  struct msg_routed_t {
	    nwk_ctrl_t nwk_ctrl;
		route_info_t route_info;
		uint8_t dataLen;
		uint8_t* data;
	  };
	  
#ifdef SUPPORT_DELIVERY_FLOOD
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
#ifdef SUPPORT_DELIVERY_ROUTED
	    msg_routed_t msg_routed;
#ifdef SUPPORT_DELIVERY_FLOOD
	    msg_flood_t msg_flood;
#endif
#endif
	  };
	  
	  typedef univmsg_any_t univmsg_t;
	  
#ifdef SUPPORT_DELIVERY_ROUTED
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

protected:
	
	/** Message sequence number. */
	uint8_t seq;
	
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
	
#ifdef SUPPORT_DELIVERY_ROUTED
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
	
#ifdef SUPPORT_DELIVERY_FLOOD
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
#ifdef SUPPORT_DELIVERY_ROUTED
		else if ( msg->nwk_ctrl.delivery & DELIVERY_ROUTED )
			return get_iovec_msg_routed(vec, msg);
#ifdef SUPPORT_DELIVERY_FLOOD
		else if ( msg->nwk_ctrl.delivery & DELIVERY_FLOOD )
			return get_iovec_msg_flood(vec, msg);
#endif
#endif
		return NULL;
	}
	
	static univmsg_t* get_msg(univmsg_t* msg, uint8_t* data, int len) {
		if ( data[1] & DELIVERY_DIRECT )
			return get_msg_direct(msg, data, len);
#ifdef SUPPORT_DELIVERY_ROUTED
		else if ( data[1] & DELIVERY_ROUTED )
			return get_msg_routed(msg, data, len);
#ifdef SUPPORT_DELIVERY_FLOOD
		else if ( data[1] & DELIVERY_FLOOD )
			return get_msg_flood(msg, data, len);
#endif
#endif
		return msg;
	}
	
	static uint8_t* get_msg_payload(univmsg_t* msg) {
		if ( msg->nwk_ctrl.delivery & DELIVERY_DIRECT )
			return msg->msg_direct.data;
#ifdef SUPPORT_DELIVERY_ROUTED
		else if ( msg->nwk_ctrl.delivery & DELIVERY_ROUTED )
			return msg->msg_routed.data;
#ifdef SUPPORT_DELIVERY_FLOOD
		else if ( msg->nwk_ctrl.delivery & DELIVERY_FLOOD )
			return msg->msg_flood.data;
#endif
#endif
		return NULL;
	}

	static uint8_t get_msg_payload_len(univmsg_t* msg) {
		if ( msg->nwk_ctrl.delivery & DELIVERY_DIRECT )
			return msg->msg_direct.dataLen;
#ifdef SUPPORT_DELIVERY_ROUTED
		else if ( msg->nwk_ctrl.delivery & DELIVERY_ROUTED )
			return msg->msg_routed.dataLen;
#ifdef SUPPORT_DELIVERY_FLOOD
		else if ( msg->nwk_ctrl.delivery & DELIVERY_FLOOD )
			return msg->msg_flood.dataLen;
#endif
#endif
		return NULL;
	}
	
protected:
#ifdef SUPPORT_DELIVERY_ROUTED
    RouteProvider* m_advisor;
	uint8_t m_maxHops;
#endif

	bool sendWithoutACK(uint8_t dest, uint8_t hopPort, iovec_t* vp, uint8_t attempts);
	bool sendWithoutACK(uint8_t dest, uint8_t hopPort, const void* buf, size_t len, uint8_t attempts);

	int sendWithACK(uint8_t attempts, uint16_t attemptsDelay,
		uint8_t ack, uint32_t ackTimeout,
		uint8_t dest, uint8_t port,
		univmsg_t* msg,
		void* bufACK, size_t& maxACKLen
#ifdef SUPPORT_DELIVERY_ROUTED
		, void* returnRoute, size_t& returnRouteSize
#endif
		);

#ifdef SUPPORT_DELIVERY_ROUTED
	int sendRoutedACK(Meshwork::L3::Network::ACKProvider* ackProvider,
						univmsg_t* msg, uint8_t hopSrc, uint8_t hopPort);
#endif
	int sendDirectACK(Meshwork::L3::Network::ACKProvider* ackProvider,
						univmsg_t* msg, uint8_t hopSrc, uint8_t hopPort);

public:
	/** The maximum payload length. */
	static const uint8_t PAYLOAD_MAX = 16;
	/** The maximum ACK payload length. */
	static const uint8_t ACK_PAYLOAD_MAX = 8;

	/** Network Control byte's ACK flag. */
	static const uint8_t ACK = 128;
	/** Timeout for single ACK receive. */
	static const uint32_t TIMEOUT_ACK_RECEIVE = (uint16_t) 500;
	/** Timeout for ACK from DIRECT delivery send. */
	static const uint32_t TIMEOUT_ACK_DIRECT = (uint16_t) 1000;
	/** Wait period before DIRECT delivery retry. */
	static const uint32_t RETRY_WAIT_DIRECT = (uint16_t) TIMEOUT_ACK_DIRECT * 2;
	
#ifdef SUPPORT_DELIVERY_ROUTED
	/** Maximum routing hops for this network design. */
	static const uint8_t MAX_ROUTING_HOPS = 8;
	
	/** Timeout for ACK from ROUTED delivery send. */
	static const uint32_t TIMEOUT_ACK_ROUTED = (uint16_t) TIMEOUT_ACK_DIRECT * (8 + 0); //max 8 hops + 0 spare cycles
	/** Wait period before ROUTED delivery retry. */
	static const uint32_t RETRY_WAIT_ROUTED = (uint16_t) TIMEOUT_ACK_DIRECT * 3;
	
#ifdef SUPPORT_DELIVERY_FLOOD
	/** Timeout for ACK from FLOOD delivery send. */
	static const uint32_t TIMEOUT_ACK_FLOOD = (uint16_t) TIMEOUT_ACK_DIRECT * (8 + 2); //max 8 hops + 2 spare cycles, just in case
	/** Wait period before FLOOD delivery retry. */
	static const uint32_t RETRY_WAIT_FLOOD = (uint16_t) TIMEOUT_ACK_DIRECT * 4;

#endif
#endif

	NetworkV1(Wireless::Driver* driver,
#ifdef SUPPORT_DELIVERY_ROUTED
			RouteProvider* advisor = NULL,
#endif
			uint8_t nwkcaps = ROLE_ROUTER_NODE,
			uint8_t delivery = DELIVERY_EXHAUSTIVE,
#ifdef SUPPORT_DELIVERY_ROUTED
			uint8_t maxHops = MAX_ROUTING_HOPS,
#endif
			uint8_t retry = 2):
				Meshwork::L3::Network(driver, nwkcaps, delivery, retry)
#ifdef SUPPORT_DELIVERY_ROUTED
				, m_advisor(advisor),
				m_maxHops(maxHops)
#endif

						{ seq = 0; };
	
	virtual RouteProvider* get_route_advisor() {
		return m_advisor;
	}
	virtual void set_route_advisor(RouteProvider* provider) {
		m_advisor = provider;
	}
	
	virtual bool begin(const void* config = NULL);
	virtual bool end();
	virtual int send(uint8_t delivery, uint8_t retry,
						uint8_t dest, uint8_t port,
						const void* buf, size_t len,
						void* bufACK, size_t& lenACK);
	
	virtual int recv(uint8_t& src, uint8_t& port, void* data, size_t& dataLenMax,
			uint32_t ms, Meshwork::L3::Network::ACKProvider* ackProvider);

};
#endif

