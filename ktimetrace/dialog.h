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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
class QFrame;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTextEdit;

#include "data.h"
#include "kttsettings.h"

class KTraceDialog : public QDialog  {
Q_OBJECT
public:
	KTraceDialog(KTTSettings oldSettings, adc *currentADC, QWidget *parent=0);
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
	QListWidget *startTriggerList;
	QLabel *startTriggerLabel;
	QSpinBox *numChannels;
	QLabel *numChannelsLabel;
	QComboBox *scanTrigger;
	QListWidget *scanTriggerList;
	QLabel *scanTriggerLabel;
	QLineEdit *scanFrequency;
	QLabel *scanFrequencyLabel;
	QComboBox *conversionTrigger;
	QListWidget *conversionTriggerList;
	QLabel *conversionTriggerLabel;
	QLineEdit *conversionFrequency;
	QLabel *conversionFrequencyLabel;
	QComboBox *stopTrigger;
	QListWidget *stopTriggerList;
	QLabel *stopTriggerLabel;
	QSpinBox *numScans;
	QLabel *numScansLabel;
	QComboBox *range;
	QLabel *rangeLabel;
	QComboBox *reference;
	QListWidget *referenceList;
	QLabel *referenceLabel;
	QCheckBox *realTime;
	QCheckBox *lowLatency;
	QCheckBox *execAtStart;
	QTextEdit *startCommand;
	QCheckBox *execAtEnd;
	QTextEdit *endCommand;
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
