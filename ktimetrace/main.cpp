//  This file is part of ktimetracer.

//  ktimetracer is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  ktimetracer is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with ktimetracer.  If not, see <https://www.gnu.org/licenses/>.

//  (C) 2001 by Frank Mori Hess <fmhess@uiuc.edu>
//  (C) 2018 by Helio Chissini de Castro <helio@kde.org># This file is part of ktimetracer.



#include <new>
#include <iostream>
#include <cstdlib>
#include <pthread.h>

#include <QApplication>

#include "ktimetrace.h"

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

	QApplication app(argc, argv);
  
	KTraceApp ktimetrace;
	ktimetrace.show();

	return app.exec();
}
