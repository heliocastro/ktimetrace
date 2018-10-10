/***************************************************************************
                          dialog.cpp  -  description
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

#include "dialog.h"

#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qwhatsthis.h>

#include <math.h>
#include <cassert>

#include "ktimetrace.h"

KTraceDialog::KTraceDialog(KTTSettings oldSettings, adc *currentADC,
	QWidget *parent, const char *name) : QDialog(parent, name, true)
{
	myADC = currentADC;
	settings = new KTTSettings;
	*settings = oldSettings;
	checkSettings();
	initDialog();
	initConnections();
	refreshValues();
}

KTraceDialog::~KTraceDialog()
{
	delete settings;
}

void KTraceDialog::refreshValues()
{
	QString temp;
	QListBoxItem* item;

	saveData->setChecked(settings->saveData);
	fileWidgetsSetEnabled(settings->saveData);
	directory->setText(settings->directory.absPath());
	fileName->setText(settings->fileStem);
	fileNum->setValue(settings->fileNum);
	saveType->setCurrentItem(settings->saveType);

	item = startTriggerList->findItem(adc::trigToText(settings->startTrigger));
	startTrigger->setCurrentItem(startTriggerList->index(item));
	item = scanTriggerList->findItem(adc::trigToText(settings->scanTrigger));
	scanTrigger->setCurrentItem(scanTriggerList->index(item));
	item = conversionTriggerList->findItem(adc::trigToText(settings->conversionTrigger));
	conversionTrigger->setCurrentItem(conversionTriggerList->index(item));
	item = stopTriggerList->findItem(adc::trigToText(settings->stopTrigger));
	stopTrigger->setCurrentItem(stopTriggerList->index(item));

	// display scanning and conversion frequencies if we can figure out what it is
	if(settings->scanTrigger == TRIG_TIMER ||
		(settings->scanTrigger == TRIG_FOLLOW && settings->conversionTrigger == TRIG_TIMER))
	{
		scanFrequency->setText(temp.setNum(settings->scanFrequency));
		scanFrequency->setEnabled(true);
	}else{
		scanFrequency->setEnabled(false);
	}
	// enable conversion frequency input as appropriate
	conversionFrequency->setEnabled(settings->conversionTrigger == TRIG_TIMER &&
		settings->scanTrigger != TRIG_FOLLOW);
	if(settings->conversionTrigger == TRIG_TIMER)
	{
		if(settings->scanTrigger == TRIG_FOLLOW)
			settings->conversionFrequency = settings->scanFrequency * settings->numChannels;
		conversionFrequency->setText(temp.setNum(settings->conversionFrequency));
	}

	numChannels->setValue(settings->numChannels);

	// enable numScans display as appropriate
	numScans->setEnabled(settings->stopTrigger == TRIG_COUNT);
	numScans->setValue(settings->numScans);

	range->setCurrentItem(settings->range);

	item = referenceList->findItem(adc::refToText(settings->reference));
	if(item)
		reference->setCurrentItem(referenceList->index(item));

	realTime->setChecked(settings->realTime);
	lowLatency->setChecked(settings->lowLatency);

	execAtStart->setChecked(settings->execAtStart);
	startCommand->setText(settings->startCommand);
	startCommand->setEnabled(settings->execAtStart);
	execAtEnd->setChecked(settings->execAtEnd);
	endCommand->setText(settings->endCommand);
	endCommand->setEnabled(settings->execAtEnd);
}

void KTraceDialog::fileWidgetsSetEnabled(bool enable)
{
	browse->setEnabled(enable);
	fileName->setEnabled(enable);
	fileNum->setEnabled(enable);
	directory->setEnabled(enable);
	saveType->setEnabled(enable);
}

void KTraceDialog::checkSettings()
{
	comedi_range *rangePtr;

	settings->startTrigger = adc::defaultTrigger(myADC->startTriggerMask(), settings->startTrigger);
	settings->scanTrigger = adc::defaultTrigger(myADC->scanTriggerMask(), settings->scanTrigger);
	settings->conversionTrigger = adc::defaultTrigger(myADC->conversionTriggerMask(), settings->conversionTrigger);
	settings->stopTrigger = adc::defaultTrigger(myADC->stopTriggerMask(), settings->stopTrigger);
	settings->reference = adc::defaultReference(myADC->subdevFlags(), settings->reference);
	// check that range index is valid
	rangePtr = myADC->getRange(settings->range);
	if(rangePtr == 0)
	{
		settings->range = 0;
		rangePtr = myADC->getRange(settings->range);	
	}
	assert(rangePtr);
	settings->rangeInfo = *rangePtr;
	// make sure we haven't forgotten the board name
	settings->boardName = myADC->boardName();
}

void KTraceDialog::initConnections()
{
	connect(saveData, SIGNAL(toggled(bool)), SLOT(slotSaveData(bool)));
	connect(browse, SIGNAL(clicked()), SLOT(slotBrowse()));
	connect(directory, SIGNAL(textChanged(const QString&)), SLOT(slotDirectory(const QString&)));
	connect(fileName, SIGNAL(textChanged(const QString&)), SLOT(slotFileName(const QString&)));
	connect(fileNum, SIGNAL(valueChanged(int)), SLOT(slotFileNum(int)));
	connect(saveType, SIGNAL(activated(int)), SLOT(slotSaveType(int)));
	connect(startTrigger, SIGNAL(activated(const QString&)), SLOT(slotStartTrigger(const QString&)));
	connect(scanTrigger, SIGNAL(activated(const QString&)), SLOT(slotScanTrigger(const QString&)));
	connect(conversionTrigger, SIGNAL(activated(const QString&)), SLOT(slotConversionTrigger(const QString&)));
	connect(stopTrigger, SIGNAL(activated(const QString&)), SLOT(slotStopTrigger(const QString&)));
	connect(scanFrequency, SIGNAL(textChanged(const QString&)), SLOT(slotScanFrequency(const QString&)));
	connect(conversionFrequency, SIGNAL(textChanged(const QString&)), SLOT(slotConversionFrequency(const QString&)));
	connect(numChannels, SIGNAL(valueChanged(int)), SLOT(slotNumChannels(int)));
	connect(numScans, SIGNAL(valueChanged(int)), SLOT(slotNumScans(int)));
	connect(range, SIGNAL(activated(int)), SLOT(slotRange(int)));
	connect(reference, SIGNAL(activated(const QString&)), SLOT(slotReference(const QString&)));
	connect(realTime, SIGNAL(toggled(bool)), SLOT(slotRealTime(bool)));
	connect(lowLatency, SIGNAL(toggled(bool)), SLOT(slotLowLatency(bool)));
	connect(execAtStart, SIGNAL(toggled(bool)), SLOT(slotExecAtStart(bool)));
	connect(startCommand, SIGNAL(textChanged()), SLOT(slotStartCommand()));
	connect(execAtEnd, SIGNAL(toggled(bool)), SLOT(slotExecAtEnd(bool)));
	connect(endCommand, SIGNAL(textChanged()), SLOT(slotEndCommand()));
	connect(okButton, SIGNAL(clicked()), SLOT(slotOk()));
	connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
	connect(defaultsButton, SIGNAL(clicked()), SLOT(slotDefaults()));
}

void  KTraceDialog::initDialog()
{
	QString string1, string2;
	comedi_range *rangePtr;
	const int hsep = 10;	//horizontal seperation between adjacent widgets
	const int vsep = 10;	//vertical seperation bewteen adjacent widgets

	boardDesc = new QLabel(this);
	boardDesc->move(hsep, vsep);
	string1 = "<b>Device: " + myADC->boardName() + " on " + myADC->devicePath() + "</b>";
	boardDesc->setTextFormat(RichText);
	boardDesc->resize(boardDesc->fontMetrics().size(0, string1));
	boardDesc->setText(string1);

	saveData = new QCheckBox(this);
	saveData->setText("Save analog input data to file");
	saveData->adjustSize();
	saveData->move(hsep, boardDesc->geometry().bottom() + vsep);

	fileFrame = new QFrame(this);
	fileFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	fileFrame->setMargin(hsep);
	fileFrame->move(saveData->x() + hsep, saveData->geometry().bottom() + vsep);

	browse = new QPushButton("&Browse...", fileFrame);
	browse->move(fileFrame->margin(), fileFrame->margin());
	browse->adjustSize();

	directory = new QLineEdit(fileFrame);
	directory->setGeometry(browse->geometry().right() + hsep, browse->y(), 250, directory->sizeHint().height());
	QWhatsThis::add(directory, "Directory where data will be saved");

	directoryLabel = new QLabel(fileFrame);
	directoryLabel->move(directory->geometry().right() + hsep, directory->y());
	directoryLabel->setText("Directory");
	directoryLabel->adjustSize();

	fileName = new QLineEdit(fileFrame);
	fileName->move(browse->x(), browse->geometry().bottom() + vsep);
	fileName->adjustSize();
	QWhatsThis::add(fileName, "The file stem of the saved data file.  The file number "
		"and a file type extension will be appended");

	fileNameLabel = new QLabel(fileFrame);
	fileNameLabel->setText("File stem");
	fileNameLabel->move(fileName->geometry().right() + hsep, fileName->y());
	fileNameLabel->adjustSize();

	fileNum = new QSpinBox(fileFrame);
	fileNum->move(fileName->x(), fileName->geometry().bottom() + vsep);
	fileNum->setRange(0, 999);
	fileNum->adjustSize();
	QWhatsThis::add(fileNum, "The file number.  It is appended to the file stem");

	fileNumLabel = new QLabel(fileFrame);
	fileNumLabel->move(fileNum->geometry().right() + hsep, fileNum->y());
	fileNumLabel->setText("File number");
	fileNumLabel->adjustSize();

	saveType = new QComboBox(false, fileFrame);
	saveType->move(fileNum->x(), fileNum->geometry().bottom() + vsep);
	saveType->insertItem("Tab delimited text", TEXT);
	saveType->insertItem("Gzipped text", GZ_TEXT);
	saveType->insertItem("True binary (deprecated)", BINARY);
	saveType->adjustSize();
	QWhatsThis::add(saveType, "Format for the saved data file.  'True binary' was included "
		"for backward compatiblity with my lab's previous data taking software.  It stores "
		"data as an array of 16 bit integers (signed or unsigned for bipolar/unipolar ranges) "
		"with little endian byte ordering.");

	saveTypeLabel = new QLabel(fileFrame);
	saveTypeLabel->move(saveType->geometry().right() + hsep, saveType->y());
	saveTypeLabel->setText("Save File Format");
	saveTypeLabel->adjustSize();

	leftColumn = new QWidget(this);

	startTrigger = new QComboBox(false, leftColumn);
	startTrigger->move(0, 0);
	startTriggerList = new QListBox(leftColumn);
	if(myADC->startTriggerMask() & TRIG_NOW)
		startTriggerList->insertItem(adc::trigToText(TRIG_NOW));
	if(myADC->startTriggerMask() & TRIG_EXT)
		startTriggerList->insertItem(adc::trigToText(TRIG_EXT));
	startTrigger->setListBox(startTriggerList);
	startTrigger->adjustSize();
	// disable combo box if it has only one option
	if(startTrigger->count() < 2)
		startTrigger->setEnabled(false);
	QWhatsThis::add(startTrigger, "The trigger which begins data aquisition");

	startTriggerLabel = new QLabel(leftColumn);
	startTriggerLabel->move(startTrigger->geometry().right() + hsep, startTrigger->y());
	startTriggerLabel->setText("Start Trigger");
	startTriggerLabel->adjustSize();

	numChannels = new QSpinBox(leftColumn);
	numChannels->move(startTrigger->x(), startTrigger->geometry().bottom() + vsep);
	numChannels->setRange(1, myADC->numChannels());
	numChannels->adjustSize();
	QWhatsThis::add(numChannels, "The number of channels to be read for each scan");

	numChannelsLabel = new QLabel(leftColumn);
	numChannelsLabel->move(numChannels->geometry().right() + hsep, numChannels->y());
	numChannelsLabel->setText("Number of channels");
	numChannelsLabel->adjustSize();

	scanTrigger = new QComboBox(false, leftColumn);
	scanTrigger->move(numChannels->x(), numChannels->geometry().bottom() + vsep);
	scanTriggerList = new QListBox(leftColumn);
	if(myADC->scanTriggerMask() & TRIG_TIMER)
		scanTriggerList->insertItem(adc::trigToText(TRIG_TIMER));
	if(myADC->scanTriggerMask() & TRIG_FOLLOW)
		scanTriggerList->insertItem(adc::trigToText(TRIG_FOLLOW));
	if(myADC->scanTriggerMask() & TRIG_EXT)
		scanTriggerList->insertItem(adc::trigToText(TRIG_EXT));
	scanTrigger->setListBox(scanTriggerList);
	scanTrigger->adjustSize();
	// disable combo box if it has only one option
	if(scanTrigger->count() < 2)
		scanTrigger->setEnabled(false);
	QWhatsThis::add(scanTrigger, "The trigger which starts a scan.  The '" +
		adc::trigToText(TRIG_FOLLOW) + "' trigger makes scans follow the "
		"conversion trigger.");

	scanTriggerLabel = new QLabel(leftColumn);
	scanTriggerLabel->move(scanTrigger->geometry().right() + hsep, scanTrigger->y());
	scanTriggerLabel->setText("Scan Trigger");
	scanTriggerLabel->adjustSize();

	scanFrequency = new QLineEdit(leftColumn);
	scanFrequency->move(scanTrigger->x(), scanTrigger->geometry().bottom() + vsep);
	scanFrequency->adjustSize();
	QWhatsThis::add(scanFrequency, "Determines the frequency with which scans are taken "
		"when based on a '" + adc::trigToText(TRIG_TIMER) + "' trigger");

	scanFrequencyLabel = new QLabel(leftColumn);
	scanFrequencyLabel->move(scanFrequency->geometry().right() + hsep, scanFrequency->y());
	scanFrequencyLabel->setText("Scan Frequency (Hz)");
	scanFrequencyLabel->adjustSize();

	conversionTrigger = new QComboBox(false, leftColumn);
	conversionTrigger->move(scanFrequency->x(), scanFrequency->geometry().bottom() + vsep);
	conversionTriggerList = new QListBox(leftColumn);
	if(myADC->conversionTriggerMask() & TRIG_TIMER)
		conversionTriggerList->insertItem(adc::trigToText(TRIG_TIMER));
	if(myADC->conversionTriggerMask() & TRIG_NOW)
		conversionTriggerList->insertItem(adc::trigToText(TRIG_NOW));
	if(myADC->conversionTriggerMask() & TRIG_EXT)
		conversionTriggerList->insertItem(adc::trigToText(TRIG_EXT));
	conversionTrigger->setListBox(conversionTriggerList);
	conversionTrigger->adjustSize();
	// disable combo box if it has only one option
	if(conversionTrigger->count() < 2)
		conversionTrigger->setEnabled(false);
	QWhatsThis::add(conversionTrigger, "The trigger which starts an individual conversion within "
		"a scan.  The '" + adc::trigToText(TRIG_NOW) + "' trigger means to complete the scan "
		"as quickly as possible.");

	conversionTriggerLabel = new QLabel("Conversion Trigger", leftColumn);
	conversionTriggerLabel->move(conversionTrigger->geometry().right() + hsep, conversionTrigger->y());
	conversionTriggerLabel->adjustSize();

	conversionFrequency = new QLineEdit(leftColumn);
	conversionFrequency->move(conversionTrigger->x(), conversionTrigger->geometry().bottom() + vsep);
	conversionFrequency->adjustSize();
	QWhatsThis::add(conversionFrequency, "Determines the frequency with which conversions are made "
		"when the conversion trigger is '" + adc::trigToText(TRIG_TIMER) + "'.  "
		"May be overriden by scan frequency if scan trigger is '"+ adc::trigToText(TRIG_FOLLOW) +"'");

	conversionFrequencyLabel = new QLabel(leftColumn);
	conversionFrequencyLabel->move(conversionFrequency->geometry().right() + hsep, conversionFrequency->y());
	conversionFrequencyLabel->setText("Conversion Frequency (Hz)");
	conversionFrequencyLabel->adjustSize();

	stopTrigger = new QComboBox(false, leftColumn);
	stopTrigger->move(conversionFrequency->x(), conversionFrequency->geometry().bottom() + vsep);
	stopTriggerList = new QListBox(leftColumn);
	if(myADC->stopTriggerMask() & TRIG_COUNT)
		stopTriggerList->insertItem(adc::trigToText(TRIG_COUNT));
	if(myADC->stopTriggerMask() & TRIG_NONE)
		stopTriggerList->insertItem(adc::trigToText(TRIG_NONE));
	if(myADC->stopTriggerMask() & TRIG_EXT)
		stopTriggerList->insertItem(adc::trigToText(TRIG_EXT));
	stopTrigger->setListBox(stopTriggerList);
	stopTrigger->adjustSize();
	// disable combo box if it has only one option
	if(stopTrigger->count() < 2)
		stopTrigger->setEnabled(false);
	QWhatsThis::add(stopTrigger, "The trigger which ends data aquisition.");

	stopTriggerLabel = new QLabel("Stop Trigger", leftColumn);
	stopTriggerLabel->move(stopTrigger->geometry().right() + hsep, stopTrigger->y());
	stopTriggerLabel->adjustSize();

	numScans = new QSpinBox(leftColumn);
	numScans->move(stopTrigger->x(), stopTrigger->geometry().bottom() + vsep);
	numScans->setRange(1, INT_MAX);
	numScans->adjustSize();
	QWhatsThis::add(numScans, "The total number of scans to perform");

	numScansLabel = new QLabel(leftColumn);
	numScansLabel->move(numScans->geometry().right() + hsep, numScans->y());
	numScansLabel->setText("Number of scans");
	numScansLabel->adjustSize();

	range = new QComboBox(false, leftColumn);
	range->move(numScans->x(), numScans->geometry().bottom() + vsep);
	for(int i = 0; (rangePtr = myADC->getRange(i)); i++)
	{
		if(rangePtr->unit == UNIT_volt)
		{
			string1.setNum(rangePtr->min);
			string1 += 'V';
			string2.setNum(rangePtr->max);
			string2 += 'V';
			string1 += " to " + string2;
		}else
		{
			string1 = "Range unknown";
		}
		range->insertItem(string1, i);
	}
	range->adjustSize();
	// disable combo box if it has only one option
	if(range->count() < 2)
		range->setEnabled(false);
	QWhatsThis::add(range,"The desired input range for your daq card");

	rangeLabel = new QLabel(leftColumn);
	rangeLabel->move(range->geometry().right() + hsep, range->y());
	rangeLabel->setText("Analog input range");
	rangeLabel->adjustSize();

	unsigned int subdevFlags = myADC->subdevFlags();
	reference = new QComboBox(false, leftColumn);
	referenceList = new QListBox(leftColumn);
	reference->move(range->x(), range->geometry().bottom() + vsep);
	if(subdevFlags & SDF_GROUND)
		referenceList->insertItem(adc::refToText(AREF_GROUND));
	if(subdevFlags & SDF_COMMON)
		referenceList->insertItem(adc::refToText(AREF_COMMON));
	if(subdevFlags & SDF_DIFF)
		referenceList->insertItem(adc::refToText(AREF_DIFF));
	if(subdevFlags & SDF_OTHER)
		referenceList->insertItem(adc::refToText(AREF_OTHER));
	reference->setListBox(referenceList);
	if(reference->count() == 0)
		referenceList->insertItem("driver bug: no supported analog references specified");	
	reference->adjustSize();
	// disable combo box if it has only one option
	if(reference->count() < 2)
		reference->setEnabled(false);
	QWhatsThis::add(reference, "The desired input reference for your daq card.  "
		"It will be ignored if your card's input reference is not software programmable.");

	referenceLabel = new QLabel(leftColumn);
	referenceLabel->move(reference->geometry().right() + hsep, reference->y());
	referenceLabel->setText("Analog input reference");
	referenceLabel->adjustSize();

	// second column
	
	rightColumn = new QWidget(this);

	realTime = new QCheckBox(rightColumn);
	realTime->move(0, 0);
	realTime->setText("Request real-time interrupts");
	realTime->adjustSize();
	QWhatsThis::add(realTime,
		"Request for the comedi driver to use an interrupt handler with hard "
		"real-time priority. It will have no effect unless you are running a real-time kernel "
		"(rtlinux or rtai) and have compiled comedi with real-time support.  "
		"This option can be useful if you are "
		"taking data at high frequency or have a card with a small onboard fifo.");

	lowLatency = new QCheckBox(rightColumn);
	lowLatency->move(realTime->x(), realTime->geometry().bottom() + vsep);
	lowLatency->setText("Request low latency mode");
	lowLatency->adjustSize();
	QWhatsThis::add(lowLatency,
		"The 'low latency' option is good for low-frequency operation if you "
		"get impatient waiting for blocks of data to arrive and be plotted.  "
		"It tends to degrade performance for high frequency input.");

	QString execAtStartHelp = "You can enter shell commands which will be "
		"executed before aquisition begins.  If you are saving data to disk, the "
		"shell variable KTT_DATA_FILE "
		"will be set to the name of your data file (except when using the deprecated "
		"binary format, in which case the extension .ts? is left off).  Also, KTT_PARAM_FILE "
		"will contain the name of the parameter file.";

	execAtStart = new QCheckBox(rightColumn);
	execAtStart->move(lowLatency->x(), lowLatency->geometry().bottom() + vsep);
	execAtStart->setText("Execute shell command at start of aquisition");
	execAtStart->adjustSize();
	QWhatsThis::add(execAtStart, execAtStartHelp);

	startCommand = new QMultiLineEdit(rightColumn);
	startCommand->move(execAtStart->x() + hsep, execAtStart->geometry().bottom() + vsep);
#if QT_VERSION >= 300
	startCommand->setWordWrap(QTextEdit::NoWrap);
#endif
	startCommand->setFixedHeight(5 * startCommand->fontMetrics().height());
	startCommand->setFixedWidth(execAtStart->geometry().right() - startCommand->geometry().left());
	QWhatsThis::add(startCommand, execAtStartHelp);

	QString execAtEndHelp = "You can enter shell commands which will be "
		"executed when aquisition is completed.  If you are saving data to disk, the "
		"shell variable KTT_DATA_FILE "
		"will be set to the name of your data file (except when using the deprecated  "
		"binary format, in which case the extension .ts? is left off).  Also, KTT_PARAM_FILE "
		"will contain the name of the parameter file.";

	execAtEnd = new QCheckBox(rightColumn);
	execAtEnd->move(execAtStart->x(), startCommand->geometry().bottom() + vsep);
	execAtEnd->setText("Execute shell command at end of aquisition");
	execAtEnd->adjustSize();
	QWhatsThis::add(execAtEnd, execAtEndHelp);

	endCommand = new QMultiLineEdit(rightColumn);
	endCommand->move(execAtEnd->x() + hsep, execAtEnd->geometry().bottom() + vsep);
#if QT_VERSION >= 300
	endCommand->setWordWrap(QTextEdit::NoWrap);
#endif
	endCommand->setFixedHeight(5 * endCommand->fontMetrics().height());
	endCommand->setFixedWidth(startCommand->width());
	QWhatsThis::add(endCommand, execAtEndHelp);

	// done with second column

	fileFrame->adjustSize();

	leftColumn->adjustSize();
	leftColumn->move(hsep, fileFrame->geometry().bottom() + vsep);

	rightColumn->adjustSize();
	rightColumn->move(leftColumn->geometry().right() + 2 * hsep, leftColumn->y());

	adjustSize();

	okButton = new QPushButton("&OK", this);
	okButton->move(hsep, height() + vsep);
	okButton->adjustSize();

	defaultsButton = new QPushButton("&Defaults", this);
	defaultsButton->adjustSize();
	defaultsButton->move( (width() - defaultsButton->width()) / 2, okButton->y());

	cancelButton = new QPushButton("&Cancel", this);
	cancelButton->adjustSize();
	cancelButton->move(width() - cancelButton->width() - hsep , okButton->y());

	adjustSize();
	setFixedSize(size());
}

void KTraceDialog::slotSaveData(bool save)
{
	settings->saveData = save;

	// enable/disable widgets related to saving to file
	fileWidgetsSetEnabled(save);
}

void KTraceDialog::slotBrowse()
{
	QString browseDir;

	browseDir = QFileDialog::getExistingDirectory(settings->directory.path());

	if(browseDir)
		directory->setText(browseDir);
}

void KTraceDialog::slotDirectory(const QString &text)
{
	settings->directory = text;
}

void KTraceDialog::slotFileName(const QString &text)
{
	settings->fileStem = text;
}

void KTraceDialog::slotFileNum(int num)
{
	settings->fileNum = num;
}

void KTraceDialog::slotSaveType(int format)
{
	settings->saveType = (FileFormat) format;
}

void KTraceDialog::slotStartTrigger(const QString& trig)
{
	settings->startTrigger = adc::textToTrig(trig);
}

void KTraceDialog::slotScanTrigger(const QString& trig)
{
	settings->scanTrigger = adc::textToTrig(trig);
	refreshValues();
}

void KTraceDialog::slotConversionTrigger(const QString& trig)
{
	settings->conversionTrigger = adc::textToTrig(trig);
	refreshValues();
}

void KTraceDialog::slotStopTrigger(const QString& trig)
{
	settings->stopTrigger = adc::textToTrig(trig);
	refreshValues();
}

void KTraceDialog::slotNumChannels(int channels)
{
	settings->numChannels = channels;
	// refresh values because this may effect scan frequency
	refreshValues();
}

void KTraceDialog::slotScanFrequency(const QString &text)
{
	settings->scanFrequency = text.toFloat();
	// if scans are following conversions, update the conversion frequency too
	if(settings->scanTrigger == TRIG_FOLLOW &&
		settings->conversionTrigger == TRIG_TIMER)
	{
		settings->conversionFrequency = settings->scanFrequency * settings->numChannels;
		conversionFrequency->setText(QString::number(settings->conversionFrequency));
	}
}

void KTraceDialog::slotConversionFrequency(const QString &text)
{
	settings->conversionFrequency = text.toFloat();
}

void KTraceDialog::slotNumScans(int points)
{
	settings->numScans = points;
}

void KTraceDialog::slotRange(int rng)
{
	comedi_range *cRange = myADC->getRange(rng);
	if(cRange)
	{
		settings->rangeInfo = *cRange;
		settings->range = rng;
	}else
	{
		std::cerr << "bug! cRange null" << std::endl;
	}
}

void KTraceDialog::slotReference(const QString &reference)
{
	settings->reference = adc::textToRef(reference);
}

void KTraceDialog::slotRealTime(bool on)
{
	settings->realTime = on;
}

void KTraceDialog::slotLowLatency(bool on)
{
	settings->lowLatency = on;
}

void KTraceDialog::slotExecAtStart(bool exec)
{
	settings->execAtStart = exec;
	startCommand->setEnabled(settings->execAtStart);
}

void KTraceDialog::slotStartCommand()
{
	settings->startCommand = startCommand->text();;
}

void KTraceDialog::slotExecAtEnd(bool exec)
{
	settings->execAtEnd = exec;
	endCommand->setEnabled(settings->execAtEnd);
}

void KTraceDialog::slotEndCommand()
{
	settings->endCommand = endCommand->text();
}

void KTraceDialog::slotOk()
{
	okButton->setFocus();
	if(settings->directory.exists())
		accept();
	else
		QMessageBox::warning(this, "Error",
			"The save directory you specified does not exist.");
}

void KTraceDialog::slotDefaults()
{
	delete settings;
	settings = new KTTSettings;
	checkSettings();
	refreshValues();
}

