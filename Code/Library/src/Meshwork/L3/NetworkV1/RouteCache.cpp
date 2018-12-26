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
#ifndef __MESHWORK_L3_NETWORKV1_ROUTECACHE_CPP__
#define __MESHWORK_L3_NETWORKV1_ROUTECACHE_CPP__

#include "Cosa/Wireless.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Meshwork/L3/NetworkV1/RouteCache.h"

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
		for ( int j = 0; j < MAX_DST_ROUTES; j ++ ) {
			if ( &m_table.lists[i].entries[j] == entry ) {
				node_index = i;
				route_index = j;
				return true;
			}
		}
	return false;
}

				
int8_t RouteCache::normalize_QoS(int8_t qos) {
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
		for ( int i = 0; i < MAX_DST_ROUTES; i ++ ) {
			if ( m_route_cache_listener != NULL )
				m_route_cache_listener->route_entry_change(this, &list->entries[i], RouteCacheListener::ROUTE_ENTRY_REMOVING);
			list->entries[i].route.dst = 0;
		}
		list->dst = 0;
	}
}
				
void RouteCache::remove_all(bool notify) {
	for ( int i = 0; i < MAX_DST_NODES; i ++ ) {
		for ( int j = 0; j < MAX_DST_ROUTES; j ++ ) {
			m_table.lists[i].entries[j].route.hops = m_table.lists[i].entries[j].route_hops;
			if ( notify && m_route_cache_listener != NULL )
				m_route_cache_listener->route_entry_change(this, &m_table.lists[i].entries[j], RouteCacheListener::ROUTE_ENTRY_REMOVING);
			m_table.lists[i].entries[j].route.dst = 0;
		}
		m_table.lists[i].dst = 0;
	}
}
				
void RouteCache::remove_all() {
	remove_all(true);
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
		int8_t qos = entry->qos + (increase ? 1 : -1);
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
		bool firstRoute = true;
		//yes, this looks weird, but single loop makes the code smaller
		MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "QoS for dst: %d, method: %d", dst, calculate);
		if ( MW_LOG_ROUTECACHE )
			print(trace, *list, 1);
		for ( int i = 0; i < MAX_DST_ROUTES; i ++ ) {
			NetworkV1::route_t r = list->entries[i].route;
			MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "Checking route with index: %d", i);
			if ( MW_LOG_ROUTECACHE )
				print(trace, r, 2);
			if ( r.dst != 0 ) {//valid route
				int8_t tmp = list->entries[i].qos;
				MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "Avg: %d, route QoS: %d", result, tmp);
				switch ( calculate ) {
					case Network::QOS_CALCULATE_BEST: result = result < tmp ? tmp : result; break;
					case Network::QOS_CALCULATE_WORST: result = result > tmp ? tmp : result; break;
					case Network::QOS_CALCULATE_AVERAGE:
						if ( firstRoute ) {
							firstRoute = false;
							result = tmp;
						} else {
							result = ((result + tmp) / 2);
						}
						break;
					default:
						MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "Unknown method: %d", calculate);
						break;
				}
			}
		}
	}
	return normalize_QoS(result);//normalize, just in case
}
				
RouteCache::route_entry_t* RouteCache::add_route_entry(NetworkV1::route_t* route, bool forceReplace) {
	route_entry_t* result = NULL;
	if ( get_route_entry(route) == NULL ) {
		MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "*** Route not in the cache. Force replace: %d", forceReplace);
		uint8_t dst = route->dst;
		route_list_t* list = get_route_list(dst);
		if ( list != NULL ) {
			MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "*** Route list exists for dst: %d", dst);
			uint8_t worst = Network::QOS_LEVEL_MAX;
			uint8_t worstIndex = MAX_DST_ROUTES - 1;
			//try to add to exising routes
			for ( int i = 0; i < MAX_DST_ROUTES; i ++ ) {
				if ( list->entries[i].route.dst == 0 ) {
					result = &list->entries[i];
					MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "*** Empty slot found at: %d, Address: %d", dst, result);
					break;
				} else if ( forceReplace ) {
					uint8_t qos = list->entries[i].qos;
					if ( worst < qos ) {
						worst = qos;
						worstIndex = i;
					}
				}
			}
			//if no free space, and we should force a replace
			//then choose the one with worst QoS
			if ( result == NULL && forceReplace ) {
				MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "*** No free slot. Replacing at: %d", worstIndex);
				result = &list->entries[worstIndex];
			}
		} else {
			MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "*** Route list doesn't exist for dst: %d", dst);
			uint8_t worst = Network::QOS_LEVEL_MAX;
			uint8_t worstIndex = MAX_DST_NODES - 1;
			//try to add a new node
			for ( int i = 0; i < MAX_DST_NODES; i ++ )
				if ( m_table.lists[i].dst == 0 ) {
					//choose the first element
					result = &m_table.lists[i].entries[0];
					//mark the list as used
					m_table.lists[i].dst = dst;
					MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "*** Found empty route slot at: %d, Address: %d", i, result);
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
				MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "*** No free slot. Replacing at: %d", worstIndex);
				//choose the first element
				result = &m_table.lists[worstIndex].entries[0];
				//mark the list as used by this dst
				m_table.lists[worstIndex].dst = dst;
				//clear other entries
				for ( int i = 1; i < MAX_DST_ROUTES; i ++ )
					m_table.lists[worstIndex].entries[i].route.dst = 0;
			}
		}
		if ( result != NULL ) {
			MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "*** Updating with new route data", NULL);
			if ( MW_LOG_ROUTECACHE )
				print(trace, *route, 0);
			
			result->route.hopCount = route->hopCount;
			result->route.src = route->src;
			memset(result->route.hops, 0, Meshwork::L3::NetworkV1::NetworkV1::MAX_ROUTING_HOPS);
			
			if ( route->hopCount > 0 )
				memcpy(result->route.hops, route->hops, route->hopCount);
			
			result->route.dst = route->dst;
			result->qos = Network::QOS_LEVEL_AVERAGE;
			
			MW_LOG_DEBUG(MW_LOG_ROUTECACHE, "*** New route data", NULL);
			if ( MW_LOG_ROUTECACHE )
				print(trace, result->route, 0);
			
			if ( m_route_cache_listener != NULL )
				m_route_cache_listener->route_entry_change(this, result, RouteCacheListener::ROUTE_ENTRY_CHANGED);
		}
	} //otherwise it is already there
	return result;
}

void RouteCache::print(IOStream& outs, NetworkV1::route_t& route, uint8_t tabs) {
  printStatic(outs, route, tabs);
}

void RouteCache::print(IOStream& outs, RouteCache::route_entry_t& route_entry, uint8_t tabs) {
  outs << PSTR("route_entry_t { ") << endl;
  printTabs(outs, ++tabs);
  outs << PSTR("qos: ") << route_entry.qos << PSTR(", ");
  Meshwork::L3::NetworkV1::NetworkV1::route_t& route = route_entry.route;
  print(outs, route, tabs);
  printTabs(outs, --tabs);
  outs << PSTR(" }");
}

void RouteCache::print(IOStream& outs, RouteCache::route_list_t& route_list, uint8_t tabs) {
  outs << PSTR("route_list_t { ") << endl;
  printTabs(outs, ++tabs);
  outs << PSTR("MAX_DST_ROUTES: ") << RouteCache::MAX_DST_ROUTES << PSTR(", ");
  outs << PSTR("dst: ") << route_list.dst << (route_list.dst == 0 ? PSTR(" (INACTIVE)") : PSTR(" (ACTIVE)")) << PSTR(", ");
  for ( int i = 0; i < RouteCache::MAX_DST_ROUTES; i ++ ) {
	outs << endl;
	printTabs(outs, tabs);
	outs << PSTR("entries[") << i << PSTR("]: ");
	print(outs, route_list.entries[i], tabs+1);
	outs << PSTR(", ") << endl;
  }
  printTabs(outs, --tabs);
  outs << PSTR(" }") << endl;
}

void RouteCache::print(IOStream& outs) {
  uint8_t tabs = 0;
  outs << PSTR("RouteCache { ") << endl;
  printTabs(outs, ++tabs);
  outs << PSTR("m_route_cache_listener: ") << (m_route_cache_listener != NULL ) << PSTR(", ");
  outs << PSTR("m_table: ") << PSTR("route_table_t { ") << endl;
  printTabs(outs, ++tabs);
  outs << PSTR("MAX_DST_NODES: ") << RouteCache::MAX_DST_NODES << PSTR(", ") << endl;
  tabs ++;
  for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ ) {
	printTabs(outs, tabs);
	outs << PSTR("lists[") << i << PSTR("]: ");
	print(outs, m_table.lists[i], tabs+1);
	outs << PSTR(", ") << endl;
  }
  tabs--;
  printTabs(outs, --tabs);
  outs << PSTR(" }") << endl;
  printTabs(outs, --tabs);
  outs << PSTR("}") << endl;
}
#endif
