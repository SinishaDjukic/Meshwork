
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
#ifndef __MESHWORK_L7_ENDPOINTS_TEMPERATURESENSOR_H__
#define __MESHWORK_L7_ENDPOINTS_TEMPERATURESENSOR_H__

#include "Cosa/Types.h"

#include "Meshwork.h"
#include "Meshwork/L7/Endpoint.h"
#include "Meshwork/L7/Unit.h"

using namespace Meshwork::L7;

namespace Meshwork {

	namespace L7 {

		namespace Endpoints {

			class TemperatureSensor: public Meshwork::L7::Endpoint {

				protected:
					//Values are always in signed Q16.16 fixed point format
					//https://en.wikipedia.org/wiki/Q_(number_format)
					int32_t m_state;
					int32_t m_lastreported_state;
					bool m_dirty;

					void notify() {
						//notify the listener
						if ( m_listener != NULL
//								&& report_threshold(m_reporting_configuration, calculate_threshold_u32(m_lastreported_state, m_state))
						) {
							endpoint_value_t value;
							uint8_t val[sizeof(m_state)];
							value.pvalue = &val[0];
							value.len = membersof(val);
							getProperty(&value);
							m_listener->propertyChanged((Endpoint*) this);
							m_lastreported_state = m_state;
						}
					}

				public:
					TemperatureSensor(EndpointListener* listener,
							endpoint_reporting_configuration_t* reporting_configuration,
							int32_t initial_state):
						Endpoint(Endpoint::TYPE_SENSOR_TEMPERATURE, Unit::UNIT_16Q16_KELVIN, listener, reporting_configuration),
						m_state(initial_state),
						m_lastreported_state(initial_state),
						m_dirty(true)
						{}

					virtual void getProperty(endpoint_value_t* value) {
						int32_t* val = (int32_t*) value->pvalue;
						val[0] = m_state;
						value->len = sizeof(m_state);
					}

					virtual void setProperty(const endpoint_value_t* value, endpoint_set_status_t* status) {
						status->status = Endpoint::STATUS_SET_INVALID;
						status->len = 0;
					}

					uint8_t getState() const {
						return m_state;
					}

					//This should be called directly only if you want to avoid subclassing and writing custom logic
					//E.g. useful when you have other means of reading the current value and want to use this only
					//as a data store
					void setState(int32_t state) {
						//set the new value
						m_state = state;
						m_dirty = report_all(m_reporting_configuration) || state != m_state && report_discrete(m_reporting_configuration);
					}
          
					virtual void poll() {
						if ( m_dirty ) {
							notify();
							m_dirty = false;
						}
					}

			};//end of Meshwork::L7::Endpoints::TemperatureSensor
		};//end of Meshwork::L7::Endpoints
	};//end of Meshwork::L7
};//end of Meshwork
#endif
