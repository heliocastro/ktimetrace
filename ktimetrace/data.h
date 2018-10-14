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

#ifndef DATA_H
#define DATA_H

#include <pthread.h>
#include <deque>
#include <signal.h>

#include "adc.h"
#include "view.h"
#include "acqbuffer.h"
#include "kttsettings.h"

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class KTraceView;

/* keeps track of how many file aquisition threads are running so that we don't
 * exit program before files are done writing.  I should make this not a
 * global variable */
extern volatile unsigned int aquisitionThreadCount;
// lock to control access to writeThreadCount variable
extern pthread_mutex_t aquisitionThreadCountLock;

class KTTEngine : public QObject
{
	Q_OBJECT
public:
	KTTEngine();
	~KTTEngine();
	/* sets and/or returns pointer to adc being used */
	adc* ADC(adc* = 0);
	/* sets device for myADC and checks settings against new device */
	int setDevice(unsigned int deviceNumber);
	/* starts data collection */
	void collect();
	/* interrupts data aquisition */
	void stop();
	/* sets the KTraceView we are using to display data */
	void setView(KTraceView *viewPtr);
	/* saves settings to configuration file */
	void saveConfig();
	/* reads settings from configuration file */
	void loadConfig();
	/* writes data files to disk */
	int writeFiles(KTTSettings localSettings, AcqBuffer *buf);
	// aquires data from card and puts it in dataFifo.  called by aquisition thread
	void aquireData();
	/* returns true if there are no file writing threads active */
	bool writeDone();
	/* settings used for aquisition */
	KTTSettings settings;
private:
	// thread safe read or write the variable 'go'
	inline void setGo(bool);
	inline bool getGo();
	// thread safe read or write of errorString
	void setErrorString(QString);
	QString getErrorString();	// also clears error, returns empty string if no error has occurred
	/* returns a string to be prepended to the user-defined shell command which sets
	* the shell variable KTT_DATA_FILE, taking into account the possiblity of special
	* characters in the file name. */
	QString shellPrefix(const KTTSettings &settings);
	/* Converts input string to be bash-friendly by escaping special characters */
	QString bashString(QString input);
	/* flag used to interrupt aquisition by setting it to false */
	volatile bool go;
	// lock for 'go'
	pthread_mutex_t goMutex;
	// used to pass error messages from aquisition thread to gui thread
	QString errorString;
	// lock for errorString
	pthread_mutex_t errorStringMutex;
	/* the view where we are plotting our data */
	KTraceView *view;
	/* the card we are getting are data from */
	adc *myADC;
	// fifo for storing data obtained by aquisition thread, read by gui thread
	std::deque<sampl_t> dataFifo;
	/* lock associated with dataFifo to prevent races */
	pthread_mutex_t dataFifoMutex;
	/* thread for aquiring data, cannot access qt GUI */
	pthread_t aquisitionThread;
	/* wait condition used to pause main GUI thread until aquisition thread
	 * has obtained all the values from memory that it needs */
	pthread_cond_t aquisitionReady;
	/* lock associated with aquisitionReady to prevent race */
	pthread_mutex_t waitMutex;
	// timer used to update plotting
	QTimer *plotTimer;
	// keeps track of how many points we have plotted
	volatile unsigned int numPointsPlotted;
private slots:
	void slotUpdatePlot();
};

inline void KTTEngine::setGo(bool value)
{
	pthread_mutex_lock(&goMutex);
	go = value;
	pthread_mutex_unlock(&goMutex);
}

inline bool KTTEngine::getGo()
{
	bool value;

	pthread_mutex_lock(&goMutex);
	value = go;
	pthread_mutex_unlock(&goMutex);

	return value;
}

#endif
