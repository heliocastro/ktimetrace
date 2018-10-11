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

#ifndef KTTSETTINGS_H
#define KTTSETTINGS_H

#include <comedilib.h>

QT_BEGIN_NAMESPACE
class QSettings;
class QDir;
class QString;
QT_END_NAMESPACE

enum FileFormat {TEXT, GZ_TEXT, BINARY};

class KTTSettings {
public:
	KTTSettings();

	/* saves settings to configuration file */
	void saveConfig();
	/* reads settings from configuration file */
	void loadConfig();
	// returns a full file name from directory, fileStem, fileNum and saveType members
	QString fullFileName() const;
	// returns full file name minus extension
	QString fullFileStem() const;
	// return file extension
	QString fileExtension() const;

	/* directory to save data files into */
	QDir directory;
	/* name of data file sans file number and extension */
	QString fileStem;
	/* file number appended to fileStem */
	unsigned int fileNum;
	/* number of channels */
	unsigned int numChannels;
	/* total number of data points per channel being collected */
	unsigned long numScans;
	// records number of scans that were actually completed
	unsigned long numScansCompleted;
	/* save in text, gzipped text or binary? */
	FileFormat saveType;
	/* input range index */
	int range;
	// range information
	comedi_range rangeInfo;
	/* input analog reference */
	int reference;
	/* aquisition triggers */
	int startTrigger;
	int scanTrigger;
	int conversionTrigger;
	int stopTrigger;
	/* scans per second (when scanTrigger is TRIG_TIMER)*/
	float scanFrequency;
	/* number of data points per second (when conversionTrigger is TRIG_TIMER) */
	float conversionFrequency;
	/* indicate whether to send TRIG_RT and/or TRIG_WAKE_EOS flags to comedi */
	bool realTime;
	bool lowLatency;
	// save data to hard disk or not?
	bool saveData;
	// name of hardware
	QString boardName;
	// should a user specified command be executed at beginning of aquisition?
	bool execAtStart;
	// shell command that user wants to execute at beginning of aquisition
	QString startCommand;
	// should a user specified command be executed at end of aquisition?
	bool execAtEnd;
	// shell command that user wants to execute at end of aquisition
	QString endCommand;
};

#endif
