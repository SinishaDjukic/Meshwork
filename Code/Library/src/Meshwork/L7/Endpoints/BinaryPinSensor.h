
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
#ifndef __MESHWORK_L7_ENDPOINTS_BINARYPINSENSOR_H__
#define __MESHWORK_L7_ENDPOINTS_BINARYPINSENSOR_H__

#include "Cosa/Types.h"
#include "Cosa/InputPin.hh"

#include "Meshwork.h"
#include "Meshwork/L7/Endpoint.h"
#include "Meshwork/L7/Unit.h"

#include "Meshwork/L7/Endpoints/BinarySensor.h"

using namespace Meshwork::L7;

namespace Meshwork {

	namespace L7 {

		namespace Endpoints {

			//This flavor only handles polling
			class BinaryPinSensor: public BinarySensor {

				protected:
					InputPin* m_pin;

				public:
					BinaryPinSensor(EndpointListener* listener,
							endpoint_reporting_configuration_t* reporting_configuration,
							InputPin* pin):
						BinarySensor(listener, reporting_configuration, pin->is_set()),
						m_pin(pin)
						{}

					//In this Endpoint the state updated only during poll while reading the pin
					virtual void poll() {
						setState(m_pin->is_set() ? 255 : 0);
						if ( m_dirty ) {
							notify();
							m_dirty = false;
						}
					}

			};//end of Meshwork::L7::Endpoints::BinaryPinSensor
		};//end of Meshwork::L7::Endpoints
	};//end of Meshwork::L7
};//end of Meshwork
#endif
