/***************************************************************************
                          zoomdialog.h  -  description
                             -------------------
    begin                : Fri Aug 10 2001
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

#ifndef ZOOMDIALOG_H
#define ZOOMDIALOG_H

class QLineEdit;
class QLabel;
class QDialog;
class QPushButton;

class ZoomDialog : public QDialog
{
	Q_OBJECT
  public:
	ZoomDialog(QWidget *parent = 0, const char *name = 0);
	~ZoomDialog();
	// set / return horizontal zoom value
	float hZoom(float zoom = -1);

  private:
	float horizontalZoom;
	QLineEdit *hZoomInput;
	QLabel *hZoomInputLabel;
	QPushButton *okButton;
	QPushButton *cancelButton;
  private slots:
	void slotHZoomInput(const QString &zoomText);
};

#endif
