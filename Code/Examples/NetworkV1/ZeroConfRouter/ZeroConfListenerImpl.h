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
#ifndef __EXAMPLES_CONSOLE_ZEROCONFLISTENERIMPL_H__
#define __EXAMPLES_CONSOLE_ZEROCONFLISTENERIMPL_H__

#include "Cosa/EEPROM.hh"
#include "Cosa/Trace.hh"
#include "Cosa/Wireless.hh"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Meshwork/L3/NetworkV1/ZeroConfSerial.h"
#include "NetworkInit.h"
#include "ZCTypes.h"
#include "Utils/EEPROMInit.h"

using namespace Meshwork::L3::NetworkV1;

class ZeroConfListenerImpl: public ZeroConfSerial::ZeroConfListener {

protected:
	EEPROM* m_eeprom;
	zctype_configuration_t* m_configuration;

public:
	ZeroConfListenerImpl(EEPROM* eeprom, zctype_configuration_t* configuration):
		m_eeprom(eeprom),
		m_configuration(configuration)
			{
			};
	
	void init() {
		EEPROMInit::init(m_eeprom, EXAMPLE_ZC_CONFIGURATION_EEPROM_OFFSET, EXAMPLE_ZC_CONFIGURATION_EEPROM_END, EXAMPLE_ZC_INIT_EEPROM_MARKER_VALUE);
	}
	
	void devconfig_updated() {
		m_eeprom->write((void*) &EXAMPLE_ZC_DEVCONFIG_EEPROM_OFFSET, (void*) &m_configuration->sernum, sizeof(m_configuration->sernum));
		MW_LOG_DEBUG(LOG_ZEROCONFROUTER, "Serial updated", NULL);
	}

	void serial_updated() {
		m_eeprom->write((void*) &EXAMPLE_ZC_SERNUM_EEPROM_OFFSET, (void*) &m_configuration->sernum, sizeof(m_configuration->sernum));
		MW_LOG_DEBUG(LOG_ZEROCONFROUTER, "Serial updated", NULL);
	}
	
	void network_updated() {
		m_eeprom->write((void*) &EXAMPLE_ZC_NWKCONFIG_EEPROM_OFFSET, (void*) &m_configuration->nwkconfig, sizeof(m_configuration->nwkconfig));
		MW_LOG_DEBUG(LOG_ZEROCONFROUTER, "Network updated", NULL);
	}
	
	void reporting_updated() {
		m_eeprom->write((void*) &EXAMPLE_ZC_REPORTING_EEPROM_OFFSET, (void*) &m_configuration->reporting, sizeof(m_configuration->reporting));
		MW_LOG_DEBUG(LOG_ZEROCONFROUTER, "Reporting updated", NULL);
	}

};
#endif

