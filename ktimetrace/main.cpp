/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Tue May 23 17:09:23 CDT 2000
    copyright            : (C) 2000 by Frank Mori Hess
    email                : fmhess@uiuc.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ktimetrace.h"

#include <new>
#include <iostream>
#include <stdlib.h>
#include <kapp.h>
#include <pthread.h>

#include "config.h"

volatile unsigned int aquisitionThreadCount = 0;
pthread_mutex_t aquisitionThreadCountLock = PTHREAD_MUTEX_INITIALIZER;

void customNewHandler()
{
	std::cerr << "Virtual memory exhausted." << std::endl;
	abort();
}

int main(int argc, char *argv[])
{
	std::set_new_handler(customNewHandler);

	KApplication a(argc, argv, PACKAGE);
  
	KTraceApp *ktimetrace = new KTraceApp();
	a.setMainWidget(ktimetrace);
	ktimetrace->show();

	return a.exec();
}
