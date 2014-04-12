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
#include "Cosa/Wireless.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

using Meshwork::L3::NetworkV1::NetworkV1;
using Meshwork::L3::NetworkV1::RouteCache;

/**
 * Holds up routes for up to m_maxNodes destinations. One route per destination only
 */
bool RouteCache::array_compare(uint8_t* a, uint8_t* b, uint8_t len) {
	bool result = true;
	for ( int i = 0; result && i < len; i ++ )
		if ( a[i] != b[i] )
			result = false;
	return result;
}

bool RouteCache::get_route_entry_index(route_entry_t* entry, uint8_t& node_index, uint8_t& route_index) {
	for ( int i = 0; i < MAX_DST_NODES; i ++ )
		for ( int j = 0; j < MAX_DST_ROUTES; j ++ )
			if ( &m_table.lists[i].entries[j] == entry ) {
				node_index = i;
				route_index = j;
				return true;
			}
	return false;
}

				
uint8_t RouteCache::normalize_QoS(uint8_t qos) {
	return qos == Network::QOS_LEVEL_UNKNOWN ? Network::QOS_LEVEL_UNKNOWN :
			( qos < Network::QOS_LEVEL_MIN ? Network::QOS_LEVEL_MIN :
				(qos > Network::QOS_LEVEL_MAX ? Network::QOS_LEVEL_MAX : qos));
}

RouteCache::route_list_t* RouteCache::get_route_list(uint8_t dst) {
	for ( int i = 0; i < MAX_DST_NODES; i ++ )
		if ( m_table.lists[i].dst == dst )
			return &m_table.lists[i];
	return NULL;
}

void RouteCache::remove_all_for_dst(uint8_t dst) {
	route_list_t* list = get_route_list(dst);
	if ( list != NULL ) {
		if ( m_route_cache_listener != NULL )
			for ( int i = 0; i < MAX_DST_ROUTES; i ++ )
				m_route_cache_listener->route_entry_change(this, &list->entries[i], RouteCacheListener::ROUTE_ENTRY_REMOVING);
		list->dst = 0;
	}
}
				
void RouteCache::remove_all() {
	for ( int i = 0; i < MAX_DST_NODES; i ++ )
		remove_all_for_dst(m_table.lists[i].dst);
}
				
uint8_t RouteCache::get_route_count(uint8_t dst) {
	uint8_t result = 0;
	route_list_t* list = get_route_list(dst);
	if ( list != NULL ) {
		for ( int i = 0; i < MAX_DST_ROUTES; i ++ )
			result = result + (list->entries[i].route.dst == dst ? 1 : 0);
	}
	return result;
}

void RouteCache::remove_route_entry(route_entry_t* entry) {
	if ( entry != NULL ) {
		if ( m_route_cache_listener != NULL )
			m_route_cache_listener->route_entry_change(this, entry, RouteCacheListener::ROUTE_ENTRY_REMOVING);
		uint8_t dst = entry->route.dst;
		entry->route.dst = 0;
		if ( get_route_count(dst) == 0 )
			remove_all_for_dst(dst);
	}
}
				
RouteCache::route_entry_t* RouteCache::get_route_entry(uint8_t dst, uint8_t index) {
	route_entry_t* result = NULL;
	route_list_t* list = get_route_list(dst);
	if ( list != NULL ) {
		for ( int i = 0; i < MAX_DST_ROUTES; i ++ ) {
			if ( list->entries[i].route.dst == dst &&
					index-- == 0 ) {//find the Nth index...
				result = &list->entries[i];
				break;
			}
		}
	}
	return result;
}
				
RouteCache::route_entry_t* RouteCache::get_route_entry(NetworkV1::route_t* route) {
	route_entry_t* result = NULL;
	uint8_t dst = route->dst;
	if ( dst != 0 ) {
	route_list_t* list = get_route_list(dst);
		if ( list != NULL ) {
			for ( int i = 0; i < MAX_DST_ROUTES; i ++ ) {
				NetworkV1::route_t r = list->entries[i].route;
				if ( r.dst == dst &&
						r.hopCount == route->hopCount &&
							array_compare(r.hops, route->hops, r.hopCount) ) {
					result = &list->entries[i];
					break;
				}
			}
		}
	}
	return result;
}
								
bool RouteCache::update_QoS(NetworkV1::route_t* route, bool increase) {
	route_entry_t* entry = get_route_entry(route);
	if ( entry != NULL ) {
		//local var saves flash
		uint8_t qos = entry->qos + increase ? 1 : -1;
		//make sure we stay in range
		entry->qos = normalize_QoS(qos);
	}
	//may be used by the caller to determine if there was such a route at all
	return entry != NULL;
}

int8_t RouteCache::get_QoS(uint8_t dst, int8_t calculate) {
	int16_t result = Network::QOS_LEVEL_UNKNOWN;
	route_list_t* list = get_route_list(dst);
	if ( list != NULL ) {
		//set initial value
		result = calculate == Network::QOS_CALCULATE_BEST ? Network::QOS_LEVEL_MIN :
					(calculate == Network::QOS_CALCULATE_WORST ? Network::QOS_LEVEL_MAX : 0);
		//yes, this looks weird, but single loop makes the code smaller
		for ( int i = 0; i < MAX_DST_ROUTES; i ++ ) {
			NetworkV1::route_t r = list->entries[i].route;
			if ( r.dst != 0 ) {//valid route
				uint8_t tmp = list->entries[i].qos;
				if ( calculate == Network::QOS_CALCULATE_BEST ) {
					result = result > tmp ? result : tmp;
				} else if ( calculate == Network::QOS_CALCULATE_WORST ) {
					result = result < tmp ? result : tmp;
				} if ( calculate == Network::QOS_CALCULATE_AVERAGE ) {
					result = (result + tmp) >> 2;
				} else {
					break;
				}
			}
		}
	}
	return normalize_QoS(result);//normalize, just in case
}
				
RouteCache::route_entry_t* RouteCache::add_route_entry(NetworkV1::route_t* route, bool forceReplace) {
	//TODO add logs
	route_entry_t* result = NULL;
	if ( get_route_entry(route) == NULL ) {
		uint8_t dst = route->dst;
		route_list_t* list = get_route_list(dst);
		if ( list != NULL ) {
			uint8_t worst = Network::QOS_LEVEL_MAX;
			uint8_t worstIndex = MAX_DST_ROUTES - 1;
			//try to add to exising routes
			for ( int i = 0; i < MAX_DST_ROUTES; i ++ )
				if ( list->entries[i].route.dst == 0 ) {
					result = &list->entries[i];
					break;
				} else if ( forceReplace ) {
					uint8_t qos = list->entries[i].qos;
					if ( worst < qos ) {
						worst = qos;
						worstIndex = i;
					}
				}
			//if no free space, and we should force a replace
			//then choose the one with worst QoS
			if ( result == NULL && forceReplace )
				result = &list->entries[worstIndex];
		} else {
			uint8_t worst = Network::QOS_LEVEL_MAX;
			uint8_t worstIndex = MAX_DST_NODES - 1;
			//try to add a new node
			for ( int i = 0; i < MAX_DST_NODES; i ++ )
				if ( m_table.lists[i].dst == 0 ) {
					//choose the first element
					result = &m_table.lists[i].entries[0];
					//mark the list as used
					m_table.lists[i].dst = dst;
					break;
				} else if ( forceReplace ) {
					uint8_t qos = get_QoS(dst, Network::QOS_CALCULATE_AVERAGE);
					if ( worst < qos ) {
						worst = qos;
						worstIndex = i;
					}
				}
			//if no free space, and we should force a replace
			//then choose the one with worst QoS
			if ( result == NULL && forceReplace ) {
				//choose the first element
				result = &m_table.lists[worstIndex].entries[0];
				//mark the list as used
				m_table.lists[worstIndex].dst = dst;
			}
		}
		if ( result != NULL ) {
			result->route.hopCount = route->hopCount;
			result->route.src = route->src;
			memset(result->route.hops, 0, Meshwork::L3::NetworkV1::NetworkV1::MAX_ROUTING_HOPS);
			if ( route->hopCount > 0 )
				memcpy(result->route.hops, route, route->hopCount);
			result->route.dst = route->dst;
			result->qos = Network::QOS_LEVEL_AVERAGE;
			
			if ( m_route_cache_listener != NULL )
				m_route_cache_listener->route_entry_change(this, result, RouteCacheListener::ROUTE_ENTRY_CHANGED);
		}
	} //otherwise it is already there
	return result;
}
