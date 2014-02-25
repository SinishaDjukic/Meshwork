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
#ifndef __MESHWORK_MESHWORK_H__
#define __MESHWORK_MESHWORK_H__

#include "Cosa/Trace.hh"
#include "Cosa/Memory.h"

namespace Meshwork {
	class Debug {
		public: 
			static void printTimestamp() {
				trace.print("[");
				trace.print(RTC::millis());
				trace.print("] ");
			}
			static void printFreeMemory() {
				trace.print("Free mem: ");
				trace.print(free_memory());
				trace.println();
			}
	};
};

//dummy file required to force Arduino IDE include
//Meshwork library dir in the project compilation path

//set to false to disable all logging, except for DEBUG (handled separately)
#ifndef MW_LOG_ALL_ENABLE
#define MW_LOG_ALL_ENABLE true
#endif

//set to false to disable debug level logging
#ifndef MW_LOG_DEBUG_ENABLE
#define MW_LOG_DEBUG_ENABLE true
#endif

#define MW_LOG_ASSERT(expr) 		if (MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); ASSERT(expr); } while (0)
#define MW_LOG_EMERG(msg, ...) 		if (MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); EMERG(msg, __VA_ARGS__); } while (0)
#define MW_LOG_ALERT(msg, ...) 		if (MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); ALERT(msg, __VA_ARGS__); } while (0)
#define MW_LOG_CRIT(msg, ...) 		if (MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); CRIT(msg, __VA_ARGS__); } while (0)
#define MW_LOG_ERROR(msg, ...) 		if (MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); ERR(msg, __VA_ARGS__); } while (0)
#define MW_LOG_WARNING(msg, ...) 	if (MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); WARNING(msg, __VA_ARGS__); } while (0)
#define MW_LOG_NOTICE(msg, ...) 	if (MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); NOTICE(msg, __VA_ARGS__); } while (0)
#define MW_LOG_INFO(msg, ...) 		if (MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); INFO(msg, __VA_ARGS__); } while (0)
#define MW_LOG_DEBUG(msg, ...) 		if (MW_LOG_DEBUG_ENABLE) do { Meshwork::Debug::printTimestamp(); DEBUG(msg, __VA_ARGS__); } while (0)


# define MW_LOG_ARRAY_BYTES(array, len)							\
  if (MW_LOG_DEBUG_ENABLE) do {									\
	trace.print((const void*) array, len, IOStream::hex, len+1); \
	trace << PSTR(" "); \
  } while (0)
  
  
# define MW_LOG_DEBUG_ARRAY(msg, array, len)							\
  if (MW_LOG_DEBUG_ENABLE) do {									\
    trace.print_P(msg);					\
    MW_LOG_ARRAY_BYTES(array, len); \
	trace << PSTR("\n"); \
  } while (0)

#define	MW_LOG_DEBUG_VP_BYTES(msg, msgvp) \
	if (MW_LOG_DEBUG_ENABLE) { \
      trace.print_P(msg);					\
	  for (const iovec_t* vp = msgvp; vp->buf != 0; vp++) \
		MW_LOG_ARRAY_BYTES((const void*)vp->buf, (uint8_t) vp->size); \
	  trace << PSTR("\n"); \
	}
	
#endif