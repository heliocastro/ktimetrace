/***************************************************************************
                          view.h  -  description
                             -------------------

KTraceView is the main class in charge of plotting data


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

#ifndef VIEW_H
#define VIEW_H

#include <kled.h>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QProgressBar;
class QPushButton;
class QLabel;
class QColor;
QT_END_NAMESPACE

#include "data.h"
#include "tracewidget.h"
#include "resource.h"

class KTraceView : public QWidget
{
Q_OBJECT
public:
	KTraceView(QWidget *parent = 0, const char *name = 0);
	~KTraceView();
	/* creates up to 8 windows (TraceWidgets) for plotting data in */	
	void createWindows(unsigned int n);
	/* inputs data to be plotted */
	inline void input(sampl_t data, unsigned int channel);
	/* sets/returns scale. It's the data value that corresponds to card's
	 * full scale (4095 for 12 bit card) */	
	unsigned int fullScale(unsigned int scale);
	// save configuration to config file	
	void saveConfig(KConfig *config);
	// load configuration from config file	
	void loadConfig(KConfig *config);
	/* plot by screen size blocks or continuously */
	bool blockMode(bool);
	/* set progress of QProgressBar*/
	void setProgress(int progress);
	/* set status led based on number of file writes in progress */
	void setWriteIndicator(unsigned int numWrites);
	/* set status led based status of data card */
	void setDeviceIndicator(bool good);
	/* plot out any dregs remaining in buffer */
	void flush();
	/* set / return horizontal zoom (pixels per data point) */
	float hZoom(float zoom = -1.0);
	// set or query foreground color for plotting
	QColor fgColor(QColor color = QColor());
	// set or query background color for plotting
	QColor bgColor(QColor color = QColor());
	/* updates plots of children */
	void updatePlots();
protected:
	/* called when the widget is resized */
	virtual void resizeEvent(QResizeEvent *event);
private:
	// updates indicator with number of samples that will fit on display
	void updateDisplayWidth();
	/* widgets for plotting - one for each channel displayed */
	TraceWidget *trace[maxChannels];
	// width in pixels of trace[] widgets
	unsigned int traceWindowWidth;
	/* number of TraceWidgets we have */
	unsigned int numWindows;
	// widget that helps with layout
	QWidget *lowerPanel;
	/* progress bar that appears at bottom of window */
	QProgressBar *progressBar;
	/* indicates number of file writes completing in background.
	 * green == none, yellow == one, red == two or more */
	KLed *writeStatusIndicator;
	/* indicates if we have a working data aquisition device open */
	KLed *deviceStatusIndicator;
	/* zoom in/out buttons */
	QPushButton *zoomInButton;
	QPushButton *zoomOutButton;
	// displays current zoom
	QLabel *zoomDisplay;
	/* stores full scale */
	unsigned int fScale;
	/* plot by screen size blocks or continuously */
	bool bMode;
	/* horizontal zoom in pixels per data point */
	float horizontalZoom;
	// foreground color used by TraceWidgets
	QColor fgCol;
	// background color used by TraceWidgets
	QColor bgCol;
private slots:
	// slots for zoom in/out buttons
	void slotHZoomIn();
	void slotHZoomOut();
};

inline void KTraceView::input(sampl_t data, unsigned int channel)
{
	if(channel >= numWindows)
		return;
	trace[channel]->input(data);
	return;
}

#endif
