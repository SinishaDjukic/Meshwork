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
#ifndef __MESHWORK_L3_NETWORKV1_ROUTECACHEPERSISTENT_H__
#define __MESHWORK_L3_NETWORKV1_ROUTECACHEPERSISTENT_H__

#include "Meshwork.h"
#include "Cosa/Wireless.hh"
#include "Cosa/EEPROM.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Utils/EEPROMUtils.h"

#ifndef MW_LOG_ROUTECACHEPERSISTENT
	#define MW_LOG_ROUTECACHEPERSISTENT		MW_FULL_DEBUG
#endif

using Meshwork::L3::NetworkV1::NetworkV1;

/**
 * Stores cached routes persistently in EEPROM.
 */
namespace Meshwork {

	namespace L3 {
	
		namespace NetworkV1 {
		
			class RouteCachePersistent: public RouteCache, public RouteCache::RouteCacheListener {
			
			protected:
				EEPROM* m_eeprom;
				uint16_t m_eeprom_offset;

			public:
			
				//EEPROM bytecount for a single route
				//Current structure:
				//	uint8_t route.src
				//	uint8_t route.dst
				//	uint8_t route.hopCount
				//  uint8_t route.hops[MAX_ROUTING_HOPS]
				// Total: 11 bytes
				static const uint16_t ROUTE_SIZE_SINGLE	= 1 + 1 + 1 +
															NetworkV1::MAX_ROUTING_HOPS;
				
				//Formatted EEPROM marker len
				static const uint8_t ROUTE_INIT_EEPROM_MARKER_LEN 		= 1;
				//Formatted EEPROM marker value
				static const uint16_t ROUTE_INIT_EEPROM_MARKER_VALUE 	= 0x02;
				//Formatted EEPROM default value
				static const uint16_t ROUTE_INIT_EEPROM_MEM_VALUE 		= 0x00;

				//This is where the data starts
				static const uint8_t ROUTE_STRUCT_DATA_START =  ROUTE_INIT_EEPROM_MARKER_LEN;
				//EEPROM bytecount for the entire routing table
				static const uint16_t ROUTE_STRUCT_END	= ROUTE_STRUCT_DATA_START +
															ROUTE_SIZE_SINGLE *
																RouteCache::MAX_DST_NODES *
																	RouteCache::MAX_DST_ROUTES;
			
				RouteCachePersistent(EEPROM* eeprom, uint16_t eeprom_offset):
					RouteCache(this),
					m_eeprom(eeprom),
					m_eeprom_offset(eeprom_offset)
				{
					init();
					read_routes();
				};
				
				void init() {
					EEPROMUtils::init(m_eeprom, m_eeprom_offset, m_eeprom_offset + ROUTE_STRUCT_END, ROUTE_INIT_EEPROM_MARKER_VALUE, ROUTE_INIT_EEPROM_MEM_VALUE);
				}
				
				void read_routes() {
					for ( int i = 0; i < MAX_DST_NODES; i ++ )
						for ( int j = 0; j < MAX_DST_ROUTES; j ++ ) {
							uint16_t data_start = m_eeprom_offset +
													i * MAX_DST_NODES * ROUTE_SIZE_SINGLE +
														j * ROUTE_SIZE_SINGLE;
							
							uint8_t tmp = 0;
							m_eeprom->read((uint8_t*) &tmp, (uint8_t*) data_start, 1);
							if ( tmp != 0 ) {
								route_entry_t entry = m_table.lists[i].entries[j];
								entry.qos = Network::QOS_LEVEL_AVERAGE;//qos not stored so reset
								//src
								entry.route.src = tmp;
								//dst
								data_start ++;
								m_eeprom->read((uint8_t*) &tmp, (uint8_t*) data_start, 1);
								m_table.lists[i].dst = tmp;
								//hopCount
								data_start ++;
								m_eeprom->read((uint8_t*) &tmp, (uint8_t*) data_start, 1);
								entry.route.hopCount = tmp;
								//hops
								if ( tmp > 0 ) {
									data_start ++;
									m_eeprom->read((uint8_t*) &entry.route.hops, (uint8_t*) data_start, tmp);
								}
							} else {
								
							}
						}
				}

				void route_entry_change(RouteCache* route_cache,
										RouteCache::route_entry_t* entry,
										const uint8_t change) {
					UNUSED(route_cache);
					uint8_t node_index, route_index;
					
					if ( get_route_entry_index(entry, node_index, route_index) ) {
						uint16_t data_start = m_eeprom_offset +
												node_index * MAX_DST_NODES * ROUTE_SIZE_SINGLE +
													route_index * ROUTE_SIZE_SINGLE;
						if ( change == ROUTE_ENTRY_REMOVING ) {
							//just reset the first byte (src) to 0 to mark the route as removed
							uint8_t empty = 0;
							m_eeprom->write((uint8_t*) data_start, (uint8_t*) &empty, 1);
						} else if ( change == ROUTE_ENTRY_CHANGED ) {
							m_eeprom->write((uint8_t*) data_start, (uint8_t*) &entry->route.src, 1);
							data_start++;
							m_eeprom->write((uint8_t*) data_start, (uint8_t*) &entry->route.dst, 1);
							data_start++;
							m_eeprom->write((uint8_t*) data_start, (uint8_t*) &entry->route.hopCount, 1);
							if ( entry->route.hopCount > 0 ) {
								data_start++;
								m_eeprom->write((uint8_t*) data_start, (uint8_t*) &entry->route.hops, entry->route.hopCount);
							}
						}
					}
				}
				
			};//RouteCachePersistent
		};
	};
};
#endif

