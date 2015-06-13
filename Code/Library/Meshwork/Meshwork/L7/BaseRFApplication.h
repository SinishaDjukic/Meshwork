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
#ifndef __MESHWORK_L7_BASERFAPPLICATION_H__
#define __MESHWORK_L7_BASERFAPPLICATION_H__

#include "Meshwork.h"
#include "Cosa/Pin.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Meshwork/L7/Endpoint.h"
#include "Meshwork/L7/Cluster.h"

using namespace Meshwork::L3;
using namespace Meshwork::L3::NetworkV1;

//Timeout for recv() in pollRF()
#ifndef MW_BASERF_POLLTIMEOUT
	#define MW_BASERF_POLLTIMEOUT 	10 * 1000L
#endif

//Debug
#ifndef MW_LOG_BASERF
	#define MW_LOG_BASERF		MW_FULL_DEBUG
#endif

#define MASK_PROPERTY_GET_META_DEFINED		1 << 0
#define MASK_PROPERTY_GET_CLUSTER_DEFINED	1 << 1
#define MASK_PROPERTY_GET_CLUSTER_EXISTS	1 << 2
#define MASK_PROPERTY_GET_ENDPOINT_DEFINED	1 << 3
#define MASK_PROPERTY_GET_ENDPOINT_EXISTS	1 << 4

namespace Meshwork {

	namespace L7 {

		//Network port for BaseRF L7 messages
		static const uint8_t BASERF_MESSAGE_PORT		= 70;

		//Min L7 message payload (with L7 headers)
		static const uint8_t BASERF_MESSAGE_PAYLOAD_MIN	= 3;
		//Max L7 message payload (with L7 headers)
		static const uint8_t BASERF_MESSAGE_PAYLOAD_MAX	= NetworkV1::NetworkV1::PAYLOAD_MAX;

		//Command Seq notes:
		// - Values [0-127] signify a command sequence number
		// - Bit 7 set to high to denote that more response will follow. Bit 7 will further be referred to as MCFLAG (Multi-Command Flag)
		// - When bit 7 set to low means this was a last response to the specified command sequence
		//Command ID notes:
		// - Values [0-127] signify a valid command ID
		// - Bit 7 set to high to denote that the request is related to the command's meta information. Bit 7 will further be referred to as METAFLAG
		// - For the Meshwork-defined commands, the METAFLAG is only valid for PROPERTY_GET and PROPERTY_REP commands. Its usage elsewhere must result in ACK(Invalid)
		// - Command ID values [0-31] are reserved for Meshwork.

		//BaseRF flags and commands
		static const uint8_t BASERF_CMD_MASK 				= 0x7F;
		static const uint8_t BASERF_CMDFLAG_MULTICOMMAND 	= 0x80;
		static const uint8_t BASERF_CMD_ACK 				= 0x00;
		static const uint8_t BASERF_CMD_PROPERTY_GET 		= 0x10;
		static const uint8_t BASERF_CMD_PROPERTY_REP 		= 0x11;
		static const uint8_t BASERF_CMD_PROPERTY_SET		= 0x12;
		static const uint8_t BASERF_SEQ_MASK 				= 0x7F;
		static const uint8_t BASERF_SEQFLAG_META 			= 0x80;

		//Status fields for BASERF_CMD_ACK
		static const uint8_t BASERF_CMD_ACK_STATUS_PROCESSED  	= 0x00;
		static const uint8_t BASERF_CMD_ACK_STATUS_INVALID  	= 0x01;
		static const uint8_t BASERF_CMD_ACK_STATUS_FORBIDDEN  	= 0x02;
		static const uint8_t BASERF_CMD_ACK_STATUS_SCHEDULED  	= 0x03;


		//BaseRF L7 header structure
		struct msg_l7_header_t {
			uint8_t src;
			uint8_t port;
			uint8_t seq;
			bool seq_meta;
			uint8_t cmd_id;
			bool cmd_mc;
			uint8_t dataLen;
		};

		//Union of L7 structures
		union univmsg_l7_any_t {
			msg_l7_header_t msg_header;
			uint8_t* data;
		};

		static size_t getDataFromMessage(size_t maxDataLen, uint8_t* data, univmsg_l7_any_t* msg) {
			//TODO add boundary checks for maxDataLen, but min 3 bytes!
			data[0] = msg->msg_header.seq | (msg->msg_header.seq_meta ? BASERF_SEQFLAG_META : 0);
			data[1] = msg->msg_header.cmd_id | (msg->msg_header.cmd_mc ? BASERF_CMDFLAG_MULTICOMMAND : 0);
			if ( msg->msg_header.dataLen > 0 ) {
				memcpy(&data[2], msg->data, msg->msg_header.dataLen);
			}
			return 2 + msg->msg_header.dataLen;
		}

		static univmsg_l7_any_t* getMessageFromData(univmsg_l7_any_t* msg, size_t dataLen, uint8_t* data) {
			msg->msg_header.seq = data[0] & BASERF_SEQ_MASK;
			msg->msg_header.seq_meta = (data[0] & BASERF_SEQFLAG_META) != 0;
			msg->msg_header.cmd_id = data[1] & BASERF_CMD_MASK;
			msg->msg_header.cmd_mc = (data[1] & BASERF_CMDFLAG_MULTICOMMAND) != 0;
			msg->msg_header.dataLen = dataLen - 2;
			msg->data = &data[2];

			//TODO extract into print method
			MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("SEQ: ") << msg->msg_header.seq << endl;

			if ( msg->msg_header.cmd_mc )
				MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_CMDFLAG_MULTICOMMAND") << endl;

			if ( msg->msg_header.seq_meta )
				MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_SEQFLAG_META") << endl;
			return msg;
		}

		class BaseRFApplication {
			class BaseRFReportListener {
				public:
					virtual bool notify_report(univmsg_l7_any_t* msg) = 0;
			};


		protected:
			Network* m_network;
			uint8_t m_cluster_count;
			Cluster** m_clusters;
			BaseRFReportListener* m_base_rf_report_listener;
			uint32_t m_poll_timeout;
			uint32_t m_last_sent_report_time;
			uint32_t m_last_recv_request_time;
			uint16_t m_sent_report_count;
			uint16_t m_recv_request_count;

		public:
			BaseRFApplication(Network* network, uint8_t cluster_count, Cluster** clusters,
								BaseRFReportListener* base_rf_report_listener = NULL,
									uint32_t poll_timeout = MW_BASERF_POLLTIMEOUT):
				m_network(network),
				m_cluster_count(cluster_count),
				m_clusters(clusters),
				m_base_rf_report_listener(base_rf_report_listener),
				m_poll_timeout(poll_timeout),
				m_last_sent_report_time(0),
				m_last_recv_request_time(0),
				m_sent_report_count(0),
				m_recv_request_count(0)
				{}


			//TODO Add REP notification function to reporting node. Update m_last_sent_report

			void pollRF() {
				//TODO Implement RF polling and response generation

				//Note: this function implies the Wireless::Driver is up and running, so the app
				//needs to manage the RF begin/end cycles

				//Sequence:
				//1) receive a message
				//2) if result != OK then return, else continue
				//3) parse the L7 payload structure (Seq, ID, Data)
				//3.1) update m_last_recv_request_time
				//3.2) update m_recv_request_count
				//4) process Command
				//4.a.1) if REP then call REP handler
				//4.b.1) if GET or SET then continue
				//4.c.1) else return
				//5) parse Cluster ID and Endpoint ID
				//5.a.1) if METADATA flag and IDs exist send back REP + METADATA message(s), return
				//5.b.1) if not METADATA flag and IDs exist continue
				//5.c.1) else send back ACK(INVALID)
				//6) invoke the Endpoint object:
				//6.a.1) GET: create empty endpoint_value_t and call getProperty
				//6.a.2) send back REP with the property value
				//6.b.1) SET: read data into empty endpoint_value_t, create empty endpoint_set_status_t and call setProperty
				//6.b.2) send back ACK(endpoint_set_status_t)


				MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("[RECV] Timeout=") << m_poll_timeout << endl;

				////////////////////////////////////////////////////////////////
				////////////////// IMPLEMENTATION //////////////////////////////
				////////////////////////////////////////////////////////////////

				//1) receive a message

				uint8_t src, port;
				size_t dataLen = BASERF_MESSAGE_PAYLOAD_MAX;
				uint8_t data[dataLen];

				uint32_t start = RTC::millis();
				while (true) {
					trace << endl;
					int result = m_network->recv(src, port, data, dataLen, m_poll_timeout, NULL);
					if ( result == Meshwork::L3::Network::OK &&
							port == BASERF_MESSAGE_PORT &&
								dataLen >= BASERF_MESSAGE_PAYLOAD_MIN &&
									dataLen <= BASERF_MESSAGE_PAYLOAD_MAX )
						break;
				//2) if result != OK then return, else continue
					if ( RTC::since(start) >= MW_BASERF_POLLTIMEOUT ) {
						MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("[RECV] No message, return") << endl;
						return;
					}
				}

				MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("[RECV] Src=") << src << PSTR(", Len=") << dataLen << endl;
				MW_LOG_DEBUG_ARRAY(MW_LOG_BASERF, PSTR("\t...L7 DATA RECV: "), data, dataLen);
				MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << endl << endl;

				univmsg_l7_any_t msg;

				msg.msg_header.src = src;
				msg.msg_header.port = port;

				//3) parse the L7 payload structure (Seq, ID, Data)
				getMessageFromData(&msg, dataLen, (uint8_t*) &data);

				//3.1) update m_last_recv_request
				m_last_recv_request_time = RTC::millis();

				//3.2) update m_recv_request_count
				m_recv_request_count ++;

				//4) process Command
				if ( msg.msg_header.cmd_id == BASERF_CMD_PROPERTY_REP ) {
					//4.a.1) if REP then call REP handler
					MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_CMD_PROPERTY_REP") << PSTR(": Listener: ") << m_base_rf_report_listener << endl;
					if ( m_base_rf_report_listener != NULL )
						m_base_rf_report_listener->notify_report(&msg);

				} else if ( msg.msg_header.cmd_id == BASERF_CMD_PROPERTY_GET ) {
					//4.b.1) if GET then continue
					MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_CMD_PROPERTY_GET") << endl;
					//5) parse Cluster ID and Endpoint ID
					//NOTE: IDs start from 1! And 0 is reserved for
					uint8_t clusterID = msg.data[0];
					uint8_t endpointID = msg.data[1];

					Cluster* cluster = clusterID < m_cluster_count ? m_clusters[clusterID] : NULL;
					Endpoint* endpoint = cluster != NULL && endpointID < cluster->getEndpointCount() ? cluster->getEndpoint(endpointID) : NULL;

					uint8_t requestType = (msg.msg_header.seq_meta ? MASK_PROPERTY_GET_META_DEFINED : 0) ||
										  (clusterID != 0xFF       ? MASK_PROPERTY_GET_CLUSTER_DEFINED : 0) ||
										  (cluster != NULL       ? MASK_PROPERTY_GET_CLUSTER_EXISTS : 0) ||
										  (endpointID != 0xFF      ? MASK_PROPERTY_GET_ENDPOINT_DEFINED : 0) ||
										  (endpoint != NULL      ? MASK_PROPERTY_GET_ENDPOINT_EXISTS : 0);

					switch (requestType) {
						//5.a.1) if METADATA flag and IDs exist send back REP + METADATA message(s), return
						case MASK_PROPERTY_GET_META_DEFINED:
							sendMetaReportFull(msg);
						break;
						case MASK_PROPERTY_GET_META_DEFINED + MASK_PROPERTY_GET_CLUSTER_DEFINED + MASK_PROPERTY_GET_CLUSTER_EXISTS:
							sendMetaReportCluster(msg, cluster, clusterID);
						break;
						case MASK_PROPERTY_GET_META_DEFINED + MASK_PROPERTY_GET_CLUSTER_DEFINED + MASK_PROPERTY_GET_CLUSTER_EXISTS + MASK_PROPERTY_GET_ENDPOINT_DEFINED + MASK_PROPERTY_GET_ENDPOINT_EXISTS:
							sendMetaReportEndpoint(msg, cluster, clusterID, endpoint, endpointID);
						break;

						//5.b.1) if not METADATA flag and IDs exist continue
						case MASK_PROPERTY_GET_CLUSTER_DEFINED + MASK_PROPERTY_GET_CLUSTER_EXISTS + MASK_PROPERTY_GET_ENDPOINT_DEFINED + MASK_PROPERTY_GET_ENDPOINT_EXISTS:
							sendValueReportEndpoint(msg, cluster, clusterID, endpoint, endpointID);
						break;

						//5.c.1) else send back ACK(INVALID)
						default:
							MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("INVALID: ClusterID=") << clusterID
															  << PSTR(", EndpointID=") << endpointID << endl;
							sendACK(&msg, BASERF_CMD_ACK_STATUS_INVALID, NULL, 0);
					}

				} else if ( msg.msg_header.cmd_id == BASERF_CMD_PROPERTY_SET ) {
					//4.b.1) if SET then continue
					MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_CMD_PROPERTY_SET") << endl;
					//TODO ...........................

				} else if ( msg.msg_header.cmd_id == BASERF_CMD_ACK ) {
					//4.c.1) else return
					MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_CMD_ACK") << PSTR(": IGNORED") << endl;

				} else {
					//4.c.1) else return
					MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("UNKNOWN: ") << msg.msg_header.cmd_id << endl;
				}
			}

			void sendACK(univmsg_l7_any_t* msg, int ackStatus, void* statusData, size_t statusDataLen) {
				MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("ackStatus=") << ackStatus;
				size_t maxReplyMsgDataLen = 3 + statusDataLen;
				uint8_t replyMsgData[maxReplyMsgDataLen];
				univmsg_l7_any_t replyMsg;
				replyMsg.msg_header.src = msg->msg_header.src;
				replyMsg.msg_header.port = msg->msg_header.port;
				replyMsg.msg_header.cmd_id = BASERF_CMD_ACK;
				replyMsg.msg_header.seq = msg->msg_header.seq;
				replyMsg.msg_header.dataLen = statusDataLen;
				replyMsg.data = (uint8_t*) statusData;
				size_t replyMsgLen = getDataFromMessage(maxReplyMsgDataLen, replyMsgData, &replyMsg);
				m_network->send(msg->msg_header.src, msg->msg_header.port, (const void*) &replyMsg, replyMsgLen, NULL, replyMsgLen);
			}

			int16_t getClusterIndex(Cluster* cluster) {
				for ( int i = 0; i < m_cluster_count; i ++ )
					if ( m_clusters[i] == cluster )
						return i;
				return -1;
			}

			uint32_t getLastRecvRequestTime() const {
				return m_last_recv_request_time;
			}

			uint32_t getLastSentReportTime() const {
				return m_last_sent_report_time;
			}

			uint16_t getRecvRequestCount() const {
				return m_recv_request_count;
			}

			uint16_t getSentReportCount() const {
				return m_sent_report_count;
			}
		};
	};
};
#endif

