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

#include "MeshworkConfiguration.h"
#include "Cosa/Types.h"
#include "Cosa/Memory.h"
#include "Cosa/RTT.hh"
#include "Cosa/Trace.hh"
#include "Cosa/Watchdog.hh"

namespace Meshwork {

	//General-purpose debug functions
	class Debug {
		public: 
			static void printTimestamp() {
				trace << PSTR("[") << RTT::millis() << PSTR("]") << PSTR(" ");
			}
			static void printFreeMemory() {
				trace << PSTR("Free mem: ") << free_memory() << endl;
			}
	};

//Multiplier factor for all delays in Meshwork
//Useful for slowing down the responses for
//debugging and demonstration purposes
#ifndef MW_DELAY_FACTOR
	#define MW_DELAY_FACTOR 1
#endif

	//General-purpose time functions
	class Time {
		public:
			static void delay(uint32_t ms)
		    	__attribute__((always_inline))
			{
				Watchdog::delay((uint32_t) (ms * MW_DELAY_FACTOR));
			}

			static bool passed(uint32_t duration, uint32_t passed)
		    	__attribute__((always_inline))
			{
				return duration >= (uint32_t) (passed * MW_DELAY_FACTOR);
			}
	};
};

#ifndef MW_SUPPORT_DELIVERY_ROUTED
	#define MW_SUPPORT_DELIVERY_ROUTED	true
#endif

#ifndef MW_SUPPORT_DELIVERY_FLOOD
	#define MW_SUPPORT_DELIVERY_FLOOD	true
#endif

#ifndef MW_SUPPORT_REROUTING
	#define MW_SUPPORT_REROUTING		true
#endif

//Set the MW_LOG_XXX debug states based on the current configuration
//ALL: includes ASSERT, EMERG, ALERT, CRIT, ERROR, WARNING, NOTICE and INFO
//For DEBUG we have a separate define below
#ifndef MW_LOG_ALL_ENABLE
	#define MW_LOG_ALL_ENABLE 	MW_FULL_DEBUG
#endif

//Includes DEBUG, DEBUG_TRACE, ARRAY_BYTES, DEBUG_ARRAY and DEBUG_VP_BYTES calls
#ifndef MW_LOG_DEBUG_ENABLE
	#define MW_LOG_DEBUG_ENABLE	MW_FULL_DEBUG
#endif

//Convenience log macros
#define MW_LOG_ASSERT(file_enable, expr) 		if (file_enable && MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); ASSERT(expr); } while (0)
#define MW_LOG_EMERG(file_enable, msg, ...) 		if (file_enable && MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); EMERG(msg, __VA_ARGS__); } while (0)
#define MW_LOG_ALERT(file_enable, msg, ...) 		if (file_enable && MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); ALERT(msg, __VA_ARGS__); } while (0)
#define MW_LOG_CRIT(file_enable, msg, ...) 		if (file_enable && MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); CRIT(msg, __VA_ARGS__); } while (0)
#define MW_LOG_ERROR(file_enable, msg, ...) 		if (file_enable && MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); ERR(msg, __VA_ARGS__); } while (0)
#define MW_LOG_WARNING(file_enable, msg, ...) 	if (file_enable && MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); WARNING(msg, __VA_ARGS__); } while (0)
#define MW_LOG_NOTICE(file_enable, msg, ...) 	if (file_enable && MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); NOTICE(msg, __VA_ARGS__); } while (0)
#define MW_LOG_INFO(file_enable, msg, ...) 		if (file_enable && MW_LOG_ALL_ENABLE) do { Meshwork::Debug::printTimestamp(); INFO(msg, __VA_ARGS__); } while (0)
#define MW_LOG_DEBUG(file_enable, msg, ...) 		if (file_enable && MW_LOG_DEBUG_ENABLE) do { Meshwork::Debug::printTimestamp(); DEBUG(msg, __VA_ARGS__); } while (0)
#define MW_LOG_DEBUG_TRACE(file_enable) 			if (file_enable && MW_LOG_DEBUG_ENABLE) trace

#define MW_LOG_ARRAY_BYTES(file_enable, array, len)							\
  if (file_enable && MW_LOG_DEBUG_ENABLE) do {									\
	trace.print((const void*) array, len, IOStream::hex, len+1); \
	trace << PSTR(" "); \
  } while (0)
  
#define MW_LOG_DEBUG_ARRAY(file_enable, msg, array, len)							\
  if (file_enable && MW_LOG_DEBUG_ENABLE) do {									\
    trace << msg;					\
    MW_LOG_ARRAY_BYTES(file_enable, array, len); \
	trace << PSTR("\n"); \
  } while (0)

#define	MW_LOG_DEBUG_VP_BYTES(file_enable, msg, msgvp) \
	if (file_enable && MW_LOG_DEBUG_ENABLE) do { \
      trace << msg;					\
	  for (const iovec_t* vp = msgvp; vp->buf != 0; vp++) \
		MW_LOG_ARRAY_BYTES(file_enable, (const void*)vp->buf, (uint8_t) vp->size); \
	  trace << PSTR("\n"); \
	} while (0)
	
#define MW_MACRO_VALUE_TO_STRING(x) 	#x
#define MW_MACRO_VALUE(x) 				MW_MACRO_VALUE_TO_STRING(x)
#define MW_MACRO_VAR_NAME_VALUE(var) 	#var "="  MW_MACRO_VALUE(var)

#if (MW_RF_SELECT == MW_RF_NRF24L01P)
	#if (MW_BOARD_SELECT == MW_BOARD_RBBB)
		#define MW_NRF24L01P_CSN 	Board::D7
		#define MW_NRF24L01P_CE 	Board::D8
		#define MW_NRF24L01P_IRQ 	Board::EXT0
	#elif (MW_BOARD_SELECT == MW_BOARD_MEGA)
		#define MW_NRF24L01P_CSN 	Board::D53
		#define MW_NRF24L01P_CE 	Board::D48
		#define MW_NRF24L01P_IRQ 	Board::EXT2
	#elif (MW_BOARD_SELECT == MW_BOARD_UNO)
		#define MW_NRF24L01P_CSN 	Board::D10
		#define MW_NRF24L01P_CE 	Board::D3
		#define MW_NRF24L01P_IRQ 	Board::EXT0
	#else
		#pragma message("[Meshwork] Unknown MW_BOARD_SELECT value: ")
		#pragma message(MW_MACRO_VAR_NAME_VALUE(MW_BOARD_SELECT))
	#endif
#else
	#pragma message()"[Meshwork] Unknown MW_RF_NRF24L01P value: ")
	#pragma message(MW_MACRO_VAR_NAME_VALUE(MW_RF_NRF24L01P))
#endif

//Don't forget to include before using:
//#include "Cosa/Wireless/Driver/NRF24L01P.hh"
#define MW_DECL_NRF24L01P(nrf)								\
	NRF24L01P nrf(0x0001, 0x01,								\
			Board::DigitalPin(MW_NRF24L01P_CSN), 			\
			Board::DigitalPin(MW_NRF24L01P_CE), 			\
			Board::ExternalInterruptPin(MW_NRF24L01P_IRQ));

#if MW_SUPPORT_DELIVERY_ROUTED
//Don't forget to include before using:
//#include <Meshwork/L3/NetworkV1/RouteCache.h>
#define MW_DECLP_ROUTEPROVIDER_ROUTECACHE_NONE(routeprovider)					\
	Meshwork::L3::NetworkV1::NetworkV1::RouteProvider* routeprovider = NULL;

//Don't forget to include before using:
//#include <Meshwork/L3/NetworkV1/RouteCache.h>
//#include <Meshwork/L3/NetworkV1/RouteCache.cpp>
//#include <Meshwork/L3/NetworkV1/CachingRouteProvider.h>
#define MW_DECLP_ROUTEPROVIDER_ROUTECACHE_RAM(routeprovider)					\
	Meshwork::L3::NetworkV1::RouteCache routecache_##routeprovider(NULL);								\
	Meshwork::L3::NetworkV1::CachingRouteProvider p_routeprovider(&(routecache_##routeprovider),			\
			Meshwork::L3::NetworkV1::CachingRouteProvider::UPDATE_REMOVE_ON_QOS_MIN |	\
			Meshwork::L3::NetworkV1::CachingRouteProvider::UPDATE_REPLACE_ON_QOS_WORST);	\
	Meshwork::L3::NetworkV1::NetworkV1::RouteProvider* routeprovider = &p_routeprovider;

//Don't forget to include before using:
//#include <Meshwork/L3/NetworkV1/RouteCache.h>
//#include <Meshwork/L3/NetworkV1/RouteCache.cpp>
//#include <Meshwork/L3/NetworkV1/CachingRouteProvider.h>
//#include <Cosa/EEPROM.hh>
//#include <Meshwork/L3/NetworkV1/RouteCachePersistent.h>
#define MW_DECLP_ROUTEPROVIDER_ROUTECACHE_PERSISTENT(routeprovider, eeprom, eepromoffset)	\
	Meshwork::L3::NetworkV1::RouteCachePersistent routecache_##routeprovider(&eeprom, eepromoffset);					\
	Meshwork::L3::NetworkV1::CachingRouteProvider p_routeprovider(&(routecache_##routeprovider),						\
			Meshwork::L3::NetworkV1::CachingRouteProvider::UPDATE_REMOVE_ON_QOS_MIN |				\
			Meshwork::L3::NetworkV1::CachingRouteProvider::UPDATE_REPLACE_ON_QOS_WORST);				\
	Meshwork::L3::NetworkV1::NetworkV1::RouteProvider* routeprovider = &p_routeprovider;
#endif

//Don't forget to include before using:
//#include <Cosa/OutputPin.hh>
//#include <Utils/LEDTracing.h>
#define MW_DECL_LEDTRACING(ledTracing, mesh, send, recv, ack)			\
		OutputPin pin_send_##ledTracing(send);							\
		OutputPin pin_recv_##ledTracing(recv);							\
		OutputPin pin_ack_##ledTracing(ack);							\
		LEDTracing ledTracing(&mesh, &pin_send_##ledTracing, &pin_recv_##ledTracing, &pin_ack_##ledTracing);

//Don't forget to include before using:
//#include <Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfPersistent.h>
#define MW_DECL_ZEROCONF_PERSISTENT(zeroConfPersistent, zeroConfConfiguration, eeprom, eepromoffset)				\
		ZeroConfPersistent::zctype_configuration_t zeroConfConfiguration;											\
		ZeroConfPersistent zeroConfPersistent(&eeprom, &zeroConfConfiguration, eepromoffset);

#endif
