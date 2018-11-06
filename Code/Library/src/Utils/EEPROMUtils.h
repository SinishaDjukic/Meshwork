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

#ifndef __MESHWORK_UTILS_EEPROMUTILS_H__
#define __MESHWORK_UTILS_EEPROMUTILS_H__

#include "Meshwork.h"
#include "Cosa/EEPROM.hh"

#ifndef MW_LOG_EEPROMUTILS
	#define MW_LOG_EEPROMUTILS  MW_FULL_DEBUG
#endif

class EEPROMUtils {
	
	private:
		static uint8_t read(EEPROM* eeprom, uint16_t address) {
			uint8_t value;
			eeprom->read((uint8_t*) &value, (uint8_t*) address);
			return value;
		}

		static void write(EEPROM* eeprom, uint16_t address, uint8_t value) {
			eeprom->write((uint8_t*) address, (uint8_t*) &value, 1);
		}

	public:

		static void format(EEPROM* eeprom, uint16_t data_start, uint16_t data_end, uint8_t marker, uint8_t memvalue) {
			uint16_t next = data_start;
			MW_LOG_DEBUG(MW_LOG_EEPROMUTILS, "Writing marker=%d at next=%d", marker, next);
			write(eeprom, next, marker);
			next++;

			MW_LOG_DEBUG(MW_LOG_EEPROMUTILS, "Formatting: data_start=%d, data_end=%d, marker=%d, memvalue=%d", data_start, data_end, marker, memvalue);

			do {
				write(eeprom, next, memvalue);
//				MW_LOG_DEBUG(MW_LOG_EEPROMUTILS, "Writing memvalue=%d at next=%d", memvalue, next);

				uint8_t tmp = read(eeprom, next);
				UNUSED(tmp);
//				MW_LOG_DEBUG(MW_LOG_EEPROMUTILS, "Read at next=%d, memvalue=%d", next, tmp);
			} while ( next++ < data_end);
			MW_LOG_DEBUG(MW_LOG_EEPROMUTILS, "Done", NULL);
		}
		
		//returns true if the EEPROM has now been formatted
		static bool init(EEPROM* eeprom, uint16_t data_start, uint16_t data_end, uint8_t marker, uint8_t memvalue) {
			uint8_t formatted = read(eeprom, data_start);
			MW_LOG_DEBUG(MW_LOG_EEPROMUTILS, "EEPROM data_start=%d, formatted=%d, marker=%d", data_start, formatted, marker);

			if ( formatted != marker ) {
				MW_LOG_DEBUG(MW_LOG_EEPROMUTILS, "EEPROM NOT formatted", NULL);
				format(eeprom, data_start, data_end, marker, memvalue);
			} else {
				MW_LOG_DEBUG(MW_LOG_EEPROMUTILS, "EEPROM already formatted: data_start=%d, data_end=%d, marker=%d", data_start, data_end, marker);
			}
			return formatted != marker;
		}
};

#endif
