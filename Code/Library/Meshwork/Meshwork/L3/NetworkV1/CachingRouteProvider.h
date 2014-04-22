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
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Meshwork/L3/NetworkV1/RouteCache.h"

using Meshwork::L3::NetworkV1::NetworkV1;

/**
 * RouteProvider implementation, which uses a given RouteCache and policy to keep the routes
 * cached and up-to-date.
 */
namespace Meshwork {

	namespace L3 {
	
		namespace NetworkV1 {
		
			class CachingRouteProvider: public Meshwork::L3::NetworkV1::NetworkV1::RouteProvider {

			protected:
				uint8_t m_update_policy;
				bool m_route_update_enabled;
				RouteCache* m_route_cache;

			public:

				//removes the route when messages to the destination consistently fail
				static const uint8_t UPDATE_REMOVE_ON_QOS_MIN 		= 0x1;
				//replaces the worst route whenever a new one is discovered
				static const uint8_t UPDATE_REPLACE_ON_QOS_WORST 	= 0x2;

				CachingRouteProvider(RouteCache* cache, uint8_t update_policy):
					m_route_cache(cache),
					m_update_policy(update_policy),
					m_route_update_enabled(true)
				{
				};

				  void set_address(uint8_t src) {
					//nothing to do currently
				  }
				  
				  uint8_t get_routeCount(uint8_t dst) {
					return m_route_cache->get_route_count(dst);
				  }
				  
				  uint8_t get_update_policy() {
					return m_update_policy;
				  }
				  
				  void set_update_policy(uint8_t update_policy) {
					m_update_policy = update_policy;
				  }
				  
				  NetworkV1::route_t* get_route(uint8_t dst, uint8_t index) {
					RouteCache::route_entry_t* entry = m_route_cache->get_route_entry(dst, index);
					return entry != NULL ? &entry->route : NULL;
				  }
				  
				  bool get_route_update_enabled() {
					return m_route_update_enabled;
				  }
				  
				  void set_route_update_enabled(bool enabled) {
					m_route_update_enabled = enabled;
				  }
				  
				  void route_found(NetworkV1::route_t* route) {
					if ( !m_route_cache->update_QoS(route, true) &&
							m_route_update_enabled ) {
						m_route_cache->add_route_entry(route, ( m_update_policy & UPDATE_REPLACE_ON_QOS_WORST ));
					}
				  }
				  
				  void route_failed(NetworkV1::route_t* route) {
					if ( m_route_cache->update_QoS(route, false) &&
							m_route_update_enabled &&
								( m_update_policy & UPDATE_REMOVE_ON_QOS_MIN ) ) {
						RouteCache::route_entry_t* entry = m_route_cache->get_route_entry(route);
						if ( entry->qos == Network::QOS_LEVEL_MIN )
							m_route_cache->remove_route_entry(entry);
					}
				  }

			};
		};
	};
};
#endif

