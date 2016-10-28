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
#ifndef __TESTS_ROUTECACHE_H__
#define __TESTS_ROUTECACHE_H__

#include <stdlib.h>
#include <Cosa/Trace.hh>
#include <Cosa/Types.h>
#include <Cosa/IOStream.hh>
#include <Cosa/UART.hh>
#include <Cosa/Watchdog.hh>
#include <Cosa/RTT.hh>
#include <Meshwork.h>
#include <Meshwork/L3/Network.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.h>
#include <Meshwork/L3/NetworkV1/RouteCache.h>
#include <Meshwork/L3/NetworkV1/RouteCache.cpp>

using Meshwork::L3::Network;
using Meshwork::L3::NetworkV1::NetworkV1;
using Meshwork::L3::NetworkV1::RouteCache;

void setupDefaultRouteData(void* route_ptr, void* route_hops_ptr) {
	//Setup hop pointers and hop data
	//Nasty... but we want a nice array-handling piece of code
	NetworkV1::route_t (&route)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES] =
		*reinterpret_cast<NetworkV1::route_t (*)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES]>(route_ptr);
	uint8_t (&route_hops)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES][NetworkV1::MAX_ROUTING_HOPS] =
		*reinterpret_cast<uint8_t (*)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES][NetworkV1::MAX_ROUTING_HOPS]>(route_hops_ptr);

	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
			route[i][j].src = 1;//1 always
			route[i][j].dst = 50 + i;//50, 51, ...
			route[i][j].hopCount = NetworkV1::MAX_ROUTING_HOPS;//max always
			route[i][j].hops = &route_hops[i][j][0];
			//hop data
			for ( int k = 0; k < NetworkV1::MAX_ROUTING_HOPS; k ++ ) {
				route_hops[i][j][k] = 50 + route[i][j].dst + j + k;
			}
			//Filled data:
			//dst=50, route=0, hops=[100, 101, ...]
			//dst=50, route=1, hops=[101, 102, ...]
			//dst=50, route=2, hops=[102, 103, ...]

			//dst=51, route=0, hops=[101, 102, ...]
			//dst=51, route=1, hops=[102, 103, ...]
			//dst=51, route=2, hops=[103, 104, ...]

			//dst=52, route=0, hops=[102, 103, ...]
			//dst=52, route=1, hops=[103, 104, ...]
			//dst=52, route=2, hops=[104, 105, ...]
		}
}

void printDelimiter1() {
	trace << PSTR("***************************") << endl;
}

void printDelimiter2() {
	trace << PSTR("---------------------------") << endl;
}

void printDelimiter3() {
	trace << PSTR("...........................") << endl;
}

void printRouteCache(RouteCache* route_cache) {
	printDelimiter3();
	route_cache->print(trace);
	trace << endl;
	printDelimiter3();
}

//Tests: get_route_count, get_route_list
bool testEmptyCache(RouteCache* route_cache) {
	printDelimiter2();
	trace << PSTR("[testEmptyCache] Started") << endl;

	bool result = true;
	for ( int i = Network::MIN_NODE_ID; i <= Network::MAX_NODE_ID && result; i ++ ) {
		//Test: get_route_count should return 0
		uint8_t route_count = route_cache->get_route_count(i);
		//Test: get_route_list should return NULL
		RouteCache::route_list_t* route_list = route_cache->get_route_list(i);
		if ( route_count > 0 ) {
			trace 	<< PSTR("[testEmptyCache] Unexpected route count: ") << route_count
					<< PSTR(" for node: ") << i << endl;
			result = false;
		} else if ( route_list != NULL ) {
			trace 	<< PSTR("[testEmptyCache] Unexpected route list NOT NULL ")
					<< PSTR(" for node: ") << i << endl;
			result = false;
		}
	}
	trace << PSTR("[testEmptyCache] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl << endl;
	if ( !result )
		printRouteCache(route_cache);
	printDelimiter2();
	return result;
}

//Tests: remove_all, remove_all_for_dst, remove_route_entry
bool testRemove(RouteCache* route_cache, void* route_ptr) {
	printDelimiter2();
	trace << PSTR("[testRemove] Started") << endl;
	bool result = true;
	NetworkV1::route_t (&route)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES] =
		*reinterpret_cast<NetworkV1::route_t (*)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES]>(route_ptr);

	//Phase 1: check remove_all()
	trace << PSTR("[testRemove][1] Phase 1: check remove_all()") << endl;
	//Step 1: add routes
	trace << PSTR("[testRemove][1] Adding routes...") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);

	trace << PSTR("[testRemove][1] Test routes added. RouteCache:") << endl;
	printRouteCache(route_cache);

	trace << PSTR("[testRemove][1] Removing all routes via remove_all()") << endl;
	//Step 2: remove all routes via remove_all
	route_cache->remove_all();

	//Step 3: verify that the cache is empty
	result &= testEmptyCache(route_cache);

	//Phase 2: check remove_all_for_dst()
	trace << PSTR("[testRemove][2] Phase 2: check remove_all_for_dst() ") << endl;
	//Step 1: add routes
	trace << PSTR("[testRemove][2] Adding routes...") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);

	trace << PSTR("[testRemove][2] Removing all routes via remove_all_for_dst()") << endl;
	//Step 2: remove all routes via remove_all_for_dst
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->remove_all_for_dst(route[i][j].dst);

	//Step 3: verify that the cache is empty
	result &= testEmptyCache(route_cache);


	//Phase 3: check remove_route_entry()
	trace << PSTR("[testRemove][3] Phase 3: check remove_route_entry() ") << endl;
	//Step 1: add routes
	trace << PSTR("[testRemove][3] Adding routes...") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);

	//Step 2: remove all routes via remove_route_entry
	trace << PSTR("[testRemove][3] Removing all routes via remove_route_entry()") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
			RouteCache::route_list_t* route_list = route_cache->get_route_list(route[i][j].dst);
			if ( route_list != NULL && route_list->dst != 0 ) {
				for ( int k = 0; k < RouteCache::MAX_DST_ROUTES; k ++ )
					route_cache->remove_route_entry(&route_list->entries[k]);
			}
		}

	//Step 3: verify that the cache is empty
	result &= testEmptyCache(route_cache);

	trace << PSTR("[testRemove] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl << endl;
	printDelimiter2();
	return result;
}

bool testRoute(NetworkV1::route_t* route1, NetworkV1::route_t* route2) {
	bool result = true;
	result &= route1->src == route2->src;
	result &= route1->dst == route2->dst;
	result &= route1->hopCount == route2->hopCount;
	if ( result ) {
		for ( int i = 0; i < route1->hopCount && result; i ++ ) {
			result &= route1->hops[i] == route2->hops[i];
		}
	}
	return result;
}

//Tests: get_route_count, get_route_list
bool testGetters1(RouteCache* route_cache, void* route_ptr) {
	printDelimiter2();
	trace << PSTR("[testGetters1] Started") << endl;
	bool result = true;
	NetworkV1::route_t (&route)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES] =
		*reinterpret_cast<NetworkV1::route_t (*)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES]>(route_ptr);

	//Setup: add routes
	trace << PSTR("[testGetters1] Adding routes...") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);

	//Phase 1: check get_route_count()
	trace << PSTR("[testGetters1][1] Testing routes via get_route_count()") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
			//Note: actually, all dst nodes for the inner loop will be the same...
			uint8_t count = route_cache->get_route_count(route[i][j].dst);
			if ( count != 3 ) {
				result = false;
				trace	<< PSTR("[testGetters1][1] Unexpected route count ") << count
						<< PSTR(" for destination: ") << route[i][j].dst << endl;
			}
		}

	//Phase 2: check get_route_list()
	trace << PSTR("[testGetters1][2] Testing routes via get_route_list()") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ ) {
		uint8_t dst = route[i][0].dst;//first route element in our default data set
		RouteCache::route_list_t* list = route_cache->get_route_list(dst);
		if ( list == NULL ) {
			result = false;
			trace	<< PSTR("[testGetters1][2] Unexpected NULL route list for dst: ") << dst << endl;
		} else if ( list->dst != dst ) {
			result = false;
			trace	<< PSTR("[testGetters1][2] Unexpected route list dst value: ") << list->dst << PSTR(", expected: ") << dst << endl;
		} else {
			for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
				RouteCache::route_entry_t* entry = &list->entries[j];
//				trace	<< PSTR("[testGetters1][2] Testing route at index: ") << j << endl;
//				route_cache->print(trace, *entry, 1);
//				trace << endl
				if ( entry == NULL ) {
					result = false;
					trace	<< PSTR("[testGetters1][2] Unexpected NULL route entry at index: ") << j << endl;
				} else if ( !testRoute(&entry->route, &route[i][j]) ) {
					result = false;
					trace	<< PSTR("[testGetters1][2] Route data not matching at index: ") << j << endl;
				}
			}
		}
	}

	trace << PSTR("[testGetters1] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl << endl;
	printDelimiter2();
	return result;
}

//Tests: get_route_entry(uint8_t, uint8_t), get_route_entry(NetworkV1::route_t*), get_route_entry_index
bool testGetters2(RouteCache* route_cache, void* route_ptr) {
	printDelimiter2();
	trace << PSTR("[testGetters2] Started") << endl;
	bool result = true;
	NetworkV1::route_t (&route)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES] =
		*reinterpret_cast<NetworkV1::route_t (*)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES]>(route_ptr);

	//Setup: add routes
	trace << PSTR("[testGetters1] Adding routes...") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);

	//Phase 1: check get_route_entry(uint8_t, uint8_t)
	trace << PSTR("[testGetters2][1] Testing routes via get_route_entry(uint8_t, uint8_t)") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ ) {
		uint8_t dst = route[i][0].dst;//first route element in our default data set
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
		RouteCache::route_entry_t* entry = route_cache->get_route_entry(dst, j);
//			trace	<< PSTR("[testGetters2] Testing route at index: ") << j << endl;
//			route_cache->print(trace, *entry, 1);
//			trace << endl
			if ( entry == NULL ) {
				result = false;
				trace	<< PSTR("[testGetters2][1] Unexpected NULL route entry at index: ") << j << endl;
			} else if ( !testRoute(&entry->route, &route[i][j]) ) {
				result = false;
				trace	<< PSTR("[testGetters2][1] Route data not matching at index: ") << j << endl;
			}
		}
	}

	//Phase 2: check get_route_entry_index()
	trace << PSTR("[testGetters2][2] Testing routes via get_route_entry(NetworkV1::route_t*)") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ ) {
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
		RouteCache::route_entry_t* entry = route_cache->get_route_entry(&route[i][j]);
			if ( entry == NULL ) {
				result = false;
				trace	<< PSTR("[testGetters2][2] Unexpected NULL route entry at index: ") << j << endl;
			} else if ( !testRoute(&entry->route, &route[i][j]) ) {
				result = false;
				trace	<< PSTR("[testGetters2][2] Route data not matching at index: ") << j << endl;
			}
		}
	}

	//Phase 3: check get_route_entry(NetworkV1::route_t*)
	trace << PSTR("[testGetters2][3] Testing routes via get_route_entry_index()") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ ) {
		uint8_t dst = route[i][0].dst;//first route element in our default data set
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
			uint8_t node_index = -1, route_index = -1;
			RouteCache::route_entry_t* entry = route_cache->get_route_entry(dst, j);
			if ( entry == NULL || !route_cache->get_route_entry_index(entry, node_index, route_index)
					|| node_index != i || route_index != j ) {
				result = false;
				trace	<< PSTR("[testGetters2][3] Route entry index wrong for node_index: ") << i << PSTR(", route_index: ") << j << endl;
				trace	<< PSTR("[testGetters2][3] Returned node_index: ") << node_index << PSTR(", route_index: ") << route_index << endl;
			}
		}
	}

	trace << PSTR("[testGetters2] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl << endl;
	printDelimiter2();
	return result;
}

//Tests: update_QoS, get_QoS
bool testQoS(RouteCache* route_cache, void* route_ptr) {
	if ( RouteCache::MAX_DST_ROUTES < 3 ) {
		trace << PSTR("[testQoS] SKIPPED: RouteCache::MAX_DST_ROUTES less than 3") << endl;
		return true;
	}
	printDelimiter2();
	trace << PSTR("[testQoS] Started") << endl;
	bool result = true;
	NetworkV1::route_t (&route)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES] =
		*reinterpret_cast<NetworkV1::route_t (*)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES]>(route_ptr);

	//Setup: add routes
	trace << PSTR("[testQoS] Adding routes...") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);
	//Setup: add QoS:
	//	route[0][0]: reach max
	//	route[0][1]: reach min
	//	route[0][2]: stay at zero
	trace	<< PSTR("[testQoS] Adding route entry with QOS_LEVEL_MAX") << endl;
	trace	<< PSTR("[testQoS] ... Adding QOS_LEVEL_MAX") << endl;
	for ( int k = 0; k < Network::QOS_LEVEL_MAX + 10; k ++ )//exceeding max on purpose
		result &= route_cache->update_QoS(&route[0][0], true);
	trace	<< PSTR("[testQoS] ... Route entry: ");//+100
	route_cache->print(trace, *route_cache->get_route_entry(route[0][0].dst, 0), 2);
	trace << endl;

	trace	<< PSTR("[testQoS] Adding route entry with QOS_LEVEL_MIN") << endl;
	trace	<< PSTR("[testQoS] ... Adding QOS_LEVEL_MIN") << endl;
	for ( int k = 0; k > Network::QOS_LEVEL_MIN - 10; k -- )//exceeding min on purpose
		result &= route_cache->update_QoS(&route[0][1], false);
	trace	<< PSTR("[testQoS] ... Route entry: ");//-100
	route_cache->print(trace, *route_cache->get_route_entry(route[0][1].dst, 1), 2);
	trace << endl;

	trace	<< PSTR("[testQoS] Adding route entry with QOS_LEVEL_AVERAGE") << endl;
	trace	<< PSTR("[testQoS] ... Adding QOS_LEVEL_MAX") << endl;
	for ( int k = 0; k < Network::QOS_LEVEL_MAX + 10; k ++ )//exceeding max on purpose
		result &= route_cache->update_QoS(&route[0][2], true);
	trace	<< PSTR("[testQoS] ... Adding QOS_LEVEL_MIN") << endl;
	for ( int k = 0; k < Network::QOS_LEVEL_MAX; k ++ )//should net to zero
		result &= route_cache->update_QoS(&route[0][2], false);
	trace	<< PSTR("[testQoS] ... Route entry: ");//0
	route_cache->print(trace, *route_cache->get_route_entry(route[0][2].dst, 2), 2);
	trace << endl;

	if ( !result )
		trace	<< PSTR("[testQoS] Failed to update QoS values") << endl;

	int8_t qos;

	trace << endl;
	//Phase 1: check QoS calculation Network::QOS_CALCULATE_BEST
	if ( (qos = route_cache->get_QoS(route[0][0].dst, Network::QOS_CALCULATE_BEST)) != Network::QOS_LEVEL_MAX ) {
		trace	<< PSTR("[testQoS] QOS_CALCULATE_BEST wrong for dst: ") << route[0][0].dst << PSTR(", equals: ") << qos << endl;
		result = false;
	} else {
		trace	<< PSTR("[testQoS] QOS_CALCULATE_BEST is correct") << endl;
	}

	trace << endl;
	//Phase 2: check QoS calculation Network::QOS_CALCULATE_WORST
	if ( (qos = route_cache->get_QoS(route[0][1].dst, Network::QOS_CALCULATE_WORST)) != Network::QOS_LEVEL_MIN ) {
		trace	<< PSTR("[testQoS] QOS_CALCULATE_WORST wrong for dst: ") << route[0][1].dst << PSTR(", equals: ") << qos << endl;
		result = false;
	} else {
		trace	<< PSTR("[testQoS] QOS_CALCULATE_WORST is correct") << endl;
	}

	trace << endl;
	//Phase 3: check QoS calculation Network::QOS_CALCULATE_AVERAGE
	if ( (qos = route_cache->get_QoS(route[0][2].dst, Network::QOS_CALCULATE_AVERAGE)) != Network::QOS_LEVEL_AVERAGE ) {
		trace	<< PSTR("[testQoS] QOS_CALCULATE_AVERAGE wrong for dst: ") << route[0][2].dst << PSTR(", equals: ") << qos << endl;
		result = false;
	} else {
		trace	<< PSTR("[testQoS] QOS_CALCULATE_AVERAGE is correct") << endl;
	}

	trace << endl;
	trace << PSTR("[testQoS] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl << endl;
	printDelimiter2();
	return result;
}

//Tests: add_route_entry(route, true)//force replace
bool testAddAndReplace(RouteCache* route_cache, void* route_ptr) {
	if ( RouteCache::MAX_DST_ROUTES < 3 ) {
		trace << PSTR("[testAddAndReplace] SKIPPED: RouteCache::MAX_DST_ROUTES less than 3") << endl;
		return true;
	}
	printDelimiter2();
	trace << PSTR("[testAddAndReplace] Started") << endl;
	bool result = true;
	NetworkV1::route_t (&route)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES] =
		*reinterpret_cast<NetworkV1::route_t (*)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES]>(route_ptr);

	//Setup: add routes
	trace << PSTR("[testAddAndReplace] Adding routes...") << endl;
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);
	//Setup: add QoS for Phase 1 and Phase 2
	//	route[0]: reach max
	//	route[1]: reach min
	//	route[2]: stay at zero
	for ( int k = 0; k < Network::QOS_LEVEL_MAX + 10; k ++ )//exceeding max on purpose
		route_cache->update_QoS(&route[0][0], true);
	for ( int k = 0; k > Network::QOS_LEVEL_MIN - 10; k -- )//exceeding min on purpose
		route_cache->update_QoS(&route[0][1], false);

	for ( int k = 0; k < Network::QOS_LEVEL_MAX + 10; k ++ )
		route_cache->update_QoS(&route[0][2], true);
	for ( int k = 0; k < Network::QOS_LEVEL_MAX; k ++ )
		route_cache->update_QoS(&route[0][2], false);

	//Setup: add QoS for Phase 3
	//all routes except for the first have progressively lower QoS
	for ( int i = 1; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			for ( int k = 0; k < j+1; k ++ )
				route_cache->update_QoS(&route[i][j], false);

	//Phase 1: check that adding an existing route doesn't modify the table
	route_cache->add_route_entry(&route[0][0], true);

	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ ) {
		uint8_t dst = route[i][0].dst;//first route element in our default data set
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
		RouteCache::route_entry_t* entry = route_cache->get_route_entry(route[i][0].dst, j);
			if ( entry == NULL ) {
				result = false;
				trace	<< PSTR("[testAddAndReplace][1] Unexpected NULL route entry at index: ") << j << endl;
			} else if ( !testRoute(&entry->route, &route[i][j]) ) {
				result = false;
				trace	<< PSTR("[testAddAndReplace][1] Route data not matching at index: ") << j << endl;
			}
		}
	}

	//Phase 2: we have the dst in the table, but need to replace the worst route
	//route[0][1] have been set up with the worst QoS
	uint8_t rni1 = 0;
	uint8_t rri1 = 1;
	NetworkV1::route_t replace_route1;
	replace_route1.src = route[rni1][rri1].src;
	replace_route1.dst = route[rni1][rri1].dst;
	replace_route1.hopCount = 2;
	uint8_t hops1[] = {3, 4};
	replace_route1.hops = hops1;

	route_cache->add_route_entry(&replace_route1, true);
	//Test 1: check that the correct entry has been replaced
	RouteCache::route_entry_t* entry1 = route_cache->get_route_entry(replace_route1.dst, rri1);
	if ( entry1 == NULL ) {
		result = false;
		trace	<< PSTR("[testAddAndReplace][2.1] Unexpected NULL route entry at index: ") << rri1 << endl;
	} else if ( !testRoute(&entry1->route, &replace_route1) ) {
		result = false;
		trace	<< PSTR("[testAddAndReplace][2.1] Route data not matching at index: ") << rri1 << endl;
	}

	//Test 2: check that other routes have not been modified
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ ) {
		if ( i == rni1 )
			continue;
		uint8_t dst = route[i][0].dst;//first route element in our default data set
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
		if ( j == rri1 )
			continue;
		RouteCache::route_entry_t* entry0 = route_cache->get_route_entry(dst, j);
			if ( entry0 == NULL ) {
				result = false;
				trace	<< PSTR("[testAddAndReplace][2.2] Unexpected NULL route entry at index: ") << j << endl;
			} else if ( !testRoute(&entry0->route, &route[i][j]) ) {
				result = false;
				trace	<< PSTR("[testAddAndReplace][2.2] Route data not matching at index: ") << j << endl;
			}
		}
	}

	//Phase 3: we don't have the dst in the table and need to replace another dst list with worst average
	//According to our setup this should be the last dst in the table
	uint8_t rni2 = RouteCache::MAX_DST_NODES - 1;
	uint8_t rri2 = 0;
	NetworkV1::route_t replace_route2;
	replace_route2.src = route[0][0].src;//keep the same src
	replace_route2.dst = route[0][0].src;//this shouldn't happen, but guarantees uniqueness against the test data set
	replace_route2.hopCount = 3;
	uint8_t hops2[] = {5, 6, 7};
	replace_route2.hops = hops2;

	route_cache->add_route_entry(&replace_route2, true);
	//Test 1: check that the correct entry has been replaced
	RouteCache::route_entry_t* entry = route_cache->get_route_entry(replace_route2.dst, rri2);
	if ( entry == NULL ) {
		result = false;
		trace	<< PSTR("[testAddAndReplace][3.1] Unexpected NULL route entry at index: ") << rri2 << endl;
	} else if ( !testRoute(&entry->route, &replace_route2) ) {
		result = false;
		trace	<< PSTR("[testAddAndReplace][3.1] Route data not matching at index: ") << rri2 << endl;
	}

	//Test 2: check that other routes have not been modified
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ ) {
		if ( i == rni1 || i == rni2 )
			continue;
		uint8_t dst = route[i][0].dst;//first route element in our default data set
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
		if ( j == rri1 || j == rri2 )
			continue;
		RouteCache::route_entry_t* entry0 = route_cache->get_route_entry(dst, j);
			if ( entry0 == NULL ) {
				result = false;
				trace	<< PSTR("[testAddAndReplace][3.2] Unexpected NULL route entry at index: ") << j << endl;
			} else if ( !testRoute(&entry0->route, &route[i][j]) ) {
				result = false;
				trace	<< PSTR("[testAddAndReplace][3.2] Route data not matching at index: ") << j << endl;
			}
		}
	}

	trace << PSTR("[testAddAndReplace] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl << endl;
	printDelimiter2();
	return result;
}

bool testStart() {
	printDelimiter1();
	trace << PSTR("[TestSuite][Test_RouteCache] Started") << endl;
	uint32_t time = RTT::millis();
	bool result = true;

	///////////////////// BEGIN /////////////////////
	//Allocate all structures in memory
	RouteCache route_cache(NULL);
//	uint16_t route_count = RouteCache::MAX_DST_NODES * RouteCache::MAX_DST_ROUTES;
	NetworkV1::route_t routes[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES];
	uint8_t route_hops[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES][NetworkV1::MAX_ROUTING_HOPS];

	//Setup: default route data
	setupDefaultRouteData((void*)routes, (void*)route_hops);

//	//Test 1: verify that the route cache is empty by default
//	trace << PSTR("[TestSuite][Test_RouteCache] Test 1: verify that the route cache is empty by default") << endl;
//	result &= testEmptyCache(&route_cache);
//	route_cache.remove_all();//ok, must assume that this works in order to use it as a cleanup
//
//	//Test 2: add all routes, remove them, then verify route cache is empty
//	trace << PSTR("[TestSuite][Test_RouteCache] Test 2: add all routes, remove them, then verify route cache is empty") << endl;
//	result &= testRemove(&route_cache, (void*)routes);
//	route_cache.remove_all();
//
//	//Test 3: verify that getters1 return correct data
//	trace << PSTR("[TestSuite][Test_RouteCache] Test 3: verify that getters1 return correct data") << endl;
//	result &= testGetters1(&route_cache, (void*)routes);
//	route_cache.remove_all();
//
//	//Test 4: verify that getters2 return correct data
//	trace << PSTR("[TestSuite][Test_RouteCache] Test 4: verify that getters2 return correct data") << endl;
//	result &= testGetters2(&route_cache, (void*)routes);
//	route_cache.remove_all();

	//Test 5: verify that QoS functions return correct data
	trace << PSTR("[TestSuite][Test_RouteCache] Test 5: verify that QoS functions return correct data") << endl;
	result &= testQoS(&route_cache, (void*)routes);
	route_cache.remove_all();

//	//Test 6: verify that add and replace works correctly
//	trace << PSTR("[TestSuite][Test_RouteCache] Test 6: verify that add and replace works correctly") << endl;
//	result &= testAddAndReplace(&route_cache, (void*)routes);
//	route_cache.remove_all();

	////////////////////// END //////////////////////

	time = RTT::millis() - time;
	trace << PSTR("Run time (ms): ") << time << PSTR("\r\n");
	trace << PSTR("[TestSuite][Test_RouteCache] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl;
	printDelimiter1();

	return result;
}

void setup()
{
	uart.begin(115200);
	Watchdog::begin();
	RTT::begin();
	trace.begin(&uart, PSTR(__FILE__));

	testStart();
}

void loop()
{
	Watchdog::delay(60000);
}

#endif
