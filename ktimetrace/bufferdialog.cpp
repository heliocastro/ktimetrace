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

#include <QMessageBox>
#include <QWhatsThis>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

#include <iostream>
#include <cmath>
#include <sys/types.h>
#include <unistd.h>

#include "bufferdialog.h"
#include "resource.h"

BufferDialog::BufferDialog(adc *myADC, QWidget *parent) :
	QDialog(parent)
{
	const int kByte = 1024;

	board = myADC;
	mBSize = myADC->maxBufferSize() / kByte;
	bSize = myADC->bufferSize() / kByte;
	maxModified = false;
	QString string;

	// create sub-widgets
	const int hsep = 10;
	const int vsep = 10;

	boardDesc = new QLabel(this);
	boardDesc->move(hsep, vsep);
	string = "<b>Device: " + board->boardName() + " on " + board->devicePath() + "</b>";
	boardDesc->setTextFormat(Qt::RichText);
	boardDesc->resize(boardDesc->fontMetrics().size(0, string));
	boardDesc->setText(string);

	bufSize = new QSpinBox(this);
	bufSize->move(boardDesc->x(), boardDesc->geometry().bottom() + vsep);
	// set max value large temporarily before calling adjustSize();
	bufSize->setMaximum(INT_MAX);
	bufSize->adjustSize();
	bufSize->setMaximum(mBSize);
	bufSize->setMinimum(1);
	bufSize->setValue(bSize);
	bufSize->setWhatsThis(
		"Changes the size of the memory buffer that comedi uses to store data "
		"waiting to be read from /dev/comediX (by this program).  A larger "
		"buffer can be helpful if you are taking data at high frequencies. "
		"The comedi_config program included with comedilib can be used to "
		"adjust the maximum allowed buffer size.");

	bufSizeLabel = new QLabel("Comedi's buffer size / (kilobytes)", this);
	bufSizeLabel->move(bufSize->geometry().right() + hsep, bufSize->y());
	bufSizeLabel->adjustSize();

	maxBufSize = new QSpinBox(this);
	maxBufSize->move(bufSize->x(), bufSize->geometry().bottom() + vsep);
	maxBufSize->setMinimum(1);
	maxBufSize->setMaximum(INT_MAX);
	maxBufSize->setValue(mBSize);
	maxBufSize->adjustSize();
	// enable if we are root
	maxBufSize->setEnabled(getuid() == 0);
	maxBufSize->setWhatsThis(
		"This allows you to adjust comedi's maximum allowed "
		"buffer size.  You must have root privilege.  "
		"The comedi_config program included with comedilib can also be used to "
		"adjust the maximum allowed buffer size.");

	maxBufSizeLabel = new QLabel("Comedi's maximum buffer size / (kilobytes)", this);
	maxBufSizeLabel->move(maxBufSize->geometry().right() + hsep, maxBufSize->y());
	maxBufSizeLabel->adjustSize();

	okButton = new QPushButton("&Ok", this);
	okButton->move(maxBufSize->x(), maxBufSize->geometry().bottom() + 2 * vsep);
	okButton->adjustSize();

	applyButton = new QPushButton("&Apply", this);
	applyButton->move(okButton->geometry().right() + hsep, okButton->y());
	applyButton->adjustSize();

	cancelButton = new QPushButton("&Cancel", this);
	cancelButton->move(applyButton->geometry().right() + hsep, okButton->y());
	cancelButton->adjustSize();

	// adjust overall dialog size
	adjustSize();

	// connect signals/slots
	connect(bufSize, SIGNAL(valueChanged(int)), SLOT(slotBufSize(int)));
	connect(maxBufSize, SIGNAL(valueChanged(int)), SLOT(slotMaxBufSize(int)));
	connect(okButton, SIGNAL(clicked()), SLOT(slotOk()));
	connect(applyButton, SIGNAL(clicked()), SLOT(slotApply()));
	connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
}

BufferDialog::~BufferDialog()
{
}

void BufferDialog::slotBufSize(int size)
{
	bSize = size;
}

void BufferDialog::slotMaxBufSize(int size)
{
	if(size != mBSize)
	{
		maxModified = true;
		mBSize = size;
		bufSize->setMaximum(mBSize);
	}
}

void BufferDialog::slotApply()
{
	const int kByte = 1024;
	int ret;

	//adjust comedi's maximum buffer size (requires root privilege)
	if(maxModified)
	{
		ret = board->maxBufferSize(mBSize * kByte);
		if(ret < 0)
		{
			QMessageBox::warning(this, "Warning",
				"An error occured while attempting to\n"
				"adjust comedi's maximum buffer size");
			return;
		}
		mBSize = ret / kByte;
		maxBufSize->setValue(mBSize);
		maxModified = false;
	}

	// adjust comedi's buffer size
	ret = board->bufferSize(bSize * kByte);
	if(ret < 0)
	{
		QMessageBox::warning(this, "Warning",
			"An error occured while attempting to\n"
			"adjust comedi's buffer size");
		return;
	}
	bSize = ret / kByte;
	bufSize->setValue(bSize);

	return;
}

void BufferDialog::slotOk()
{
	slotApply();
	accept();
}
