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
#include <stdio.h>
#include <stdlib.h>
#include "Cosa/Trace.hh"
#include "Cosa/Types.h"
#include "Cosa/IOStream.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/RTC.hh"
#include "Cosa/Wireless.hh"
#include <Meshwork.h>
#include "Utils/BoolSet.h"

void setup()
{  
  uart.begin(9600);
  trace.begin(&uart, PSTR("BoolSetTest: started"));
  Watchdog::begin();
  RTC::begin();  
}

void loop()
{
	static int const MAX = 10;
	BoolSet<MAX> bs(false);
	bs.set(0, true);
	bs.set(1, true);
	bs.set(7, true);
	bs.set(8, true);
	bs.set(9, true);
	for ( int i = 0; i < MAX; i ++ )
		trace << bs.get(i) << PSTR("\n");
}
