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
#ifndef __MESHWORK_MESH_CACHINGROUTEADVISOR_H__
#define __MESHWORK_MESH_CACHINGROUTEADVISOR_H__

#include "Cosa/Wireless.hh"
#include "Meshwork/Mesh.h"

/**
 * Holds up routes for up to m_maxNodes destinations. One route per destination only
 */
class RouteCache {

protected:
	unit8_t m_maxNodes;
	unit8_t m_maxHops;
	//TODO structure, route_t*

public:
	RouteCache(unit8_t maxNodes, unit8_t maxHops)
	{
		m_maxNodes(maxNodes);
		m_maxHops(maxHops);
	};

	unit8_t get_maxNodes() {
		return m_maxNodes;
	}

	unit8_t addOrUpdate(uint8_t dst, void* route, uint8_t hops) {
		if ( hops > m_maxHops )
			return -1;
		//TODO
	}

	void remove(uint8_t dst) {
		//TODO
	}

	//TODO extend the class with the one writing to EEPROM?

};
#endif

