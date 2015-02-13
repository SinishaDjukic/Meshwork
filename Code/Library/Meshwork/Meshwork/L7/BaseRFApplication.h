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

using namespace Meshwork::L3;

namespace Meshwork {

	namespace L7 {
	
		class BaseRFApplication {
		protected:
			Network* m_network;
			uint8_t m_cluster_count;
			Cluster** m_clusters;

		public:
			BaseRFApplication(Network* network,  uint8_t cluster_count, Cluster** clusters):
				m_network(network),
				m_cluster_count(cluster_count),
				m_clusters(clusters)
				{}

			void pollRF() {
				//TODO
			}

			int16_t getClusterIndex(Cluster* cluster) {
				for ( int i = 0; i < m_cluster_count; i ++ )
					if ( m_clusters[i] == cluster )
						return i;
				return -1;
			}
		};
	};
};
#endif

