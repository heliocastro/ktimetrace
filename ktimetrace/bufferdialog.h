/***************************************************************************
                          bufferdialog.h  -  description
                             -------------------
    begin                : Thu Jun 14 2001
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


#ifndef BUFFERDIALOG_H
#define BUFFERDIALOG_H

#include <qdialog.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <adc.h>

/**Dialog for adjusting comedi's buffer size

  *@author Frank Mori Hess
  */

class BufferDialog : public QDialog
{
Q_OBJECT
public:
	BufferDialog(adc* myADC, QWidget *parent = 0, const char *name = 0);
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
