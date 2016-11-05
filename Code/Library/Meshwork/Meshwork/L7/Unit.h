
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
#ifndef __MESHWORK_L7_UNIT_H__
#define __MESHWORK_L7_UNIT_H__

#include "Cosa/Types.h"

#include "Meshwork.h"

namespace Meshwork {

	namespace L7 {

		class Unit {

			public:

				//Bitmask for custom type
				const static uint16_t UNIT_MASK_CUSTOM			= 0x8000;//most significant bit

				//Unspecified type (shouldn't really be used)
				const static uint16_t UNIT_UNSPECIFIED			= 0x0000;

				//Some predefined units follow...

				//Single byte representing true (255) and false (0)
				const static uint16_t UNIT_8D_BINARY			= 0x0100;//0, 255

				//Single byte representing a percentage value range (0-100)
				const static uint16_t UNIT_8D_PERCENTAGE		= 0x0101;//0-255 = 0-100%

				////////// Units in Q16.16 fixed point format //////////

				//Temperature in Kelvin
				const static uint16_t UNIT_16Q16_KELVIN		= 0x1000;
				//Temperature in Celsius
				const static uint16_t UNIT_16Q16_CELSIUS	= 0x1001;
				//Temperature in Fahrenheit
				const static uint16_t UNIT_16Q16_FAHRENHEIT	= 0x1002;

				////////// Units in signed 16b with signed 8b decimal precision format //////////

				//Volt
				const static uint16_t UNIT_24D8_VOLT		= 0x2000;
				//Ampere
				const static uint16_t UNIT_24D8_AMPERE		= 0x2001;

		};//end of Meshwork::L7::Unit

	};//end of Meshwork::L7

};//end of Meshwork
#endif
