/***************************************************************************
                          resource.h  -  description
                             -------------------
    begin                : Tue May 23 2000
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
#ifndef RESOURCE_H
#define RESOURCE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static const unsigned int maxChannels = 8;

#define ID_START 0
#define ID_STOP 10

#define ID_DEVICE_MENU 1000
#define ID_HELP_MENU 1010
#define ID_VIEW_MENU 1020

#define ID_COMEDI_0 2010
#define ID_COMEDI_1 2020
#define ID_COMEDI_2 2030
#define ID_COMEDI_3 2040

#define ID_QUIT 2050

#endif	//RESOURCE_H

