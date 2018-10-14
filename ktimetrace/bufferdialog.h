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

#ifndef BUFFERDIALOG_H
#define BUFFERDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

#include <adc.h>

class BufferDialog : public QDialog
{
Q_OBJECT
	public:
		BufferDialog(adc* myADC, QWidget *parent = 0);
		~BufferDialog();
	private:
		adc *board;
		QLabel *boardDesc;
		QSpinBox *bufSize;
		QLabel *bufSizeLabel;
		QSpinBox *maxBufSize;
		QLabel *maxBufSizeLabel;
		QPushButton *okButton;
		QPushButton *applyButton;
		QPushButton *cancelButton;

		// buffer size in kilobytes
		int bSize;
		// maximum buffer size in kilobytes
		int mBSize;
		// flag to indicate user has adjusted maximum buffer size
		bool maxModified;

	private slots:
		void slotBufSize(int);
		void slotMaxBufSize(int);
		void slotApply();
		void slotOk();
};

#endif
