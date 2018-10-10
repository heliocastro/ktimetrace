/***************************************************************************
                          ktimetrace.h  -  description
                             -------------------

KTraceApp is the top-level class for the program.  Everything starts from here.

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

#ifndef KTRACEAPP_H
#define KTRACEAPP_H


#include <kmainwindow.h>

#include "data.h"
#include "resource.h"

class KTraceApp : public KMainWindow
{
Q_OBJECT
public:
	KTraceApp(QWidget *parent = 0, const char *name = 0);
	~KTraceApp();
	/* creates a caption for the main window and displays it */
	void defaultCaption();
	/* our daq card object - this shouldn't need to be public! */
	adc dataCard;
	/* turns various gui controls on and off according to whether or not
	 * we are currently aquiring data*/
	void setControlsEnabled(bool go);
protected:
	virtual bool queryClose();
	virtual bool queryExit();
private:
	/* go! */
	void startTrace();
	/* stop! */
	void stopTrace();
	/* sets the /dev/comediX device we want to use */
	int setDevice(unsigned int);
	// save settings to confuration file
	void saveConfig();

	/* control menu on menubar */
	QPopupMenu *controlMenu;
	/* settings menu on menubar */
	QPopupMenu *settingsMenu;
	/* device submenu in settings menu */
	QPopupMenu *deviceMenu;
	/* view menu on menubar */
	QPopupMenu *viewMenu;
	/* help menu on menubar */
	QPopupMenu *myHelpMenu;
	// toolbar
	KToolBar *myToolBar;
	/* the main graphical display widget */
	KTraceView *view;
	/* our poorly thought-out catch-all class*/
	KTTEngine data;
	/* used to periodically update the status led */
	QTimer *statusTimer;
	/* total number of possible /dev/comediX files */
	static const unsigned int maxDev = 4;
private slots:
	void commandCallback(int id);
	// handles 'comedi buffer' item on settings menu
	void slotBuffer();
	// handles 'zoom' item on view menu
	void slotZoom();
	// handles 'colors' item on view menu
	void slotColors();
	// handles 'about' item on help menu
	void slotAbout();
	// updates LED indicators
	void slotUpdateLED();
};

#endif
