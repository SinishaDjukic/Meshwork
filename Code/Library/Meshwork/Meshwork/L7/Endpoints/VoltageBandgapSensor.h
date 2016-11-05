
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
#ifndef __MESHWORK_L7_ENDPOINTS_VOLTAGEPERCENTAGESENSOR_H__
#define __MESHWORK_L7_ENDPOINTS_VOLTAGEPERCENTAGESENSOR_H__

#include "Cosa/Trace.hh"
#include "Cosa/Types.h"

#include "Meshwork.h"
#include "Meshwork/L7/Endpoint.h"
#include "Meshwork/L7/Unit.h"

#include "Meshwork/L7/Endpoints/MultilevelSensor.h"

using namespace Meshwork::L7;

namespace Meshwork {

	namespace L7 {

		namespace Endpoints {

			class VoltageBandgapSensor: public Meshwork::L7::Endpoint {

				protected:
					uint32_t m_state;
					uint32_t m_lastreported_state;
					int16_t m_min_value;
					int16_t m_max_value;
					bool m_dirty;

					void notify() {
						  //notify the listener
						  if ( m_listener != NULL
//								  && report_threshold(m_reporting_configuration, calculate_threshold_u32(m_lastreported_state, m_state))
						  ) {
							m_listener->propertyChanged((Endpoint*) this);
							m_lastreported_state = m_state;
						  }
					}

					//TODO move to utils
					static inline long map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
					  return x <= in_min ? out_min :
							  	  ( x >= in_max ? out_max :
									  (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
					}

				public:
					VoltageBandgapSensor(EndpointListener* listener,
							endpoint_reporting_configuration_t* reporting_configuration,
							uint16_t unit = Unit::UNIT_24D8_VOLT,
							int16_t min_value = 0, int16_t max_value = 0):
								//min_value and max_value are in 24D8, but used only for UNIT_8D_PERCENTAGE
						Endpoint(Endpoint::TYPE_SENSOR_VOLTAGE, unit, listener, reporting_configuration),
						m_state(INT_MIN),
						m_lastreported_state(INT_MIN),
						m_min_value(min_value),
						m_max_value(max_value),
						m_dirty(true)
						{
							ASSERT(unit == Unit::UNIT_8D_PERCENTAGE || unit == Unit::UNIT_24D8_VOLT);
						}

					virtual void getProperty(endpoint_value_t* value) {
						uint32_t* val = (uint32_t*) value->pvalue;
						*val = m_state;
						value->len = sizeof(m_state);
					}

					virtual void setProperty(const endpoint_value_t* value, endpoint_set_status_t* status) {
						status->status = Endpoint::STATUS_SET_INVALID;
						status->len = 0;
					}

					uint8_t getState() const {
						return m_state;
					}

					void setState(uint32_t state) {
					  m_state = state;
					  m_dirty = report_all(m_reporting_configuration) || state != m_state && report_discrete(m_reporting_configuration);
					}

					virtual void poll() {
						uint32_t value;
						uint8_t adc_old = bit_get(ADCSRA, ADEN);
						if ( !adc_old )
							Power::adc_enable();
						value = AnalogPin::bandgap(1100);
						if ( !adc_old )
							Power::adc_disable();
						uint16_t unit = getUnitType();
						if ( unit == Unit::UNIT_8D_PERCENTAGE ) {
							value = m_min_value == m_max_value ? 0 : map(value, m_min_value, m_max_value, 0, 100);
						} else {//Unit::UNIT_24D8_VOLT
							//Bangdap returns a mV value, so set 10^-3 precision
							value = value << 8;
							((int8_t*) &value)[0] = -3;//LSB
						}
						setState(value);
						if ( m_dirty ) {
							notify();
							m_dirty = false;
						}
					}

			};//end of Meshwork::L7::Endpoints::VoltagePercentageSensor
		};//end of Meshwork::L7::Endpoints
	};//end of Meshwork::L7
};//end of Meshwork
#endif
