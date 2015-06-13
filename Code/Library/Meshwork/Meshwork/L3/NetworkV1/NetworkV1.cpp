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
#ifndef __MESHWORK_L3_NETWORK_NETWORKV1_CPP__
#define __MESHWORK_L3_NETWORK_NETWORKV1_CPP__

#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Power.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Meshwork.h"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

bool Meshwork::L3::NetworkV1::NetworkV1::sendWithoutACK(uint8_t dest, uint8_t hopPort, iovec_t* vp, uint8_t attempts) {
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Send to: %d:%d", dest, hopPort);
	int sendCode = -1;
	for (int i = 0; i < attempts && sendCode < 0; i ++) {
		if ((sendCode = m_driver->send(dest, hopPort, vp)) < 0) {//send back ACK
			MW_LOG_ERROR(MW_LOG_NETWORKV1, "Driver send failed: %d", sendCode);
			if ( m_sendAbort )
				break;
			if ( i < attempts - 1 )//don't sleep after last send
				Meshwork::Time::delay(RETRY_WAIT_DIRECT);
		}
	}
	if ( sendCode >= 0 ) {
		MW_LOG_INFO(MW_LOG_NETWORKV1, "Sent", NULL);
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKV1, "Send failed", NULL);
	}
	return sendCode >= 0;
}

bool Meshwork::L3::NetworkV1::NetworkV1::sendWithoutACK(uint8_t dest, uint8_t hopPort, const void* buf, size_t len, uint8_t attempts) {
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Send to: %d:%d", dest, hopPort);
	int sendCode = -1;
	for (int i = 0; i < attempts && sendCode < 0; i ++) {
		if ((sendCode = m_driver->send(dest, hopPort, buf, len)) < 0) {//send back ACK
			MW_LOG_ERROR(MW_LOG_NETWORKV1, "Driver send failed, code: %d", sendCode);
			if ( m_sendAbort )
				break;
			if ( i < attempts - 1 )//don't sleep after last send
				Meshwork::Time::delay(RETRY_WAIT_DIRECT);
		}
	}
	if ( sendCode >= 0 ) {
		MW_LOG_INFO(MW_LOG_NETWORKV1, "Sent", NULL);
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKV1, "Send failed", NULL);
	}
	return sendCode >= 0;
}

//dest should be the next immediate hop
int Meshwork::L3::NetworkV1::NetworkV1::sendWithACK(uint8_t attempts, uint16_t attemptsDelay,
						uint8_t ack, uint32_t ackTimeout,
						uint8_t dest, uint8_t port,
						univmsg_t* msg,
						void* bufACK, size_t& maxACKLen
#if MW_SUPPORT_DELIVERY_ROUTED
						, route_t* returnRoute, size_t& returnRouteSize//returnRouteSize should be initialized with the MAX size of the buffer
#endif
						) {
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Send to: %d:%d, ACK=%d", dest, port, ack);
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "msg=%d, bufACK=%d, maxACKLen=%d", msg, bufACK, maxACKLen);
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "attempts=%d, attemptsDelay=%d, ackTimeout=%l", attempts, attemptsDelay, ackTimeout);
#if MW_SUPPORT_DELIVERY_ROUTED
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "returnRoute=%d, returnRouteSize=%d", returnRoute, returnRouteSize);
#endif
	
	int result = ack != 0 ? ERROR_ACK_NOT_RECEIVED : 0;
	
	iovec_t toSend[MAX_IOVEC_MSG_SIZE];
	iovec_t* vp = toSend;
	vp = get_iovec_msg(vp, msg);
	univmsg_t reply_msg;
	
	MW_LOG_DEBUG_VP_BYTES(MW_LOG_NETWORKV1, PSTR("L2 DATA TO SEND: "), toSend);
	
	for (int i = 0; i < attempts; i ++) {
#if MW_SUPPORT_DELIVERY_FLOOD
			bool oneFloodACK = false;
#endif

		MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_BEGIN(m_driver->get_device_address(), dest, port, msg);

		//Currently, we don't differentiate between regular fail and m_sendAbort within sendWithoutACK
		bool sent = sendWithoutACK(dest, port, vp, attempts);

		MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_END(m_driver->get_device_address(), dest, port, msg, sent);

		if ( !sent ) {
			result = Meshwork::L3::NetworkV1::NetworkV1::ERROR_DRIVER_SEND_FAILED;
		} else if (ack != 0) {//wait for ACK (covers the FLOOD case as well)
			MW_LOG_INFO(MW_LOG_NETWORKV1, "Wait ACK", NULL);
			uint8_t reply_src, reply_port, reply_len;
			int reply_result;
			//the next recv may come with an irrelevant message/data, so recv some more until timeout is reached
			uint32_t start = RTC::millis();
			uint8_t dataACK[ACK_PAYLOAD_MAX];
			bool ignored = false;

			MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_RECV_ACK_BEGIN();

			do {
				ignored = false;
				
				reply_result = m_driver->recv(reply_src, reply_port, &dataACK, ACK_PAYLOAD_MAX, TIMEOUT_ACK_RECEIVE); //no ack received
				reply_len = reply_result >= 0 ? (uint8_t) reply_result : 0;
				MW_LOG_DEBUG(MW_LOG_NETWORKV1, "Reply byte count=%d", reply_len);
				
				if ( reply_result > 0 ) {
					get_msg(&reply_msg, dataACK, reply_result);
					MW_LOG_DEBUG(MW_LOG_NETWORKV1, "Reply port=%d, Exp. port=%d, Reply seq=%d, Exp. seq=%d", reply_port, port, reply_msg.nwk_ctrl.seq, seq);
					MW_LOG_DEBUG(MW_LOG_NETWORKV1, "Deliv is ACK=%d", (reply_msg.nwk_ctrl.delivery & ACK), reply_port, port);
					
#if MW_SUPPORT_DELIVERY_FLOOD
					//In case of FLOOD the above sendWithoutACK might have been missed by ALL neighbour nodes,
					//so we try to detect this, fail quickly and re-send our FLOOD message, instead of
					//waiting for the full timeout
					if ( (msg->nwk_ctrl.delivery & DELIVERY_FLOOD) && (reply_msg.nwk_ctrl.delivery && DELIVERY_FLOOD) ) {
						oneFloodACK = oneFloodACK || (( reply_msg.nwk_ctrl.delivery & ACK ) &&
													  ( reply_port == port && reply_msg.nwk_ctrl.seq == seq ));
						MW_LOG_DEBUG(MW_LOG_NETWORKV1, "At least one FLOOD ACK received: %d", oneFloodACK);
					}
#endif
					
					if ( reply_port != port || reply_msg.nwk_ctrl.seq != seq || !(reply_msg.nwk_ctrl.delivery & ACK)
								|| ((reply_msg.nwk_ctrl.delivery & DELIVERY_DIRECT ) && reply_src != dest)
#if MW_SUPPORT_DELIVERY_ROUTED
								|| ((msg->nwk_ctrl.delivery & DELIVERY_ROUTED
	#if MW_SUPPORT_DELIVERY_FLOOD
								|| msg->nwk_ctrl.delivery & DELIVERY_FLOOD
	#endif
								//flood ack will always be via DELIVERY_ROUTED, so it is safe to use msg_routed here
								) && reply_msg.msg_routed.route_info.route.dst != msg->msg_routed.route_info.route.dst )
#endif							
//								|| reply_port != port )// not sure if this is always the case?
								) {
							ignored = true;
							MW_LOG_NOTICE(MW_LOG_NETWORKV1, "Not ACK, ignore, code: %d", reply_result);
							MW_LOG_NOTICE(MW_LOG_NETWORKV1, "\t\tPort=%d, seq=%d, deliv=%d, reply src=%d", reply_port, reply_msg.nwk_ctrl.seq, reply_msg.nwk_ctrl.delivery, reply_src);
						}
				}
#if MW_SUPPORT_DELIVERY_FLOOD
				if ( (msg->nwk_ctrl.delivery & DELIVERY_FLOOD) && Meshwork::Time::passed(RTC::since(start), TIMEOUT_ACK_RECEIVE) && (!oneFloodACK) ) {
					MW_LOG_NOTICE(MW_LOG_NETWORKV1, "Noone received our FLOOD ACK. Resend...", NULL);
					//noone heard our FLOOD. re-send
					result = FLOOD_NOT_RECEIVED_BY_NEIGHBOURS;
					ignored = false;
					break;
				}
#endif							
				MW_LOG_DEBUG(MW_LOG_NETWORKV1, "Reply code=%d", reply_result);
			} while ( !m_sendAbort &&
						(reply_result < 0 || ignored) &&
							(!Meshwork::Time::passed(RTC::since(start), ackTimeout)) );
			MW_LOG_DEBUG(MW_LOG_NETWORKV1, "Out of loop w code: %d, reply len: %d, sendAbort: %d", reply_result, reply_len, m_sendAbort);
			
			if ( m_sendAbort ) { //user aborted; break the loop
				result = Meshwork::L3::Network::ERROR_DRIVER_SEND_ABORTED;
				break;
			} else if ( ignored ) { //internal message to be ignored; loop back
				result = OK_MESSAGE_IGNORED;
			} else if ( reply_result >= 0 ) { //response received correctly; break the loop
				result = reply_result;
				if ( reply_len > 0 )
					MW_LOG_DEBUG_ARRAY(MW_LOG_NETWORKV1, PSTR("L2 DATA RECV: "), dataACK, reply_len);
				
#if MW_SUPPORT_DELIVERY_ROUTED
				//check if we have the route
				uint8_t hopCount = reply_msg.msg_routed.route_info.route.hopCount;
				if ( returnRoute != NULL && (msg->nwk_ctrl.delivery & DELIVERY_ROUTED
	#if MW_SUPPORT_DELIVERY_FLOOD
						|| msg->nwk_ctrl.delivery & DELIVERY_FLOOD
	#endif
						) && hopCount <= returnRouteSize && reply_msg.msg_routed.route_info.route.hops != NULL ) {
					MW_LOG_DEBUG(MW_LOG_NETWORKV1, "Copy hops=%d", hopCount);
					returnRoute->hopCount = hopCount;
					returnRoute->src = reply_msg.msg_routed.route_info.route.src;
					if ( hopCount > 0 )
						memcpy(returnRoute->hops, reply_msg.msg_routed.route_info.route.hops, hopCount);
					returnRoute->dst = reply_msg.msg_routed.route_info.route.dst;
					returnRouteSize = hopCount;
				} else {
					returnRouteSize = 0;
				}
#endif
				reply_len = reply_result >= 0 && reply_len > 0 ? get_msg_payload_len(&reply_msg) : 0;
				
				if ( reply_len > 0 ) {
					//copy only the L3 payload, which is the ACK data
					if ( reply_len <= maxACKLen ) {
						memcpy(bufACK, get_msg_payload(&reply_msg), reply_len);
						MW_LOG_DEBUG_ARRAY(MW_LOG_NETWORKV1, PSTR("Reply data: "), dataACK, reply_len);
						result = OK;
					} else {
						result = OK_WARNING_ACK_TOO_LONG;
					}
				} else {
					result = OK;//just need to make it > 0 to mark success
				}
				maxACKLen = reply_len;
				break;
			} else { //some other error happened
				if ( reply_result == -1 ) {//lenACK or RF RCV buffer overflow; break loop
					result = ERROR_ACK_TOO_LONG;
					break;
				}
				//else mark as not received; loop back
				result = ERROR_ACK_NOT_RECEIVED;
				ignored = true;
			}
		} else {//message sent, no need to wait for ACK, so we are OK
			result = OK;
			break;
		}
	}

	if ( ack !=0 ) {
		MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_RECV_ACK_END(&reply_msg, result);
	}

#if MW_SUPPORT_DELIVERY_ROUTED
	//notify RouteProvider about a failed route
	if ( result == ERROR_ACK_NOT_RECEIVED && m_advisor != NULL && (msg->nwk_ctrl.delivery & DELIVERY_ROUTED) ) {
		m_advisor->route_failed(&msg->msg_routed.route_info.route);
	}
#endif
	if ( result >= 1 ) {
		MW_LOG_INFO(MW_LOG_NETWORKV1, "Result: %d:", result);
	} else {
		MW_LOG_ERROR(MW_LOG_NETWORKV1, "Send failed, code: %d", result);
	}
	return result;
}

int Meshwork::L3::NetworkV1::NetworkV1::sendDirectACK(Meshwork::L3::Network::ACKProvider* ackProvider, univmsg_t* msg, uint8_t hopSrc, uint8_t hopPort) {
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Send to: %d:%d", hopSrc, hopPort);
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "ackProvider=%d, msg=%d, msq.seq=%d", ackProvider, msg, msg->nwk_ctrl.seq);
	
	int result = -1;
	univmsg_t reply_msg;
	reply_msg.msg_direct.nwk_ctrl.seq = msg->nwk_ctrl.seq;
	reply_msg.msg_direct.nwk_ctrl.delivery = DELIVERY_DIRECT | ACK;
	uint8_t origin = 0;
	uint8_t dest = origin = hopSrc;
	void* data = get_msg_payload(msg);
	uint8_t dataLen = get_msg_payload_len(msg);
	
	uint8_t bufACK[ACK_PAYLOAD_MAX];
	uint8_t bufACKsize = 0;
	if (ackProvider != NULL) {
		bufACKsize = ackProvider->returnACKPayload(origin, hopPort, data, dataLen, bufACK, ACK_PAYLOAD_MAX);
	}
	reply_msg.msg_direct.data = bufACKsize == 0 ? NULL : bufACK;
	reply_msg.msg_direct.dataLen = bufACKsize;
	
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Send Direct ACK to node: %d via node: %d, payload size: %d", origin, dest, bufACKsize);
	
	iovec_t toSend[MAX_IOVEC_MSG_SIZE];
	iovec_t* vp = toSend;
	vp = get_iovec_msg_direct(vp, &reply_msg);
	
	MW_LOG_DEBUG_VP_BYTES(MW_LOG_NETWORKV1, PSTR("L2 DATA SEND DIRECT ACK: "), toSend);
	
	MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_BEGIN(m_driver->get_device_address(), dest, hopPort, &reply_msg);

	bool sent = sendWithoutACK(dest, hopPort, vp, m_retry+1);

	MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_END(m_driver->get_device_address(), dest, hopPort, &reply_msg, sent);

	result = sent ? OK : Meshwork::L3::NetworkV1::NetworkV1::ERROR_ACK_SEND_FAILED;
	
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "Result: %d", result);
	return result;
}

#if MW_SUPPORT_DELIVERY_ROUTED
int Meshwork::L3::NetworkV1::NetworkV1::sendRoutedACK(Meshwork::L3::Network::ACKProvider* ackProvider, univmsg_t* msg, uint8_t hopSrc, uint8_t hopPort) {
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Send to: hopSrc=%d, hopPort=%d", hopSrc, hopPort);
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "ackProvider=%d, msg=%d", ackProvider, msg);
	
	int result = OK;
	univmsg_t reply_msg;
	reply_msg.msg_routed.nwk_ctrl.seq = msg->nwk_ctrl.seq;
	reply_msg.msg_routed.nwk_ctrl.delivery = msg->nwk_ctrl.delivery | ACK;
	memcpy(&reply_msg.msg_routed.route_info, &msg->msg_routed.route_info, sizeof(msg->msg_routed.route_info));
	reply_msg.msg_routed.route_info.breadcrumbs = 0;
	uint8_t origin = 0;
	uint8_t dest = 0;
	void* data = NULL;
	uint8_t dataLen = 0;
	
	if ( msg->nwk_ctrl.delivery & DELIVERY_DIRECT ) {
		dest = origin = hopSrc;
		data = msg->msg_direct.data;
		dataLen = msg->msg_direct.dataLen;
	} else if ( msg->nwk_ctrl.delivery & DELIVERY_ROUTED ) {
		origin = msg->msg_routed.route_info.route.src;
		uint8_t hopCount = msg->msg_routed.route_info.route.hopCount;
		uint8_t* hops = (uint8_t *)msg->msg_routed.route_info.route.hops;
		dest = hopCount > 0 ? hops[hopCount-1] : origin;
		data = msg->msg_routed.data;
		dataLen = msg->msg_routed.dataLen;
#if MW_SUPPORT_DELIVERY_FLOOD
	} else if ( msg->nwk_ctrl.delivery & DELIVERY_FLOOD ) {
		origin = msg->msg_flood.flood_info.route.src;
		uint8_t hopCount = msg->msg_flood.flood_info.route.hopCount;
		uint8_t* hops = (uint8_t *)msg->msg_flood.flood_info.route.hops;
		dest = hopCount > 0 ? hops[hopCount-1] : origin;
		data = msg->msg_flood.data;
		dataLen = msg->msg_flood.dataLen;
#endif
	}
	
	uint8_t bufACKsize = 0;
	uint8_t bufACK[ACK_PAYLOAD_MAX];
	if (ackProvider != NULL) {
		bufACKsize = ackProvider->returnACKPayload(origin, hopPort, data, dataLen, bufACK, ACK_PAYLOAD_MAX);
	}
	reply_msg.msg_routed.data = bufACKsize == 0 ? NULL : bufACK;
	reply_msg.msg_routed.dataLen = bufACKsize;

	MW_LOG_INFO(MW_LOG_NETWORKV1, "Send Routed ACK to node: %d via node: %d, payload size: %d", origin, dest, bufACKsize);
	
	iovec_t toSend[MAX_IOVEC_MSG_SIZE];
	iovec_t* vp = toSend;
	vp = get_iovec_msg_routed(vp, &reply_msg);
	
	MW_LOG_DEBUG_VP_BYTES(MW_LOG_NETWORKV1, PSTR("L2 DATA SEND ROUTED ACK: "), toSend);
	
	MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_BEGIN(m_driver->get_device_address(), dest, hopPort, &reply_msg);

	bool sent = sendWithoutACK(dest, hopPort, vp, m_retry+1);

	MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_END(m_driver->get_device_address(), dest, hopPort, &reply_msg, sent);

	result = sent ? OK : Meshwork::L3::NetworkV1::NetworkV1::ERROR_ACK_SEND_FAILED;
	
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "Result: %d", result);
	return result;
}
#endif

int Meshwork::L3::NetworkV1::NetworkV1::send(uint8_t delivery, uint8_t retry,
					uint8_t dest, uint8_t port,
					const void* buf, size_t len,
					void* bufACK, size_t& lenACK) {
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Send: %d:%d, len=%d", dest, port, len);
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "deliv=%d, retry=%d, buf=%d, bufACK=%d, lenACK=%d", delivery, retry, buf, bufACK, lenACK);
	
	int result = -1;
	if (len <= PAYLOAD_MAX) {
		seq++;
		MW_LOG_INFO(MW_LOG_NETWORKV1, "New SEQ=%d", seq);
		size_t none = 0;
		uint8_t count = 1 + (retry == 255 ? m_retry : retry);
		uint8_t deliv = delivery == 0 ? m_delivery : delivery;

		univmsg_t send_msg;
		send_msg.nwk_ctrl.seq = seq;

		//TODO If RouteCache is supported and DELIVERY_ROUTED is set - fist check if Last Working Route is present for the destination node before other options

		//try all set delivery methods, starting from LSB
		if (deliv & DELIVERY_DIRECT) {
			MW_LOG_INFO(MW_LOG_NETWORKV1, "Send DIRECT", NULL);
			send_msg.nwk_ctrl.delivery = DELIVERY_DIRECT;
			send_msg.msg_direct.dataLen = len;
			send_msg.msg_direct.data = (uint8_t*) buf;
			result = sendWithACK(count, RETRY_WAIT_DIRECT, dest == Wireless::Driver::BROADCAST ? 0 : ACK, TIMEOUT_ACK_DIRECT,
									dest, port,	&send_msg, bufACK, lenACK
#if MW_SUPPORT_DELIVERY_ROUTED
									, NULL, none
#endif
									);
			result = result > 0 ? OK : result;
		}
		if ( result != Meshwork::L3::Network::ERROR_DRIVER_SEND_ABORTED ) {
#if MW_SUPPORT_DELIVERY_ROUTED
			if ( (result <= 0) && (deliv & DELIVERY_ROUTED) ) {
				MW_LOG_INFO(MW_LOG_NETWORKV1, "Send ROUTED", NULL);
				if ( dest == Wireless::Driver::BROADCAST ) {
					result = Meshwork::L3::NetworkV1::NetworkV1::ERROR_DELIVERY_METHOD_INVALID;
				} else {
					result = Meshwork::L3::NetworkV1::NetworkV1::ERROR_NO_KNOWN_ROUTES;
					uint8_t routeCount = m_advisor != NULL ? m_advisor->get_routeCount(dest) : 0;
					bool triedOnce = false;
					if ( routeCount > 0 ) {
						for ( int i = 0; i < routeCount; i ++ ) {
							route_t* route = m_advisor->get_route(dest, i);
							if (route != NULL) {
								if (route->hopCount > m_maxHops) {
									MW_LOG_NOTICE(MW_LOG_NETWORKV1, "Route exceeds max hops, ignoring", NULL);
									continue;
								}
								triedOnce = true;
								send_msg.nwk_ctrl.delivery = DELIVERY_ROUTED;
								send_msg.msg_routed.route_info.route = *route;
								send_msg.msg_routed.route_info.breadcrumbs = 0;
								send_msg.msg_routed.dataLen = len;
								send_msg.msg_routed.data = (uint8_t*) buf;

								uint8_t hop = route->hopCount == 0 ? dest : ((uint8_t*)route->hops)[0];

								result = sendWithACK(count, RETRY_WAIT_ROUTED, ACK, TIMEOUT_ACK_ROUTED,
													hop, port,	&send_msg, bufACK, lenACK, NULL, none);
								result = result > 0 ? OK : result;
								if ( result == OK ||
									 result == Meshwork::L3::Network::ERROR_DRIVER_SEND_ABORTED )
									break;
							}
						}
					}
					if ( !triedOnce) {
						MW_LOG_NOTICE(MW_LOG_NETWORKV1, "No routes to: %d", dest);
					}
				}
			}
	#if MW_SUPPORT_DELIVERY_FLOOD
			if ( (result <= 0) && (deliv & DELIVERY_FLOOD) ) {
				MW_LOG_INFO(MW_LOG_NETWORKV1, "Send FLOOD", NULL);
				//Optimized two-step process
				//Step 1: find the route by sending an empty payload and wait for the ACK
				//node count is initially 0, since we don't have to count Src and Dst
				send_msg.nwk_ctrl.delivery = DELIVERY_FLOOD;
				send_msg.msg_flood.flood_info.route.hopCount = 0;
				send_msg.msg_flood.flood_info.route.src = m_driver->get_device_address();
				send_msg.msg_flood.flood_info.route.dst = dest;
				send_msg.msg_flood.dataLen = 0;
				send_msg.msg_flood.data = NULL;
				size_t hopCount = MAX_ROUTING_HOPS;
				route_t returnRoute;
				uint8_t hops[MAX_ROUTING_HOPS];
				returnRoute.hops = hops;
				result = sendWithACK(count, RETRY_WAIT_FLOOD, ACK, TIMEOUT_ACK_FLOOD, Wireless::Driver::BROADCAST, port,
										&send_msg, NULL, none, &returnRoute, hopCount);

				//Step 2: send the real message using DELIVERY_ROUTED
				if ( result > 0 ) {
					MW_LOG_INFO(MW_LOG_NETWORKV1, "Route found, hops=%d", hopCount);
					//call the impl method, which will increment the seq as well
					if ( hopCount == 0 ) {//no hops inbetween, use direct
						send_msg.nwk_ctrl.delivery = DELIVERY_DIRECT;
						send_msg.msg_direct.dataLen = len;
						send_msg.msg_direct.data = (uint8_t*) buf;
						result = sendWithACK(count, RETRY_WAIT_ROUTED, dest == Wireless::Driver::BROADCAST ? 0 : ACK, TIMEOUT_ACK_DIRECT,
												dest, port,	&send_msg, bufACK, lenACK, NULL, none);
						result = result > 0 ? OK : result;
					} else {//use routed and hops that we discovered
						send_msg.nwk_ctrl.delivery = DELIVERY_ROUTED;
						memcpy(&send_msg.msg_routed.route_info.route, &returnRoute, sizeof(returnRoute));
						send_msg.msg_routed.route_info.breadcrumbs = 0;
						send_msg.msg_routed.dataLen = len;
						send_msg.msg_routed.data = (uint8_t*) buf;

						result = sendWithACK(count, RETRY_WAIT_ROUTED, ACK, TIMEOUT_ACK_ROUTED,
												send_msg.msg_routed.route_info.route.hops[0], port, &send_msg, bufACK, lenACK, NULL, none);
						result = result > 0 ? OK : result;
					}
				} else {
					MW_LOG_NOTICE(MW_LOG_NETWORKV1, "No routes to: %d", dest);
				}
			}
	#endif
		}//abort send check
#endif
	} else {
		result = Meshwork::L3::NetworkV1::NetworkV1::ERROR_PAYLOAD_TOO_LONG;
	}
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Result: %d", result);
	return result;
}



//receive a new message and call ackProvider to provide ACK payload. ACKs are ignored, since they are handled within send
int Meshwork::L3::NetworkV1::NetworkV1::recv(uint8_t& src, uint8_t& port,
		void* newData, size_t& newDataLenMax,
		uint32_t ms, Meshwork::L3::Network::ACKProvider* ackProvider) {
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Recv: timeout(ms)=%l, newDataLenMax=%d, ackProvider=%d", ms, newDataLenMax, ackProvider);
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "&src=%d, &port=%d, newData=%d, newDataLenMax=%d, ackProvider=%d", &src, &port, newData, newDataLenMax, ackProvider);

	uint8_t data[ACK_PAYLOAD_MAX];
//	uint8_t src, port;//use local vars to reduce code size

	MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_RECV_BEGIN();

	int dataLen = m_driver->recv(src, port, &data, PAYLOAD_MAX, ms);

	int result = dataLen;
//	srcA = src;
//	portA = port;
	
	//TODO Update Last Working Route per node in RouteCache when receiving messages from any node

	if (result > 0) { //not timeouted, no crc error
		MW_LOG_INFO(MW_LOG_NETWORKV1, "Received data, len: %d", result);
		MW_LOG_DEBUG(MW_LOG_NETWORKV1, "src=%d, port=%d", src, port);
		MW_LOG_DEBUG_ARRAY(MW_LOG_NETWORKV1, PSTR("L2 DATA RECV: "), data, result);

		univmsg_t recv_msg;
		get_msg(&recv_msg, data, result);//fill in the msg structure

		MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_RECV_END(m_driver->is_broadcast(), src, port, &recv_msg);

		if ( !m_driver->is_broadcast() ) {//send to a specific destination
			if (recv_msg.nwk_ctrl.delivery & DELIVERY_DIRECT) { //Direct Send
				//here we assume the driver does not let us receive messages that are not for us!
				MW_LOG_INFO(MW_LOG_NETWORKV1, "Received DIRECT", NULL);
				//copy real payload. use temp var to reduce code size
				uint8_t len = recv_msg.msg_direct.dataLen;
				newDataLenMax = len;
				if ( len > 0 )
					memcpy(newData, recv_msg.msg_direct.data, len);
				MW_LOG_INFO(MW_LOG_NETWORKV1, "Payload len: %d", len);
				
				result = sendDirectACK(ackProvider, &recv_msg, src, port);
				result = result > 0 ? OK : result;
			}
#if MW_SUPPORT_DELIVERY_ROUTED
			else if (recv_msg.nwk_ctrl.delivery & DELIVERY_ROUTED) { //Routed Send
				uint8_t devaddr = m_driver->get_device_address();
				if (devaddr == recv_msg.msg_routed.route_info.route.dst) { //we are the route dest
					MW_LOG_INFO(MW_LOG_NETWORKV1, "Received ROUTED to us", NULL);
					if (recv_msg.msg_routed.route_info.route.hopCount > 0 && m_advisor != NULL)
						m_advisor->route_found(&recv_msg.msg_routed.route_info.route);
					//copy real payload. use temp var to reduce code size
					uint8_t len = recv_msg.msg_routed.dataLen;
					newDataLenMax = len;
					if ( len > 0 )
						memcpy(newData, recv_msg.msg_routed.data, len);
					MW_LOG_INFO(MW_LOG_NETWORKV1, "Payload len: %d", len);

					result = sendRoutedACK(ackProvider, &recv_msg, src, port);
					result = result > 0 ? OK : result;
				} else {//re-route, but first check and update breadcrumbs. if ACK use reverse order to determine next dest
	#if MW_SUPPORT_REROUTING
					MW_LOG_INFO(MW_LOG_NETWORKV1, "Received ROUTED to reroute", NULL);
					uint8_t myHop = 1 + get_msg_routed_hop_index(&recv_msg, m_driver->get_device_address());
					if (myHop > 0) {//offset by 1 since we use it for bitmask
						if ( !(recv_msg.msg_routed.route_info.breadcrumbs & (1 << (myHop - 1))) ) {//our bit not set
							//ok, modifyfing the buf directly instead of using recv_msg
							//and transforming back to a data array is ugly, but more efficient
							((uint8_t *)data)[5 + recv_msg.msg_routed.route_info.route.hopCount] |= 1 << (myHop - 1);//update breadcrumbs
							//ACK route traverses -1 to Src, send route traverses +1 to Dst
							uint8_t hopIndex = myHop + ((recv_msg.nwk_ctrl.delivery == (DELIVERY_ROUTED | ACK)) ? -1 : 1);
							uint8_t dest = ((uint8_t *)data)[3 + hopIndex];
							
							//ACK to the immediate sender first is NOT needed, since:
							//1) The sender only needs RF-level confirmation that the message has been received,
							//   which is currently provided by the Cosa Wireless::Driver::send() function
							//2) The sender waits for a proper ACK from the target (whether direct, routed or flood),
							//   so no need for a Direct ACK here
							//Hence, we comment this out
							//sendDirectACK(NULL, &recv_msg, src, port);
							
							//we don't send with ACK here, since we assume the RF driver takes care of RF-level ACK
							//we only care about NWK ACK end-to-end, which is handled within the orignator's sendWithACK
							MW_LOG_DEBUG_ARRAY(MW_LOG_NETWORKV1, PSTR("L2 DATA SEND REROUTE: "), data, dataLen);

		//so annoying that we have to create the message object here just to notify the listener...
		#if MW_SUPPORT_RADIO_LISTENER
							univmsg_t msg;
							get_msg(&msg, data, dataLen);
							MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_BEGIN(src, dest, port, &msg);
		#endif
							
							bool sent = sendWithoutACK(dest, port, data, dataLen, m_retry+1);

		#if MW_SUPPORT_RADIO_LISTENER
							MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_BEGIN(src, dest, port, &msg);
		#endif

							result = sent ? OK_MESSAGE_IGNORED : Meshwork::L3::NetworkV1::NetworkV1::ERROR_REROUTE_FAILED;
							if ( !sent )
								MW_LOG_NOTICE(MW_LOG_NETWORKV1, "REROUTE driver send failed to: dest=%d", dest, port);
						} else {//we are already in the breadcrumbs
							result = OK_MESSAGE_IGNORED;
						}
					} else {//we are not part of the route
						result = OK_MESSAGE_IGNORED;
					}
	#else
					result = OK_MESSAGE_IGNORED;
	#endif
				}
			}
#endif
			else {
				result = Meshwork::L3::NetworkV1::NetworkV1::ERROR_DELIVERY_METHOD_INVALID;
			}
		} else {//it is broadcast
			MW_LOG_INFO(MW_LOG_NETWORKV1, "Received BROADCAST, delivery=%d", recv_msg.nwk_ctrl.delivery);
			if (recv_msg.nwk_ctrl.delivery & DELIVERY_DIRECT) {
				MW_LOG_INFO(MW_LOG_NETWORKV1, "Received BROADCAST DIRECT, will not ACK", NULL);
				//nothing to do, ACK not required with direct broadcast
			}
#if MW_SUPPORT_DELIVERY_ROUTED
	#if MW_SUPPORT_DELIVERY_FLOOD
			else if (recv_msg.nwk_ctrl.delivery & DELIVERY_FLOOD) {
				uint8_t routeHops = recv_msg.msg_flood.flood_info.route.hopCount;
				MW_LOG_INFO(MW_LOG_NETWORKV1, "Received BROADCAST FLOOD from addr=%d, hopCount=%d", src, routeHops);
				
				//Send FLOOD ACK to allow for early fail at the sender
				MW_LOG_INFO(MW_LOG_NETWORKV1, "Sending FLOOD ACK", NULL);
				uint8_t floodACKMsg[2] = {recv_msg.msg_flood.nwk_ctrl.seq, DELIVERY_FLOOD | ACK};
				
		//so annoying that we have to create the message object here just to notify the listener...
		#if MW_SUPPORT_RADIO_LISTENER
				univmsg_t msg;
				get_msg(&msg, floodACKMsg, sizeof(floodACKMsg));
				MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_BEGIN(m_driver->get_device_address(), src, port, &msg);
		#endif

				bool sent = sendWithoutACK(src, port, floodACKMsg, sizeof(floodACKMsg), m_retry+1);

		#if MW_SUPPORT_RADIO_LISTENER
				MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_END(m_driver->get_device_address(), src, port, &msg, sent);
		#else
				UNUSED(sent);
		#endif

				uint8_t devaddr = m_driver->get_device_address();
				if (devaddr == recv_msg.msg_flood.flood_info.route.dst) {//we are the ultimate receiver, ask for payload and generate a routed ACK
					//we could have optimized to check if hopCount == 0 and send direct ack
					//but that would have increased the code at both sender and receiver side
					MW_LOG_INFO(MW_LOG_NETWORKV1, "Message to us, sending ROUTED ACK", NULL);
					if (routeHops > 0 && m_advisor != NULL)
						m_advisor->route_found(&recv_msg.msg_flood.flood_info.route);
					uint8_t len = recv_msg.msg_flood.dataLen;//local var reduces code size
					newDataLenMax = len;
					if ( len > 0 )
						memcpy(newData, recv_msg.msg_flood.data, len);
					result = sendRoutedACK(ackProvider, &recv_msg, src, port);
					//empty payload means internal flood discovery message, which the user should not care about
					result = result > 0 ? (len == 0 ? OK_MESSAGE_INTERNAL : OK ) : ERROR_ACK_SEND_FAILED;
		#if MW_SUPPORT_REROUTING
				} else if (routeHops < m_maxHops) {//rebroadcast the message
					uint8_t myHop = 1 + get_msg_routed_hop_index(&recv_msg, m_driver->get_device_address());
					if (myHop == 0) {//not in the hop list, add us
						MW_LOG_INFO(MW_LOG_NETWORKV1, "Will REBROADCAST", NULL);
						uint8_t newHops[routeHops + 1];
						if (routeHops > 0)	//copy existing hops incl src, excl dst
							memcpy(newHops, recv_msg.msg_flood.flood_info.route.hops, routeHops);
						newHops[routeHops] = recv_msg.msg_flood.flood_info.route.dst;
						uint8_t newMsg[++dataLen];
						get_msg_flood(&recv_msg, newMsg, dataLen);
						
						MW_LOG_DEBUG_ARRAY(MW_LOG_NETWORKV1, PSTR("L2 DATA SEND REBROADCAST: "), newMsg, dataLen);
						
						MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_BEGIN(src, Wireless::Driver::BROADCAST, port, &recv_msg);

						bool sent = sendWithoutACK(Wireless::Driver::BROADCAST, port, newMsg, dataLen, m_retry+1);

						MW_DECL_IF_SUPPORT_RADIO_LISTENER NOTIFY_SEND_END(src, Wireless::Driver::BROADCAST, port, &recv_msg, sent);

						result = sent ? OK_MESSAGE_IGNORED : Meshwork::L3::NetworkV1::NetworkV1::ERROR_REROUTE_FAILED;
						if ( !sent )
								MW_LOG_NOTICE(MW_LOG_NETWORKV1, "REBROADCAST driver send failed to: dest=%d", Wireless::Driver::BROADCAST, port);
					} else {//we're in the hop list meaning we already retransmitted the message, so ignore
						MW_LOG_NOTICE(MW_LOG_NETWORKV1, "Already rebroadcasted this message, ignoring", NULL);
					}
					result = OK_MESSAGE_IGNORED;
		#endif
				} else {
					MW_LOG_NOTICE(MW_LOG_NETWORKV1, "Max hops reached, ignoring", NULL);
					result = Meshwork::L3::NetworkV1::NetworkV1::ERROR_MESSAGE_IGNORED_MAX_HOPS_REACHED;
				}
			}
	#endif
#endif
			else {
				result = Meshwork::L3::NetworkV1::NetworkV1::ERROR_DELIVERY_METHOD_INVALID;
			}
		}
	}
	MW_LOG_INFO(MW_LOG_NETWORKV1, "Receive result: %d", result);
	//reset abort flag before returning
	m_sendAbort = false;
	return result;
}

bool Meshwork::L3::NetworkV1::NetworkV1::begin(const void* config) {
	UNUSED(config);
	if ( m_advisor != NULL && m_driver != NULL ) {
		m_advisor->set_address(m_driver->get_device_address());
	}
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "[Begin] NwkID=%d, NodeID=%d, NwkKeyLen=%d, NwkKeyPtr=d", getNetworkID(), getNodeID(), getNetworkKeyLen(), getNetworkKey());
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "[Begin] NwkChannel=%d, NwkCaps=%d, Delivery=%d", getChannel(), getNetworkCaps(), getDelivery());
	return m_driver == NULL ? false : m_driver->begin();
}

bool Meshwork::L3::NetworkV1::NetworkV1::end() {
	MW_LOG_DEBUG(MW_LOG_NETWORKV1, "[End]", NULL);
	return m_driver == NULL ? false : m_driver->end();
}
#endif
