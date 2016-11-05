
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
#include "Cosa/PinChangeInterrupt.hh"

#include "Meshwork.h"
#include "Meshwork/L7/Endpoint.h"
#include "Meshwork/L7/Unit.h"

#include "Meshwork/L7/Endpoints/BinarySensor.h"

using namespace Meshwork::L7;

namespace Meshwork {

	namespace L7 {

		namespace Endpoints {

			//This flavor only handles polling
			class BinaryPinChangeSensor: public Endpoints::BinarySensor, public PinChangeInterrupt {

				protected:
					InputPin* m_pin;
					bool m_last;

				public:
					BinaryPinChangeSensor(EndpointListener* listener,
							endpoint_reporting_configuration_t* reporting_configuration,
							InputPin* pin, InterruptMode mode = ON_CHANGE_MODE, bool pullup = true):
						BinarySensor(listener, reporting_configuration, pin->is_set()),
						PinChangeInterrupt((Board::InterruptPin) pin->pin(), mode, pullup),
						m_pin(pin),
						m_last(m_state)
						{
						}

						void on_interrupt(uint16_t arg) {
							if ( m_listener != NULL )
								m_listener->wakeup(this);
							//check if change report is already pending
							if ( !m_dirty )
								setState(m_pin->is_set());
						}

						//Tricky stuff: in case of a quick on/off switch we can't simply send
						//the latest state. We guarg against this by checking m_dirty in the
						//ISR, but here we need to check again if the latest state differs
						virtual void poll() {
							if ( m_dirty ) {
								notify();
								bool state = m_pin->is_set();
								if ( state != m_state ) {
									setState(state);
									notify();
								}
								m_dirty = false;
							}
						}

						void setInterrupts(bool state) {
							if ( state ) {
								enable();
							} else {
								disable();
							}
						}
			};//end of Meshwork::L7::Endpoints::BinaryPinChangeSensor
		};//end of Meshwork::L7::Endpoints
	};//end of Meshwork::L7
};//end of Meshwork
#endif
