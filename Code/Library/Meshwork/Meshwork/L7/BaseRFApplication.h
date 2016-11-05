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
#include "Meshwork/L7/Device.h"

//Timeout for recv() in pollRF()
#ifndef MW_BASERF_POLLTIMEOUT
	#define MW_BASERF_POLLTIMEOUT 	10 * 1000L
#endif

//Debug
#ifndef MW_LOG_BASERF
	#define MW_LOG_BASERF		MW_FULL_DEBUG
#endif

#ifndef MW_SUPPORT_BASERF_SUPPORTED_META
	#define MW_SUPPORT_BASERF_SUPPORTED_META	true
#endif

#define MASK_PROPERTY_GET_CLUSTER_DEFINED  (1 << 0)
#define MASK_PROPERTY_GET_CLUSTER_EXISTS   (1 << 1)
#define MASK_PROPERTY_GET_ENDPOINT_DEFINED (1 << 2)
#define MASK_PROPERTY_GET_ENDPOINT_EXISTS  (1 << 3)

using Meshwork::L3::Network;

namespace Meshwork {

	namespace L7 {

		//Network port for BaseRF L7 messages
		static const uint8_t BASERF_MESSAGE_PORT		= 0x88;//136 dec

		//L7 message headers length
		static const uint8_t BASERF_MESSAGE_PAYLOAD_HEADERLEN	= 2;
		//Max L7 message payload (incl. headers)
		static const uint8_t BASERF_MESSAGE_PAYLOAD_MAXLEN	= Meshwork::L3::NetworkV1::NetworkV1::PAYLOAD_MAX;

		//Command Seq notes:
		// - Values [0-127] signify a command sequence number
		// - Bit 7 set to high to denote that more response will follow. Bit 7 will further be referred to as MCFLAG (Multi-Command Flag)
		// - When bit 7 set to low means this was a last response to the specified command sequence
		//Command ID notes:
		// - Values [0-127] signify a valid command ID
		// - Bit 7 set to high to denote that the request is related to the command's meta information. Bit 7 will further be referred to as METAFLAG
		// - For the Meshwork-defined commands, the METAFLAG is only valid for PROPERTY_GET and PROPERTY_REP commands. Its usage elsewhere must result in ACK(Invalid)
		// - Command ID values [0-31] are reserved for Meshwork.

		//Command flags and masks
		static const uint8_t BASERF_CMD_MASK 				= 0x7F;
		static const uint8_t BASERF_CMDFLAG_MULTICOMMAND 	= 0x80;

		//General commands
		static const uint8_t BASERF_CMD_ACK 				= 0x00;

		//PROPERTY commands
		static const uint8_t BASERF_CMD_PROPERTY_GET 		= 0x10;
		static const uint8_t BASERF_CMD_PROPERTY_REP 		= 0x11;
		static const uint8_t BASERF_CMD_PROPERTY_SET		= 0x12;

#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
		//META commands and flags
		static const uint8_t BASERF_CMD_META_DEVICE_GET		= 0x20;
		static const uint8_t BASERF_CMD_META_DEVICE_REP 	= 0x21;
		static const uint8_t BASERF_CMD_META_CLUSTER_GET	= 0x22;
		static const uint8_t BASERF_CMD_META_CLUSTER_REP 	= 0x23;
		static const uint8_t BASERF_CMD_META_ENDPOINT_GET	= 0x24;
		static const uint8_t BASERF_CMD_META_ENDPOINT_REP 	= 0x25;
		static const uint8_t BASERF_CMD_METAFLAG_ALL_ENDPOINTS	= 1 << 7;
		static const uint8_t BASERF_CMD_METAFLAG_ALL_CLUSTERS	= 1 << 6;
#endif

		//Sequence flags and commands
		static const uint8_t BASERF_SEQ_MASK 				= 0x7F;
		static const uint8_t BASERF_SEQFLAG_META 			= 0x80;

		//Status fields for BASERF_CMD_ACK
		static const uint8_t BASERF_CMD_ACKFLAG_ERROR  				= 1 << 7;
		static const uint8_t BASERF_CMD_ACK_STATUS_PROCESSED  		= 0x01;
		static const uint8_t BASERF_CMD_ACK_STATUS_SCHEDULED  		= 0x02;
		static const uint8_t BASERF_CMD_ACK_STATUS_INVALID  		= 0x01 | BASERF_CMD_ACKFLAG_ERROR;
		static const uint8_t BASERF_CMD_ACK_STATUS_FORBIDDEN  		= 0x02 | BASERF_CMD_ACKFLAG_ERROR;
		static const uint8_t BASERF_CMD_ACK_STATUS_NOT_SUPPORTED	= 0x03 | BASERF_CMD_ACKFLAG_ERROR;
		static const uint8_t BASERF_CMD_ACK_STATUS_NO_ACK			= 0x04 | BASERF_CMD_ACKFLAG_ERROR;//timeout or general network failure


		/** First error code from the L7 range. */
		static const int8_t ERROR_BEGIN_L7 = -64;
		/** Invalid data acquired for sending. */
		static const int8_t ERROR_INVALID_DATA = -64;
		/** Last error code from the L7 range. */
		static const int8_t ERROR_END_L7 = -95;

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
		struct univmsg_l7_any_t {
			msg_l7_header_t msg_header;
			uint8_t* data;
		};

		//defining ack status here for a clearer separation of results
		typedef int msg_l7_ack_status_t;

		static size_t getDataFromMessage(size_t maxDataLen, uint8_t* data, univmsg_l7_any_t* msg) {
			UNUSED(maxDataLen);
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
			msg->msg_header.dataLen = dataLen - BASERF_MESSAGE_PAYLOAD_HEADERLEN;
			msg->data = &data[2];

			//TODO extract into print method
			MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("SEQ: ") << msg->msg_header.seq << endl;

			if ( msg->msg_header.cmd_mc )
				MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_CMDFLAG_MULTICOMMAND") << endl;

			if ( msg->msg_header.seq_meta )
				MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_SEQFLAG_META") << endl;
			return msg;
		}



		class BaseRFApplication : public Meshwork::L3::Network::ACKProvider {
		public:
			class BaseRFReportListener {
				public:
					virtual bool notify_report(univmsg_l7_any_t* msg) = 0;
			};


		protected:
			Meshwork::L3::Network* m_network;
			Device* m_device;
			BaseRFReportListener* m_base_rf_report_listener;
			uint8_t m_seq;
			uint32_t m_poll_timeout;
			uint32_t m_last_sent_report_time;
			uint32_t m_last_recv_request_time;
			uint16_t m_sent_report_count;
			uint16_t m_recv_request_count;

			uint8_t m_last_message_raw_data[BASERF_MESSAGE_PAYLOAD_MAXLEN];
			univmsg_l7_any_t m_last_message;
			bool m_last_message_valid;

			uint8_t m_last_message_clusterID;
			uint8_t m_last_message_endpointID;
			uint8_t m_last_message_meta_flags;

			bool isLastMessageValid() {
				return m_last_message_valid;
			}

			void setLastMessageValid(bool valid) {
				m_last_message_valid = valid;
			}

			bool isMessageReport(univmsg_l7_any_t* msg) {
				return
#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
						msg->msg_header.cmd_id == BASERF_CMD_META_CLUSTER_REP ||
						msg->msg_header.cmd_id == BASERF_CMD_META_DEVICE_REP ||
						msg->msg_header.cmd_id == BASERF_CMD_META_ENDPOINT_REP ||
#endif
						msg->msg_header.cmd_id == BASERF_CMD_PROPERTY_REP;
			}

			Network::msg_l3_status_t sendMessage(univmsg_l7_any_t* msg);

			msg_l7_ack_status_t sendMessageWithACK(univmsg_l7_any_t* msg, void* bufACK, size_t& bufLen);

			void prepareACK(univmsg_l7_any_t* msg, int ackStatus, void* statusData, size_t statusDataLen);

			size_t prepareACKPayload(univmsg_l7_any_t* msg, int ackStatus, void* statusData, size_t statusDataLen, void* bufACK, size_t& lenACK);

			virtual size_t handleCustomReturnACKPayload(univmsg_l7_any_t* msg, int ackStatus, void* statusData, size_t statusDataLen) {
				UNUSED(msg);UNUSED(ackStatus);UNUSED(statusData);UNUSED(statusDataLen);
				return 0;
			}

			//returns 0 for no payload or number of bytes written in the payload buffer
			int returnACKPayload(uint8_t src, uint8_t port, void* buf, uint8_t len, void* bufACK, size_t lenACK);

			virtual int handleCustomCommand(univmsg_l7_any_t* msg) {
				UNUSED(msg);
				return BASERF_CMD_ACK_STATUS_NOT_SUPPORTED;
			}

			virtual size_t handleCustomReturnACKPayload(uint8_t src, uint8_t port, void* buf, uint8_t len, void* bufACK, size_t lenACK) {
				UNUSED(src);UNUSED(port);UNUSED(buf);UNUSED(len);UNUSED(bufACK);UNUSED(lenACK);
				return 0;
			}

			//Note that these methods will reuse the univmsg_l7_any_t memory and overwrite its data!
			//If it needs to be preserved - copy explicitly elsewhere before calling!
			//It therefore requires univmsg_l7_any_t.data buffer to be allocated to BASERF_MESSAGE_PAYLOAD_MAXLEN - BASERF_MESSAGE_PAYLOAD_HEADERLEN
			//Yes, this seems obscure, but it allows us to reuse the memory already allocated on the stack in most cases
			int sendACK(univmsg_l7_any_t* msg, int ackStatus, void* statusData, size_t statusDataLen);

#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
			Network::msg_l3_status_t sendMetaReportEndpoint(univmsg_l7_any_t* msg, uint8_t flags, bool cmd_mc_last,
					uint8_t clusterID, uint8_t endpointID);

			Network::msg_l3_status_t sendMetaReportCluster(univmsg_l7_any_t* msg, uint8_t flags, bool cmd_mc_last,
					uint8_t clusterID);

			Network::msg_l3_status_t sendMetaReportDevice(univmsg_l7_any_t* msg, uint8_t flags);
#endif

		public:
			BaseRFApplication(Meshwork::L3::Network* network, Device* device,
								BaseRFReportListener* base_rf_report_listener = NULL,
									uint32_t poll_timeout = MW_BASERF_POLLTIMEOUT):
				m_network(network),
				m_device(device),
				m_base_rf_report_listener(base_rf_report_listener),
				m_seq(0),
				m_poll_timeout(poll_timeout),
				m_last_sent_report_time(0),
				m_last_recv_request_time(0),
				m_sent_report_count(0),
				m_recv_request_count(0),
				m_last_message_valid(false)
				{
				}

			uint8_t nextSeq() {
			  m_seq = m_seq < 127 ? m_seq + 1 : 0;
			  return m_seq;
			}

			//TODO Add REP notification function to reporting node. Update m_last_sent_report

			void pollMessage(uint32_t poll_timeout);

			void pollMessage();

#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
			//TODO add sendMetaDeviceGet
			//TODO add sendMetaClusterGet
			//TODO add sendMetaEndpointGet
#endif

			msg_l7_ack_status_t sendPropertyGet(uint8_t nodeID, bool cmd_mc_last, uint8_t clusterID, uint8_t endpointID,
												void* bufAck, size_t& bufAckLen);

			msg_l7_ack_status_t sendPropertySet(uint8_t nodeID, bool cmd_mc_last, uint8_t clusterID, uint8_t endpointID,
												size_t dataLen, uint8_t* data, void* bufAck, size_t& bufAckLen);

			Network::msg_l3_status_t sendPropertyReport(univmsg_l7_any_t* msg, bool cmd_mc_last, uint8_t clusterID, uint8_t endpointID);

			int16_t getClusterIndex(Cluster* cluster) {
				for ( int i = 0; i < m_device->getClusterCount(); i ++ )
					if ( m_device->getCluster(i) == cluster )
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

