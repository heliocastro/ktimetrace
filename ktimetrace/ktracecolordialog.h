/***************************************************************************
                          ktracecolordialog.h  -  description
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


#ifndef KTRACECOLORDIALOG_H
#define KTRACECOLORDIALOG_H

#include <qdialog.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <qcolor.h>

/**
  *@author Frank Mori Hess
  */

class KTraceColorDialog : public QDialog
{
Q_OBJECT
public:
	KTraceColorDialog(QWidget *parent = 0, const char *name = 0);
	~KTraceColorDialog();
	QColor fgColor(QColor color = QColor());
	QColor bgColor(QColor color = QColor());
private:
	QPushButton *foregroundColorButton;
	QPushButton *backgroundColorButton;
	QWidget *foregroundColorDisplay;
	QWidget *backgroundColorDisplay;
	QPushButton *okButton;
	QPushButton *cancelButton;
	QColor fgCol;
	QColor bgCol;
private slots:
	void slotFgColor();
	void slotBgColor();
};

#endif
