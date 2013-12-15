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
#ifndef __MESHWORK_MESH_MESHSERIAL_H__
#define __MESHWORK_MESH_MESHSERIAL_H__

#include "Cosa/Pins.hh"
#include "Cosa/Wireless.hh"
#include "Cosa/Types.h"
#include "Cosa/Power.hh"
#include "Cosa/IOStream/Driver/UART.hh"
#include "Mesh.h"

//SEQ | LEN | MSG
//MSG = MSGCODE | MSGDATA
//CFGBASIC = NKWCAPS | DELIVERY | RETRY
//CFGNWK = NWK ID | NODE ID
//RFRECV = SRC | PORT | DATALEN | DATA
//RFRECVACK = DATALEN | DATA
//RFSTARTRECV = TIMEOUT
//RFSEND = DST | PORT | DATALEN | DATA
//RFSENDACK = DATALEN | DATA
//RFBCAST = PORT | DATALEN | DATA
class MeshSerial : public Mesh::Network::ACKProvider {

public:
  struct serialmsg_t {
	uint8_t seq;
	uint8_t len;
	uint8_t code;
	uint8_t* data;
  };
  
  struct data_cfgbasic_t {
	uint8_t nwkcaps;
	uint8_t delivery;
	uint8_t retry;
  };
  
  struct data_cfgnwk_t {
	uint16_t nwkid;
	uint8_t nodeid;
  };
  
  struct data_rfrecv_t {
	uint8_t src;
	uint8_t port;
	uint8_t datalen;
	uint8_t* data;
  };
  
  struct data_rfrecvack_t {
	uint8_t datalen;
	uint8_t* data;
  };
  
  struct data_rfstartrecv_t {
	uint32_t timeout;
  };
  
  struct data_rfsend_t {
	uint8_t dst;
	uint8_t port;
	uint8_t datalen;
	uint8_t* data;
  };
  
  struct data_rfbcast_t {
	uint8_t port;
	uint8_t datalen;
	uint8_t* data;
  };  
  
public:
	static const uint8_t MAX_SERIALMSG_LEN 			= 64;//TODO calculate the right size!
	
	static const uint8_t MSGCODE_OK 				= 0;
	static const uint8_t MSGCODE_NOK 				= 1;
	static const uint8_t MSGCODE_UNKNOWN 			= 2;
	static const uint8_t MSGCODE_INTERNAL 			= 3;
	static const uint8_t MSGCODE_CFGBASIC 			= 10;
	static const uint8_t MSGCODE_CFGNWK 			= 11;
	static const uint8_t MSGCODE_RFINIT 			= 20;
	static const uint8_t MSGCODE_RFDEINIT 			= 21;
	static const uint8_t MSGCODE_RFRECV 			= 22;
	static const uint8_t MSGCODE_RFRECVACK 			= 23;
	static const uint8_t MSGCODE_RFSTARTRECV 		= 24;
	static const uint8_t MSGCODE_RFSEND 			= 25;
	static const uint8_t MSGCODE_RFSENDACK 			= 26;
	static const uint8_t MSGCODE_RFBCAST 			= 27;
	
	static const uint8_t ERROR_GENERAL 				= 0;
	static const uint8_t ERROR_INSUFFICIENT_DATA 	= 1;
	static const uint8_t ERROR_TOO_LONG_DATA 		= 2;
	static const uint8_t ERROR_ILLEGAL_STATE 		= 3;
	static const uint8_t ERROR_RECV 				= 4;
	static const uint8_t ERROR_SEND 				= 5;
	static const uint8_t ERROR_BCAST 				= 6;
	
	static const uint16_t TIMEOUT_RESPONSE 			= 3000;
	
protected:
	Mesh::Network* m_network;
	UART* m_serial;
	serialmsg_t* currentmsg;

	
	virtual void respondWCode(serialmsg_t* msg, uint8_t code);
	virtual void respondNOK(serialmsg_t* msg, uint8_t error);
	virtual void respondSendACK(serialmsg_t* msg, uint8_t datalen, uint8_t* data);
	
	virtual bool processCfgBasic(serialmsg_t* msg);
	virtual bool processCfgNwk(serialmsg_t* msg);
	virtual bool processRFInit(serialmsg_t* msg);
	virtual bool processRFDeinit(serialmsg_t* msg);

	virtual bool processRFStartRecv(serialmsg_t* msg);
	virtual bool processRFSend(serialmsg_t* msg);
	virtual bool processRFBroadcast(serialmsg_t* msg);

public:
	MeshSerial(Mesh::Network* network, UART* serial):
		m_network(network),
		m_serial(serial)
	{ }
	
	virtual bool processOneMessage(serialmsg_t* msg);
	
	virtual int returnACKPayload(uint8_t src, uint8_t port, void* buf, uint8_t len, void* bufACK, size_t lenACK);
};
#endif