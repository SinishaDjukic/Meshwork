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
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Power.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Trace.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/MeshV1/MeshV1.h"

#define IF_MESHV1_DEBUG

#ifndef IF_MESHV1_DEBUG
#define IF_MESHV1_DEBUG if(false)
#endif

//dest should be the next immediate hop
int MeshV1::sendWithACK(uint8_t attempts, uint16_t attemptsDelay,
						uint8_t ack, uint32_t ackTimeout,
						uint8_t dest, uint8_t port,
						univmsg_t* msg,
						void* bufACK, size_t& maxACKLen
#ifdef SUPPORT_DELIVERY_ROUTED
						, void* returnRoute, size_t& returnRouteSize//returnRouteSize should be initialized with the MAX size of the buffer
#endif
						) {
	IF_MESHV1_DEBUG TRACE_LOG("attempts=%d, attemptsDelay=%d, ack=%d, ackTimeout=%l", attempts, attemptsDelay, ack, ackTimeout);
	IF_MESHV1_DEBUG TRACE_LOG("dest=%d, port=%d, msg=%d, bufACK=%d, maxACKLen=%d", dest, port, msg, bufACK, maxACKLen);
#ifdef SUPPORT_DELIVERY_ROUTED
	IF_MESHV1_DEBUG TRACE_LOG("returnRoute=%d, returnRouteSize=%d", returnRoute, returnRouteSize);
#endif
	
	int result = ack != 0 ? ERROR_ACK_NOT_RECEIVED : 0;
	
	iovec_t toSend[MAX_IOVEC_MSG_SIZE];
	iovec_t* vp = toSend;
	vp = get_iovec_msg(vp, msg);
	
	IF_MESHV1_DEBUG TRACE_VP_BYTES(PSTR("L2 DATA SEND: "), toSend);

	for (int i = 0; i < attempts; i ++) {
		if ((result = m_driver->send(dest, port, vp)) < 1) {
			IF_MESHV1_DEBUG ERR("Driver send failed: %d", result);
			break;
		}
		//we wait for ACK if not broadcast
		if (ack != 0) { //need nwk ack
			IF_MESHV1_DEBUG DEBUG("Wait ACK", NULL);
			uint8_t reply_src, reply_port, reply_len;
			int reply_result;
			//the next recv may come with an irrelevant message/data, so recv some more until timeout is reached
			uint32_t start = RTC::millis();
			uint8_t dataACK[ACK_PAYLOAD_MAX];
			univmsg_t reply_msg;
			do {
				reply_result = m_driver->recv(reply_src, reply_port, &dataACK, ACK_PAYLOAD_MAX, TIMEOUT_ACK_RECEIVE); //no ack received
				reply_len = reply_result >= 0 ? (uint8_t) reply_result : 0;
				IF_MESHV1_DEBUG TRACE_LOG("Reply byte count=%d", reply_len);
				
				if ( reply_result > 0 ) {
					get_msg(&reply_msg, dataACK, reply_result);
					IF_MESHV1_DEBUG TRACE_LOG("Reply port=%d, Port=%d, Reply seq=%d, Seq=%d", reply_port, port, reply_msg.nwk_ctrl.seq, seq);
					IF_MESHV1_DEBUG TRACE_LOG("Deliv is ACK=%d", (reply_msg.nwk_ctrl.delivery & ACK), reply_port, port);
					if ( reply_port != port || reply_msg.nwk_ctrl.seq != seq || !(reply_msg.nwk_ctrl.delivery & ACK)
								|| ((reply_msg.nwk_ctrl.delivery & DELIVERY_DIRECT ) && reply_src != dest)
#ifdef SUPPORT_DELIVERY_ROUTED
								|| ((reply_msg.nwk_ctrl.delivery & DELIVERY_ROUTED
#ifdef SUPPORT_DELIVERY_FLOOD
								|| reply_msg.nwk_ctrl.delivery & DELIVERY_FLOOD
#endif
								//flood ack will always be via DELIVERY_ROUTED, so it is safe to use msg_routed here
								) && reply_msg.msg_routed.route_info.route.dst != msg->msg_routed.route_info.route.dst )
#endif							
//								|| reply_port != port )// not sure if this is always the case?
								) {
							reply_result = MeshV1::OK_MESSAGE_IGNORED;
							IF_MESHV1_DEBUG DEBUG("Message not ACK, ignoring, code: %d", reply_result);
							IF_MESHV1_DEBUG DEBUG("\t\tDetails: port=%d, seq=%d, deliv=%d, reply src=%d", reply_port, reply_msg.nwk_ctrl.seq, reply_msg.nwk_ctrl.delivery, reply_src);
						}
						//TODO check reply_result-wrong.txt
				}
				//TODO should check if the error is caused by lenACK overflow
				IF_MESHV1_DEBUG TRACE_LOG("Reply result code=%d", reply_result);
			} while (reply_result < 0 && (RTC::since(start) < ackTimeout));
			IF_MESHV1_DEBUG TRACE_LOG("Out of loop w code: %d, reply len: %d", reply_result, reply_len);
			
			if ( reply_result >= 0 && reply_len > 0 )
				IF_MESHV1_DEBUG TRACE_ARRAY(PSTR("L2 DATA RECV: "), dataACK, reply_len);
			//TODO check if we should use a diff criteria here, since IGNORED = 3, which is a valid byte count too!
			if (reply_result >= 0) {
				if (reply_result == MeshV1::OK_MESSAGE_IGNORED) {
					MSLEEP(attemptsDelay + (seq & 0x03) * 8); //delay simple pseudo-random ms before retry
				} else {
#ifdef SUPPORT_DELIVERY_ROUTED
					//check if we have the route
					uint8_t hopCount = reply_msg.msg_routed.route_info.route.hopCount;
					if ( returnRoute != NULL && (msg->nwk_ctrl.delivery & DELIVERY_ROUTED
#ifdef SUPPORT_DELIVERY_FLOOD
							|| msg->nwk_ctrl.delivery & DELIVERY_FLOOD
#endif
							) && hopCount <= returnRouteSize && reply_msg.msg_routed.route_info.route.hops != NULL ) {
						IF_MESHV1_DEBUG TRACE_LOG("Copying route, hopcCount=%d", hopCount);
						((uint8_t*)returnRoute)[0] = hopCount;
						((uint8_t*)returnRoute)[1] = reply_msg.msg_routed.route_info.route.src;
						if ( hopCount > 0 )
							memcpy(returnRoute, reply_msg.msg_routed.route_info.route.hops, hopCount);
						((uint8_t*)returnRoute)[2 + hopCount] = reply_msg.msg_routed.route_info.route.dst;
						returnRouteSize = hopCount;
					} else {
						returnRouteSize = 0;
					}
#endif
					reply_len = reply_result >= 0 && reply_len > 0 ? get_msg_payload_len(&reply_msg) : 0;
					if ( reply_len > 0 ) {
						//copy only the L3 payload, which is the ACK data
						uint8_t dataLen = 0;
						if ( reply_len <= maxACKLen ) {
							dataLen = get_msg_payload_len(&reply_msg);
							memcpy(bufACK, get_msg_payload(&reply_msg), dataLen);
							result = MeshV1::OK;
						} else {
							result = OK_WARNING_ACK_TOO_LONG;
						}
						maxACKLen = dataLen;
					} else {
						result = MeshV1::OK;//just need to make it > 0 to mark success
					}
					break;
				}
			} else {
				result = ERROR_ACK_NOT_RECEIVED;
			}
		} else {
			result = OK;
		}
	}
#ifdef SUPPORT_DELIVERY_ROUTED
	//notify RouteProvider about a failed route
	if ( result == ERROR_ACK_NOT_RECEIVED && m_advisor != NULL && (msg->nwk_ctrl.delivery & DELIVERY_ROUTED) ) {
		m_advisor->route_failed(&msg->msg_routed.route_info.route);
	}
#endif
	if ( result >= 1 ) {
		IF_MESHV1_DEBUG TRACE_LOG("Result: %d:", result);
	} else {
		IF_MESHV1_DEBUG ERR("Send failed, code: %d", result);
	}
	return result;
}

int MeshV1::sendDirectACK(Meshwork::L3::Network::ACKProvider* ackProvider, univmsg_t* msg, uint8_t hopSrc, uint8_t hopPort) {
	IF_MESHV1_DEBUG TRACE_LOG("ackProvider=%d, msg=%d, hopSrc=%d, hopPort=%d", ackProvider, msg, hopSrc, hopPort);
	
	int result = -1;
	univmsg_t reply_msg;
	reply_msg.msg_direct.nwk_ctrl.seq = msg->nwk_ctrl.seq;
	reply_msg.msg_direct.nwk_ctrl.delivery = DELIVERY_DIRECT | ACK;
	uint8_t origin = 0;
	uint8_t dest = origin = hopSrc;
	void* data = get_msg_payload(msg);
	uint8_t dataLen = get_msg_payload_len(msg);
	
	uint8_t bufACK[ACK_PAYLOAD_MAX];
	if (ackProvider != NULL) {
		uint8_t bufACKsize = ackProvider->returnACKPayload(origin, hopPort, data, dataLen, bufACK, ACK_PAYLOAD_MAX);
		if (bufACKsize > 0) {
			IF_MESHV1_DEBUG DEBUG("Send ACK with payload size: %d", bufACKsize);
		reply_msg.msg_direct.dataLen = bufACKsize;
		reply_msg.msg_direct.data = bufACK;
		}
	} else {
		reply_msg.msg_direct.dataLen = 0;
		reply_msg.msg_direct.data = NULL;
	}

	IF_MESHV1_DEBUG DEBUG("Send ACK to node: %d via node: %d", origin, dest);
	iovec_t toSend[MAX_IOVEC_MSG_SIZE];
	iovec_t* vp = toSend;
	vp = get_iovec_msg_direct(vp, &reply_msg);
	
	IF_MESHV1_DEBUG TRACE_VP_BYTES(PSTR("L2 DATA SEND DIRECT ACK: "), toSend);
	
	if (m_driver->send(dest, hopPort, vp) < 1) {//send back ACK
		result = MeshV1::ERROR_ACK_SEND_FAILED;
		IF_MESHV1_DEBUG ERR("Driver send failed, code: %d", result);
	} else {
		result = MeshV1::OK;
	}
	IF_MESHV1_DEBUG TRACE_LOG("Result: %d", result);
	return result;
}

#ifdef SUPPORT_DELIVERY_ROUTED
int MeshV1::sendRoutedACK(Meshwork::L3::Network::ACKProvider* ackProvider, univmsg_t* msg, uint8_t hopSrc, uint8_t hopPort) {
	IF_MESHV1_DEBUG TRACE_LOG("ackProvider=%d, msg=%d, hopSrc=%d, hopPort=%d", ackProvider, msg, hopSrc, hopPort);
	
	int result = MeshV1::OK;
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
#ifdef SUPPORT_DELIVERY_FLOOD
	} else if ( msg->nwk_ctrl.delivery & DELIVERY_FLOOD ) {
		origin = msg->msg_flood.flood_info.route.src;
		uint8_t hopCount = msg->msg_flood.flood_info.route.hopCount;
		uint8_t* hops = (uint8_t *)msg->msg_flood.flood_info.route.hops;
		dest = hopCount > 0 ? hops[hopCount-1] : origin;
		data = msg->msg_flood.data;
		dataLen = msg->msg_flood.dataLen;
#endif
	}
	
	uint8_t bufACK[ACK_PAYLOAD_MAX];
	if (ackProvider != NULL) {
		uint8_t bufACKsize = ackProvider->returnACKPayload(origin, hopPort, data, dataLen, bufACK, ACK_PAYLOAD_MAX);
		if (bufACKsize > 0) {
			IF_MESHV1_DEBUG DEBUG("Send ACK with payload size: %d", bufACKsize);
		reply_msg.msg_routed.dataLen = bufACKsize;
		reply_msg.msg_routed.data = bufACK;
		}
	} else {
		reply_msg.msg_routed.dataLen = 0;
		reply_msg.msg_routed.data = NULL;
	}

	IF_MESHV1_DEBUG DEBUG("Send ACK to node: %d via node: %d", origin, dest);
	iovec_t toSend[MAX_IOVEC_MSG_SIZE];
	iovec_t* vp = toSend;
	vp = get_iovec_msg_routed(vp, &reply_msg);
	
	IF_MESHV1_DEBUG TRACE_VP_BYTES(PSTR("L2 DATA SEND ROUTED ACK: "), toSend);
	
	if (m_driver->send(dest, hopPort, vp) < 1) {//send back ACK
		result = MeshV1::ERROR_ACK_SEND_FAILED;
		IF_MESHV1_DEBUG ERR("Driver send failed, code: %d", result);
	}
	IF_MESHV1_DEBUG TRACE_LOG("Result: %d", result);
	return result;
}
#endif

//TODO size_t len should be by reference, but the compiler complains!
int MeshV1::send(uint8_t delivery, uint8_t retry,
					uint8_t dest, uint8_t port,
					const void* buf, size_t len,
					void* bufACK, size_t& lenACK) {
	IF_MESHV1_DEBUG TRACE_LOG("delivery=%d, retry=%d, dest=%d, port=%d, buf=%d, len=%d, bufACK=%d, lenACK=%d", delivery, retry, dest, port, buf, len, bufACK, lenACK);
	
	int result = -1;
	if (retry >= -1) {
		if (len <= PAYLOAD_MAX) {
			seq++;
			size_t none = 0;
			uint8_t count = 1 + retry == -1 ? m_retry : retry;
			uint8_t deliv = delivery == 0 ? m_delivery : delivery;
			
			univmsg_t send_msg;
			send_msg.nwk_ctrl.seq = seq;

			//try all set delivery methods, starting from LSB
			if (deliv & DELIVERY_DIRECT) {
				IF_MESHV1_DEBUG DEBUG("Send DIRECT", NULL);
				send_msg.nwk_ctrl.delivery = DELIVERY_DIRECT;
				send_msg.msg_direct.dataLen = len;
				send_msg.msg_direct.data = (uint8_t*) buf;
				result = sendWithACK(count, RETRY_WAIT_DIRECT, dest == Wireless::Driver::BROADCAST ? 0 : ACK, TIMEOUT_ACK_DIRECT,
										dest, port,	&send_msg, bufACK, lenACK
#ifdef SUPPORT_DELIVERY_ROUTED
										, NULL, none
#endif
										);
				result = result > 0 ? MeshV1::OK : result;
			}
			
#ifdef SUPPORT_DELIVERY_ROUTED
			if (result <= 0 && (deliv & DELIVERY_ROUTED)) {
				IF_MESHV1_DEBUG DEBUG("Send ROUTED", NULL);
				if ( dest == Wireless::Driver::BROADCAST ) {
					result = MeshV1::ERROR_DELIVERY_METHOD_INVALID;
				} else {
					result = MeshV1::ERROR_NO_KNOWN_ROUTES;
					uint8_t routeCount = m_advisor != NULL ? m_advisor->get_routeCount(dest) : 0;
					if ( routeCount > 0 ) {
						for ( int i = 0; i < routeCount; i ++ ) {
							route_t* route = m_advisor->get_route(dest, i);
							if (route != NULL) {
								if (route->hopCount > m_maxHops) {
									IF_MESHV1_DEBUG NOTICE("Max hops reached, ignoring", NULL);
									continue;
	//								return (MeshV1::ERROR_MESSAGE_IGNORED_MAX_HOPS_REACHED);
								}
								send_msg.nwk_ctrl.delivery = DELIVERY_ROUTED;
								send_msg.msg_routed.route_info.route = *route;
								send_msg.msg_routed.route_info.breadcrumbs = 0;
								send_msg.msg_routed.dataLen = len;
								send_msg.msg_routed.data = (uint8_t*) buf;

								result = sendWithACK(count, RETRY_WAIT_ROUTED, ACK, TIMEOUT_ACK_ROUTED,
													dest, port,	&send_msg, bufACK, lenACK, NULL, none);
								result = result > 0 ? MeshV1::OK : result;
							}
						}
					} else {
						IF_MESHV1_DEBUG NOTICE("No routes known to: %d", dest);
					}
				}
			}
#ifdef SUPPORT_DELIVERY_FLOOD
			if (result <= 0 && (deliv & DELIVERY_FLOOD)) {
				IF_MESHV1_DEBUG DEBUG("Send FLOOD", NULL);
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
				result = sendWithACK(count, RETRY_WAIT_FLOOD, ACK, TIMEOUT_ACK_FLOOD, Wireless::Driver::BROADCAST, port,
										&send_msg, NULL, none, &returnRoute, hopCount);

				//Step 2: send the real message using DELIVERY_ROUTED
				if ( result > 0 ) {
					IF_MESHV1_DEBUG DEBUG("Route discovery success, hopCount=%d", hopCount);
					//call the impl method, which will increment the seq as well
					if ( hopCount == 0 ) {//no hops inbetween, use direct
						//TODO reuse above code instead of duplicating
						send_msg.nwk_ctrl.delivery = DELIVERY_DIRECT;
						send_msg.msg_direct.dataLen = len;
						send_msg.msg_direct.data = (uint8_t*) buf;
						result = sendWithACK(count, RETRY_WAIT_ROUTED, dest == Wireless::Driver::BROADCAST ? 0 : ACK, TIMEOUT_ACK_DIRECT,
												dest, port,	&send_msg, bufACK, lenACK, NULL, none);
						result = result > 0 ? MeshV1::OK : result;
					} else {//use routed and hops that we discovered
						//TODO check if we can reuse some of the routing send code above, but pass specific route
						send_msg.nwk_ctrl.delivery = DELIVERY_ROUTED;
/*						send_msg.msg_routed.route_info.route.hopCount = hopCount;
						send_msg.msg_routed.route_info.route.src = m_driver->get_device_address();
						send_msg.msg_routed.route_info.route.hops = &returnRoute;
						send_msg.msg_routed.route_info.route.dst = dest;
*/
						memcpy(&send_msg.msg_routed.route_info.route, &returnRoute, sizeof(returnRoute));
						send_msg.msg_routed.route_info.breadcrumbs = 0;
						send_msg.msg_routed.dataLen = len;
						send_msg.msg_routed.data = (uint8_t*) buf;
						
						result = sendWithACK(count, RETRY_WAIT_ROUTED, ACK, TIMEOUT_ACK_ROUTED,
												send_msg.msg_routed.route_info.route.hops[0], port, &send_msg, bufACK, lenACK, NULL, none);
						result = result > 0 ? MeshV1::OK : result;
					}
				} else {
					IF_MESHV1_DEBUG NOTICE("No routes found  to: %d", dest);
				}
			}
#endif
#endif
		} else {
			result = MeshV1::ERROR_PAYLOAD_TOO_LONG;
		}
	} else {
		result = MeshV1::ERROR_INVALID_RETRY_COUNT;
	}
	IF_MESHV1_DEBUG TRACE_LOG("Result: %d", result);
	return result;
}



//receive a new message and call ackProvider to provide ACK payload. ACKs are ignored, since they are handled within send
int MeshV1::recv(uint8_t& srcA, uint8_t& portA,
		void* newData, size_t& newDataLenMax,
		uint32_t ms, Meshwork::L3::Network::ACKProvider* ackProvider) {
	IF_MESHV1_DEBUG TRACE_LOG("src=%d, port=%d, newData=%d, newDataLenMax=%d, ms=%l, ackProvider=%d", srcA, portA, newData, newDataLenMax, ms, ackProvider);

	uint8_t data[ACK_PAYLOAD_MAX];
	//TODO should we m_driver->recv(..., ms)?
	uint8_t src, port;//use local vars to reduce code size
	int dataLen = m_driver->recv(src, port, &data, ACK_PAYLOAD_MAX, ms);
	int result = dataLen;
	srcA = src;
	portA = port;
	
	if (result > 0) { //not timeouted, no crc error
		IF_MESHV1_DEBUG TRACE_LOG("Raw data count: %d", result);
		IF_MESHV1_DEBUG TRACE_ARRAY(PSTR("L2 DATA RECV: "), data, result);
//		trace.print(data, result, IOStream::hex, MeshV1::PAYLOAD_MAX);

		if ( (data[0] & ACK) ) {//explicit ACKs are ignored, as they are handled within send
			IF_MESHV1_DEBUG NOTICE("Ignoring explicit ACK", NULL);
			result = MeshV1::OK_MESSAGE_IGNORED;
		} else {
			univmsg_t recv_msg;
			get_msg(&recv_msg, data, result);//fill in the msg structure
			if ( !m_driver->is_broadcast() ) {//send to a specific destination
				if (recv_msg.nwk_ctrl.delivery == DELIVERY_DIRECT) { //Direct Send
					IF_MESHV1_DEBUG DEBUG("Recv DIRECT", NULL);
					//copy real payload. use temp var to reduce code size
					uint8_t len = recv_msg.msg_direct.dataLen;
					newDataLenMax = len;
					if ( len > 0 )
						memcpy(newData, recv_msg.msg_direct.data, len);
					IF_MESHV1_DEBUG DEBUG("Payload len: %d", len);
					result = sendDirectACK(ackProvider, &recv_msg, src, port);
					result = result > 0 ? MeshV1::OK : result;
				}
#ifdef SUPPORT_DELIVERY_ROUTED
				else if (recv_msg.nwk_ctrl.delivery == DELIVERY_ROUTED) { //Routed Send
					uint8_t devaddr = m_driver->get_device_address();
					if (devaddr == recv_msg.msg_routed.route_info.route.dst) { //we are the route dest
						IF_MESHV1_DEBUG DEBUG("Recv DIRECT to us", NULL);
						if (recv_msg.msg_routed.route_info.route.hopCount > 0 && m_advisor != NULL)
							m_advisor->route_found(&recv_msg.msg_routed.route_info.route);
						//copy real payload. use temp var to reduce code size
						uint8_t len = recv_msg.msg_routed.dataLen;
						newDataLenMax = len;
						if ( len > 0 )
							memcpy(newData, recv_msg.msg_routed.data, len);
						IF_MESHV1_DEBUG DEBUG("Payload len: %d", len);
						result = sendRoutedACK(ackProvider, &recv_msg, src, port);
						result = result > 0 ? MeshV1::OK : result;
					} else {//re-route, but first check and update breadcrumbs. if ACK use reverse order to determine next dest
#ifdef SUPPORT_REROUTING
						IF_MESHV1_DEBUG DEBUG("Recv DIRECT to reroute", NULL);
						uint8_t myHop = 1 + get_msg_routed_hop_index(&recv_msg, m_driver->get_device_address());
						if (myHop > 0) {//offset by 1 since we use it for bitmask
							if ( !(recv_msg.msg_routed.route_info.breadcrumbs & (1 << (myHop - 1))) ) {//our bit not set
								//ok, modifyfing the buf directly instead of using recv_msg
								//and transforming back to a data array is ugly, but more efficient
								((uint8_t *)data)[5 + recv_msg.msg_routed.route_info.route.hopCount] |= 1 << (myHop - 1);//update breadcrumbs
								uint8_t dest = ((uint8_t *)data)[3 + myHop +
												((recv_msg.nwk_ctrl.delivery & (DELIVERY_ROUTED | ACK)) ? -1 : 1)];//ACK route traverses -1 to Src, send route traverses +1 to Dst
								//ACK to the immediate sender first
								sendDirectACK(NULL, &recv_msg, src, port);
								//we don't send with ACK here, since we assume the RF driver takes care of RF-level ACK
								//we only care about NWK ACK end-to-end, which is handled within the orignator's sendWithACK
								IF_MESHV1_DEBUG TRACE_ARRAY(PSTR("L2 DATA SEND REROUTE: "), data, dataLen);
								if (m_driver->send(dest, port, data, dataLen) < 1) {//re-route
									IF_MESHV1_DEBUG ERR("Driver send failed, code: %d", result);
									result = MeshV1::ERROR_REROUTE_FAILED;
								}
							} else {//we are already in the breadcrumbs
								result = MeshV1::OK_MESSAGE_IGNORED;
							}
						} else {//we are not part of the route
							result = MeshV1::OK_MESSAGE_IGNORED;
						}
#else
						result = MeshV1::OK_MESSAGE_IGNORED;
#endif
					}
				}
#endif
				else {
					result = MeshV1::ERROR_DELIVERY_METHOD_INVALID;
				}
			} else {//it is broadcast
			//TODO check why this fails at the receiver?!
				IF_MESHV1_DEBUG DEBUG("Recv BROADCAST delivery=%d", recv_msg.nwk_ctrl.delivery);
				if (recv_msg.nwk_ctrl.delivery == DELIVERY_DIRECT) {
					IF_MESHV1_DEBUG DEBUG("Recv BROADCAST DIRECT, will not ACK", NULL);
					//nothing to do, ACK not required with direct broadcast
				}
#ifdef SUPPORT_DELIVERY_ROUTED
#ifdef SUPPORT_DELIVERY_FLOOD
				else if (recv_msg.nwk_ctrl.delivery == DELIVERY_FLOOD) {
					uint8_t devaddr = m_driver->get_device_address();
					uint8_t routeHops = recv_msg.msg_flood.flood_info.route.hopCount;
					IF_MESHV1_DEBUG DEBUG("Recv BROADCAST FLOOD from addr=%d, hopCount=%d", devaddr, routeHops);
					if (devaddr == recv_msg.msg_flood.flood_info.route.dst) {//we are the ultimate receiver, ask for payload and generate a routed ACK
						//we could have optimized to check if hopCount == 0 and send direct ack
						//but that would have increased the code at both sender and receiver side
						IF_MESHV1_DEBUG DEBUG("Message to us, sending ROUTED ACK", NULL);
						if (routeHops > 0 && m_advisor != NULL)
							m_advisor->route_found(&recv_msg.msg_flood.flood_info.route);
						uint8_t len = recv_msg.msg_flood.dataLen;//local var reduces code size
						newDataLenMax = len;
						if ( len > 0 )
							memcpy(newData, recv_msg.msg_flood.data, len);
						result = sendRoutedACK(ackProvider, &recv_msg, src, port);
						//empty payload means internal flood discovery message, which the user should not care about
						result = result > 0 ? (len == 0 ? OK_MESSAGE_INTERNAL : OK ) : ERROR_ACK_SEND_FAILED;
#ifdef SUPPORT_REROUTING
					} else if (routeHops >= 0 && routeHops < m_maxHops) {//rebroadcast the message
						uint8_t myHop = 1 + get_msg_routed_hop_index(&recv_msg, m_driver->get_device_address());
						if (myHop == 0) {//not in the hop list, add us
							IF_MESHV1_DEBUG NOTICE("Will rebroadcast", NULL);
							uint8_t newHops[routeHops + 1];
							if (routeHops > 0)	//copy existing hops incl src, excl dst
								memcpy(newHops, recv_msg.msg_flood.flood_info.route.hops, routeHops);
							newHops[routeHops] = recv_msg.msg_flood.flood_info.route.dst;
							uint8_t newMsg[++dataLen];
							get_msg_flood(&recv_msg, newMsg, dataLen);
							
							IF_MESHV1_DEBUG TRACE_ARRAY(PSTR("L2 DATA SEND REBROADCAST: "), newMsg, dataLen);
							
							result = m_driver->send(Wireless::Driver::BROADCAST, port, newMsg, dataLen);//don't care about errors?
							if ( result < 1 )
								IF_MESHV1_DEBUG NOTICE("Driver send failed, code: ", result);
						} else {//we're in the hop list meaning we already retransmitted the message, so ignore
							IF_MESHV1_DEBUG NOTICE("Already rebroadcasted, ignoring", NULL);
						}
						result = MeshV1::OK_MESSAGE_IGNORED;
#endif
					} else {
						IF_MESHV1_DEBUG NOTICE("Max hops reached, ignoring", NULL);
						result = MeshV1::ERROR_MESSAGE_IGNORED_MAX_HOPS_REACHED;
					}
				}
#endif
#endif
				else {
					result = MeshV1::ERROR_DELIVERY_METHOD_INVALID;
				}
			}
		}//not ack
		IF_MESHV1_DEBUG TRACE_LOG("Result: %d", result);
	}
	return result;
}

bool MeshV1::begin(const void* config) {
	if ( m_advisor != NULL && m_driver != NULL ) {
		m_advisor->set_address(m_driver->get_device_address());
	}
	return m_driver == NULL ? false : m_driver->begin();
}

bool MeshV1::end() {
	return m_driver == NULL ? false : m_driver->end();
}
