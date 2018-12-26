/**
 * This file is part of the Meshwork project.
 *
 * Copyright (C) 2016, Sinisha Djukic
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
#ifndef __EXAMPLES_MULTISENSORL7_H__
#define __EXAMPLES_MULTISENSORL7_H__

//First, include the configuration constants file
#include "MeshworkConfiguration.h"

#include "Config.h"

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: INCLUDES AND USES /////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <Cosa/Trace.hh>
#include <Cosa/Types.h>
#include <Cosa/Watchdog.hh>
#include <Cosa/AnalogPin.hh>
#include <Cosa/OutputPin.hh>
#include <Cosa/RTT.hh>

#include <Cosa/IOPin.hh>
#include <Cosa/ExternalInterrupt.hh>

#include <OWI.h>
#include <DS18B20.h>

#include <Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfPersistent.h>

#include <Meshwork/L7/BaseRFApplication.h>
#include <Meshwork/L7/BaseRFApplication.cpp>
#include <Meshwork/L7/Endpoint.h>
#include <Meshwork/L7/Cluster.h>
#include <Meshwork/L7/Cluster.cpp>
#include <Meshwork/L7/Device.h>
#include <Meshwork/L7/Device.cpp>
#include <Meshwork/L7/Unit.h>
#include <Meshwork/L7/Endpoints/BinaryPinChangeSensor.h>
#include <Meshwork/L7/Endpoints/TemperatureDS18B20Sensor.h>
#include <Meshwork/L7/Endpoints/VoltageBandgapSensor.h>

using namespace Meshwork::L7::Endpoints;

///////////////////////////////////////////////////////////////////////////////
////////////////////////// SECTION: MEMBER DECLARATION ////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef EX_BATT_V_MAX
  #define EX_BATT_V_MAX    4100
#endif
#ifndef EX_BATT_V_MIN
  #define EX_BATT_V_MIN    3100
#endif

#ifndef EX_DEVICE_TYPE
  #define EX_DEVICE_TYPE		0
#endif
#ifndef EX_DEVICE_SUBTYPE
  #define EX_DEVICE_SUBTYPE		1
#endif

#define EX_CLUSTER_0_TYPE		10
#define EX_CLUSTER_0_SUBTYPE	11

#define EX_CLUSTER_1_TYPE		20
#define EX_CLUSTER_1_SUBTYPE	21

static const uint8_t CLUSTER_0_EP_COUNT 	= 1;
static const uint8_t CLUSTER_1_EP_COUNT 	= 2;
static const uint8_t CLUSTER_COUNT 			= 2;

//must declare here due to stack overflow :(
Device m_deviceinstance(EX_DEVICE_TYPE, EX_DEVICE_SUBTYPE, 0, NULL);

////////// CLUSTER 0
Endpoint* m_cluster_0_endpoints[CLUSTER_0_EP_COUNT];
Cluster m_cluster_0(EX_CLUSTER_0_TYPE, EX_CLUSTER_0_SUBTYPE, 0, NULL);

////////// CLUSTER 1
OWI m_OWI_DS18B20(EX_TEMP_DATA);
DS18B20 m_DS18B20(&m_OWI_DS18B20);
OutputPin m_DS18B20_CS(EX_TEMP_VCC);
OutputPin m_DS18B20_GND(EX_TEMP_GND);
InputPin m_binaryPin((Board::DigitalPin) EX_BINARY_PININT);
Endpoint* m_cluster_1_endpoints[CLUSTER_1_EP_COUNT];
Cluster m_cluster_1(EX_CLUSTER_1_TYPE, EX_CLUSTER_1_SUBTYPE, 0, NULL);

Cluster* m_clusters[CLUSTER_COUNT];

////////// RF APP
class Ex_MultisensorL7_App: public BaseRFApplication, public Endpoint::EndpointListener {

	protected:
		bool m_polling;
		bool m_wakeup;
		ZeroConfPersistent::zctype_configuration_t* m_zeroconf_configuration;
		Endpoint::endpoint_reporting_configuration_t m_reporting_configuration;
		VoltageBandgapSensor m_voltageBandgapSensor;
		TemperatureDS18B20Sensor m_temperatureSensor;
		BinaryPinChangeSensor m_binaryPinChangeSensor;
#if defined(EX_LED_BOOTUP)
		OutputPin ledPin;
#endif

	public:
		Ex_MultisensorL7_App(Meshwork::L3::Network* network):
			m_polling(false),
			m_wakeup(false),
			EndpointListener(),
			m_zeroconf_configuration(NULL),
			m_reporting_configuration(),
			m_voltageBandgapSensor(this, NULL, Unit::UNIT_24D8_VOLT),
			m_temperatureSensor((Endpoint::EndpointListener*) this, NULL, &m_DS18B20, &m_DS18B20_CS, false),
			m_binaryPinChangeSensor(this, NULL, &m_binaryPin),
#if defined(EX_LED_BOOTUP)
			ledPin(EX_LED_BOOTUP),
#endif

			BaseRFApplication(network, &m_deviceinstance)
			{
				m_DS18B20_GND.off();
				m_DS18B20_CS.off();

				m_cluster_0_endpoints[0] = &m_voltageBandgapSensor;
				m_cluster_0.setEndpoints(m_cluster_0_endpoints, membersof(m_cluster_0_endpoints));
				m_clusters[0] = &m_cluster_0;

				m_cluster_1_endpoints[0] = &m_temperatureSensor;
				m_cluster_1_endpoints[1] = &m_binaryPinChangeSensor;
				m_cluster_1.setEndpoints(m_cluster_1_endpoints, membersof(m_cluster_1_endpoints));
				m_clusters[1] = &m_cluster_1;

				m_device->setClusters(m_clusters, membersof(m_clusters));

				for ( uint8_t i = 0; i < m_device->getClusterCount(); i ++ ) {
					Cluster* cluster = m_device->getCluster(i);
					for ( uint8_t j = 0; j < cluster->getEndpointCount(); j ++ ) {
						Endpoint* endpoint = cluster->getEndpoint(j);
						endpoint->setReportingConfiguration(&m_reporting_configuration);
					}
				}
			}

		void setZeroConfConfiguration(ZeroConfPersistent::zctype_configuration_t* zeroconf_configuration) {
			m_zeroconf_configuration = zeroconf_configuration;
			m_reporting_configuration.max_threshold = 5;//TODO must be set by the zerconf tool
			m_reporting_configuration.reporting_flags = zeroconf_configuration->reporting.repflags;
		}

		bool isWakeup() {
			return m_wakeup;
		}

		void clearWakeup() {
			synchronized m_wakeup = false;
		}

		void wakeup(Endpoint* endpoint) {
			m_wakeup = true;
		}

		void propertyChanged(Endpoint* endpoint) {
			Cluster* cluster = endpoint->getCluster();
			int16_t clusterID = m_device->getClusterIndex(cluster);
			int16_t endpointID = cluster->getEndpointIndex(endpoint);
			//TODO sanity check for IDs != -1
			bool cmd_mc_last = true;

			//NodeID==255 means we are broadcasting, otherwise sending unicast
			if ( m_zeroconf_configuration->reporting.targetnodeid != 255 ) {
				MW_LOG_INFO(EX_LOG, "*** Sending: NodeID=%d", m_zeroconf_configuration->reporting.targetnodeid);
			} else {
				MW_LOG_INFO(EX_LOG, "*** Broadcasting", NULL);
			}

			univmsg_l7_any_t msg;
			msg.msg_header.src = m_zeroconf_configuration->reporting.targetnodeid;
			msg.msg_header.port = Meshwork::L7::BASERF_MESSAGE_PORT;
			msg.msg_header.seq = nextSeq();
			uint8_t data[NetworkV1::PAYLOAD_MAX];
			msg.data = data;

			m_network->getDriver()->powerup();

			uint16_t time = RTT::millis();
			Network::msg_l3_status_t status = sendPropertyReport(&msg, cmd_mc_last, clusterID, endpointID);
			time = RTT::since(time);

			m_network->getDriver()->powerdown();

			MW_LOG_INFO(EX_LOG, "*** Send status: %d, Time spent: %d ms", status, time);
		}

		virtual void poll() {
			m_polling = true;
			//TODO for some reason enable/disable doesn't work well here and upon re-enabling the board hangs
//			m_binaryPinChangeSensor.setInterrupts(false);
			for ( uint8_t i = 0; i < m_device->getClusterCount(); i ++ ) {
				Cluster* cluster = m_device->getCluster(i);
				for ( uint8_t j = 0; j < cluster->getEndpointCount(); j ++ ) {
					Endpoint* endpoint = cluster->getEndpoint(j);
					MW_LOG_INFO(EX_LOG, "*** Polling endpoint %d:%d", i, j);
					endpoint->poll();
				}
			}
//			m_binaryPinChangeSensor.setInterrupts(true);
			m_polling = false;
		}

		void begin() {
			m_binaryPinChangeSensor.setInterrupts(true);
		}
};

#endif
