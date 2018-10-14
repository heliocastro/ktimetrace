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

#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QFrame>
#include <QMessageBox>
#include <QTextEdit>
#include <QFileDialog>

#include <cmath>
#include <cassert>

#include "ktimetrace.h"
#include "dialog.h"

KTraceDialog::KTraceDialog(KTTSettings oldSettings, adc *currentADC,
	QWidget *parent) : QDialog(parent)
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
	int item = 0;

	saveData->setChecked(settings->saveData);
	fileWidgetsSetEnabled(settings->saveData);
	directory->setText(settings->directory.absolutePath());
	fileName->setText(settings->fileStem);
	fileNum->setValue(settings->fileNum);
	saveType->setCurrentIndex(settings->saveType);

	item = startTrigger->findText(adc::trigToText(settings->startTrigger));
	if(item >= 0)
		startTrigger->setCurrentIndex(item);

	item = scanTrigger->findText(adc::trigToText(settings->scanTrigger));
	if(item >= 0)
		scanTrigger->setCurrentIndex(item);

	item = conversionTrigger->findText(adc::trigToText(settings->conversionTrigger));
	if(item >= 0)
		conversionTrigger->setCurrentIndex(item);

	item = stopTrigger->findText(adc::trigToText(settings->stopTrigger));
	if(item >= 0)
		stopTrigger->setCurrentIndex(item);

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

	range->setCurrentIndex(settings->range);

	item = reference->findText(adc::refToText(settings->reference));
	if(item >=0)
		reference->setCurrentIndex(item);

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
	boardDesc->setTextFormat(Qt::RichText);
	boardDesc->resize(boardDesc->fontMetrics().size(0, string1));
	boardDesc->setText(string1);

	saveData = new QCheckBox(this);
	saveData->setText("Save analog input data to file");
	saveData->adjustSize();
	saveData->move(hsep, boardDesc->geometry().bottom() + vsep);

	fileFrame = new QFrame(this);
	fileFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	fileFrame->move(saveData->x() + hsep, saveData->geometry().bottom() + vsep);

	browse = new QPushButton("&Browse...", fileFrame);
	browse->move(10, 10);
	browse->adjustSize();

	directory = new QLineEdit(fileFrame);
	directory->setGeometry(browse->geometry().right() + hsep, browse->y(), 250, directory->sizeHint().height());
	directory->setWhatsThis("Directory where data will be saved");

	directoryLabel = new QLabel(fileFrame);
	directoryLabel->move(directory->geometry().right() + hsep, directory->y());
	directoryLabel->setText("Directory");
	directoryLabel->adjustSize();

	fileName = new QLineEdit(fileFrame);
	fileName->move(browse->x(), browse->geometry().bottom() + vsep);
	fileName->adjustSize();
	fileName->setWhatsThis(
		"The file stem of the saved data file.  The file number "
		"and a file type extension will be appended");

	fileNameLabel = new QLabel(fileFrame);
	fileNameLabel->setText("File stem");
	fileNameLabel->move(fileName->geometry().right() + hsep, fileName->y());
	fileNameLabel->adjustSize();

	fileNum = new QSpinBox(fileFrame);
	fileNum->move(fileName->x(), fileName->geometry().bottom() + vsep);
	fileNum->setRange(0, 999);
	fileNum->adjustSize();
	fileNum->setWhatsThis("The file number.  It is appended to the file stem");

	fileNumLabel = new QLabel(fileFrame);
	fileNumLabel->move(fileNum->geometry().right() + hsep, fileNum->y());
	fileNumLabel->setText("File number");
	fileNumLabel->adjustSize();

	saveType = new QComboBox(fileFrame);
	saveType->move(fileNum->x(), fileNum->geometry().bottom() + vsep);
	saveType->addItem("Tab delimited text", TEXT);
	saveType->addItem("Gzipped text", GZ_TEXT);
	saveType->addItem("True binary (deprecated)", BINARY);
	saveType->adjustSize();
	saveType->setWhatsThis(
		"Format for the saved data file.  'True binary' was included "
		"for backward compatiblity with my lab's previous data taking software.  It stores "
		"data as an array of 16 bit integers (signed or unsigned for bipolar/unipolar ranges) "
		"with little endian byte ordering.");

	saveTypeLabel = new QLabel(fileFrame);
	saveTypeLabel->move(saveType->geometry().right() + hsep, saveType->y());
	saveTypeLabel->setText("Save File Format");
	saveTypeLabel->adjustSize();

	leftColumn = new QWidget(this);

	startTrigger = new QComboBox(leftColumn);
	startTrigger->move(0, 0);
	if(myADC->startTriggerMask() & TRIG_NOW)
		startTrigger->addItem(adc::trigToText(TRIG_NOW));
	if(myADC->startTriggerMask() & TRIG_EXT)
		startTrigger->addItem(adc::trigToText(TRIG_EXT));
	startTrigger->adjustSize();
	// disable combo box if it has only one option
	if(startTrigger->count() < 2)
		startTrigger->setEnabled(false);
	startTrigger->setWhatsThis("The trigger which begins data aquisition");

	startTriggerLabel = new QLabel(leftColumn);
	startTriggerLabel->move(startTrigger->geometry().right() + hsep, startTrigger->y());
	startTriggerLabel->setText("Start Trigger");
	startTriggerLabel->adjustSize();

	numChannels = new QSpinBox(leftColumn);
	numChannels->move(startTrigger->x(), startTrigger->geometry().bottom() + vsep);
	numChannels->setRange(1, myADC->numChannels());
	numChannels->adjustSize();
	numChannels->setWhatsThis("The number of channels to be read for each scan");

	numChannelsLabel = new QLabel(leftColumn);
	numChannelsLabel->move(numChannels->geometry().right() + hsep, numChannels->y());
	numChannelsLabel->setText("Number of channels");
	numChannelsLabel->adjustSize();

	scanTrigger = new QComboBox(leftColumn);
	scanTrigger->move(numChannels->x(), numChannels->geometry().bottom() + vsep);
	if(myADC->scanTriggerMask() & TRIG_TIMER)
		scanTrigger->addItem(adc::trigToText(TRIG_TIMER));
	if(myADC->scanTriggerMask() & TRIG_FOLLOW)
		scanTrigger->addItem(adc::trigToText(TRIG_FOLLOW));
	if(myADC->scanTriggerMask() & TRIG_EXT)
		scanTrigger->addItem(adc::trigToText(TRIG_EXT));
	scanTrigger->adjustSize();
	// disable combo box if it has only one option
	if(scanTrigger->count() < 2)
		scanTrigger->setEnabled(false);
	scanTrigger->setWhatsThis(
		"The trigger which starts a scan.  The '" +
		adc::trigToText(TRIG_FOLLOW) + "' trigger makes scans follow the "
		"conversion trigger.");

	scanTriggerLabel = new QLabel(leftColumn);
	scanTriggerLabel->move(scanTrigger->geometry().right() + hsep, scanTrigger->y());
	scanTriggerLabel->setText("Scan Trigger");
	scanTriggerLabel->adjustSize();

	scanFrequency = new QLineEdit(leftColumn);
	scanFrequency->move(scanTrigger->x(), scanTrigger->geometry().bottom() + vsep);
	scanFrequency->adjustSize();
	scanFrequency->setWhatsThis(
		"Determines the frequency with which scans are taken "
		"when based on a '" + adc::trigToText(TRIG_TIMER) + "' trigger");

	scanFrequencyLabel = new QLabel(leftColumn);
	scanFrequencyLabel->move(scanFrequency->geometry().right() + hsep, scanFrequency->y());
	scanFrequencyLabel->setText("Scan Frequency (Hz)");
	scanFrequencyLabel->adjustSize();

	conversionTrigger = new QComboBox(leftColumn);
	conversionTrigger->move(scanFrequency->x(), scanFrequency->geometry().bottom() + vsep);
	if(myADC->conversionTriggerMask() & TRIG_TIMER)
		conversionTrigger->addItem(adc::trigToText(TRIG_TIMER));
	if(myADC->conversionTriggerMask() & TRIG_NOW)
		conversionTrigger->addItem(adc::trigToText(TRIG_NOW));
	if(myADC->conversionTriggerMask() & TRIG_EXT)
		conversionTrigger->addItem(adc::trigToText(TRIG_EXT));

	conversionTrigger->adjustSize();
	// disable combo box if it has only one option
	if(conversionTrigger->count() < 2)
		conversionTrigger->setEnabled(false);
	conversionTrigger->setWhatsThis("The trigger which starts an individual conversion within "
		"a scan.  The '" + adc::trigToText(TRIG_NOW) + "' trigger means to complete the scan "
		"as quickly as possible.");

	conversionTriggerLabel = new QLabel("Conversion Trigger", leftColumn);
	conversionTriggerLabel->move(conversionTrigger->geometry().right() + hsep, conversionTrigger->y());
	conversionTriggerLabel->adjustSize();

	conversionFrequency = new QLineEdit(leftColumn);
	conversionFrequency->move(conversionTrigger->x(), conversionTrigger->geometry().bottom() + vsep);
	conversionFrequency->adjustSize();
	conversionFrequency->setWhatsThis("Determines the frequency with which conversions are made "
		"when the conversion trigger is '" + adc::trigToText(TRIG_TIMER) + "'.  "
		"May be overriden by scan frequency if scan trigger is '"+ adc::trigToText(TRIG_FOLLOW) +"'");

	conversionFrequencyLabel = new QLabel(leftColumn);
	conversionFrequencyLabel->move(conversionFrequency->geometry().right() + hsep, conversionFrequency->y());
	conversionFrequencyLabel->setText("Conversion Frequency (Hz)");
	conversionFrequencyLabel->adjustSize();

	stopTrigger = new QComboBox(leftColumn);
	stopTrigger->move(conversionFrequency->x(), conversionFrequency->geometry().bottom() + vsep);
	//stopTriggerList = new QListBox(leftColumn);
	if(myADC->stopTriggerMask() & TRIG_COUNT)
		stopTrigger->addItem(adc::trigToText(TRIG_COUNT));
	if(myADC->stopTriggerMask() & TRIG_NONE)
		stopTrigger->addItem(adc::trigToText(TRIG_NONE));
	if(myADC->stopTriggerMask() & TRIG_EXT)
		stopTrigger->addItem(adc::trigToText(TRIG_EXT));
	stopTrigger->adjustSize();
	// disable combo box if it has only one option
	if(stopTrigger->count() < 2)
		stopTrigger->setEnabled(false);
	stopTrigger->setWhatsThis("The trigger which ends data aquisition.");

	stopTriggerLabel = new QLabel("Stop Trigger", leftColumn);
	stopTriggerLabel->move(stopTrigger->geometry().right() + hsep, stopTrigger->y());
	stopTriggerLabel->adjustSize();

	numScans = new QSpinBox(leftColumn);
	numScans->move(stopTrigger->x(), stopTrigger->geometry().bottom() + vsep);
	numScans->setRange(1, INT_MAX);
	numScans->adjustSize();
	numScans->setWhatsThis("The total number of scans to perform");

	numScansLabel = new QLabel(leftColumn);
	numScansLabel->move(numScans->geometry().right() + hsep, numScans->y());
	numScansLabel->setText("Number of scans");
	numScansLabel->adjustSize();

	range = new QComboBox(leftColumn);
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
		range->addItem(string1, i);
	}
	range->adjustSize();
	// disable combo box if it has only one option
	if(range->count() < 2)
		range->setEnabled(false);
	range->setWhatsThis("The desired input range for your daq card");

	rangeLabel = new QLabel(leftColumn);
	rangeLabel->move(range->geometry().right() + hsep, range->y());
	rangeLabel->setText("Analog input range");
	rangeLabel->adjustSize();

	unsigned int subdevFlags = myADC->subdevFlags();
	reference = new QComboBox(leftColumn);
	reference->move(range->x(), range->geometry().bottom() + vsep);
	if(subdevFlags & SDF_GROUND)
		reference->addItem(adc::refToText(AREF_GROUND));
	if(subdevFlags & SDF_COMMON)
		reference->addItem(adc::refToText(AREF_COMMON));
	if(subdevFlags & SDF_DIFF)
		reference->addItem(adc::refToText(AREF_DIFF));
	if(subdevFlags & SDF_OTHER)
		reference->addItem(adc::refToText(AREF_OTHER));
	if(reference->count() == 0)
		reference->addItem("driver bug: no supported analog references specified");
	reference->adjustSize();
	// disable combo box if it has only one option
	if(reference->count() < 2)
		reference->setEnabled(false);
	reference->setWhatsThis(
		"The desired input reference for your daq card.  "
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
	realTime->setWhatsThis(
		"Request for the comedi driver to use an interrupt handler with hard "
		"real-time priority. It will have no effect unless you are running a real-time kernel "
		"(rtlinux or rtai) and have compiled comedi with real-time support.  "
		"This option can be useful if you are "
		"taking data at high frequency or have a card with a small onboard fifo.");

	lowLatency = new QCheckBox(rightColumn);
	lowLatency->move(realTime->x(), realTime->geometry().bottom() + vsep);
	lowLatency->setText("Request low latency mode");
	lowLatency->adjustSize();
	lowLatency->setWhatsThis(
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
	execAtStart->setWhatsThis(execAtStartHelp);

	startCommand = new QTextEdit(rightColumn);
	startCommand->move(execAtStart->x() + hsep, execAtStart->geometry().bottom() + vsep);
#if QT_VERSION >= 300
	startCommand->setWordWrapMode(QTextOption::NoWrap);
#endif
	startCommand->setFixedHeight(5 * startCommand->fontMetrics().height());
	startCommand->setFixedWidth(execAtStart->geometry().right() - startCommand->geometry().left());
	startCommand->setWhatsThis(execAtStartHelp);

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
	execAtEnd->setWhatsThis(execAtEndHelp);

	endCommand = new QTextEdit(rightColumn);
	endCommand->move(execAtEnd->x() + hsep, execAtEnd->geometry().bottom() + vsep);
#if QT_VERSION >= 300
	endCommand->setWordWrapMode(QTextOption::NoWrap);
#endif
	endCommand->setFixedHeight(5 * endCommand->fontMetrics().height());
	endCommand->setFixedWidth(startCommand->width());
	endCommand->setWhatsThis(execAtEndHelp);

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

	browseDir = QFileDialog::getExistingDirectory(this, "Settings", settings->directory.path());

	if(!browseDir.isEmpty())
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
	settings->startCommand = startCommand->toPlainText();;
}

void KTraceDialog::slotExecAtEnd(bool exec)
{
	settings->execAtEnd = exec;
	endCommand->setEnabled(settings->execAtEnd);
}

void KTraceDialog::slotEndCommand()
{
	settings->endCommand = endCommand->toPlainText();
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

