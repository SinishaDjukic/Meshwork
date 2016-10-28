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
#ifndef __TEST_BASERF_BASECOMMANDS_COMMONCONFIG_H__
#define __TEST_BASERF_BASECOMMANDS_COMMONCONFIG_H__

#include <Meshwork/L7/Unit.h>
#include <Meshwork/L7/Device.h>
#include <Meshwork/L7/Cluster.h>
#include <Meshwork/L7/Endpoint.h>
#include <Meshwork/L7/BaseRFApplication.h>
#include <Meshwork/L7/Cluster.cpp>
#include <Meshwork/L7/Device.cpp>
#include <Meshwork/L7/BaseRFApplication.cpp>

using namespace Meshwork::L7;

///////////////////////////////////////////////////////////////////////////////
/////////////////////// SECTION: BUILD-TIME CONFIGURATION /////////////////////
////////////////// ~feel free to edit and adapt to your needs~ ////////////////
///////////////////////////////////////////////////////////////////////////////

//Select your RF chip. Currently, only NRF24L01P is supported
#define MW_RF_SELECT 				MW_RF_NRF24L01P

//Network configuration is hardcoded here
#define TEST_NODE_NWK_ID				1
#define TEST_NODE_CHANNEL_ID			0

#define TEST_SENDER_NODE_ID				230
#define TEST_SENDER_DEVICE_TYPE			255
#define TEST_SENDER_DEVICE_SUBTYPE		255
#define TEST_SENDER_CLUSTER_COUNT		0

#define TEST_RECEIVER_NODE_ID			231
#define TEST_RECEIVER_DEVICE_TYPE		1
#define TEST_RECEIVER_DEVICE_SUBTYPE	2
#define TEST_RECEIVER_CLUSTER_COUNT		2
#define TEST_RECEIVER_CLUSTER_0_ENDPOINT_COUNT		3
#define TEST_RECEIVER_CLUSTER_1_ENDPOINT_COUNT		2

struct test_endpoint_t {
	uint16_t type;
	uint16_t subtype;
	bool verified;
};

test_endpoint_t desc_receiver_0_0 = {Endpoint::TYPE_SWITCH_BINARY, Unit::UNIT_BINARY_BYTE, false};
test_endpoint_t desc_receiver_0_1 = {Endpoint::TYPE_SWITCH_BINARY, Unit::UNIT_BINARY_BYTE, false};
test_endpoint_t desc_receiver_0_2 = {Endpoint::TYPE_SWITCH_BINARY, Unit::UNIT_BINARY_BYTE, false};
test_endpoint_t* desc_receiver_0_endpoints[TEST_RECEIVER_CLUSTER_0_ENDPOINT_COUNT] = {&desc_receiver_0_0, &desc_receiver_0_1, &desc_receiver_0_2};

test_endpoint_t desc_receiver_1_0 = {Endpoint::TYPE_SENSOR_BINARY, Unit::UNIT_BINARY_BYTE, false};
test_endpoint_t desc_receiver_1_1 = {Endpoint::TYPE_SENSOR_MULTILEVEL, Unit::UNIT_PERCENTAGE_BYTE, false};
test_endpoint_t* desc_receiver_1_endpoints[TEST_RECEIVER_CLUSTER_1_ENDPOINT_COUNT] = {&desc_receiver_1_0, &desc_receiver_1_1};

struct test_cluster_t {
	uint8_t type;
	uint8_t subtype;
	size_t endpoint_count;
	test_endpoint_t** endpoints;
	bool verified;
};

test_cluster_t desc_receiver_0 = {20, 10, TEST_RECEIVER_CLUSTER_0_ENDPOINT_COUNT, desc_receiver_0_endpoints, false};
test_cluster_t desc_receiver_1 = {21, 11, TEST_RECEIVER_CLUSTER_1_ENDPOINT_COUNT, desc_receiver_1_endpoints, false};
test_cluster_t* desc_receiver_clusters[TEST_RECEIVER_CLUSTER_COUNT] = {&desc_receiver_0, &desc_receiver_1};

struct test_device_t {
	uint8_t type;
	uint8_t subtype;
	size_t cluster_count;
	test_cluster_t** clusters;
	bool verified;
};

test_device_t desc_receiver_device = {TEST_RECEIVER_DEVICE_TYPE, TEST_RECEIVER_DEVICE_SUBTYPE, TEST_RECEIVER_CLUSTER_COUNT, desc_receiver_clusters, false};

#endif
