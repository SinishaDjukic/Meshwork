
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

				const static uint16_t UNIT_UNSPECIFIED			= 0x0000;
				const static uint16_t UNIT_BINARY_BYTE			= 0x0100;//0, 255
				const static uint16_t UNIT_PERCENTAGE_BYTE		= 0x0101;//0-255 = 0-100%


		};//end of Meshwork::L7::Unit

	};//end of Meshwork::L7

};//end of Meshwork
#endif
