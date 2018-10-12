/***************************************************************************
                          view.cpp  -  description
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

#include <cmath>

#include <QProgressBar>
#include <QWhatsThis>
#include <QPushButton>
#include <QLabel>

#include "view.h"

KTraceView::KTraceView(QWidget *parent) : QWidget(parent)
{
	const int ledSize = 16;
	const int buttonBorder = 6;

	fScale = 4095;
	bMode = true;
	horizontalZoom = 1.0;
	numWindows = 0;
	traceWindowWidth = 0;
	fgCol = Qt::black;
	bgCol = Qt::white;

	lowerPanel = new QWidget(this);

	progressBar = new QProgressBar(lowerPanel);
	progressBar->setRange(0, 100);

	writeStatusIndicator = new KLed(lowerPanel);
	writeStatusIndicator->setLook(KLed::Sunken);
	writeStatusIndicator->off();
	writeStatusIndicator->setColor(Qt::white);
	writeStatusIndicator->resize(ledSize, ledSize);
	// QWhatsThis::add(writeStatusIndicator, "Indicates how many file writes are being processed "
	// 	"in the background.  Green means one, yellow means two, and red means three or more.");

	deviceStatusIndicator = new KLed(lowerPanel);
	deviceStatusIndicator->setLook(KLed::Sunken);
	deviceStatusIndicator->off();
	deviceStatusIndicator->setColor(Qt::white);
	deviceStatusIndicator->resize(ledSize, ledSize);
	// QWhatsThis::add(deviceStatusIndicator, "Indicates whether or not we have a working "
	// 	"analog input device currently open.");

	zoomInButton = new QPushButton(lowerPanel);
	// zoomInButton->setPixmap(KGlobal::iconLoader()->loadIcon("viewmag+", KIcon::Toolbar));
	// zoomInButton->resize(zoomInButton->pixmap()->width() + buttonBorder,
	// 	zoomInButton->pixmap()->height() + buttonBorder);
	connect(zoomInButton, SIGNAL(clicked()), SLOT(slotHZoomIn()));
	// QWhatsThis::add(zoomInButton, "Horinzontal zoom in.");

	zoomOutButton = new QPushButton(lowerPanel);
	// zoomOutButton->setPixmap(KGlobal::iconLoader()->loadIcon("viewmag-", KIcon::Toolbar));
	// zoomOutButton->resize(zoomOutButton->pixmap()->width() + buttonBorder,
	// 	zoomOutButton->pixmap()->height() + buttonBorder);
	connect(zoomOutButton, SIGNAL(clicked()), SLOT(slotHZoomOut()));
	// QWhatsThis::add(zoomOutButton, "Horizontal zoom out.");

	zoomDisplay = new QLabel(lowerPanel);
	// QWhatsThis::add(zoomDisplay,
	// 	"Indicates the number of samples that will fit in the current display.");

	createWindows(1);
}

KTraceView::~KTraceView()
{
}

void KTraceView::createWindows(unsigned int n)
{
	if(n > maxChannels) n = maxChannels;
	// create more windows if necessary
	if(n > numWindows)
	{
		for(unsigned int i = numWindows; i < n; i++)
		{
			trace[i] = new TraceWidget(i, this);
			trace[i]->show();
		}
	// delete excess windows
	}else if(n < numWindows)
		for(unsigned int i = n; i < numWindows; i++) delete trace[i];
	numWindows = n;
	// initialize windows
	for(unsigned int i = 0; i < numWindows; i++)
	{
		trace[i]->reinit();
		trace[i]->fullScale(fScale);
		trace[i]->blockMode(bMode);
		trace[i]->hZoom(horizontalZoom);
		trace[i]->setPlottingColor(fgCol);
		//trace[i]->setBackgroundColor(bgCol);
	}
	resizeEvent(0);
	progressBar->reset();
}

unsigned int KTraceView::fullScale(unsigned int scale)
{
	fScale = scale;
	for(unsigned int i = 0; i < numWindows; i++)
	{
		trace[i]->fullScale(fScale);
	}
	return fScale;
}

void KTraceView::saveConfig()
{
	// config->setGroup("view settings");
	// config->writeEntry("horizontalZoom", horizontalZoom);
	// config->writeEntry("fgRed", fgCol.red());
	// config->writeEntry("fgGreen", fgCol.green());
	// config->writeEntry("fgBlue", fgCol.blue());
	// config->writeEntry("bgRed", bgCol.red());
	// config->writeEntry("bgGreen", bgCol.green());
	// config->writeEntry("bgBlue", bgCol.blue());
}

void KTraceView::loadConfig()
{
	QColor color;
	int red, green, blue;

	// config->setGroup("view settings");
	// hZoom(config->readDoubleNumEntry("horizontalZoom", horizontalZoom));
	// // load foreground color
	// red = config->readNumEntry("fgRed", fgCol.red());
	// green = config->readNumEntry("fgGreen", fgCol.green());
	// blue = config->readNumEntry("fgBlue", fgCol.blue());
	// color.setRgb(red, green, blue);
	// fgColor(color);
	// // load background color
	// red = config->readNumEntry("bgRed", bgCol.red());
	// green = config->readNumEntry("bgGreen", bgCol.green());
	// blue = config->readNumEntry("bgBlue", bgCol.blue());
	// color.setRgb(red, green, blue);
	// bgColor(color);
}

bool KTraceView::blockMode(bool on)
{
	bMode = on;
	for(unsigned int i = 0; i < numWindows; i++)
	{
		trace[i]->blockMode(bMode);
	}
	return bMode;
}

void KTraceView::setProgress(int progress)
{
	progressBar->setValue(progress);
}

void KTraceView::setWriteIndicator(unsigned int writeCount)
{
	QColor statusColor;

	switch(writeCount)
	{
		case 0:
			writeStatusIndicator->off();
			statusColor = Qt::white;
			break;
		case 1:
			writeStatusIndicator->on();
			statusColor = Qt::green;
			break;
		case 2:
			writeStatusIndicator->on();
			statusColor = Qt::yellow;
			break;
		default:
			writeStatusIndicator->on();
			statusColor = Qt::red;
			break;
	}
	writeStatusIndicator->setColor(statusColor);
	return;
}

void KTraceView::setDeviceIndicator(bool good)
{
	if(good)
		deviceStatusIndicator->setColor(Qt::green);
	else
		deviceStatusIndicator->setColor(Qt::red);

	deviceStatusIndicator->on();
	return;
}

void KTraceView::flush()
{
	for(unsigned int i = 0; i < numWindows; i++)
		trace[i]->flush();
}

float KTraceView::hZoom(float zoom)
{
	if(zoom <= 0)
		return horizontalZoom;
	if(zoom < 1.0 / 16.0)
		zoom = 1.0 / 16.0;
	if(zoom > 16.0)
		zoom = 16.0;
	horizontalZoom = zoom;
	for(unsigned int i = 0; i < numWindows; i++)
	{
		trace[i]->hZoom(horizontalZoom);
	}

	updateDisplayWidth();

	return horizontalZoom;
}

QColor KTraceView::fgColor(QColor color)
{
	if(color.isValid() == false)
		return fgCol;
	fgCol = color;
	for(unsigned int i = 0; i < numWindows; i++)
	{
		trace[i]->setPlottingColor(fgCol);
	}
	return fgCol;
}

QColor KTraceView::bgColor(QColor color)
{
	if(color.isValid() == false)
		return bgCol;
	bgCol = color;
	for(unsigned int i = 0; i < numWindows; i++)
	{
		//trace[i]->setBackgroundColor(bgCol);
	}
	return bgCol;
}

void KTraceView::updateDisplayWidth()
{
	QString temp;
	unsigned int displayWidth = TraceWidget::getDisplayWidth(traceWindowWidth, horizontalZoom);
	temp.sprintf("Display width / (samples) = %i\n"
		"Zoom = %g", displayWidth, horizontalZoom);
	zoomDisplay->setText(temp);
	zoomDisplay->adjustSize();
}

void KTraceView::updatePlots()
{
	for(unsigned int i = 0; i < numWindows; i++)
		trace[i]->plot();
}

void KTraceView::resizeEvent(QResizeEvent *event)
{
	const int hsep = 10, vsep = 10;

	lowerPanel->setFixedWidth(width());

	deviceStatusIndicator->move(width() - deviceStatusIndicator->width() - hsep, 0);
	writeStatusIndicator->move(deviceStatusIndicator->geometry().left() - writeStatusIndicator->width() - hsep, 0);

	progressBar->move(hsep, 0);
	progressBar->setFixedWidth((lowerPanel->width() - 2 * hsep) / 3);

	zoomInButton->move(progressBar->geometry().right() + hsep, 0);
	zoomOutButton->move(zoomInButton->geometry().right(), 0);
	zoomDisplay->move(zoomOutButton->geometry().right() + hsep, 0);

	lowerPanel->adjustSize();
	lowerPanel->move(0, height() - lowerPanel->height() - vsep);

	traceWindowWidth = width() - 2 * hsep;
	for(unsigned int i = 0; i < numWindows; i++)
	{
		int traceWindowHeight = (lowerPanel->y() - vsep) / numWindows - vsep;
		trace[i]->setGeometry(hsep, vsep + (traceWindowHeight + vsep) * i,
			traceWindowWidth, traceWindowHeight);
	}

	updateDisplayWidth();
}

void KTraceView::slotHZoomIn()
{
	float zoom = hZoom();
	zoom *= sqrt(2);
	hZoom(zoom);
}

void KTraceView::slotHZoomOut()
{
	float zoom = hZoom();
	zoom /= sqrt(2);
	hZoom(zoom);
}
