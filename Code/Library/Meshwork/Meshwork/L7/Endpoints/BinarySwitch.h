
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
#ifndef __MESHWORK_L7_ENDPOINTS_BINARYSWITCH_H__
#define __MESHWORK_L7_ENDPOINTS_BINARYSWITCH_H__

#include "Cosa/Types.h"

#include "Meshwork.h"
#include "Meshwork/L7/Endpoint.h"
#include "Meshwork/L7/Unit.h"

using namespace Meshwork::L7;

namespace Meshwork {

	namespace L7 {

		class BinarySwitch: public Meshwork::L7::Endpoint {

			protected:
				bool m_state;
				bool m_target_state;

				virtual void setStateImpl(bool state, uint16_t millis) {
					m_state = state;
				}

			public:
				BinarySwitch(EndpointListener* listener,
						endpoint_reporting_configuration_t* reporting_configuration,
						bool initial_state):
					Endpoint(Endpoint::TYPE_SWITCH_BINARY, Unit::UNIT_BINARY_BYTE, listener, reporting_configuration),
					m_state(initial_state),
					m_target_state(initial_state)
					{}

				void getProperty(endpoint_value_t &value) {
					uint8_t* val = (uint8_t*) value.pvalue;
					val[0] = m_state ? 255 : 0;
					val[1] = m_target_state ? 255 : 0;
					value.len = 2;
				}

				void setProperty(const endpoint_value_t* value, endpoint_set_status_t* status) {
					uint8_t* val = (uint8_t*) value->pvalue;
					uint16_t millis = val[1] << 8 || val[2];

					setState(val[0] != 0, millis);

					status->status = Endpoint::STATUS_SET_PROCESSED;
					status->len = 0;
				}

				bool getState() const {
					return m_state;
				}

				bool getTargetState() const {
					return m_target_state;
				}

				void setState(bool state, uint16_t millis) {
					if ( state != m_target_state ) {
						//target state changes immediately
						m_target_state = state;

						//sub-class defines when m_state changes
						setStateImpl(state, millis);

						m_state = state;

						//notify the listener
						if ( m_listener != NULL && report_discrete(m_reporting_configuration) ) {
							endpoint_value_t value;
							uint8_t val[2];
							value.pvalue = &val;
							value.len = 2;
							getProperty(value);
							m_listener->propertyChanged((Endpoint*) this, (const endpoint_value_t*) &value);
						}
					}
				}

		};//end of Meshwork::L7::BinarySwitch

	};//end of Meshwork::L7

};//end of Meshwork
#endif
