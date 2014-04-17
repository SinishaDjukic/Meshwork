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
#ifndef __MESHWORK_L3_NETWORKV1_ROUTECACHE_H__
#define __MESHWORK_L3_NETWORKV1_ROUTECACHE_H__

#include "Cosa/Wireless.hh"
#include "Cosa/IOStream.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"

using Meshwork::L3::NetworkV1::NetworkV1;

/**
 * Holds up routes for up to m_maxNodes destinations. One route per destination only
 */
namespace Meshwork {

	namespace L3 {
	
		namespace NetworkV1 {
		
			class RouteCache {
			public:
				/** Maximum number of destination nodes for which routes are cached. */
				static const uint8_t MAX_DST_NODES = 3;
				/** Maximum number of routes for each destination note. */
				static const uint8_t MAX_DST_ROUTES = 3;
				
				//Note: the below structure is not extremely optimal RAM-wise,
				//since every route_t already contains the dst field
				//However, for the sake of code simplicity and maintenance we
				//sacrifice an extra 1 byte per route
				
				//[20140410] RAM usage:
				// - 1 extra route per node: 14 bytes
				// - 1 extra node with 3 routes: 43 bytes
				// Total for 3 nodes X 3 routes: 129 bytes
				
				struct route_entry_t {
					NetworkV1::route_t route;
					uint8_t route_hops[Meshwork::L3::NetworkV1::NetworkV1::MAX_ROUTING_HOPS];
					int8_t qos;//range: [QOS_LEVEL_MIN, QOS_LEVEL_MAX]
				};
			
				struct route_list_t {
					int8_t dst;
					route_entry_t entries[MAX_DST_ROUTES];
				};
				
				struct route_table_t {
					route_list_t lists[MAX_DST_NODES];
				};
				
				class RouteCacheListener {
				public:
					
//					//currently removed since it complicates the implementation, and CHANGED should be enough
//					//listener called after adding, so that data can be read and stored
//					static const uint8_t ROUTE_ENTRY_ADDED 		= 0;
					//listener called before the entry has been removed/cleared
					//in order to have the chance to read the data first
					static const uint8_t ROUTE_ENTRY_REMOVING 	= 1;
					//listener called after the add/change, so that data can be read and stored
					static const uint8_t ROUTE_ENTRY_CHANGED 	= 2;
					
					virtual void route_entry_change(RouteCache* route_cache, route_entry_t* entry, const uint8_t change) = 0;
				};

			protected:
				route_table_t m_table;
				RouteCacheListener* m_route_cache_listener;
				
			private:
				bool array_compare(uint8_t* a, uint8_t* b, uint8_t len);
				
				uint8_t normalize_QoS(uint8_t qos);

			public:
				
				RouteCache(RouteCacheListener* listener):
					m_route_cache_listener(listener)
				{
					//make sure we start clean
					remove_all();
				};
				

				route_entry_t* add_route_entry(NetworkV1::route_t* route, bool forceReplace);
				
				
				void remove_all();
				
				void remove_all_for_dst(uint8_t dst);
				
				void remove_route_entry(route_entry_t* entry);
				
				
				uint8_t get_route_count(uint8_t dst);
				
				route_list_t* get_route_list(uint8_t dst);

				route_entry_t* get_route_entry(uint8_t dst, uint8_t index);
				
				route_entry_t* get_route_entry(NetworkV1::route_t* route);
								
				bool get_route_entry_index(route_entry_t* entry, uint8_t& node_index, uint8_t& route_index);
				
				
				bool update_QoS(NetworkV1::route_t* route, bool increase);
				
				int8_t get_QoS(uint8_t dst, int8_t calculate);
				
				
				//Well, for some reason overloading << with RouteCache's structs caused ambiguous declarations
				void print(IOStream& outs, NetworkV1::route_t& route, uint8_t tabs);
				void print(IOStream& outs, RouteCache::route_entry_t& route_entry, uint8_t tabs);
				void print(IOStream& outs, RouteCache::route_list_t& route_list, uint8_t tabs);
				void print(IOStream& outs);

			};//RouteCache
		};
	};
};

#endif

