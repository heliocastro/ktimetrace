/***************************************************************************
                          ktracecolordialog.cpp  -  description
                             -------------------
    begin                : Fri Sep 21 2001
    copyright            : (C) 2001 by Frank Mori Hess
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


#include <iostream>

#include <qcolordialog.h>

#include "ktracecolordialog.h"

KTraceColorDialog::KTraceColorDialog(QWidget *parent, const char *name) : QDialog(parent, name, true)
{
	const int hsep = 10;
	const int vsep = 10;
	unsigned int characteristicLength;

	fgCol = black;
	bgCol = white;

	foregroundColorButton = new QPushButton(this);
	foregroundColorButton->setText("&Foreground color...");
	foregroundColorButton->adjustSize();

	characteristicLength = foregroundColorButton->height();
	foregroundColorDisplay = new QWidget(this);
	foregroundColorDisplay->resize(characteristicLength, characteristicLength);
	foregroundColorDisplay->move(hsep, vsep);
	foregroundColorDisplay->setBackgroundColor(fgCol);

	foregroundColorButton->move(foregroundColorDisplay->geometry().right() + hsep, foregroundColorDisplay->y());

	backgroundColorDisplay = new QWidget(this);
	backgroundColorDisplay->resize(characteristicLength, characteristicLength);
	backgroundColorDisplay->move(hsep, foregroundColorDisplay->geometry().bottom() + vsep);
	backgroundColorDisplay->setBackgroundColor(bgCol);

	backgroundColorButton = new QPushButton(this);
	backgroundColorButton->setText("&Background color...");
	backgroundColorButton->adjustSize();
	backgroundColorButton->move(backgroundColorDisplay->geometry().right() + hsep, backgroundColorDisplay->y());

	okButton = new QPushButton(this);
	okButton->setText("&Ok");
	okButton->adjustSize();
	okButton->move(hsep, backgroundColorDisplay->geometry().bottom() + 2 * vsep);

	cancelButton = new QPushButton(this);
	cancelButton->setText("&Cancel");
	cancelButton->adjustSize();
	cancelButton->move(okButton->geometry().right() + hsep, okButton->y());

	// initialize signal-slot connections
	connect(foregroundColorButton, SIGNAL(clicked()), SLOT(slotFgColor()));
	connect(backgroundColorButton, SIGNAL(clicked()), SLOT(slotBgColor()));
	connect(okButton, SIGNAL(clicked()), SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));

	adjustSize();
	setFixedSize(size());
}

KTraceColorDialog::~KTraceColorDialog()
{
}

void KTraceColorDialog::slotFgColor()
{
	QColor color;
	color = QColorDialog::getColor(fgCol, this);
	fgColor(color);
}

void KTraceColorDialog::slotBgColor()
{
	QColor color;
	color = QColorDialog::getColor(bgCol, this);
	bgColor(color);
}

QColor KTraceColorDialog::fgColor(QColor color)
{
	if(color.isValid())
	{
		fgCol = color;
		foregroundColorDisplay->setBackgroundColor(fgCol);
	}
	return fgCol;
}

QColor KTraceColorDialog::bgColor(QColor color)
{
	if(color.isValid())
	{
		bgCol = color;
		backgroundColorDisplay->setBackgroundColor(bgCol);
	}
	return bgCol;
}
