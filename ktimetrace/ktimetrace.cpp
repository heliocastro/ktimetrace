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

#include <iostream>

#include <QtWidgets>

#include "ktimetrace.h"

#include "dialog.h"
#include "bufferdialog.h"
#include "view.h"
#include "zoomdialog.h"
#include "ktracecolordialog.h"

KTraceApp::KTraceApp()
{
	createActions();
	createToolbars();
	createMenus();

	view = new KTraceView(this);
	view->loadConfig();
	setCentralWidget(view);

	data.ADC(&dataCard);
	data.loadConfig();
	data.setView(view);

	// look for an analog input device
	unsigned int i;
	for(i = 0; i < maxDev; i++)
	{
		if(data.setDevice(i) == 0)
		{
			dynamic_cast<QAction*>(devicesActionList[i])->setChecked(true);
			view->fullScale(dataCard.maxData());
			break;
		}
	}
	// if we failed to find any good analog inputs
	if(i == maxDev)
	{
		static QMessageBox warning("Warning",
			"No available asynchronous analog input subdevices were found.\n"
			"KTimeTrace will not function.", QMessageBox::Warning,
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
			this);
		QTimer::singleShot(0, &warning , SLOT( show() ) );
	}

	// // initialize state of various menu options and toolbar buttons
	// applyMainWindowSettings(kapp->config(), "window");
	setControlsEnabled(false);

	// set timer up to update status led once per second
	statusTimer = new QTimer(this);
	statusTimer->start(1000);
	connect(statusTimer, SIGNAL(timeout()), SLOT(slotUpdateLED()));
}

void KTraceApp::createActions() {
	startTraceAction = new QAction(QIcon(":/images/start.png"), tr("start"), this);
	startTraceAction->setStatusTip("Start a time trace");
	connect(startTraceAction, SIGNAL(triggered()), this, SLOT(startTrace()));

	stopTraceAction = new QAction(QIcon(":/images/stop.png"), tr("stop"), this);
	stopTraceAction->setStatusTip("Stop a time trace");
	connect(stopTraceAction, SIGNAL(triggered()), this, SLOT(stopTrace()));

	aboutAction = new QAction(tr("&About..."), this);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(slotAbout()));

	whatsThisAction = new QAction(tr("What's &This"), this);
	connect(whatsThisAction, SIGNAL(triggered()), this, SLOT(WhatsThis()));

	colorsAction = new QAction(tr("&Colors"), this);
	connect(colorsAction, SIGNAL(triggered()), this, SLOT(slotColors()));

	zoomAction = new QAction(tr("&Zoom..."), this);
	connect(zoomAction, SIGNAL(triggered()), this, SLOT(slotZoom()));

	exitAction = new QAction(tr("E&xit"), this);
	connect(exitAction, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

	for(int dev=0; dev <= 3; dev++) {
		QString name = "/dev/comedi" + QString::number(dev);
		QAction *act = new QAction(name, this);
		act->setCheckable(true);
		connect(act, SIGNAL(toggled(bool)), this, SLOT(commandCallback()));
		devicesActionList.append(act);
	}

	bufferAction = new QAction("Comedi &Buffer...", this);
	connect(bufferAction, SIGNAL(triggered()), this, SLOT(slotBuffer()));
}

void KTraceApp::createToolbars() {
	mainToolBar = new QToolBar(this);
	addToolBar(Qt::LeftToolBarArea, mainToolBar);
	mainToolBar->addAction(startTraceAction);
	mainToolBar->addAction(stopTraceAction);
}

void KTraceApp::createMenus() {
	controlMenu = menuBar()->addMenu(tr("&Control"));
	// controlMenu->insertItem(KGlobal::iconLoader()->loadIcon("start", KIcon::Small), "Start aquisition", ID_START);
	// controlMenu->insertItem(KGlobal::iconLoader()->loadIcon("stop", KIcon::Small), "&Interrupt aquisition", ID_STOP);
	controlMenu->addSeparator();
	controlMenu->addAction(exitAction);

	settingsMenu = menuBar()->addMenu(tr("&Settings"));

	deviceMenu = settingsMenu->addMenu(tr("&Ports"));
	for(int i = 0; i < devicesActionList.size(); ++i) {
		deviceMenu->addAction(devicesActionList[i]);
	}

	settingsMenu->addAction(bufferAction);

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(colorsAction);
	viewMenu->addAction(zoomAction);

	myHelpMenu = menuBar()->addMenu(tr("&Help"));
	myHelpMenu->addAction(whatsThisAction);
	myHelpMenu->addSeparator();
	myHelpMenu->addAction(aboutAction);
}

void KTraceApp::defaultCaption()
{
	// const KTTSettings *settings = &data.settings;

	// QString caption = "";
	// caption += PACKAGE;
	// caption += " ";
	// caption += VERSION;
	// caption += " : ";
	// if(settings->saveData)
	// {
	// 	caption += settings->directory.absPath();
	// 	caption += '/';
	// 	QString fileName;
	// 	fileName.sprintf("%s%.3i", (const char*)settings->fileStem, settings->fileNum);
	// 	caption += fileName;
	// } else
	// {
	// 	caption += "no file";
	// }
	// setCaption(caption);
}

bool KTraceApp::queryClose()
{
	if(data.writeDone() == false)
	{
		data.stop();
		QMessageBox::information(this, "Information",
			"Writing of data files is still in progress.\n"
			"This may cause a delay before the application closes.");
	}

	return true;
}

void KTraceApp::queryExit()
{
	// save configuration
	saveConfig();

	// wait until all file writing threads have finished
	if(data.writeDone() == false)
	{
		std::cerr << "waiting for data file save to complete..." << std::endl;
		while(data.writeDone() == false)
		{
			usleep(100000);
		}
		std::cerr << "data save completed" << std::endl;
	}

	QCoreApplication::quit();
}

void KTraceApp::startTrace()
{
	if(dataCard.good() == false)
	{
		QMessageBox::warning(this, "Warning",
			"No analog input device is configured");
		return;
	}
	KTraceDialog *dialog = new KTraceDialog(data.settings, &dataCard, this);
	if(dialog->exec())
	{
		setControlsEnabled(true);
		data.settings = *dialog->settings;
		defaultCaption();
		data.collect();
	}
	delete dialog;
}

void KTraceApp::stopTrace()
{
	data.stop();
}

// void KTraceApp::commandCallback(int id)
// {
// 	switch(id)
// 	{
// 		case ID_COMEDI_0:
// 		case ID_COMEDI_1:
// 		case ID_COMEDI_2:
// 		case ID_COMEDI_3:
// 			setDevice(deviceMenu->indexOf(id));
// 			break;
// 		default:
// 			break;
// 	}
// }

void KTraceApp::slotBuffer()
{
	if(dataCard.good() == false)
	{
		QMessageBox::warning(this, "Warning",
			"No analog input device is configured");
		return;
	}

	BufferDialog *dialog = new BufferDialog(&dataCard, this);
	dialog->exec();
}

void KTraceApp::slotZoom()
{
	ZoomDialog *dialog = new ZoomDialog(this);

	// initialize dialog with current zoom values
	dialog->hZoom(view->hZoom());
	if(dialog->exec())
	{
		// write new zoom value
		view->hZoom(dialog->hZoom());
	}

	if(dialog) {
		delete dialog;
	}
}

void KTraceApp::slotColors()
{
	KTraceColorDialog *dialog = new KTraceColorDialog(this);
	dialog->fgColor(view->fgColor());
	dialog->bgColor(view->bgColor());
	if(dialog->exec())
	{
		view->fgColor(dialog->fgColor());
		view->bgColor(dialog->bgColor());
	}
	if(dialog) {
		delete dialog;
	}
}

void KTraceApp::slotAbout()
{
	QMessageBox::about(this, "About", "KTimeTrace 0.3.0\n"
		"Home page: http://ktimetrace.sourceforge.net\n"
		"Author: Frank Mori Hess <fmhess@users.sourceforge.net>\n"
		"Author: Helio Chissini de Castro <helio@kde.org>");
}

void KTraceApp::slotUpdateLED()
{
	pthread_mutex_lock(&aquisitionThreadCountLock);
	view->setWriteIndicator(aquisitionThreadCount);
	pthread_mutex_unlock(&aquisitionThreadCountLock);

	view->setDeviceIndicator(dataCard.good());
}

void KTraceApp::setControlsEnabled(bool go)
{
	// toolBar()->setItemEnabled(ID_START, !go);
	// toolBar()->setItemEnabled(ID_STOP, go);
	// toolBar()->setEnableContextMenu(!go);
	// controlMenu->setItemEnabled(ID_START, !go);
	// controlMenu->setItemEnabled(ID_STOP, go);
	// controlMenu->setItemEnabled(ID_QUIT, !go);
	// menuBar()->setItemEnabled(ID_DEVICE_MENU, !go);
	// menuBar()->setItemEnabled(ID_VIEW_MENU, !go);
	// menuBar()->setItemEnabled(ID_HELP_MENU, !go);
}

int KTraceApp::setDevice(unsigned int devNum)
{
	QString devName;

	if(devNum >= maxDev)
	{
		return -1;
	}
	// uncheck devices in devMenu
	for(unsigned int i = 0; i < maxDev; i++)
		dynamic_cast<QAction*>(devicesActionList[i])->setChecked(false);

	if(data.setDevice(devNum) == 0)
	{
		dynamic_cast<QAction*>(devicesActionList[devNum])->setChecked(true);
		view->fullScale(dataCard.maxData());
	}else
	{
		devName.setNum(devNum);
		devName = "/dev/comedi" + devNum;
		QMessageBox::warning(this, "Warning",
			"No asynchronous analog input subdevice was found\n"
			"on " + devName + ".  Choose a different device.");
		return -1;
	}
	return devNum;
}

void KTraceApp::saveConfig()
{
	// save configuration
	//saveMainWindowSettings("window");
	data.saveConfig();
	view->saveConfig();
}
