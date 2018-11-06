
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
#ifndef __MESHWORK_L7_ENDPOINTS_BINARYSENSOR_H__
#define __MESHWORK_L7_ENDPOINTS_BINARYSENSOR_H__

#include "Cosa/Types.h"

#include "Meshwork.h"
#include "Meshwork/L7/Endpoint.h"
#include "Meshwork/L7/Unit.h"

using namespace Meshwork::L7;

namespace Meshwork {

	namespace L7 {

		namespace Endpoints {

			class BinarySensor: public Meshwork::L7::Endpoint {

				protected:
					bool m_state;
					bool m_dirty;

					virtual void notify() {
						if ( m_dirty && m_listener != NULL
//								&& report_discrete(m_reporting_configuration)
						) {
							m_listener->propertyChanged((Endpoint*) this);
						}
					}

				public:
					BinarySensor(EndpointListener* listener,
							endpoint_reporting_configuration_t* reporting_configuration,
							bool initial_state):
						Endpoint(Endpoint::TYPE_SENSOR_BINARY, Unit::UNIT_8D_BINARY, listener, reporting_configuration),
						m_state(initial_state),
						m_dirty(true)
						{}

					virtual void getProperty(endpoint_value_t* value) {
						uint8_t* val = (uint8_t*) value->pvalue;
						val[0] = m_state ? 255 : 0;
						value->len = 1;
					}

					virtual void setProperty(const endpoint_value_t* value, endpoint_set_status_t* status) {
						status->status = Endpoint::STATUS_SET_INVALID;
						status->len = 0;
					}

					bool getState() const {
						return m_state;
					}

					virtual void poll() {
						if ( m_dirty ) {
							notify();
							m_dirty = false;
						}
					}

					void setState(bool state) {
						m_state = state;
						m_dirty = report_all(m_reporting_configuration) || state != m_state && report_discrete(m_reporting_configuration);
					}

			};//end of Meshwork::L7::Endpoints::BinarySensor
		};//end of Meshwork::L7::Endpoints
	};//end of Meshwork::L7
};//end of Meshwork
#endif
