/**
 * This file is part of the Meshwork project.
 *
 * Copyright (C) 2014, Sinisha Djukic
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
#ifndef __EXAMPLES_ZEROCONFBEACON_H__
#define __EXAMPLES_ZEROCONFBEACON_H__

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

#include <Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfSerial.h>
#include <Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfSerial.cpp>
#include <Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfPersistent.h>

#include <Utils/SerialMessageAdapter.h>
#include <Utils/SerialMessageAdapter.cpp>

using namespace Meshwork::L3::NetworkV1;


///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: MEMBER DECLARATION ////////////////////////
///////////////////////////////////////////////////////////////////////////////

//Our EEPROM instance
EEPROM eeprom;

//RF and Mesh
MW_DECL_NRF24L01P(rf)
#if ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT )
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_PERSISTENT(routeprovider, eeprom, EX_ROUTECACHE_TABLE_EEPROM_OFFSET)
#elif ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_RAM )
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_RAM(routeprovider)
#else // MW_ROUTECACHE_NONE
	MW_DECLP_ROUTEPROVIDER_ROUTECACHE_NONE(routeprovider)
#endif
NetworkV1 mesh(&rf, routeprovider, NetworkV1::NWKCAPS_NONE);

//Tracing LEDs
#if EX_LED_TRACING
	MW_DECL_LEDTRACING(ledTracing, mesh, EX_LED_TRACING_SEND, EX_LED_TRACING_RECV, EX_LED_TRACING_ACK)
#endif

MW_DECL_ZEROCONF_PERSISTENT(zeroConfPersistent, zeroConfConfiguration, eeprom, EX_ZC_CONFIGURATION_EEPROM_OFFSET)

//Setup extra UART on Mega
#if MW_BOARD_SELECT == MW_BOARD_MEGA
	#include "Cosa/IOBuffer.hh"
	// Create buffer for HC UART
	static IOBuffer<UART::RX_BUFFER_MAX> ibuf;
	static IOBuffer<UART::TX_BUFFER_MAX> obuf;
	// HC UART will be used for Host-Controller communication
	UART uartHC(3, &ibuf, &obuf);
	SerialMessageAdapter serialMessageAdapter(&uartHC);
#else
	SerialMessageAdapter serialMessageAdapter(&uart);
	IOStream::Device null_device;
#endif

//ZeroConf Serial support
ZeroConfSerial zeroConfSerial(&mesh, &serialMessageAdapter,
								&zeroConfConfiguration.sernum, &zeroConfConfiguration.reporting,
									&zeroConfConfiguration.nwkconfig, &zeroConfConfiguration.devconfig,
										&zeroConfPersistent);
SerialMessageAdapter::SerialMessageListener* serialMessageListeners[1];

//Bootup notification LED
#if defined(EX_LED_BOOTUP)
	OutputPin ledPin(EX_LED_BOOTUP);
	#define LED(state)	ledPin.set(state)
	#define LED_BLINK(state, x)	\
		LED(state); \
		Watchdog::delay(x); \
		LED(!state);
#else
	#define LED(state)	(void) (state)
	#define LED_BLINK(state, x)	(void) (state)
#endif

static const uint8_t 	BEACON_PORT 	= EX_PORT;
static uint8_t seq_counter = 0;

#ifndef EX_TEMP_DISABLE
	OWI sensor_data(EX_TEMP_DATA);
	DS18B20 sensor_temperature(&sensor_data);
	OutputPin sensor_gnd(EX_TEMP_GND);
	OutputPin sensor_vcc(EX_TEMP_VCC);
#endif

// Message from the device; just a sequence number
struct dt_msg_t {
  uint8_t nr;
	int16_t temperature;
  uint16_t battery;
};

dt_msg_t msg;
size_t replySize = 0;

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: BUSINESS LOGIC/CODE ///////////////////////
///////////////////////////////////////////////////////////////////////////////


//Read stored configuration
void readConfig() {
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("[Config] Reading ZeroConf EEPROM...") << endl;
	zeroConfPersistent.init();
	zeroConfPersistent.read_configuration();

	mesh.setNetworkID(zeroConfConfiguration.nwkconfig.nwkid);
	mesh.setNodeID(zeroConfConfiguration.nwkconfig.nodeid);
	mesh.setNetworkKeyLen(zeroConfConfiguration.nwkconfig.nwkkeylen);
	mesh.setNetworkKey(zeroConfConfiguration.nwkconfig.nwkkey);
	mesh.setChannel(zeroConfConfiguration.nwkconfig.channel);
	mesh.setNetworkCaps(zeroConfConfiguration.devconfig.m_nwkcaps);
	mesh.setDelivery(zeroConfConfiguration.devconfig.m_delivery);

#if ( MW_ROUTECACHE_SELECT == MW_ROUTECACHE_PERSISTENT )
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("[Config] Reading RouteCache EEPROM...") << endl;
	routecache_routeprovider.init();
	routecache_routeprovider.read_routes();
#endif

	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("[Config] Done") << endl;
}

//Setup sequence
void setup()
{
	//Basic setup
	Watchdog::begin();
	RTT::begin();

#ifndef EX_TEMP_DISABLE
	sensor_gnd.off();
	sensor_vcc.off();
#endif

	//Enable UART for the boot-up config sequence. Blink once and keep lit during config
	LED_BLINK(true, 500);
	LED(true);

	uart.begin(115200);

	//Trace debugs only supported on Mega, since it has extra UARTs
#if MW_BOARD_SELECT == MW_BOARD_MEGA
	trace.begin(&uart, NULL);
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("ZeroConf Beacon: started") << endl;
	uartHC.begin(115200);
#else
	trace.begin(&null_device, NULL);
#endif

	//Init listeners
	serialMessageListeners[0] = &zeroConfSerial;
	serialMessageAdapter.setListeners(serialMessageListeners);

	//Read current configuration from EEPROM
	readConfig();

	//Wait for boot-time reconfiguration via serial
	MW_LOG_DEBUG_TRACE(EX_LOG) << PSTR("Waiting for ZeroConfSerial connection...") << endl;
	bool reconfigured = zeroConfSerial.processConfigSequence(EX_STARTUP_AUTOCONFIG_INIT_TIMEOUT, EX_STARTUP_AUTOCONFIG_DEINIT_TIMEOUT, EX_SERIAL_NEXT_MSG_TIMEOUT);

	readConfig();

	MW_LOG_DEBUG_TRACE(EX_LOG) << (reconfigured ? PSTR("New configuration applied") : PSTR("Previous configuration used")) << endl;

	RTT::end();

	//Flush all chars
	uart.flush();

#if MW_BOARD_SELECT != MW_BOARD_MEGA
  #if !EX_LOG
	  //Don't disable the UART when debugging
	  uart.end();
  #endif
#endif

  //Blink differently if reconfigured
	LED_BLINK(false, reconfigured ? 2000 : 500);
	LED(false);

#if EX_LED_TRACING
	mesh.set_radio_listener(&ledTracing);
#endif

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

  Watchdog::delay(16);
  rf.powerup();

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

  uint16_t time = RTT::millis();
	//NodeID==255 means we are broadcasting, otherwise sending unicast
	if ( zeroConfConfiguration.reporting.targetnodeid != 255 ) {
	  MW_LOG_INFO(EX_LOG, "*** Sending: NodeID=%d, Delivery=%d",
			zeroConfConfiguration.reporting.targetnodeid,
			zeroConfConfiguration.devconfig.m_delivery);
	  int result = mesh.send(zeroConfConfiguration.devconfig.m_delivery, -1,
			BEACON_PORT, zeroConfConfiguration.reporting.targetnodeid,
			 &msg, sizeof(msg), (void*) NULL, replySize);
  } else {
		MW_LOG_INFO(EX_LOG, "*** Broadcasting", NULL);
		mesh.broadcast(BEACON_PORT, &msg, sizeof(msg));
	}
  time = RTT::since(time);
  MW_LOG_INFO(EX_LOG, "*** Sending: End *** Time spent: %d ms", time);

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
  Watchdog::delay(EX_BEACON_INTERVAL);//will be rounded to ~64s by Cosa to be a multiple of 1024*8
  //Restore mode
  Power::set(mode);
#else
  //Normal sleep
  Watchdog::delay(EX_BEACON_INTERVAL);
#endif
}

#endif
