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
#ifndef __EXAMPLES_CONSOLE_STATICROUTEPROVIDER_H__
#define __EXAMPLES_CONSOLE_STATICROUTEPROVIDER_H__

#include "Cosa/Trace.hh"
#include "Cosa/Wireless.hh"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

class StaticRouteProvider: public Meshwork::L3::NetworkV1::NetworkV1::RouteProvider {

protected:
	static const uint8_t MAX_STATIC_ROUTES = 5;
	static const uint8_t MAX_STATIC_ROUTES_HOPS = 8;
	Meshwork::L3::NetworkV1::NetworkV1::route_t routes[MAX_STATIC_ROUTES];
	uint8_t hops[MAX_STATIC_ROUTES][MAX_STATIC_ROUTES_HOPS];
	
	void print_route(Meshwork::L3::NetworkV1::NetworkV1::route_t* route) {
		trace << PSTR("Route: src=") << route->src << PSTR(", dst=") << route->dst << PSTR(", hopCount=") << route->hopCount;
		trace << PSTR("\tHops: ");
		if ( route->hopCount > 0 )
			trace.print(route->hops, route->hopCount, IOStream::hex, route->hopCount);
		trace.println();
	}

public:
	StaticRouteProvider()
	{
		route_reset();
	};
	
	void set_route(uint8_t index, uint8_t src, uint8_t dst, uint8_t* routeHops, uint8_t hopCount) {
		routes[index].src = src;
		routes[index].dst = dst;
		routes[index].hopCount = hopCount;
		memcpy(hops[index], routeHops, hopCount);
		trace << PSTR("Set route at=") << index << PSTR(", src=") << src << PSTR(", dst=") << dst << PSTR(", hopCount=") << hopCount;
		trace << PSTR("\tHops: ");
		if ( hopCount > 0 )
			trace.print(hops[index], hopCount, IOStream::hex, hopCount);
		trace.println();
	}
	
	void route_reset() {
		for (int i = 0; i < MAX_STATIC_ROUTES; i ++ ) {
			routes[i].hopCount = 0;
			routes[i].src = 0;
			routes[i].dst = 0;
			routes[i].hops = hops[i];
			for (int j = 0; i < MAX_STATIC_ROUTES_HOPS; i ++ ) {
				hops[i][j] = 0;
			}
		}
	}
	
	uint8_t get_max_routes() {
		return MAX_STATIC_ROUTES;
	}

	uint8_t get_max_route_hops() {
		return MAX_STATIC_ROUTES_HOPS;
	}
	
	void set_address(uint8_t src) {
		trace << PSTR("Set address: src=") << src << PSTR("\n");
	}
	uint8_t get_routeCount(uint8_t dst) {
		uint8_t result = 0;
		for ( int i = 0; i < MAX_STATIC_ROUTES; i ++ ) {
			if ( routes[i].dst == dst ) {
				result ++;
			}
		}
		trace << PSTR("Get route count: dst=") << dst << PSTR(", result=") << result << PSTR("\n");
		return result;
	}
	Meshwork::L3::NetworkV1::NetworkV1::route_t* get_route(uint8_t dst, uint8_t index) {
		Meshwork::L3::NetworkV1::NetworkV1::route_t* result = NULL;
		int current = 0;
		for ( int i = 0; i < MAX_STATIC_ROUTES; i ++ ) {
			if ( routes[i].dst == dst ) {
				if ( current == index ) {
					result = &routes[i];
					break;
				}
				current ++;
			}
		}
		trace << PSTR("Get route: dst=") << dst << PSTR(", index=") << index << PSTR(", result=") << result << PSTR("\n");
		return result;
	}
	void route_found(Meshwork::L3::NetworkV1::NetworkV1::route_t* route) {
		trace << PSTR("Route found:\n\t");
		print_route(route);
	}
	void route_failed(Meshwork::L3::NetworkV1::NetworkV1::route_t* route) {
		trace << PSTR("Route failed:\n\t");
		print_route(route);
	}

};
#endif

