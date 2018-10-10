/***************************************************************************
                          dialog.h  -  description

KTraceDialog opens when the users asks to start aquisition, and allows
the user to specifiy the details.

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

#ifndef DIALOG_H
#define DIALOG_H

#include <qdialog.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qcheckbox.h>

#include "data.h"
#include "kttsettings.h"

class KTraceDialog : public QDialog  {
Q_OBJECT
public:
	KTraceDialog(KTTSettings oldSettings, adc *currentADC, QWidget *parent=0, const char *name=0);
	~KTraceDialog();
	/* dialog's local copy of the settings the user wants */
	KTTSettings *settings;
protected:
	// widgets for organizing layout
	QFrame *fileFrame;
	QWidget *leftColumn;
	QWidget *rightColumn;
	/* widgets that appear in dialog */
	QLabel *boardDesc;
	QCheckBox *saveData;
	QPushButton *browse;
	QLineEdit *directory;
	QLabel *directoryLabel;
	QLineEdit *fileName;
	QLabel *fileNameLabel;
	QSpinBox *fileNum;
	QLabel *fileNumLabel;
	QComboBox *saveType;
	QLabel *saveTypeLabel;
	QComboBox *startTrigger;
	QListBox *startTriggerList;
	QLabel *startTriggerLabel;
	QSpinBox *numChannels;
	QLabel *numChannelsLabel;
	QComboBox *scanTrigger;
	QListBox *scanTriggerList;
	QLabel *scanTriggerLabel;
	QLineEdit *scanFrequency;
	QLabel *scanFrequencyLabel;
	QComboBox *conversionTrigger;
	QListBox *conversionTriggerList;
	QLabel *conversionTriggerLabel;
	QLineEdit *conversionFrequency;
	QLabel *conversionFrequencyLabel;
	QComboBox *stopTrigger;
	QListBox *stopTriggerList;
	QLabel *stopTriggerLabel;
	QSpinBox *numScans;
	QLabel *numScansLabel;
	QComboBox *range;
	QLabel *rangeLabel;
	QComboBox *reference;
	QListBox *referenceList;
	QLabel *referenceLabel;
	QCheckBox *realTime;
	QCheckBox *lowLatency;
	QCheckBox *execAtStart;
	QMultiLineEdit *startCommand;
	QCheckBox *execAtEnd;
	QMultiLineEdit *endCommand;
	QPushButton *okButton;
	QPushButton *cancelButton;
	QPushButton *defaultsButton;
private:
	/* makes connections betweens signals emitted when user pushes buttons etc.
	 * and slots which respond to signals */
	void initConnections();
	/* creates all the widgets and controls in the dialog */
	void initDialog();
	/* checks that 'newSettings' is consistent with adc and modifies newSettings
	 * as necessary */
	void checkSettings();
	/* displays current settings, also used to initialize displayed values
	 * in dialog widgets */
	void refreshValues();
	// enables / disables widgets related to saving to file
	void fileWidgetsSetEnabled(bool);
	// pointer to open adc device
	adc *myADC;
private slots:
	/* slots that handle user input to the dialog */
	void slotSaveData(bool);
	void slotBrowse();
	void slotDirectory(const QString&);
	void slotFileName(const QString&);
	void slotFileNum(int);
	void slotSaveType(int);
	void slotStartTrigger(const QString&);
	void slotScanTrigger(const QString&);
	void slotConversionTrigger(const QString&);
	void slotStopTrigger(const QString&);
	void slotNumChannels(int);
	void slotScanFrequency(const QString&);
	void slotConversionFrequency(const QString&);
	void slotNumScans(int);
	void slotRange(int);
	void slotReference(const QString&);
	void slotRealTime(bool);
	void slotLowLatency(bool);
	void slotExecAtStart(bool);
	void slotStartCommand();
	void slotExecAtEnd(bool);
	void slotEndCommand();
	void slotOk();
	void slotDefaults();
};

#endif
