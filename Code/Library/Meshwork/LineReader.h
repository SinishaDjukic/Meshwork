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

#ifndef __MESHWORK_LINEREADER_H__
#define __MESHWORK_LINEREADER_H__

#include "Cosa/Types.h"
#include "Cosa/IOStream.hh"
#include "Cosa/Power.hh"

/**
 * Line reader template class, typically used for interactive console
 * access. May be used
 * to read text lines one by one. Buffer size should be power of 2 and
 * max 128. 
 * Usage pattern:
 * uint_t len = linereader.readline();
 * if ( len > 0 ) {
 *     char* line = linereader.get_line();
 *     //do something with line and len, e.g. parse command
 *     linereader.clear();
 * 	} else if ( len == -1 ) {
 *		//overflow
 * 		linereader.clear();
 * 	}
 *
 * @param[in] SIZE number of bytes in buffer, max 128
 */
template <uint8_t SIZE>
class LineReader {
private:
  IOStream::Device* m_device;
  char m_buffer[SIZE];
  int8_t m_linesize;
  int8_t m_lineready;
  int8_t m_nextchar;

public:
  /**
   * Construct line reader given an IO device
   */
  LineReader(IOStream::Device* device) :
	m_device(device),
	m_linesize(SIZE),
    m_lineready(0),
    m_nextchar(0)
  {
  }

  /**
   * Returns the line bytes available or error status
   * @return int8_t number of bytes in the line if available, 0 if not available, and -1 if line overflowed
   */
  int8_t is_lineready()
  {
    return m_lineready;
  }

  /**
   * Return true(1) if the buffer is full, otherwise false(0).
   * @return bool buffer full status
   */
  bool is_full()
  {
    return m_nextchar == SIZE;
  }

  void clear() {
	  for ( int i = 0; i < m_nextchar; i ++ )
		  m_buffer[i] = 0;
	  m_nextchar = 0;
	  m_lineready = 0;
  }

  int8_t readline() {
      char tmp = 0;
      while( m_device->available() && m_nextchar <= SIZE) {
          tmp = m_device->getchar();
		  //should not happen, unless available() is buggy
          if ( tmp == -1 )
        	  break;
          if ( tmp == 10 || tmp == 13 ) {//LF, CR
			  //only if we have some command ready
        	  m_lineready = m_nextchar > 0 ? m_nextchar : 0;
              break;
          } else {
        	  m_buffer[m_nextchar++] = tmp;
          }
      }
      if ( m_nextchar == SIZE + 1 ) {
	      //too long...
    	  m_lineready = -1;
      }
      return m_lineready;
  }

  char* get_line() {
	  return m_buffer;
  }

};

#endif
