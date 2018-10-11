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

#ifndef KTRACECOLORDIALOG_H
#define KTRACECOLORDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QColor;
class QPushButton;
class QWidget;
QT_END_NAMESPACE

class KTraceColorDialog : public QDialog
{
	Q_OBJECT

public:
	KTraceColorDialog(QWidget *parent = 0);
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
