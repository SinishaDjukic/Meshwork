
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

namespace Meshwork {

	namespace L7 {

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

				const static uint8_t REPORTING_MASK_DISCRETE	= 0x01;
				const static uint8_t REPORTING_MASK_THRESHOLD	= 0x02;

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

				//Calculate difference between two unsigned bytes, return as percentage range [0-100]
				static uint8_t calculate_threshold_u8(uint8_t value1, uint8_t value2) {
					uint8_t a = value1 >= value2 ? value1 : value2;
					uint8_t b = value1 < value2 ? value1 : value2;
					return ( (uint16_t) (a - b) * 100 ) >> 8;
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
				const static uint16_t TYPE_SENSOR_MULTILEVEL	= 0x2001;

				//Callback for value changes, which will send RF messages
				class EndpointListener {
					public:
						virtual void propertyChanged(const Endpoint* endpoint, const endpoint_value_t* value) = 0;
				};

			protected:
				uint16_t m_type;
				uint16_t m_unit_type;
				EndpointListener* m_listener;
				endpoint_reporting_configuration_t* m_reporting_configuration;
				Cluster* m_cluster;

			public:
				Endpoint(uint8_t type, uint8_t unit_type,
						EndpointListener* listener,
						endpoint_reporting_configuration_t* reporting_configuration):
					m_type(type),
					m_unit_type(unit_type),
					m_listener(listener),
					m_reporting_configuration(reporting_configuration),
					m_cluster(NULL)
					{}

				virtual void getProperty(endpoint_value_t &value) = 0;

				virtual void setProperty(const endpoint_value_t* value, endpoint_set_status_t* status) = 0;
				
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

		};//end of Meshwork::L7::Endpoint

	};//end of Meshwork::L7

};//end of Meshwork
#endif
