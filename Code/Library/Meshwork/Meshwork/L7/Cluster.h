
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
#ifndef __MESHWORK_L7_CLUSTER_H__
#define __MESHWORK_L7_CLUSTER_H__

#include "Cosa/Types.h"

#include "Meshwork.h"
#include "Meshwork/L3/Network.h"
#include "Endpoint.h"
#include "Device.h"

namespace Meshwork {

	namespace L7 {

		class Endpoint;

		class Device;

		class Cluster {

			protected:
				uint8_t m_type;
				uint8_t m_subtype;
				uint8_t m_endpoint_count;
				Endpoint** m_endpoints;
				Device* m_device;

				void initEndpoints();

			public:
				Cluster(uint8_t type, uint8_t subtype, uint8_t endpoint_count, Endpoint* endpoints[]):
					m_type(type),
					m_subtype(subtype),
					m_endpoint_count(endpoint_count),
					m_endpoints(endpoints),
					m_device(NULL)
					{
						initEndpoints();
					}

				uint8_t getSubtype() {
					return m_subtype;
				}

				uint8_t getType() {
					return m_type;
				}

				uint8_t getEndpointCount() {
					return m_endpoint_count;
				}

				Endpoint** getEndpoints() {
					return m_endpoints;
				}

				void setEndpoints(Endpoint* endpoints[], uint8_t endpoint_count) {
					m_endpoint_count = endpoint_count;
					m_endpoints = endpoints;
					initEndpoints();
				}

				Endpoint* getEndpoint(uint8_t index) {
					return m_endpoints[index];
				}

				int16_t getEndpointIndex(Endpoint* endpoint) {
					for ( int i = 0; i < m_endpoint_count; i ++ )
						if ( m_endpoints[i] == endpoint )
							return i;
					return -1;
				}

				Device* getDevice() {
					return m_device;
				}

				void setDevice(Device* device);


		};//end of Meshwork::L7::Cluster

	};//end of Meshwork::L7

};//end of Meshwork
#endif
