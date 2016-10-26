
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
#ifndef __MESHWORK_L7_ENDPOINTS_MULTILEVELSWITCH_H__
#define __MESHWORK_L7_ENDPOINTS_MULTILEVELSWITCH_H__

#include "Cosa/Types.h"

#include "Meshwork.h"
#include "Meshwork/L7/Endpoint.h"
#include "Meshwork/L7/Unit.h"

using namespace Meshwork::L7;

namespace Meshwork {

	namespace L7 {

		namespace Endpoints {

			class MultilevelSwitch: public Meshwork::L7::Endpoint {

				protected:
					uint8_t m_state;
					uint8_t m_target_state;
					uint8_t m_lastreported_state;

					virtual void setStateImpl(uint8_t state, uint16_t millis) = 0;

				public:
					MultilevelSwitch(EndpointListener* listener,
							endpoint_reporting_configuration_t* reporting_configuration,
							uint8_t initial_state):
						Endpoint(Endpoint::TYPE_SWITCH_MULTILEVEL, Unit::UNIT_PERCENTAGE_BYTE, listener, reporting_configuration),
						m_state(initial_state),
						m_target_state(initial_state),
						m_lastreported_state(initial_state)
						{}

					void getProperty(endpoint_value_t* value) {
						uint8_t* val = (uint8_t*) value->pvalue;
						val[0] = m_state;
						val[1] = m_target_state;
						value->len = 2;
					}

					void setProperty(const endpoint_value_t* value, endpoint_set_status_t* status) {
						uint8_t* val = (uint8_t*) value->pvalue;
						uint16_t millis = val[1] << 8 || val[2];

						setState(val[0], millis);

						status->status = Endpoint::STATUS_SET_PROCESSED;
						status->len = 0;
					}

					uint8_t getState() const {
						return m_state;
					}

					uint8_t getTargetState() const {
						return m_target_state;
					}

					void setState(uint8_t state, uint16_t millis) {
						if ( state != m_target_state ) {
							//target state changes immediately
							m_target_state = state;

							//sub-class defines when m_state changes
							setStateImpl(state, millis);

							m_state = state;

							//notify the listener
							if ( m_listener != NULL &&
									report_threshold(m_reporting_configuration, calculate_threshold_u8(m_lastreported_state, m_state)) ) {
								endpoint_value_t value;
								uint8_t val[2];
								value.pvalue = &val;
								value.len = 2;
								getProperty(&value);
								m_listener->propertyChanged((Endpoint*) this, (const endpoint_value_t*) &value);
								m_lastreported_state = m_state;
							}
						}
					}

			};//end of Meshwork::L7::Endpoints::MultilevelSwitch
		};//end of Meshwork::L7::Endpoints
	};//end of Meshwork::L7
};//end of Meshwork
#endif
