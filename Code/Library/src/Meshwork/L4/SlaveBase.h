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
#ifndef __MESHWORK_L4_SLAVEBASE_H__
#define __MESHWORK_L4_SLAVEBASE_H__

#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/BitSet.hh"
#include "Meshwork/L3/Network.h"

namespace Meshwork {

	namespace L4 {
	
		class SlaveBase: NodeBase {
		protected:
			virtual int setModeRequestImpl(uint8_t mode, uint32_t timeout);
		
		public:


		};//end of Meshwork::L4::SlaveBase
		
	}//end of Meshwork:L4
	
};//end of Meshwork
#endif