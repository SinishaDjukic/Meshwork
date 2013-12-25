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
#ifndef __MESHWORK_L3_NETWORKV1_CACHINGROUTEPROVIDER_H__
#define __MESHWORK_L3_NETWORKV1_CACHINGROUTEPROVIDER_H__

#include "Cosa/Wireless.hh"
#include "Meshwork/L3/Network.h"

/**
 * TODO
 */
class CachingRouteProvider: public Meshwork::L3::Network::RouteProvider {

protected:

	uint8_t src;
	unit8_t updatePolicy;
	RouteCache cache;

public:

	//invalidates a route when the first message to Dst fails
	const uint8_t UPDATE_ON_FAIL = 0;
	//never invalidate once we have acquired a route
	const uint8_t UPDATE_NEVER = 1;
	//always update with latest route to node
	const uint8_t UPDATE_ALWAYS = 2;

	CachingRouteProvider(unit8_t src, unit8_t maxNodes,
			unit8_t maxHops, unit8_t updatePolicy)
	{
		m_src(src);
		m_updatePolicy(updatePolicy);
		//TODO will this invoke the constructor correctly?
		//TODO should we add an option to pass a cache instance impl explicitly?
		cache(maxNodes, maxHops);
	};

	  virtual void set_address(uint8_t src) = 0;
	  virtual uint8_t get_routeCount(uint8_t dst) = 0;
	  virtual route_t* get_route(uint8_t dst, uint8_t index) = 0;
	  virtual void route_found(route_t* route) = 0;
	  virtual void route_failed(route_t* route) = 0;


};
#endif

