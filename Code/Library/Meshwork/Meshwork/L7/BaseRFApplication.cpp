/**
 * This file is part of the Meshwork project.
 *
 * Copyright (C) 2015, Sinisha Djukic
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
#ifndef __MESHWORK_L7_BASERFAPPLICATION_CPP__
#define __MESHWORK_L7_BASERFAPPLICATION_CPP__

#include "Meshwork/L3/Network.h"
#include "Meshwork/L7/BaseRFApplication.h"

using namespace Meshwork::L7;
using Meshwork::L3::Network;

//TODO review after latest changes and ACK handler
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
//Note 1: we will NOT send an automatic REP to the originator of the message;
//        they can ask for a value explicitly in a separate message, if interested
//Note 2: the Endpoint implementation must decide when to send the REP to the reporting node, based on the threshold setting



//returns 0 for no payload or number of bytes written in the payload buffer
int BaseRFApplication::returnACKPayload(uint8_t src, uint8_t port, void* buf, uint8_t len, void* bufACK, size_t lenACK) {

	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("[ACK] Src=") << src << PSTR(", Len=") << len << endl;
	MW_LOG_DEBUG_ARRAY(MW_LOG_BASERF, PSTR("\t...L7 DATA RECV: "), buf, len);
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << endl << endl;

	setLastMessageValid(false);

	if ( src != BASERF_MESSAGE_PORT ) {
		return handleCustomReturnACKPayload(src, port, buf, len, bufACK, lenACK);
	} else {
		m_last_message.msg_header.src = src;
		m_last_message.msg_header.port = port;

		//3) parse the L7 payload structure (Seq, ID, Data)
		getMessageFromData(&m_last_message, len, (uint8_t*) buf);

		//3.1) update m_last_recv_request
		m_last_recv_request_time = RTT::millis();

		//3.2) update m_recv_request_count
		m_recv_request_count ++;

		if ( m_last_message.msg_header.cmd_id == BASERF_CMD_PROPERTY_GET || m_last_message.msg_header.cmd_id == BASERF_CMD_PROPERTY_SET ) {
			m_last_message_clusterID = m_last_message.data[0];
			m_last_message_endpointID = m_last_message.data[1];
			Endpoint* endpoint = m_device->getEndpoint(m_last_message_clusterID, m_last_message_endpointID);

			if ( endpoint != NULL ) {
				setLastMessageValid(true);//really needed?

				//4.b.1) if SET then continue
				Endpoint::endpoint_set_status_t status;
				Endpoint::endpoint_value_t value;
				value.len = m_last_message.msg_header.dataLen;
				value.pvalue = m_last_message.data;

				//6.b.1) SET: read data into empty endpoint_value_t, create empty endpoint_set_status_t and call setProperty
				endpoint->setProperty(&value, &status);

				//6.b.2) send back ACK(endpoint_set_status_t)
				return prepareACKPayload(&m_last_message, status.status, status.pvalue, status.len, &bufACK, lenACK);
			} else {
				return prepareACKPayload(&m_last_message, BASERF_CMD_ACK_STATUS_INVALID, NULL, 0, &bufACK, lenACK);
			}

#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
		} else if ( m_last_message.msg_header.cmd_id == BASERF_CMD_META_ENDPOINT_GET ) {
			m_last_message_meta_flags = m_last_message.data[0];
			m_last_message_clusterID = m_last_message.data[1];
			m_last_message_endpointID = m_last_message.data[2];
			if ( m_device->getEndpoint(m_last_message_clusterID, m_last_message_endpointID) ) {
				setLastMessageValid(true);
				return prepareACKPayload(&m_last_message, BASERF_CMD_ACK_STATUS_PROCESSED, NULL, 0, &bufACK, lenACK);
			} else {
				return prepareACKPayload(&m_last_message, BASERF_CMD_ACK_STATUS_INVALID, NULL, 0, &bufACK, lenACK);
			}
		} else if ( m_last_message.msg_header.cmd_id == BASERF_CMD_META_CLUSTER_GET ) {
			m_last_message_meta_flags = m_last_message.data[0];
			m_last_message_clusterID = m_last_message.data[1];
			if ( m_device->getCluster(m_last_message_clusterID) ) {
				setLastMessageValid(true);
				return prepareACKPayload(&m_last_message, BASERF_CMD_ACK_STATUS_PROCESSED, NULL, 0, &bufACK, lenACK);
			} else {
				return prepareACKPayload(&m_last_message, BASERF_CMD_ACK_STATUS_INVALID, NULL, 0, &bufACK, lenACK);
			}
		} else if ( m_last_message.msg_header.cmd_id == BASERF_CMD_META_DEVICE_GET ) {
			m_last_message_meta_flags = m_last_message.data[0];
			setLastMessageValid(true);
			return prepareACKPayload(&m_last_message, BASERF_CMD_ACK_STATUS_PROCESSED, NULL, 0, &bufACK, lenACK);
#endif

		} else if ( m_last_message.msg_header.cmd_id == BASERF_CMD_ACK ) {
			//4.c.1) else return
			MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("INVALID") << m_last_message.msg_header.cmd_id << endl;
			return prepareACKPayload(&m_last_message, BASERF_CMD_ACK_STATUS_INVALID, NULL, 0, &bufACK, lenACK);
		} else {
			//4.c.1) else return
			if ( handleCustomCommand(&m_last_message) == BASERF_CMD_ACK_STATUS_NOT_SUPPORTED ) {
				MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("NOT_SUPPORTED: ") << m_last_message.msg_header.cmd_id << endl;
				return prepareACKPayload(&m_last_message, BASERF_CMD_ACK_STATUS_NOT_SUPPORTED, NULL, 0, &bufACK, lenACK);
			} else {//else no ACK
				return 0;
			}
		}
	}
}

void BaseRFApplication::pollMessage(uint32_t poll_timeout) {
	//Note: this function implies the Wireless::Driver is up and running, so the app
	//needs to manage the RF begin/end cycles

	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("[RECV] Timeout=") << poll_timeout << endl;

	////////////////////////////////////////////////////////////////
	////////////////// IMPLEMENTATION //////////////////////////////
	////////////////////////////////////////////////////////////////

	//1) receive a message

	uint8_t src, port;
	size_t dataLen = BASERF_MESSAGE_PAYLOAD_MAXLEN;

	uint32_t start = RTT::millis();
	while (true) {
		trace << endl;
		setLastMessageValid(false);

		int result = m_network->recv(src, port, m_last_message_raw_data, dataLen, poll_timeout, this);
		if ( result == Meshwork::L3::Network::OK &&
				port == BASERF_MESSAGE_PORT &&
					dataLen >= BASERF_MESSAGE_PAYLOAD_HEADERLEN &&
						dataLen <= BASERF_MESSAGE_PAYLOAD_MAXLEN ) {
			break;
		}
	//2) if result != OK then return, else continue
		if ( RTT::since(start) >= MW_BASERF_POLLTIMEOUT ) {
			MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("[RECV] No valid message, return") << endl;
			return;
		}
	}
	if ( !isLastMessageValid() )
		return;

	//4) process Command
	if ( m_last_message.msg_header.cmd_id == BASERF_CMD_PROPERTY_REP ) {
		//4.a.1) if REP then call REP handler
		MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_CMD_PROPERTY_REP") << PSTR(": Listener: ") << m_base_rf_report_listener << endl;
		if ( m_base_rf_report_listener != NULL )
			m_base_rf_report_listener->notify_report(&m_last_message);

	//5) parse Cluster ID and Endpoint ID
	} else if ( m_last_message.msg_header.cmd_id == BASERF_CMD_PROPERTY_GET ) {
		//4.b.1) if GET then continue
		MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("BASERF_CMD_PROPERTY_GET") << endl;
		sendPropertyReport(&m_last_message, true, m_last_message_clusterID, m_last_message_endpointID);

	} else if ( m_last_message.msg_header.cmd_id == BASERF_CMD_PROPERTY_SET ) {
		//nothing here... already handled in returnACKPayload

#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
	} else if ( m_last_message.msg_header.cmd_id == BASERF_CMD_META_ENDPOINT_GET ) {
		sendMetaReportEndpoint(&m_last_message, m_last_message_meta_flags, true, m_last_message_clusterID, m_last_message_endpointID);
	} else if ( m_last_message.msg_header.cmd_id == BASERF_CMD_META_CLUSTER_GET ) {
		sendMetaReportCluster(&m_last_message, m_last_message_meta_flags, true, m_last_message_clusterID);
	} else if ( m_last_message.msg_header.cmd_id == BASERF_CMD_META_DEVICE_GET ) {
		uint8_t flags = m_last_message.data[0];
		sendMetaReportDevice(&m_last_message, flags);
#endif

	}
}

void BaseRFApplication::pollMessage() {
	pollMessage(m_poll_timeout);
}

#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
Network::msg_l3_status_t BaseRFApplication::sendMetaReportDevice(univmsg_l7_any_t* msg, uint8_t flags) {
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF);

	bool allClusters = flags & BASERF_CMD_METAFLAG_ALL_CLUSTERS;
	uint8_t cluster_count = m_device->getClusterCount();
	Network::msg_l3_status_t result = ERROR_INVALID_DATA;

	msg->data[2] = m_device->getType();
	msg->data[3] = m_device->getSubtype();
	msg->data[4] = cluster_count;

	msg->msg_header.cmd_id = BASERF_CMD_META_DEVICE_REP;
	msg->msg_header.cmd_mc = allClusters && cluster_count > 0;
	msg->msg_header.seq_meta = true;
	msg->msg_header.dataLen = 5;

	result = sendMessage(msg);
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;

	if ( result >= Meshwork::L3::Network::OK ) {
		if ( allClusters ) {
			result = sendMetaReportCluster(msg, flags, true, 0);
		}
	}
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;
	return result;
}
#endif

#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
Network::msg_l3_status_t BaseRFApplication::sendMetaReportCluster(univmsg_l7_any_t* msg, uint8_t flags, bool cmd_mc_last,
		uint8_t clusterID) {
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF);

	bool allEndpoints = flags & BASERF_CMD_METAFLAG_ALL_ENDPOINTS;
	bool allClusters = flags & BASERF_CMD_METAFLAG_ALL_CLUSTERS;
	uint8_t nextClusterID = allClusters? 0 : clusterID;
	uint8_t cluster_count = m_device->getClusterCount();
	uint8_t lastClusterID = allClusters ? cluster_count : clusterID;
	Network::msg_l3_status_t result = ERROR_INVALID_DATA;

	Cluster* nextCluster = NULL;

	do {
		nextCluster = NULL;m_device->getCluster(nextClusterID);

		msg->data[2] = nextClusterID;
		msg->data[3] = nextCluster->getType();
		msg->data[4] = nextCluster->getSubtype();
		msg->data[5] = nextCluster->getEndpointCount();

		msg->msg_header.cmd_id = BASERF_CMD_META_CLUSTER_REP;
		//about the cmd_mc flag:
		//a) if cmd_mc_last is false then we are not the last reporter and all our reports have it as true
		//a) if cmd_mc_last is true then we are the last reporter in the chain and then:
		//b.1) if we are sending just one cluster report then it is false
		//b.2) if we are sending report for all clusters then it is true for all but the last one, unless we also send allEndpoints
		msg->msg_header.cmd_mc = cmd_mc_last ? (allClusters ? (nextClusterID == lastClusterID && !allEndpoints) : false) : true;
		msg->msg_header.seq_meta = true;
		msg->msg_header.dataLen = 6;

		result = sendMessage(msg);
		MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;

		if ( result >= Meshwork::L3::Network::OK ) {
			if ( allEndpoints ) {
				result = sendMetaReportEndpoint(msg, flags, cmd_mc_last ? (allClusters ? (nextClusterID == lastClusterID) : false) : true, nextClusterID, 0);
				if ( result < Meshwork::L3::Network::OK )
					break;
			}
		}

	} while ( ++nextClusterID <= lastClusterID );
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;
	return result;
}
#endif

#ifdef MW_SUPPORT_BASERF_SUPPORTED_META
Network::msg_l3_status_t BaseRFApplication::sendMetaReportEndpoint(univmsg_l7_any_t* msg, uint8_t flags, bool cmd_mc_last,
		uint8_t clusterID, uint8_t endpointID) {
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF);

	bool allEndpoints = flags & BASERF_CMD_METAFLAG_ALL_ENDPOINTS;
	uint8_t nextEndpointID = allEndpoints ? 0 : endpointID;
	Cluster* cluster = m_device->getCluster(clusterID);
	uint8_t lastEndpointID = allEndpoints ? cluster->getEndpointCount() - 1 : endpointID;
	Network::msg_l3_status_t result = ERROR_INVALID_DATA;

	Endpoint* nextEndpoint = cluster->getEndpoint(nextEndpointID);

	do {
		msg->data[2] = clusterID;
		msg->data[3] = nextEndpointID;

		uint16_t temp = nextEndpoint->getType();
		msg->data[4] = (temp >> 8) & 0xFF;
		msg->data[5] = (temp >> 0) & 0xFF;

		temp = nextEndpoint->getUnitType();
		msg->data[6] = (temp >> 8) & 0xFF;
		msg->data[7] = (temp >> 0) & 0xFF;

		msg->msg_header.cmd_id = BASERF_CMD_META_ENDPOINT_REP;
		//about the cmd_mc flag:
		//a) if cmd_mc_last is false then we are not the last reporter and all our reports have it as true
		//a) if cmd_mc_last is true then we are the last reporter in the chain and then:
		//b.1) if we are sending just one endpoint report then it is false
		//b.2) if we are sending report for all endpoints then it is true for all but the last one
		msg->msg_header.cmd_mc = cmd_mc_last ? (allEndpoints ? (nextEndpointID == lastEndpointID) : false) : true;
		msg->msg_header.seq_meta = true;
		msg->msg_header.dataLen = 8;

		result = sendMessage(msg);
		MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;

		if ( result < Meshwork::L3::Network::OK )
			break;

	} while ( ++nextEndpointID <= lastEndpointID );
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;
	return result;
}
#endif


msg_l7_ack_status_t BaseRFApplication::sendPropertyGet(uint8_t nodeID, bool cmd_mc_last, uint8_t clusterID, uint8_t endpointID,
														void* bufAck, size_t& bufAckLen) {
	UNUSED(clusterID);UNUSED(endpointID);
	univmsg_l7_any_t msg;
	msg.msg_header.src = nodeID;
	msg.msg_header.port = BASERF_MESSAGE_PORT;
	msg.msg_header.cmd_id = BASERF_CMD_PROPERTY_SET;
	msg.msg_header.cmd_mc = cmd_mc_last;
	msg.msg_header.seq_meta = false;
	msg.msg_header.dataLen = 2;
	uint8_t cmd_data[2] = {clusterID, endpointID};
	msg.data = cmd_data;

	msg_l7_ack_status_t result = sendMessageWithACK(&msg, bufAck, bufAckLen);

	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("Result: ") << result;
	return result;
}

msg_l7_ack_status_t BaseRFApplication::sendPropertySet(uint8_t nodeID, bool cmd_mc_last, uint8_t clusterID, uint8_t endpointID,
														size_t dataLen, uint8_t* data, void* bufAck, size_t& bufAckLen) {
	//TODO range check: dataLen < BASERF_MESSAGE_PAYLOAD_MAXLEN - BASERF_MESSAGE_PAYLOAD_HEADERLEN - 2;//12
	univmsg_l7_any_t msg;
	msg.msg_header.src = nodeID;
	msg.msg_header.port = BASERF_MESSAGE_PORT;
	msg.msg_header.cmd_id = BASERF_CMD_PROPERTY_SET;
	msg.msg_header.cmd_mc = cmd_mc_last;
	msg.msg_header.seq_meta = false;

	//TODO add the property set command params (cluster, endpoint, etc) - and then the data itself
	msg.msg_header.dataLen = dataLen;
	msg.data = data;

	msg_l7_ack_status_t result = sendMessageWithACK(&msg, bufAck, bufAckLen);

	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("Result: ") << result;
	return result;
}

Network::msg_l3_status_t BaseRFApplication::sendPropertyReport(univmsg_l7_any_t* msg, bool cmd_mc_last,
		uint8_t clusterID, uint8_t endpointID) {
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF);

	//6.a.1) GET: create empty endpoint_value_t and call getProperty
	Endpoint::endpoint_value_t value;
	value.len = BASERF_MESSAGE_PAYLOAD_MAXLEN - BASERF_MESSAGE_PAYLOAD_HEADERLEN - 2;//12
	uint8_t maxLen = value.len;//12
	value.pvalue = &msg->data[2];//reusing the allocated buffer with a proper offset
	Endpoint* endpoint = m_device->getCluster(clusterID)->getEndpoint(endpointID);
	endpoint->getProperty(&value);

	int result = ERROR_INVALID_DATA;

	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("Endpoint value len: ") << value.len;
	if ( value.len > 0 && value.len <= maxLen ) {
		//prepare the message structure
		msg->msg_header.cmd_id = BASERF_CMD_PROPERTY_REP;
		msg->msg_header.cmd_mc = cmd_mc_last ? false : true;
		msg->msg_header.dataLen = value.len + 2;//extra bytes in the header

		msg->data[0] = clusterID;
		msg->data[1] = endpointID;

		result = sendMessage(msg);
	} else {
		MW_LOG_WARNING(MW_LOG_BASERF, "No report sent! Invalid data for C:E=%d:%d", clusterID, endpointID);
	}
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;
	return result;
}

void BaseRFApplication::prepareACK(univmsg_l7_any_t* msg, int ackStatus, void* statusData, size_t statusDataLen) {
	UNUSED(ackStatus);
	msg->msg_header.cmd_id = BASERF_CMD_ACK;
	msg->msg_header.dataLen = statusDataLen;
	msg->data = (uint8_t *) statusData;
}

size_t BaseRFApplication::prepareACKPayload(univmsg_l7_any_t* msg, int ackStatus, void* statusData, size_t statusDataLen, void* bufACK, size_t& lenACK) {
	prepareACK(msg, ackStatus, statusData, statusDataLen);
	size_t rawDataLen = getDataFromMessage(lenACK, (uint8_t*) bufACK, msg);
	return rawDataLen;
}

Network::msg_l3_status_t BaseRFApplication::sendACK(univmsg_l7_any_t* msg, int ackStatus, void* statusData, size_t statusDataLen) {
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("ackStatus=") << ackStatus;
	prepareACK(msg, ackStatus, statusData, statusDataLen);
	Network::msg_l3_status_t result = sendMessage(msg);
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;
	return result;
}

//returns L3 send status
Network::msg_l3_status_t BaseRFApplication::sendMessage(univmsg_l7_any_t* msg) {
	//generate the raw message payload
	size_t rawDataLen = BASERF_MESSAGE_PAYLOAD_HEADERLEN + msg->msg_header.dataLen;
	uint8_t rawData[rawDataLen];
	rawDataLen = getDataFromMessage(rawDataLen, rawData, msg);
	size_t bufAckLen = 0;
	int result = m_network->send(msg->msg_header.src, msg->msg_header.port, (const void*) &rawData, rawDataLen, (void*) NULL, bufAckLen);
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;
	if ( result >= Meshwork::L3::Network::OK && isMessageReport(msg) ) {
		m_last_sent_report_time = RTT::millis();
		m_sent_report_count ++;
	}
	return result;
}

//returns ACK status, and fills in the ACK data, if required
msg_l7_ack_status_t BaseRFApplication::sendMessageWithACK(univmsg_l7_any_t* msg, void* bufACK, size_t& bufLen) {
	//generate the raw message payload
	size_t rawDataLen = BASERF_MESSAGE_PAYLOAD_HEADERLEN + msg->msg_header.dataLen;
	uint8_t rawData[rawDataLen];
	size_t ackDataLen = Meshwork::L3::NetworkV1::NetworkV1::ACK_PAYLOAD_MAX;
	uint8_t ackData[ackDataLen];
	rawDataLen = getDataFromMessage(rawDataLen, rawData, msg);
	int result = m_network->send(msg->msg_header.src, msg->msg_header.port, (const void*) &rawData, rawDataLen, ackData, ackDataLen);
	if ( result != Meshwork::L3::Network::OK ) {
		result = BASERF_CMD_ACK_STATUS_NO_ACK;
		bufLen = 0;
	} else {//OK, now parse the response and verify its structure
		univmsg_l7_any_t response;
		getMessageFromData(&response, ackDataLen, ackData);
		if ( response.msg_header.cmd_id != BASERF_CMD_ACK ) {
			result = BASERF_CMD_ACK_STATUS_NO_ACK;
			bufLen = 0;
		} else {//is ACK, now get the status
			result = response.data[0];
			if ( response.msg_header.dataLen > 0 ) {
				bufLen = response.msg_header.dataLen;
				memcpy(&bufACK, msg->data, bufLen);
			}
		}
	}
	MW_LOG_DEBUG_TRACE(MW_LOG_BASERF) << PSTR("result=") << result;
	if ( result >= Meshwork::L3::Network::OK && isMessageReport(msg) ) {
		m_last_sent_report_time = RTT::millis();
		m_sent_report_count ++;
	}
	return result;
}

#endif
