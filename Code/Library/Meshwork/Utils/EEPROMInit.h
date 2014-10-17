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

#ifndef __MESHWORK_UTILS_EEPROMINIT_H__
#define __MESHWORK_UTILS_EEPROMINIT_H__

#include "Cosa/EEPROM.hh"
#include "Meshwork.h"

#ifndef LOG_EEPROMINIT
#define LOG_EEPROMINIT  true
#endif

class EEPROMInit {
	public: 
		static void format(EEPROM* eeprom, uint16_t data_start, uint16_t data_end, uint8_t marker) {
			MW_LOG_DEBUG(LOG_EEPROMINIT, "EEPROM formatting: data_start=%d, data_end=%d, marker=%d", data_start, data_end, marker);
			//start backwards to optimize the loop check
			uint16_t next = data_end;
			do {
				eeprom->write((void*) &next, (void*) &marker, 1);
			} while (--next >= data_start);
			MW_LOG_DEBUG(LOG_EEPROMINIT, "Done", NULL);
		}
		
		//returns true if the EEPROM has now been formatted
		static bool init(EEPROM* eeprom, uint16_t data_start, uint16_t data_end, uint8_t marker) {
			uint8_t formatted = eeprom->read((void*) &formatted, (void*) &data_start, 1);
			if ( formatted != marker ) {
				format(eeprom, data_start, data_end, marker);
			} else {
				MW_LOG_DEBUG(LOG_EEPROMINIT, "EEPROM already formatted: data_start=%d, data_end=%d, marker=%d", data_start, data_end, marker);
			}
			return formatted != marker;
		}
};

#endif
