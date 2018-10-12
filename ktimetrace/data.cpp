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

#include "data.h"
#include "ktimetrace.h"

#include <kapp.h>

#include <qmessagebox.h>

#include <iostream>
#include <fstream>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static void* aquisitionThreadFunction(void*);

KTTEngine::KTTEngine() : QObject()
{
	go = false;
	errorString.truncate(0);
	myADC = 0;
	pthread_cond_init(&aquisitionReady, 0);
	pthread_mutex_init(&waitMutex, 0);
	pthread_mutex_init(&dataFifoMutex, 0);
	pthread_mutex_init(&goMutex, 0);
	pthread_mutex_init(&errorStringMutex, 0);
	aquisitionThread = 0;

	plotTimer = new QTimer(this);
	connect(plotTimer, SIGNAL(timeout()), SLOT(slotUpdatePlot()));
}

KTTEngine::~KTTEngine()
{
}

adc* KTTEngine::ADC(adc *card)
{
	if(card == 0)
		return 0;

	myADC = card;

	return myADC;
}

int KTTEngine::setDevice(unsigned int deviceNumber)
{
	if(myADC == NULL)
		return -1;

	int ret = myADC->setDevice(deviceNumber);
	if(ret != 0)
		return ret;

	// check current settings against new adc device
	settings.startTrigger = adc::defaultTrigger(myADC->startTriggerMask(), settings.startTrigger);
	settings.scanTrigger = adc::defaultTrigger(myADC->scanTriggerMask(), settings.scanTrigger);
	settings.conversionTrigger = adc::defaultTrigger(myADC->conversionTriggerMask(), settings.conversionTrigger);
	if(settings.range >= myADC->getNumRanges())
		settings.range = 0;
	int channelLimit = myADC->numChannels();
	if(channelLimit > 0 &&
		settings.numChannels > (unsigned int) channelLimit)
	{
		settings.numChannels = channelLimit;
	}
	settings.reference = adc::defaultReference(myADC->subdevFlags(), settings.reference);

	return 0;
}

void KTTEngine::collect()
{
	QString prefix, errorMessage;
	int retval;

	// make sure this function isn't executed while data aquisition is already in progress
	pthread_mutex_lock(&myADC->mutex);

	setGo(true);

	view->blockMode(!settings.lowLatency);
	view->createWindows(settings.numChannels);
	// set progress bar to zero
	view->setProgress(0);
	numPointsPlotted = 0;
	// empty dataFifo
	pthread_mutex_lock(&dataFifoMutex);
	dataFifo.clear();
	pthread_mutex_unlock(&dataFifoMutex);
	// update plot with 50 millisecond period
	plotTimer->start(50);

	settings.numScansCompleted = 0;

	// prepare for starting data aquisition
	myADC->setRange(settings.range);
	myADC->setARef(settings.reference);
	myADC->startTrigger(settings.startTrigger);
	myADC->scanTrigger(settings.scanTrigger);
	myADC->scanFrequency(settings.scanFrequency);
	myADC->conversionTrigger(settings.conversionTrigger);
	myADC->conversionFrequency(settings.conversionFrequency);
	myADC->stopTrigger(settings.stopTrigger);
	myADC->useRT(settings.realTime);
	myADC->useLowLatency(settings.lowLatency);

	// execute user specified shell command if requested
	prefix = shellPrefix(settings);
	if(settings.execAtStart)
	{
		retval = system(prefix + settings.startCommand);
		if(retval)
		{
			errorMessage = "nonzero system() return value: " + QString::number(retval) + "  at start of aquisition";

			QMessageBox::warning(view, "Aquisition error", errorMessage);
			pthread_mutex_unlock(&myADC->mutex);
			return;
		}
	}

	retval = myADC->start(settings.numScans , settings.numChannels - 1);
	if(retval != 0)
	{
		switch(retval)
		{
			case 1:
				errorMessage = "One or more selected triggers are unsupported by the driver.";
				break;
			case 2:
				errorMessage = "The selected triggers are incompatible with each other.";;
				break;
			case 3:
				errorMessage = "One or more command arguments are out of range.";
				break;
			case 4:
				errorMessage = "Incompatible command arguments.";
				break;
			case 5:
				errorMessage = "Driver does not support your channel/gain list.";
				break;
			default:
				errorMessage = "Error starting aquisition.  Check kernel logs or dmesg.";
				break;
		}
		QMessageBox::warning(view, "Aquisition error", errorMessage);
		pthread_mutex_unlock(&myADC->mutex);
		return;
	}
	settings.scanFrequency = myADC->scanFrequency();
	settings.conversionFrequency = myADC->conversionFrequency();

	// release board mutex since aquisition thread is going to lock it
	pthread_mutex_unlock(&myADC->mutex);

	// start data aquisition thread

	// increment count of number of aquisition threads running
	pthread_mutex_lock(&aquisitionThreadCountLock);
	aquisitionThreadCount++;
	// set LED displays according to how many files are being written
	view->setWriteIndicator(aquisitionThreadCount);
	pthread_mutex_unlock(&aquisitionThreadCountLock);

	/* lock waitMutex to make sure aquisition thread does not call
	 * pthread_cond_signal() prior to this thread calling pthread_cond_wait() */
	pthread_mutex_lock(&waitMutex);
	/* create aquisition thread */
	pthread_create(&aquisitionThread, 0, aquisitionThreadFunction, this);
	// detach thread so it deallocates its resources on termination
	pthread_detach(aquisitionThread);
	// wait until aquisition thread has gotten all the information it needs
	pthread_cond_wait(&aquisitionReady, &waitMutex);
	pthread_mutex_unlock(&waitMutex);
	// increment file number for next time;
	if(settings.saveData)
		settings.fileNum++;
}

void KTTEngine::stop()
{
	KTraceApp *mainWindow = (KTraceApp*) kapp->mainWidget();

	setGo(false);

	plotTimer->stop();
	mainWindow->setControlsEnabled(false);
}

void KTTEngine::setView(KTraceView *viewPtr)
{
	view = viewPtr;
}

void KTTEngine::saveConfig()
{
	settings.saveConfig();
}

void KTTEngine::loadConfig()
{
	settings.loadConfig();
}

void KTTEngine::aquireData()
{
	int ret = 0;
	AcqBuffer *aBuf = 0;
	KTTSettings localSettings = settings;
	ssize_t bytes;
	unsigned int samples;
	static const int bufferSize = 4096;
	static sampl_t buffer[bufferSize];
	unsigned int channel = 0;
	QString prefix;

	/* we have made local copy of settings, so signal GUI thread
	 * it is okay to continue */
	pthread_mutex_lock(&waitMutex);
	pthread_cond_signal(&aquisitionReady);
	pthread_mutex_unlock(&waitMutex);

	aBuf = new AcqBuffer(myADC);
	// enable / disable writing to disk
	ret = aBuf->saveToDisk(localSettings.saveData);
	{
		if(ret < 0)
			exit(1);
	}

	// lock access to board
	pthread_mutex_lock(&myADC->mutex);

	while(getGo())
	{
		// get data from adc
		bytes = aBuf->read(buffer, bufferSize * sizeof(buffer[0]));
		// loop while waiting for data to be ready
		while(bytes < 0 && errno == EAGAIN)
		{
			// interrupt aquisition if stop buttin was pressed
			if(getGo() == false) goto cleanup;
			// try polling card if we are in low latency mode
			if(settings.lowLatency)
			{
				if(myADC->poll() > 0)
					bytes = aBuf->read(buffer, bufferSize * sizeof(buffer[0]));
			}
			// if there is still no data we might as well sleep
			if(bytes < 0)
			{
				usleep(10000);
				// look for data again
				bytes = aBuf->read(buffer, bufferSize * sizeof(buffer[0]));
			}
		}
		// check for error
		if(bytes < 0)
		{
			setErrorString("Unexpected end of data, check dmesg or system logs for comedi errors.");
			setGo(false);
			goto cleanup;
		}
		// check for aquisition completed
		if(bytes == 0)
		{
			goto cleanup;
		}
		// move data into dataFifo
		samples = bytes / sizeof(buffer[0]);
		pthread_mutex_lock(&dataFifoMutex);
		for(unsigned int i = 0; i < samples; i++)
		{
			dataFifo.push_back(buffer[i]);
		}
		pthread_mutex_unlock(&dataFifoMutex);
		// increment count of number of scans completed
		localSettings.numScansCompleted += (samples + channel) / localSettings.numChannels ;
		// remember how far through current scan we are
		channel = (samples + channel) % localSettings.numChannels ;
	}

cleanup:
	// if aquisition was interrupted, stop hardware
	if(getGo() == false) myADC->stop();
	// set go false so GUI thread knows aquisition is done
	setGo(false);
	// make sure we don't unlock an already unlocked mutex (don't know if this actually matters)
	pthread_mutex_trylock(&myADC->mutex);
	// unlock mutexes
	pthread_mutex_unlock(&myADC->mutex);
	// write data files if appropriate
	if(localSettings.saveData && localSettings.numScansCompleted)
	{
		writeFiles(localSettings, aBuf);
	}
	// execute user shell command at end of aquisition
	prefix = shellPrefix(localSettings);
	if(localSettings.execAtEnd)
	{
		ret = system(prefix + localSettings.endCommand);
		if(ret)
		{
			std::cerr << "nonzero system() return value: " << ret << "  at end of aquisition" << std::endl;
		}
	}
	if(aBuf)
		delete aBuf;

	// decrement count of number of aquisition threads running
	pthread_mutex_lock(&aquisitionThreadCountLock);
	aquisitionThreadCount--;
	pthread_mutex_unlock(&aquisitionThreadCountLock);
}

int KTTEngine::writeFiles(KTTSettings localSettings, AcqBuffer *aBuf)
{
	QString fileName, paramFileName;
	std::ofstream paramFile;
	int ret = 0;
	time_t now;
	/* this mutex exists to serialize file writing, when more than one thread calls
	 * this function at once */
	static pthread_mutex_t writeFilesMutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&writeFilesMutex);

	fileName = localSettings.fullFileStem();
	// write parameter file containing useful info
	paramFileName = fileName + ".ts-par";
	paramFile.open(paramFileName);
	if(!paramFile)
	{
		std::cerr << "error opening parameter save file" << std::endl;
		ret = -1;
		goto cleanup;
	}
	paramFile << "[info]" << std::endl;
	paramFile << "program=" << PACKAGE << std::endl;
	paramFile << "version=" << VERSION << std::endl;
	paramFile << "filename=" << fileName << std::endl;
	paramFile << "hardware=" << localSettings.boardName << std::endl;
	now = time(NULL);
	paramFile << "time stamp=" << asctime(localtime(&now));
	paramFile << "[settings]" << std::endl;
	paramFile << "scan trigger=" << adc::trigToText(localSettings.scanTrigger) << std::endl;
	if(localSettings.scanTrigger == TRIG_TIMER ||
		(localSettings.conversionTrigger == TRIG_TIMER &&
		localSettings.scanTrigger == TRIG_FOLLOW))
		paramFile << "scan frequency=" << localSettings.scanFrequency << std::endl;
	paramFile << "conversion trigger=" << adc::trigToText(localSettings.conversionTrigger) << std::endl;
	if(localSettings.conversionTrigger == TRIG_TIMER)
		paramFile << "conversion frequency=" << localSettings.conversionFrequency << std::endl;
	paramFile << "stop trigger=" << adc::trigToText(localSettings.stopTrigger) << std::endl;
	if(localSettings.stopTrigger == TRIG_COUNT)
		paramFile << "scans requested=" << localSettings.numScans << std::endl;
	paramFile << "scans completed=" << localSettings.numScansCompleted << std::endl;
	paramFile << "number of channels=" << localSettings.numChannels << std::endl;
	paramFile << "input range=" << localSettings.rangeInfo.min << "," 
		<< localSettings.rangeInfo.max << std::endl;
	switch(localSettings.rangeInfo.unit)
	{
		case UNIT_volt:
			paramFile << "input unit=volt" << std::endl;
			break;
		case UNIT_mA:
			paramFile << "input unit=mA" << std::endl;
			break;
		default:
			paramFile << "input unit=unknown" << std::endl;
			break;
	}
	paramFile << "input reference=" << adc::refToText(localSettings.reference) << std::endl;
	if(paramFile.good() == false)
	{
		std::cerr << "error writing parameter file" << std::endl;
		paramFile.close();
		ret = -1;		
		goto cleanup;
	}
	paramFile.close();		

	// write data files
	fileName = localSettings.fullFileName();
	switch(localSettings.saveType)
	{
		case TEXT:
			ret = aBuf->writeTextFile(fileName, localSettings.numChannels);
			break;
		case GZ_TEXT:
			ret = aBuf->writeGZTextFile(fileName, localSettings.numChannels);
			break;
		case BINARY:
			ret = aBuf->writeBinaryFiles(fileName, localSettings.numChannels);
			break;
		default:
			std::cerr << "save type not specified" << std::endl;
			ret = -1;
			goto cleanup;
			break;
	}

cleanup:

	pthread_mutex_unlock(&writeFilesMutex);

	return ret;
}

bool KTTEngine::writeDone()
{
	bool ret;

	pthread_mutex_lock(&aquisitionThreadCountLock);
	if(aquisitionThreadCount)
		ret = false;
	else
		ret = true;
	pthread_mutex_unlock(&aquisitionThreadCountLock);

	return ret;
}

void KTTEngine::slotUpdatePlot()
{
	unsigned int channel;
	KTraceApp *mainWindow = (KTraceApp*) kapp->mainWidget();
	QString errorMessage;

	pthread_mutex_lock(&dataFifoMutex);
	if(dataFifo.empty() == false)
	{
		// plot any new points
		while(dataFifo.empty() == false)
		{
			channel =  numPointsPlotted % settings.numChannels;
			view->input(dataFifo[0], channel);
			dataFifo.pop_front();
			numPointsPlotted++;
		}
		pthread_mutex_unlock(&dataFifoMutex);

		// update progress bar
		if(settings.stopTrigger == TRIG_COUNT)
			view->setProgress((int)((100.0 * (numPointsPlotted)) / (settings.numScans * settings.numChannels)));
	}else
		pthread_mutex_unlock(&dataFifoMutex);
	
	// check if aquisition is over
	if(getGo() == false)
	{
		view->flush();
		mainWindow->setControlsEnabled(false);
		plotTimer->stop();
	}

	// do actual plotting
	view->updatePlots();

	// check for errors and display them
	errorMessage = getErrorString();
	if(errorMessage.isEmpty() == false)
	{
		QMessageBox::warning(view, "Aquisition error", errorMessage);	
	}
}

void KTTEngine::setErrorString(QString newError)
{
	pthread_mutex_lock(&errorStringMutex);
	errorString = newError;
	pthread_mutex_unlock(&errorStringMutex);
}

QString KTTEngine::getErrorString()
{
	QString temp;

	pthread_mutex_lock(&errorStringMutex);
	temp = errorString;
	errorString.truncate(0);  // empty errorString
	pthread_mutex_unlock(&errorStringMutex);

	return temp;
}

QString KTTEngine::bashString(QString input)
{
	int index;

	// escape any special characters in string by inserting a backslash before them
	// escape backslash (must be done first before we insert any of our own)
	for(index = input.findRev('\\', -1); index >= 0; index = input.findRev('\\', index))
	{
		input.insert(index, '\\');
		// need to decrement index since we are searching for the same character we are inserting
		if(--index < 0) break;	
	}
	// escape double-quotes
	for(index = input.findRev('\"', -1); index >= 0; index = input.findRev('\"', index))
		input.insert(index, '\\');
	// escape dollar sign
	for(index = input.findRev('$', -1); index >= 0; index = input.findRev('$', index))
		input.insert(index, '\\');
	// put string in double quotes
	input = "\"" + input + "\"";

	return input;
}

QString KTTEngine::shellPrefix(const KTTSettings &settings)
{
	// name of environment variable where data file name is stored
	static const QString dataFileShellVar = "KTT_DATA_FILE";
	// name of environment variable where parameter file name is stored
	static const QString paramFileShellVar = "KTT_PARAM_FILE";
	QString prefix, paramFileName;

	if(settings.saveData)
	{
		prefix = dataFileShellVar + '=' + bashString(settings.fullFileName()) + '\n';
		paramFileName = settings.fullFileStem() + ".ts-par";
		prefix += paramFileShellVar + '=' + bashString(paramFileName) + '\n';
	}else
	{
		prefix = "";
	}
	return prefix;
}

void* aquisitionThreadFunction(void *arg)
{
	KTTEngine *data = (KTTEngine*) arg;
	data->aquireData();
	return NULL;
}

