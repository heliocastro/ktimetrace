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

#include <QPainter>
#include <QPalette>
#include <QPaintEvent>

#include <iostream>

#include "tracewidget.h"

TraceWidget::TraceWidget(int ch, QWidget *parent)
	: QWidget(parent)
{
	channel = ch;
	fScale = 4096;
	current = 0;
	left = 0;
	lastPlotted = 0;
	plotByBlock = true;

	QPalette pal = palette();
	pal.setColor(QPalette::Background, Qt::white);
	setAutoFillBackground(true);
	setPalette(pal);
	horizontalZoom = 1.0;
	plotWidth = getDisplayWidth(width(), horizontalZoom);

}

TraceWidget::~TraceWidget()
{
}

// updates as small as possible part of plot window
void TraceWidget::plot()
{
	int leftUpdate, rightUpdate;

	if(plotByBlock)
	{
		if((current - lastPlotted) >= plotWidth)
			update();
	}else{	// point-by-point plot mode
		// only update minimum required part of display
		leftUpdate = indexToX(lastPlotted);
		rightUpdate = indexToX(current - 1) + blankSpace;
		if(current < lastPlotted + plotWidth &&
			leftUpdate < rightUpdate)
		{
			QRect updateRect;
			updateRect.setLeft(leftUpdate);
			updateRect.setRight(rightUpdate);
			updateRect.setY(0);
			updateRect.setHeight(height());
			update(updateRect);
		}else update();
	}
}

void TraceWidget::reinit()
{
	current = left = lastPlotted = 0;
	repaint();
}

unsigned int TraceWidget::fullScale(unsigned int scale)
{
	fScale = scale;
	return fScale;
}

bool TraceWidget::blockMode(bool on)
{
	plotByBlock = on;
	return plotByBlock;
}

float TraceWidget::hZoom(float zoom)
{
	if(zoom <= 0)
		return horizontalZoom;
	horizontalZoom = zoom;
	plotWidth = getDisplayWidth(width(), horizontalZoom);
	update();
	return horizontalZoom;
}

void TraceWidget::flush()
{
	while(current > left + plotWidth)
		left += plotWidth;
	update();
}

void TraceWidget::setPlottingColor(QColor color)
{
	if(color.isValid() == false)
		return;

	plotColor = color;
}

void TraceWidget::paintEvent(QPaintEvent *event)
{
	long long newest;	// newest datapoint index displayed
	long long oldest;	// oldest datapoint displayed (in point by point mode)
	long long temp;

	// figure out indices of oldest and newest data point we are displaying
	if(plotByBlock)
	{
		while(current >= left + 2 * plotWidth)
			left += plotWidth;
		oldest = left;
		newest = left + plotWidth - 1;
		if(newest >= current)
			newest = current - 1;
	// else we are ploting data point by point
	}else{
		while(current >= left + plotWidth)
			left += plotWidth;
		oldest = current - plotWidth + (int)(blankSpace / horizontalZoom);
		if(oldest < 0)
			oldest = 0;
		newest = current - 1;

		// see if we can narrow down region that needs to be redrawn
		if(indexToX(oldest) < event->rect().left() ||
			indexToX(oldest) >= event->rect().right())
		{
			temp = XToIndex(event->rect().left() - 1);
			// sanity check
			if(temp > oldest)
				oldest = temp;
		}
		if(indexToX(newest) < event->rect().left() ||
			indexToX(newest) > event->rect().right())
		{
			temp = XToIndex(event->rect().right() + 1) + 1;
			// sanity check
			if(temp < newest)
				newest = temp;
		}
	}

	QPainter p(&pix);
	QPainterPath path;
	p.fillRect(event->rect(), palette().background());
	p.setPen(plotColor);
	for(long long i = oldest; i <= newest; i++)
	{
		if(i == oldest)
		{
			path.moveTo(indexToX(i),
				(int) ((height() - 1) * (1.0 - (float) plotArray[i % plotArrayLength] / fScale)));
		}else if(i == left)
		{
			path.lineTo((int) (indexToX(i - 1) + horizontalZoom),
				(int) ((height() - 1) * (1.0 - (float) plotArray[i % plotArrayLength] / fScale)));
			path.moveTo(indexToX(i),
				(int) ((height() - 1) * (1.0 - (float) plotArray[i % plotArrayLength] / fScale)));
		}else
			path.lineTo(indexToX(i),
				(int) ((height() - 1) * (1.0 - (float) plotArray[i % plotArrayLength] / fScale)));
		p.drawPath(path);
	}

	QPainter pp(this);
	pp.setClipRegion(event->region());
	pp.drawPixmap(0, 0, pix);

	if(newest > lastPlotted)
		lastPlotted = newest;
}

void TraceWidget::resizeEvent(QResizeEvent *event)
{
	plotWidth = getDisplayWidth(width(), horizontalZoom);
	pix = pix.scaled(event->size());
}

unsigned int TraceWidget::getDisplayWidth(unsigned int width, float zoom)
{
	return (unsigned int)((width - 1) / zoom);
}

