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
#ifndef __TEST_BASERF_BASECOMMANDS_UTILS_H__
#define __TEST_BASERF_BASECOMMANDS_UTILS_H__

//#include <Nucleo/Nucleo.h>
//#include <Nucleo/Thread.cpp>
//
//class Executor: public Nucleo::Thread {
//protected:
//	void (*m_func)(void);
//public:
//	Executor(void (*func)(void)) :
//		Thread(),
//		m_func(func) {}
//	virtual void run() {
//		m_func();
//	}
//};

#include <Meshwork/L7/BaseRFApplication.h>

class ReportListener: public BaseRFApplication::BaseRFReportListener {
protected:
	bool (*m_func)(univmsg_l7_any_t*);
public:
	ReportListener(bool (*func)(univmsg_l7_any_t*)) :
		m_func(func) {}
	bool notify_report(univmsg_l7_any_t* msg) {
		return m_func(msg);
	}
};


#endif
