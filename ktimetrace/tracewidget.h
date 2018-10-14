//  This file is part of ktimetrace.

//  ktimetrace is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  ktimetrace is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with ktimetrace.  If not, see <https://www.gnu.org/licenses/>.

//  (C) 2001 by Frank Mori Hess <fmhess@uiuc.edu>
//  (C) 2018 by Helio Chissini de Castro <helio@kde.org>

#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include <QWidget>
#include <QColor>

#include <comedilib.h>

class QPixmap;

class TraceWidget : public QWidget
{
Q_OBJECT
public:
	TraceWidget(int ch, QWidget *parent = 0);
	~TraceWidget();
	/* plot a point in this widget (or not) */
	inline void input(sampl_t point) {
		plotArray[current++ % plotArrayLength] = point;
	}
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
	inline int indexToX(long long index) {
		int ret = (int)(((plotWidth + index - left) % plotWidth) * horizontalZoom);
		return ret;
	}
	/* utility function that gives index whose x coordinate is nearest
	 * and <= to the given argument */
	inline long long XToIndex(int x) {
		long long ret = left + (int)(x / horizontalZoom);
		if(ret >= current)
			ret -= plotWidth;
		return ret;
	}
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

#endif
