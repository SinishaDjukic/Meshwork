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
#ifndef __EXAMPLES_MESHV1CONSOLE_STATICACKPROVIDER_H__
#define __EXAMPLES_MESHV1CONSOLE_STATICACKPROVIDER_H__

#include "Cosa/Trace.hh"
#include "Cosa/Wireless.hh"
#include "Meshwork/L3/MeshV1/MeshV1.h"

class StaticACKProvider: public Meshwork::L3::Network::ACKProvider {

public:
	StaticACKProvider()
	{
	};

	  //fills the buffer for the sender's src and port, and also the len of the previously sent message
	  int returnACKPayload(uint8_t src, uint8_t port, void* buf, uint8_t len, void* bufACK, size_t lenACK) {
		  TRACE_LOG("src=%d, port=%d, buf=%d, len=%d, bufACK=%d, lenACK=%d", src, port, buf, len, bufACK, lenACK);
		  ((uint8_t *)bufACK)[0] = src;
		  ((uint8_t *)bufACK)[1] = port;
		  ((uint8_t *)bufACK)[2] = len;
		  return 3;
	  }

};
#endif

