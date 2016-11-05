
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
#ifndef __MESHWORK_L7_ENDPOINTS_TEMPERATUREDS18B20SENSOR_H__
#define __MESHWORK_L7_ENDPOINTS_TEMPERATUREDS18B20SENSOR_H__

#include "Cosa/Types.h"

#include "Meshwork.h"
#include "Meshwork/L7/Endpoint.h"
#include "Meshwork/L7/Unit.h"
#include "Meshwork/L7/Endpoints/TemperatureSensor.h"

#include <Cosa/Watchdog.hh>
#include <Cosa/OutputPin.hh>

#include <DS18B20.h>

using namespace Meshwork::L7;

namespace Meshwork {

	namespace L7 {

		namespace Endpoints {

			class TemperatureDS18B20Sensor: public Endpoints::TemperatureSensor {

				protected:
					DS18B20* m_sensor_temperature;
					OutputPin* m_sensor_cs;
					bool m_sensor_cs_activelow;
					uint8_t m_sensor_index;

				public:
					TemperatureDS18B20Sensor(EndpointListener* listener,
							endpoint_reporting_configuration_t* reporting_configuration,
							DS18B20* sensor_temperature, OutputPin* sensor_cs, bool sensor_cs_activelow,
							uint8_t sensor_index = 0):
						TemperatureSensor(listener, reporting_configuration, 0),
						m_sensor_temperature(sensor_temperature),
						m_sensor_cs(sensor_cs),
						m_sensor_cs_activelow(sensor_cs_activelow),
						m_sensor_index(sensor_index)
						{
							m_type = Endpoint::TYPE_SENSOR_TEMPERATURE;
							m_unit_type = Unit::UNIT_16Q16_CELSIUS;
						}

					DS18B20* getSensor() {
						return m_sensor_temperature;
					}

					void setSensor(DS18B20* sensor_temperature) {
						m_sensor_temperature = sensor_temperature;
					}

					//sensor chip select pin, optional
					OutputPin* getSensorCS() {
						return m_sensor_cs;
					}

					void setSensorCS(OutputPin* sensor_cs) {
						m_sensor_cs = sensor_cs;
					}

					bool getSensorCSActiveLow() {
						return m_sensor_cs_activelow;
					}

					void setSensorCSActiveLow(bool sensor_cs_activelow) {
						m_sensor_cs_activelow = sensor_cs_activelow;
					}

					bool getSensorIndex() {
						return m_sensor_index;
					}

					void setSensorIndex(bool sensor_index) {
						m_sensor_index = sensor_index;
					}

					virtual void poll() {
						int32_t value = ((int32_t) INT_MIN) << 16;//default value for temp N/A
						if ( m_sensor_cs != NULL ) {
							m_sensor_cs->set(!m_sensor_cs_activelow);
							Watchdog::delay(16);//TODO check if the sensor needs less time
						}
						if ( m_sensor_temperature->connect(m_sensor_index) ) {
							//We set parasite = true only if separate CS not available
							//Default resolution = 12.4
							m_sensor_temperature->convert_request();
							m_sensor_temperature->read_scratchpad();
							int16_t temp = m_sensor_temperature->temperature();//Q12.4
							value = ( temp & 0xFFF0 ) << 12 | ( temp & 0x000F);//Q16.16
						}
						if ( m_sensor_cs != NULL )
							m_sensor_cs->set(m_sensor_cs_activelow);
						setState(value);
						if ( m_dirty ) {
							notify();
							m_dirty = false;
						}
					}

			};//end of Meshwork::L7::Endpoints::TemperatureDS18B20Sensor
		};//end of Meshwork::L7::Endpoints
	};//end of Meshwork::L7
};//end of Meshwork
#endif
