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

#include <iostream>

#include <QColorDialog>
#include <QPushButton>
#include <QPalette>

#include "ktracecolordialog.h"

KTraceColorDialog::KTraceColorDialog(QWidget *parent)
	: QDialog(parent)
{
	const int hsep = 10;
	const int vsep = 10;
	unsigned int characteristicLength;

	fgCol = Qt::black;
	bgCol = Qt::white;


	foregroundColorButton = new QPushButton(this);
	foregroundColorButton->setText("&Foreground color...");
	foregroundColorButton->adjustSize();

	characteristicLength = foregroundColorButton->height();
	foregroundColorDisplay = new QWidget(this);
	foregroundColorDisplay->resize(characteristicLength, characteristicLength);
	foregroundColorDisplay->move(hsep, vsep);
	foregroundColorDisplay->setAutoFillBackground(true);
	QPalette pal = palette();
	pal.setColor(QPalette::Window, fgCol);
	foregroundColorDisplay->setPalette(pal);

	foregroundColorButton->move(foregroundColorDisplay->geometry().right() + hsep, foregroundColorDisplay->y());

	backgroundColorDisplay = new QWidget(this);
	backgroundColorDisplay->resize(characteristicLength, characteristicLength);
	backgroundColorDisplay->move(hsep, foregroundColorDisplay->geometry().bottom() + vsep);
	backgroundColorDisplay->setAutoFillBackground(true);
	pal.setColor(QPalette::Window, bgCol);
	backgroundColorDisplay->setPalette(pal);

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
		QPalette pal = palette();
		pal.setColor(QPalette::Window, fgCol);
		foregroundColorDisplay->setPalette(pal);
	}
	return fgCol;
}

QColor KTraceColorDialog::bgColor(QColor color)
{
	if(color.isValid())
	{
		bgCol = color;
		QPalette pal = palette();
		pal.setColor(QPalette::Window, bgCol);
		backgroundColorDisplay->setPalette(pal);
	}
	return bgCol;
}
