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

#include "Cosa/Wireless.hh"
#include "Cosa/EEPROM.hh"
#include "Meshwork/L3/Network.h"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Utils/EEPROMInit.h"

#ifndef LOG_ROUTECACHEPERSISTENT
#define LOG_ROUTECACHEPERSISTENT  true
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
				
				//Header storing a formatting marker
				static const uint8_t ROUTE_SIZE_TABLE_MARKER = 1;
				//Marker value denoting a formatted eeprom
				static const uint8_t ROUTE_VALUE_TABLE_MARKER = 0x00;
				//EEPROM bytecount for the entire routing table
				static const uint16_t ROUTE_SIZE_TABLE	= ROUTE_SIZE_TABLE_MARKER + 
															ROUTE_SIZE_SINGLE *
																RouteCache::MAX_DST_NODES *
																	RouteCache::MAX_DST_ROUTES;
			
				RouteCachePersistent(EEPROM* eeprom, uint16_t eeprom_offset):
					RouteCache(this),
					m_eeprom(eeprom),
					m_eeprom_offset(eeprom_offset)
				{
					init_eeprom();
					read_routes();
				};
				
				void init_eeprom() {
					EEPROMInit::init(m_eeprom, m_eeprom_offset, m_eeprom_offset + ROUTE_SIZE_TABLE_MARKER + ROUTE_SIZE_TABLE - 1, ROUTE_VALUE_TABLE_MARKER);
				}
				
				void read_routes() {
					for ( int i = 0; i < MAX_DST_NODES; i ++ )
						for ( int j = 0; j < MAX_DST_ROUTES; j ++ ) {
							uint16_t data_start = m_eeprom_offset +
													i * MAX_DST_NODES * ROUTE_SIZE_SINGLE +
														j * ROUTE_SIZE_SINGLE;
							
							//TODO check if all the pointer converstions are ok
							
							uint8_t tmp = 0;
							m_eeprom->read((void*) &tmp, (void*) &data_start, 1);
							if ( tmp != 0 ) {
								route_entry_t entry = m_table.lists[i].entries[j];
								entry.qos = Network::QOS_LEVEL_AVERAGE;//qos not stored so reset
								//src
								entry.route.src = tmp;
								//dst
								data_start ++;
								m_eeprom->read((void*) &tmp, (void*) &data_start, 1);
								m_table.lists[i].dst = tmp;
								//hopCount
								data_start ++;
								m_eeprom->read((void*) &tmp, (void*) &data_start, 1);
								entry.route.hopCount = tmp;
								//hops
								if ( tmp > 0 ) {
									data_start ++;
									m_eeprom->read((void*) &entry.route.hops, (void*) &data_start, tmp);
								}
							} else {
								
							}
						}
				}

				void route_entry_change(RouteCache* route_cache,
										RouteCache::route_entry_t* entry,
										const uint8_t change) {
					uint8_t node_index, route_index;
					
					//TODO check if all the pointer converstions are ok
					
					if ( get_route_entry_index(entry, node_index, route_index) ) {
						uint16_t data_start = m_eeprom_offset +
												node_index * MAX_DST_NODES * ROUTE_SIZE_SINGLE +
													route_index * ROUTE_SIZE_SINGLE;
						if ( change == ROUTE_ENTRY_REMOVING ) {
							//just reset the first byte (src) to 0 to mark the route as removed
							uint8_t empty = 0;
							m_eeprom->write((void*) &data_start, (void*) &empty, 1);
						} else if ( change == ROUTE_ENTRY_CHANGED ) {
							m_eeprom->write((void*) &data_start, (void*) entry->route.src, 1);
							data_start++;
							m_eeprom->write((void*) &data_start, (void*) entry->route.dst, 1);
							data_start++;
							m_eeprom->write((void*) &data_start, (void*) entry->route.hopCount, 1);
							if ( entry->route.hopCount > 0 ) {
								data_start++;
								m_eeprom->write((void*) &data_start, (void*) entry->route.hops, entry->route.hopCount);
							}
						}
					}
				}
				
			};//RouteCachePersistent
		};
	};
};
#endif

