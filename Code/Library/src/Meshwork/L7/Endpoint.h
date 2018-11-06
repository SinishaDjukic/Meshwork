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
#ifndef __MESHWORK_L7_ENDPOINT_H__
#define __MESHWORK_L7_ENDPOINT_H__

#include "Cosa/Types.h"

#include "Meshwork.h"

#include "Cluster.h"
#include "Device.h"

namespace Meshwork {

	namespace L7 {

		class Cluster;

		class Device;

		class Endpoint {

			public:
				typedef struct {
					uint8_t len;
					void* pvalue;
				} endpoint_value_t;

				typedef struct {
					uint8_t status;
					uint8_t len;
					void* pvalue;
				} endpoint_set_status_t;

				const static uint8_t REPORTING_VALUE_ALL		= 0x00;
				const static uint8_t REPORTING_MASK_ADD_REMOVE	= 0x01;
				const static uint8_t REPORTING_MASK_THRESHOLD	= 0x02;
				const static uint8_t REPORTING_MASK_DISCRETE	= 0x04;

				typedef struct {
					uint16_t reporting_flags;//discrete, threshold
					uint8_t max_threshold;//percentage up/down
				} endpoint_reporting_configuration_t;

				static bool report_discrete(endpoint_reporting_configuration_t* rep) {
					return (rep->reporting_flags & REPORTING_MASK_DISCRETE);
				}

				static bool report_threshold(endpoint_reporting_configuration_t* rep, uint8_t threshold) {
					return (rep->reporting_flags & REPORTING_MASK_THRESHOLD) && (rep->max_threshold <= threshold);
				}

				static bool report_all(endpoint_reporting_configuration_t* rep) {
					return rep->reporting_flags == REPORTING_VALUE_ALL;
				}

				//TODO move to utils

				//Calculate difference between two unsigned bytes, return as a percentage range [0-100]
				static uint8_t calculate_threshold_u8(uint8_t value1, uint8_t value2) {
					uint8_t a = value1 >= value2 ? value1 : value2;
					uint8_t b = value1 < value2 ? value1 : value2;
					return ( (uint16_t) (a - b) * 100 ) >> 8;
				}

				//Calculate difference between two unsigned ints, return as a percentage range [0-100]
				static uint8_t calculate_threshold_u16(uint16_t value1, uint16_t value2) {
					uint16_t a = value1 >= value2 ? value1 : value2;
					uint16_t b = value1 < value2 ? value1 : value2;
					return ( (uint32_t) (a - b) * 100 ) >> 16;
				}

				//Calculate difference between two unsigned longs, return as a percentage range [0-100]
				//Clamp if more than 100%. Positive/negative difference treated equally
				static uint8_t calculate_threshold_u32(uint32_t value1, uint32_t value2) {
					uint32_t bigger = value1 >= value2 ? value1 : value2;
					uint32_t lesser = value1 < value2 ? value1 : value2;
					return (uint8_t) ( (bigger - lesser) / bigger );
				}
        
				const static uint8_t STATUS_SET_PROCESSED		= 0;
				const static uint8_t STATUS_SET_INVALID			= 1;
				const static uint8_t STATUS_SET_FORBIDDEN		= 2;
				const static uint8_t STATUS_SET_SCHEDULED		= 3;

				/////////// Type definitions follow ///////////

				//Bitmask for custom type
				const static uint16_t TYPE_MASK_CUSTOM			= 0x8000;//most significant bit

				//BinarySwitch
				const static uint16_t TYPE_SWITCH_BINARY		= 0x1000;
				//MultilevelSwitch
				const static uint16_t TYPE_SWITCH_MULTILEVEL	= 0x1001;

				//BinarySensor
				const static uint16_t TYPE_SENSOR_BINARY		= 0x2000;
				//MultilevelSensor
				const static uint16_t TYPE_SENSOR_MULTILEVEL	= 0x2100;
				//VoltageSensor
				const static uint16_t TYPE_SENSOR_VOLTAGE		= 0x2200;
				//TemperatureSensor
				const static uint16_t TYPE_SENSOR_TEMPERATURE	= 0x2300;

				//Callback for value changes, which will send RF messages
				class EndpointListener {
					public:
						virtual void propertyChanged(Endpoint* endpoint) = 0;
						virtual void wakeup(Endpoint* endpoint) = 0;
				};

			protected:
				uint16_t m_type;
				uint16_t m_unit_type;
				EndpointListener* m_listener;
				endpoint_reporting_configuration_t* m_reporting_configuration;
				Cluster* m_cluster;
				Device* m_device;

			public:
				Endpoint(uint16_t type, uint16_t unit_type,
						EndpointListener* listener,
						endpoint_reporting_configuration_t* reporting_configuration):
					m_type(type),
					m_unit_type(unit_type),
					m_listener(listener),
					m_reporting_configuration(reporting_configuration),
					m_cluster(NULL),
					m_device(NULL)
					{}

				//get the current value; to be used in RF handling loop
				virtual void getProperty(endpoint_value_t* value) = 0;

				//get the current value; to be used in RF handling loop or directly by the app logic
				virtual void setProperty(const endpoint_value_t* value, endpoint_set_status_t* status) = 0;

				//Poll the state and update it, if needed.
				//This function is called by the app whenever it is ready to process
				//outgoing RF messages for endpoints that have changes.
				//E.g. a sleeping device would wake up periodically (or upon interrupt)
				//and call poll() on all endpoints, which would trigger listener notifications,
				//which would then send property change messages
				//If woken during an interrupt the new value would be written into the field
				//and the Endpoint would be marked dirty, so that upon next poll()
				//the listener would be notified
				virtual void poll() = 0;

				uint16_t getUnitType() {
					return m_unit_type;
				}

				uint16_t getType() {
					return m_type;
				}

				Cluster* getCluster() {
					return m_cluster;
				}

				void setCluster(Cluster* cluster) {
					m_cluster = cluster;
				}

				Device* getDevice() {
					return m_device;
				}

				void setDevice(Device* device) {
					m_device = device;
				}

				EndpointListener* getListener() {
					return m_listener;
				}

				void setListener(EndpointListener* listener) {
					m_listener = listener;
				}

				endpoint_reporting_configuration_t* getReportingConfiguration() {
					return m_reporting_configuration;
				}

				void setReportingConfiguration(endpoint_reporting_configuration_t* reporting_configuration) {
					m_reporting_configuration = reporting_configuration;
				}


		};//end of Meshwork::L7::Endpoint

	};//end of Meshwork::L7

};//end of Meshwork
#endif
