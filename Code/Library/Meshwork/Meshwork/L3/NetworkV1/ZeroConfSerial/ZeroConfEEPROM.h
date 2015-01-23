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
#ifndef __EXAMPLES_CONSOLE_ZEROCONFEEPROM_H__
#define __EXAMPLES_CONSOLE_ZEROCONFEEPROM_H__

#include "Cosa/EEPROM.hh"
#include "Cosa/Trace.hh"
#include "Cosa/Wireless.hh"
#include "Meshwork/L3/NetworkV1/NetworkV1.h"
#include "Meshwork/L3/NetworkV1/ZeroConfSerial/ZeroConfSerial.h"
#include "Utils/EEPROMUtils.h"

using namespace Meshwork::L3::NetworkV1;

#ifndef LOG_ZEROCONFEEPROM
#define LOG_ZEROCONFEEPROM true
#endif

class ZeroConfEEPROM: public ZeroConfSerial::ZeroConfListener {
	
	public:
		struct zctype_configuration_t {
			ZeroConfSerial::zctype_sernum_t sernum;
			ZeroConfSerial::zctype_reporting_t reporting;
			ZeroConfSerial::zctype_nwkconfig_t nwkconfig;
			ZeroConfSerial::zctype_devconfig_t devconfig;
		};

		//Formatted EEPROM marker len
		static const uint8_t ZC_INIT_EEPROM_MARKER_LEN 		= 1;
		//Formatted EEPROM marker value
		static const uint16_t ZC_INIT_EEPROM_MARKER_VALUE 	= 0x02;
		//Formatted EEPROM default value
		static const uint16_t ZC_INIT_EEPROM_MEM_VALUE 		= 0x00;
		//Offset for storing the serial number structure
		static const uint16_t ZC_SERNUM_EEPROM_OFFSET		= ZC_INIT_EEPROM_MARKER_LEN;
		//This is also our firts meaningful data byte, which corresponds to the zctype_configuration structure
		static const uint16_t ZC_STRUCT_DATA_START			= ZC_SERNUM_EEPROM_OFFSET;
		//Offset for storing the reporting structure
		static const uint16_t ZC_REPORTING_EEPROM_OFFSET 	= ZC_STRUCT_DATA_START +
																sizeof(ZeroConfSerial::zctype_sernum_t);
		//Offset for storing the network configuration structure
		static const uint16_t ZC_NWKCONFIG_EEPROM_OFFSET 	= ZC_REPORTING_EEPROM_OFFSET +
																sizeof(ZeroConfSerial::zctype_reporting_t);
		//Offset for storing the devic configuration flags structure
		static const uint16_t ZC_DEVCONFIG_EEPROM_OFFSET 	= ZC_NWKCONFIG_EEPROM_OFFSET +
																sizeof(ZeroConfSerial::zctype_nwkconfig_t);
		//Final address might be useful for the app
		static const uint16_t ZC_CONFIGURATION_EEPROM_END 	= ZC_DEVCONFIG_EEPROM_OFFSET +
																sizeof(ZeroConfSerial::zctype_devconfig_t);

	protected:
		EEPROM* m_eeprom;
		zctype_configuration_t* m_configuration;
		uint16_t m_eepromStartOffset;
	
	public:
		ZeroConfEEPROM(EEPROM* eeprom, zctype_configuration_t* configuration, uint16_t eepromStartOffset):
			m_eeprom(eeprom),
			m_configuration(configuration),
			m_eepromStartOffset(eepromStartOffset)
				{
				};

		void init() {
			EEPROMUtils::init(m_eeprom, m_eepromStartOffset, m_eepromStartOffset + ZC_CONFIGURATION_EEPROM_END, ZC_INIT_EEPROM_MARKER_VALUE, ZC_INIT_EEPROM_MEM_VALUE);
		}

		void read_configuration() {
			m_eeprom->read((void*) m_configuration, (uint8_t*) (m_eepromStartOffset + ZC_STRUCT_DATA_START), (size_t) sizeof(zctype_configuration_t));
			MW_LOG_DEBUG(LOG_ZEROCONFEEPROM, "Full configuration read. Start=%d, Count=%d", ((uint8_t*) (m_eepromStartOffset + ZC_STRUCT_DATA_START)), (size_t) sizeof(zctype_configuration_t));
		}

		void devconfig_updated() {
			m_eeprom->write((uint8_t*) (m_eepromStartOffset + ZC_DEVCONFIG_EEPROM_OFFSET), (void*) &m_configuration->devconfig, sizeof(m_configuration->devconfig));
			MW_LOG_DEBUG(LOG_ZEROCONFEEPROM, "Device config updated", NULL);
		}
	
		void serial_updated() {
			m_eeprom->write((uint8_t*) (m_eepromStartOffset + ZC_SERNUM_EEPROM_OFFSET), (void*) &m_configuration->sernum, sizeof(m_configuration->sernum));
			MW_LOG_DEBUG(LOG_ZEROCONFEEPROM, "Serial updated", NULL);
		}

		void network_updated() {
			m_eeprom->write((uint8_t*) (m_eepromStartOffset + ZC_NWKCONFIG_EEPROM_OFFSET), (void*) &m_configuration->nwkconfig, sizeof(m_configuration->nwkconfig));
			MW_LOG_DEBUG(LOG_ZEROCONFEEPROM, "Network updated", NULL);
		}

		void reporting_updated() {
			m_eeprom->write((uint8_t*) (m_eepromStartOffset + ZC_REPORTING_EEPROM_OFFSET), (void*) &m_configuration->reporting, sizeof(m_configuration->reporting));
			MW_LOG_DEBUG(LOG_ZEROCONFEEPROM, "Reporting updated", NULL);
		}
	
		void factory_reset() {
			EEPROMUtils::format(m_eeprom, m_eepromStartOffset, m_eepromStartOffset + ZC_CONFIGURATION_EEPROM_END, ZC_INIT_EEPROM_MARKER_VALUE, ZC_INIT_EEPROM_MEM_VALUE);
			MW_LOG_DEBUG(LOG_ZEROCONFEEPROM, "Factory reset. EEPROM flashed to: %d", ZC_INIT_EEPROM_MEM_VALUE);
		}
};
#endif

