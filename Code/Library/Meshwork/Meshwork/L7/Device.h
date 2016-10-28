
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
#ifndef __MESHWORK_L7_DEVICE_H__
#define __MESHWORK_L7_DEVICE_H__

#include "Cosa/Types.h"

#include "Meshwork.h"
#include "Cluster.h"
#include "Endpoint.h"

namespace Meshwork {

	namespace L7 {

		class Cluster;

		class Endpoint;

		class Device {

			protected:
				uint8_t m_type;
				uint8_t m_subtype;
				uint8_t m_cluster_count;
				Cluster** m_clusters;

				void initClusters();

			public:
				Device(uint8_t type, uint8_t subtype, uint8_t cluster_count, Cluster** clusters):
					m_type(type),
					m_subtype(subtype),
					m_cluster_count(cluster_count),
					m_clusters(clusters)
					{
						initClusters();
					}

				uint8_t getSubtype() {
					return m_subtype;
				}

				uint8_t getType() {
					return m_type;
				}

				uint8_t getClusterCount() {
					return m_cluster_count;
				}

				Cluster** getClusters() {
					return m_clusters;
				}

				Cluster* getCluster(uint8_t index) {
					return m_clusters[index];
				}

				Endpoint* getEndpoint(uint8_t clusterID, uint8_t endpointID);

				int16_t getClusterIndex(Cluster* cluster) {
					for ( int i = 0; i < m_cluster_count; i ++ )
						if ( m_clusters[i] == cluster )
							return i;
					return -1;
				}


		};//end of Meshwork::L7::Device

	};//end of Meshwork::L7

};//end of Meshwork
#endif
