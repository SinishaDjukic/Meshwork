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
#ifndef __MESHWORK_L7_DEVICE_CPP__
#define __MESHWORK_L7_DEVICE_CPP__

#include "Meshwork/L7/Device.h"

using namespace Meshwork::L7;
using Meshwork::L3::Network;
using Meshwork::L7::Device;
using Meshwork::L7::Endpoint;

void Device::initClusters() {
	for ( int i = 0; i < m_cluster_count; i ++ )
		m_clusters[i]->setDevice(this);
}

Endpoint* Device::getEndpoint(uint8_t clusterID, uint8_t endpointID) {
	Endpoint* result = NULL;
	if ( clusterID < m_cluster_count ) {
		Cluster* cluster = m_clusters[clusterID];
		if ( endpointID < cluster->getEndpointCount() )
			result = cluster->getEndpoint(endpointID);
	}
	return result;
}
#endif
