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
#ifndef __EXAMPLES_ZCTYPES_H__
#define __EXAMPLES_ZCTYPES_H__

#include "Meshwork/L3/NetworkV1/ZeroConfSerial.h"

using namespace Meshwork::L3::NetworkV1;

struct zctype_configuration_t {
	ZeroConfSerial::zctype_sernum_t sernum;
	ZeroConfSerial::zctype_reporting_t reporting;
	ZeroConfSerial::zctype_nwkconfig_t nwkconfig;
	//dev config flags
	uint8_t m_nwkcaps;
	uint8_t m_delivery;
};

//Formatted EEPROM marker len
static uint8_t EXAMPLE_ZC_INIT_EEPROM_MARKER_LEN = 1;
//Formatted EEPROM marker value
static uint16_t EXAMPLE_ZC_INIT_EEPROM_MARKER_VALUE = 0x00;
//Offset for storing ZC device configuration in the EEPROM
static uint16_t EXAMPLE_ZC_CONFIGURATION_EEPROM_OFFSET = 64;
//Offset for storing the serial number structure
static uint16_t EXAMPLE_ZC_SERNUM_EEPROM_OFFSET = EXAMPLE_ZC_CONFIGURATION_EEPROM_OFFSET +
													EXAMPLE_ZC_INIT_EEPROM_MARKER_LEN;
//Offset for storing the reporting structure
static uint16_t EXAMPLE_ZC_REPORTING_EEPROM_OFFSET = EXAMPLE_ZC_SERNUM_EEPROM_OFFSET +
														sizeof(ZeroConfSerial::zctype_sernum_t);
//Offset for storing the network configuration structure
static uint16_t EXAMPLE_ZC_NWKCONFIG_EEPROM_OFFSET = EXAMPLE_ZC_REPORTING_EEPROM_OFFSET +
														sizeof(ZeroConfSerial::zctype_reporting_t);
//Offset for storing the devic configuration flags structure
static uint16_t EXAMPLE_ZC_DEVCONFIG_EEPROM_OFFSET = EXAMPLE_ZC_REPORTING_EEPROM_OFFSET +
														sizeof(ZeroConfSerial::zctype_nwkconfig_t);
//Final address might be useful for the app
static uint16_t EXAMPLE_ZC_CONFIGURATION_EEPROM_END = EXAMPLE_ZC_NWKCONFIG_EEPROM_OFFSET +
														sizeof(uint8_t) * 2;
					
#endif

