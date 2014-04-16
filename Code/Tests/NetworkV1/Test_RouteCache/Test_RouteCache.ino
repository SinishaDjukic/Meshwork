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
#include <stdlib.h>
#include "Cosa/Trace.hh"
#include "Cosa/Types.h"
#include "Cosa/IOStream.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/RTC.hh"
#include <Meshwork.h>
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Meshwork/L3/NetworkV1/RouteCache.h"
#include "Meshwork/L3/NetworkV1/RouteCache.cpp"

using Meshwork::L3::Network;
using Meshwork::L3::NetworkV1::NetworkV1;
using Meshwork::L3::NetworkV1::RouteCache;

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
	trace << PSTR("[testRemove] Phase 1: check remove_all() ") << endl;
	//Step 1: add routes
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);
	
	trace << PSTR("[testRemove] Test routes added. RouteCache:") << endl;
	printRouteCache(route_cache);
	
	//Step 2: remove all routes via remove_all
	route_cache->remove_all();
	
	//Step 3: verify that the cache is empty
	result &= testEmptyCache(route_cache);
	
	//Phase 2: check remove_all_for_dst()
	trace << PSTR("[testRemove] Phase 2: check remove_all_for_dst() ") << endl;
	//Step 1: add routes
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);
	
	//Step 2: remove all routes via remove_all_for_dst
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->remove_all_for_dst(route[i][j].dst);
	
	//Step 3: verify that the cache is empty
	result &= testEmptyCache(route_cache);
	
	
	//Phase 3: check remove_route_entry()
	trace << PSTR("[testRemove] Phase 3: check remove_route_entry() ") << endl;
	//Step 1: add routes
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);
	
	//Step 2: remove all routes via remove_route_entry
	//Right way: remove only the ones we added
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

//Tests: get_route_count, get_route_list, get_route_entry, get_route_entry, get_route_entry_index	
bool testGetters(RouteCache* route_cache, void* route_ptr) {
	printDelimiter2();
	trace << PSTR("[testGetters] Started") << endl;
	bool result = true;
	NetworkV1::route_t (&route)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES] = 
		*reinterpret_cast<NetworkV1::route_t (*)[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES]>(route_ptr);
	
	//Setup: add routes
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ )
			route_cache->add_route_entry(&route[i][j], false);
	
	//Phase 1: check get_route_count()
	for ( int i = 0; i < RouteCache::MAX_DST_NODES; i ++ )
		for ( int j = 0; j < RouteCache::MAX_DST_ROUTES; j ++ ) {
			//Note: actually, all dst nodes for the inner loop will be the same...
			uint8_t count = route_cache->get_route_count(route[i][j].dst);
			if ( count != 3 ) {
				result = false;
				trace	<< PSTR("[testGetters] Unexpected route count ") << count
						<< PSTR(" for destination: ") << route[i][j].dst << endl;
			}
		}
	
	trace << PSTR("[testGetters] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl << endl;
	printDelimiter2();
	return result;
}

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
			/*
			trace << PSTR("Setting hops: ");
			for ( int k = 0; k < NetworkV1::MAX_ROUTING_HOPS; k ++ )
				trace << ((uint8_t*)route[i][j].hops)[k] << PSTR(", ");
			trace << endl;
			*/
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

bool testStart() {
	printDelimiter1();
	trace << PSTR("[TestSuite][Test_RouteCache] Started") << endl;
	uint32_t time = RTC::millis();
	bool result = true;
	
	///////////////////// BEGIN /////////////////////
	//Allocate all structures in memory
	RouteCache route_cache(NULL);
//	uint16_t route_count = RouteCache::MAX_DST_NODES * RouteCache::MAX_DST_ROUTES;
	NetworkV1::route_t routes[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES];
	uint8_t route_hops[RouteCache::MAX_DST_NODES][RouteCache::MAX_DST_ROUTES][NetworkV1::MAX_ROUTING_HOPS];
	
	//Setup: default route data
	setupDefaultRouteData((void*)routes, (void*)route_hops);
	
	//Test 1: verify that the route cache is empty by default
	result &= testEmptyCache(&route_cache);
	//ok, must assume that this works in order to use it as a cleanup
	route_cache.remove_all();
	
	//Test 2: add all routes, remove them, then verify route cache is empty
	result &= testRemove(&route_cache, (void*)routes);
	//ok, must assume that this works in order to use it as a cleanup
	route_cache.remove_all();
	
	//Test 3: verify that getters return correct data
	result &= testGetters(&route_cache, (void*)routes);
//	if (!result)
//		printRouteCache(&route_cache);
	//ok, must assume that this works in order to use it as a cleanup
	route_cache.remove_all();
	
	
	//add_route_entry(route, true)//force replace
	//update_QoS, get_QoS
	
	////////////////////// END //////////////////////
	
	time = RTC::millis() - time;
	trace << PSTR("Run time (ms): ") << time << PSTR("\r\n");
	trace << PSTR("[TestSuite][Test_RouteCache] Finished: ") << (result ? PSTR("PASSED") : PSTR("FAILED")) << endl;
	printDelimiter1();
}

void setup()
{
	uart.begin(115200);
	uint8_t mode = SLEEP_MODE_IDLE;
	Watchdog::begin(16, mode);  
	RTC::begin();
	trace.begin(&uart, PSTR(__FILE__));

	testStart();
}

void loop()
{
}

