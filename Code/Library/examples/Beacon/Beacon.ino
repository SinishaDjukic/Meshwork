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

#define MW_NRF24L01P_CSN 	Board::D10 //Uno: D10
#define MW_NRF24L01P_CE 	Board::D3   //Uno: D3 // ALSO VCC!
#define MW_NRF24L01P_IRQ 	Board::EXT1 //Uno: EXT0=D2 ... Leonardo EXT1=SDA=D2


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
#include <DS18B20.h>

#include <Meshwork.h>
#include <Meshwork/L3/Network.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.h>
#include <Meshwork/L3/NetworkV1/NetworkV1.cpp>

#if EX_LED_TRACING
	#include "Utils/LEDTracing.h"
#endif

#if ( MW_RF_SELECT == MW_RF_NRF24L01P )
	#include <NRF24L01P.h>
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

#if MW_SUPPORT_DELIVERY_ROUTED
	#if ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT )
		MW_DECLP_ROUTEPROVIDER_ROUTECACHE_PERSISTENT(routeprovider, eeprom, EX_ROUTECACHE_TABLE_EEPROM_OFFSET)
	#elif ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_RAM )
		MW_DECLP_ROUTEPROVIDER_ROUTECACHE_RAM(routeprovider)
	#else // MW_ROUTECACHE_NONE
		MW_DECLP_ROUTEPROVIDER_ROUTECACHE_NONE(routeprovider)
	#endif
	NetworkV1 mesh(&rf, routeprovider, NetworkV1::NWKCAPS_SLEEPING);
#else
	NetworkV1 mesh(&rf, NetworkV1::NWKCAPS_SLEEPING);
#endif

//Tracing LEDs
#if EX_LED_TRACING
	MW_DECL_LEDTRACING(ledTracing, mesh, EX_LED_TRACING_SEND, EX_LED_TRACING_RECV, EX_LED_TRACING_ACK)
#endif

static const uint8_t 	BEACON_PORT 	= EX_PORT;
static uint8_t seq_counter = 0;

#ifndef EX_TEMP_DISABLE
	OWI sensor_data(EX_TEMP_DATA);
	DS18B20 sensor_temperature(&sensor_data);
	OutputPin sensor_gnd(EX_TEMP_GND);
	OutputPin sensor_vcc(EX_TEMP_VCC);
#endif

OutputPin radio_vcc(Board::D9);

// Message from the device; temperatures and voltage reading
struct dt_msg_t {
  uint8_t nr;
  int16_t temperature;
  uint16_t battery;
};

dt_msg_t msg;

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: BUSINESS LOGIC/CODE ///////////////////////
///////////////////////////////////////////////////////////////////////////////

//Setup sequence
void setup()
{
	Watchdog::begin();
	
	radio_vcc.on();

#ifndef EX_TEMP_DISABLE
	sensor_gnd.off();
	sensor_vcc.off();
#endif

#if EX_LOG
	uart.begin(115200);
	trace.begin(&uart, PSTR("Beacon: started\n"));
#endif

	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Network ID: ") << EX_NWK_ID << endl;
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Channel ID: ") << EX_CHANNEL_ID << endl;
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Node ID: ") << EX_NODE_ID << endl;

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

  msg.nr = seq_counter ++;
  msg.battery = AnalogPin::bandgap(1100);

#ifndef EX_TEMP_DISABLE
	//Wait for the temperature sensor
	sensor_vcc.on();
	Watchdog::delay(16);//TODO check if the sensor needs less time

	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Enter: sensor_temperature.connect") << endl;

	if ( sensor_temperature.connect(0) ) {
		MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Enter: DS18B20.covert_request") << endl;
		// Make a conversion request and read the temperature (scratchpad)
		DS18B20::convert_request(&sensor_data, 12, true);
		MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Enter: sensor_temperature.read_scratchpad") << endl;
		sensor_temperature.read_scratchpad();
    sensor_vcc.off();
    msg.temperature = sensor_temperature.temperature();//12.4 fp
  } else {
		MW_LOG_ERROR(EX_LOG, "*** Temperature Sensor not found", NULL);
    msg.temperature = -1000 << 4;
	  trace << PSTR("... Temperature Sensor not found: ") << msg.temperature << endl;
  }
#else
	msg.temperature = -1000 << 4;
#endif

  MW_LOG_INFO(EX_LOG, "***   Message:\r\n        Seq #: %d\r\n  Temperature: %d.%d C\r\n      Battery: %d mV\r\n", msg.nr, (msg.temperature >> 4), (msg.temperature & 0x15), msg.battery);

	MW_LOG_DEBUG_ARRAY(EX_LOG, PSTR("Raw bytes: "), (char*) &msg, sizeof(msg));

  MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Enter: RTT, Mesh and RF start") << endl;

  RTT::begin();
  Watchdog::delay(16);
  rf.powerup();

//  MW_LOG_INFO(EX_LOG, "*** Broadcasting: Start", NULL);
MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("*** Broadcasting: Start") << endl;
  uint16_t time = RTT::millis();
  mesh.broadcast(BEACON_PORT, &msg, sizeof(msg));
  time = RTT::since(time);
//  MW_LOG_INFO(EX_LOG, "*** Broadcasting: End *** Time spent: %d ms", time);
MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("*** Broadcasting: End *** Time spent: ") << time << endl;

  rf.powerdown();
  RTT::end();

#if !EX_LOG
  //Power down messes up the UART, so don't do it unless debugging
  //We change the watchdog granularity to sleep more efficiently
  Watchdog::end();
  Watchdog::begin(EX_BEACON_WATCHDOG_INTERVAL);

	Power::all_disable();
  //Deep sleep
  uint8_t mode = Power::set(SLEEP_MODE_PWR_DOWN);
//  Watchdog::delay(EX_BEACON_INTERVAL);//will be rounded to ~64s by Cosa to be a multiple of 1024*8
  //Restore mode
  Power::set(mode);
#else
  //Normal sleep
//  Watchdog::delay(EX_BEACON_INTERVAL);
#endif
}

#endif
