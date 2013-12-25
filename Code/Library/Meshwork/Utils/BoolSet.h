/**
 * This file is part of the Meshwork project.
 *
 * Based on:
 * http://ideone.com/WH1W22
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

#ifndef __MESHWORK_UTILS_BOOLSET_H__
#define __MESHWORK_UTILS_BOOLSET_H__

#include "Cosa/Types.h"
 
template<uint8_t S> class BoolSet {
	static int const CHAR_BIT = 8;
    static int const SIZE = ((S / CHAR_BIT) + (0 != (S % CHAR_BIT)));
	unsigned char m_bits[SIZE];
	
public:
	BoolSet(bool init) :
		m_bits() {
			for(int i = 0; i < SIZE; ++i)
				m_bits[i] = init ? 255 : 0;
		}
 
	int8_t get(int i) const {
		return i >= SIZE ? (m_bits[i / CHAR_BIT] & (1 << (i % CHAR_BIT))) : -1;
	}
 
	int8_t set(int i, bool v) {
		if (i >= SIZE)
			return -1;
		char bit = 1 << (i % CHAR_BIT);
		int index = i / CHAR_BIT;
		unsigned char oldvalue = m_bits[index];
		m_bits[index] = v ? ( oldvalue | (bit) ) : ( oldvalue & (~bit) );
		return oldvalue;
	}
 
};
#endif
