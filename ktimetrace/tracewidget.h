/***************************************************************************
                          tracewidget.h  -  description
                             -------------------
TraceWidget is a plotting widget used by KTraceView.  Each channel gets its
own TraceWidget.

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

#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

class TraceWidget;

#include <qwidget.h>
#include <qpixmap.h>
#include <qcolor.h>

#include <comedilib.h>

class TraceWidget : public QWidget
{
Q_OBJECT
public:
	TraceWidget(int ch, QWidget *parent = 0, const char *name = 0);
	~TraceWidget();
	/* plot a point in this widget (or not) */
	inline void input(sampl_t);
	void plot();
	/* ready widget for next run */
	void reinit();
	/* set/return full display scale */
	unsigned int fullScale(unsigned int scale);
	/* plot data as it comes point by point or wait until a full screen of data has accumulated */
	bool blockMode(bool);
	// set /reset horizontal zoom
	float hZoom(float zoom = -1.0);
	/* plot out any remaining dregs */
	void flush();
	// returns number of points we can display as function of widget width and zoom
	static unsigned int getDisplayWidth(unsigned int width, float zoom);
	// set plotting color
	void setPlottingColor(QColor color);
protected:
	/* does actual painting of widget */
	virtual void paintEvent(QPaintEvent *event);
	/* adjusts plotWidth accordingly */
	virtual void resizeEvent(QResizeEvent *event);
private:
	// utility function that gives x coordinate for plotting
	inline int indexToX(long long index);
	/* utility function that gives index whose x coordinate is nearest
	 * and <= to the given argument */
	inline long long XToIndex(int x);
	/* the channel that this widget is plotting */
	int channel;
	/* buffer of data that has/will be plotted */
	static const int plotArrayLength = 32000;
	sampl_t plotArray[plotArrayLength];
	// number of pixels to leave blank after 'update bar'
	static const int blankSpace = 15;
	/* keeps track of how many points have been received for plotting */
	long long current;
	// index of last point actually plotted
	long long lastPlotted;
	/* index of data point to start plot with on left side of display */
	long long left;
	/* data value that corresponds to full display scale */
	unsigned int fScale;
	/* plot data as it comes point by point or wait until a full screen of data has accumulated */
	bool plotByBlock;
	// horizontal zoom (pixels per data point)
	float horizontalZoom;
	// plot width in number of data points, determined from horizontalZoom and width()
	unsigned int plotWidth;
	// pixmap used to plot with
	QPixmap pix;
	// color used for plotting
	QColor plotColor;
};


// inline functions

inline void TraceWidget::input(sampl_t point)
{
	plotArray[current++ % plotArrayLength] = point;
}

inline int TraceWidget::indexToX(long long index)
{
	int ret = (int)(((plotWidth + index - left) % plotWidth) * horizontalZoom);
	return ret;
}

inline long long TraceWidget::XToIndex(int x)
{
	long long ret = left + (int)(x / horizontalZoom);
	if(ret >= current)
		ret -= plotWidth;
	return ret;
}
#endif
