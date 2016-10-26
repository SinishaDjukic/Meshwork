/**
 * This file is part of the Meshwork project.
 *
 * Copyright (C) 2016, Sinisha Djukic
 * 
 * Code excerpts from Michael Patel's fascinating Cosa project
 * https://github.com/mikaelpatel/Cosa/
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
#ifndef __EXAMPLES_BEACON_H__
#define __EXAMPLES_BEACON_H__


//First, include the configuration constants file
#include "MeshworkConfiguration.h"



//All build-time configuration, RF selection, EEPROM usage,
//Route Cache table, etc. is defined in a single place here
#include "Config.h"

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: INCLUDES AND USES /////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <Cosa/EEPROM.hh>
#include <Cosa/Trace.hh>
#include <Cosa/Types.h>
#include <Cosa/IOStream.hh>
#include <Cosa/UART.hh>
#include <Cosa/Watchdog.hh>
#include <Cosa/AnalogPin.hh>
#include <Cosa/InputPin.hh>
#include <Cosa/OutputPin.hh>
#include <Cosa/RTT.hh>
#include <Cosa/Wireless.hh>

#include <Cosa/IOPin.hh>
#include <Cosa/ExternalInterrupt.hh>

#include <OWI.h>
#include <OWI.cpp>
#include <DS18B20.h>
#include <DS18B20.cpp>

#include <Meshwork.h>
#include <Meshwork/L3/Network.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.cpp>

#if EX_LED_TRACING
	#include "Utils/LEDTracing.h"
#endif

#if ( MW_RF_SELECT == MW_RF_NRF24L01P )
	#include <NRF24L01P/NRF24L01P.hh>
	#include <NRF24L01P/NRF24L01P.cpp>
#endif

#if ( ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_RAM ) || ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT ) )
	#include <Meshwork/L3/NetworkV1/RouteCache.h>
	#include <Meshwork/L3/NetworkV1/RouteCache.cpp>
	#include <Meshwork/L3/NetworkV1/CachingRouteProvider.h>
	#if ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT )
		#include <Meshwork/L3/NetworkV1/RouteCachePersistent.h>
	#endif
#endif

using namespace Meshwork::L3::NetworkV1;


///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: MEMBER DECLARATION ////////////////////////
///////////////////////////////////////////////////////////////////////////////

//RF and Mesh
MW_DECL_NRF24L01P(rf)
#if ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT )
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_PERSISTENT(routeprovider, eeprom, EX_ROUTECACHE_TABLE_EEPROM_OFFSET)
#elif ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_RAM )
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_RAM(routeprovider)
#else // MW_ROUTECACHE_NONE
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_NONE(routeprovider)
#endif
NetworkV1 mesh(&rf, routeprovider, NetworkV1::NWKCAPS_SLEEPING);

//Tracing LEDs
#if EX_LED_TRACING
	MW_DECL_LEDTRACING(ledTracing, mesh, EX_LED_TRACING_SEND, EX_LED_TRACING_RECV, EX_LED_TRACING_ACK)
#endif

static const uint8_t 	BEACON_BCAST_PORT 	= EX_PORT;
static uint8_t seq_counter = 0;
	

// Connect to one-wire device
// D4: GND
// D5: DATA <---------q
// D6: VCC --- 4.7K --d
//Uno
OWI sensor_data(Board::D5);
DS18B20 sensor_temperature(&sensor_data);
OutputPin sensor_gnd(Board::D4);
OutputPin sensor_vcc(Board::D6);

// Message from the device; temperatures and voltage reading
struct dt_msg_t {
  uint8_t nr;
  int16_t temperature;
  uint16_t battery;
};

// Power-down sleep (ms)
#define DEEP_SLEEP(ms)					\
  do {							\
    uint8_t mode = Power::set(SLEEP_MODE_PWR_DOWN);	\
    Watchdog::delay(ms);				\
    Power::set(mode);					\
  } while (0)

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: BUSINESS LOGIC/CODE ///////////////////////
///////////////////////////////////////////////////////////////////////////////

//Setup sequence
void setup()
{
	Watchdog::begin();
	
	sensor_gnd.off();
	sensor_vcc.off();
	
#if EX_LOG_BEACON
	uart.begin(115200);
	trace.begin(&uart, PSTR("Beacon: started\n"));
#endif

	MW_LOG_DEBUG_TRACE(EX_LOG_BEACON) << PSTR("Network ID: ") << EX_NWK_ID << endl;
	MW_LOG_DEBUG_TRACE(EX_LOG_BEACON) << PSTR("Channel ID: ") << EX_CHANNEL_ID << endl;
	MW_LOG_DEBUG_TRACE(EX_LOG_BEACON) << PSTR("Node ID: ") << EX_NODE_ID << endl;
  
#if EX_LED_TRACING
	mesh.set_radio_listener(&ledTracing);
#endif

	mesh.setNetworkID(EX_NWK_ID);
	mesh.setChannel(EX_CHANNEL_ID);
	mesh.setNodeID(EX_NODE_ID);
	
  ASSERT(mesh.begin());
  rf.powerdown();
}

//Main loop
void loop()
{
  Watchdog::end();
  Watchdog::begin(16);
  
	// Turn on necessary hardware modules
	Power::all_enable();
	Watchdog::delay(32);//TODO check if required
  
  dt_msg_t msg;
  msg.nr = seq_counter ++;
  MW_LOG_DEBUG_TRACE(EX_LOG_BEACON) << PSTR("Enter: get bandgap") << endl;
  msg.battery = AnalogPin::bandgap(1100);
  
	//Wait for the temperature sensor
	sensor_vcc.on();
	Watchdog::delay(16);//TODO check if the sensor needs less time
	
	MW_LOG_DEBUG_TRACE(EX_LOG_BEACON) << PSTR("Enter: sensor_temperature.connect") << endl;

	//Connect to read
	if ( sensor_temperature.connect(0) ) {
		MW_LOG_DEBUG_TRACE(EX_LOG_BEACON) << PSTR("Enter: DS18B20.covert_request") << endl;
		// Make a conversion request and read the temperature (scratchpad)
		DS18B20::convert_request(&sensor_data, 12, true);
		MW_LOG_DEBUG_TRACE(EX_LOG_BEACON) << PSTR("Enter: sensor_temperature.read_scratchpad") << endl;
		sensor_temperature.read_scratchpad();
    sensor_vcc.off();
    msg.temperature = sensor_temperature.temperature();//12.4 fp
  } else {
		MW_LOG_ERROR(EX_LOG_BEACON, "*** Temperature Sensor not found", NULL);
    msg.temperature = 0;
  }

  MW_LOG_INFO(EX_LOG_BEACON, "***   Message:\r\n        Seq #: %d\r\n  Temperature: %d.%d C\r\n      Battery: %d mV\r\n", msg.nr, (msg.temperature >> 4), (msg.temperature & 0x15), msg.battery);
  
	MW_LOG_DEBUG_ARRAY(EX_LOG_BEACON, PSTR("Raw bytes: "), (char*) &msg, sizeof(msg));
  
  MW_LOG_DEBUG_TRACE(EX_LOG_BEACON) << PSTR("Enter: RTT, Mesh and RF start") << endl;
  
  RTT::begin();
  Watchdog::delay(16);
  rf.powerup();
  
  MW_LOG_INFO(EX_LOG_BEACON, "*** Broadcasting: Start", NULL);
  uint16_t time = RTT::millis();
  mesh.broadcast(BEACON_BCAST_PORT, &msg, sizeof(msg));
  time = RTT::since(time);
  MW_LOG_INFO(EX_LOG_BEACON, "*** Broadcasting: End *** Time spent: %d ms", time);
  
  rf.powerdown();
  RTT::end();

#if !EX_LOG_BEACON
  //Power down messes up the UART, so don't do it unless debugging
  //We change the watchdog granularity to sleep more efficiently
  Watchdog::end();
  Watchdog::begin(1024*8);

	Power::all_disable();
  //Deep sleep
  uint8_t mode = Power::set(SLEEP_MODE_PWR_DOWN);
  Watchdog::delay(60000);//will be rounded to ~64s by Cosa to be a multiple of 1024*8
  //Restore mode
  Power::set(mode);
#else
  //Normal sleep
  Watchdog::delay(EX_BEACON_INTERVAL);
#endif
}

#endif
