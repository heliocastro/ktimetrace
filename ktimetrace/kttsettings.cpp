/***************************************************************************
                          kttsettings.cpp  -  description
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


#include "kttsettings.h"
#include <comedilib.h>

KTTSettings::KTTSettings()
{
	directory = QDir::homeDirPath();
	fileStem = "temp";
	fileNum = 0;
	saveType = TEXT;
	numChannels = 1;
	numScans = 100000;
	range = 0;
	reference = AREF_DIFF;
	startTrigger = TRIG_NOW;
	scanTrigger = TRIG_FOLLOW;
	conversionTrigger = TRIG_TIMER;
	stopTrigger = TRIG_COUNT;
	scanFrequency = 1000.0;
	conversionFrequency = 1000.0;
	realTime = false;
	lowLatency = false;
	execAtStart = false;
	startCommand = "echo starting aquisition\n";
	execAtEnd = false;
	endCommand = "echo aquisition complete\n"
		"echo data file: \"$KTT_DATA_FILE\"\n"
		"echo parameter file:  \"$KTT_PARAM_FILE\"\n";
	saveData = true;
}

KTTSettings::~KTTSettings(){
}

void KTTSettings::saveConfig(KConfig *config)
{
	config->setGroup("settings");
	config->writeEntry("saveData", saveData);
	config->writeEntry("directory", directory.absPath());
	config->writeEntry("fileNum", fileNum);
	config->writeEntry("saveType", saveType);
	config->writeEntry("fileStem", fileStem);
	config->writeEntry("startTrigger", startTrigger);
	config->writeEntry("scanTrigger", scanTrigger);
	config->writeEntry("conversionTrigger", conversionTrigger);
	config->writeEntry("stopTrigger", stopTrigger);
	config->writeEntry("scanFrequency", scanFrequency);
	config->writeEntry("conversionFrequency", conversionFrequency);
	config->writeEntry("numScans", numScans);
	config->writeEntry("numChannels", (int) numChannels);
	config->writeEntry("range", range);
	config->writeEntry("reference", reference);
	config->writeEntry("realTime", realTime);
	config->writeEntry("lowLatency", lowLatency);
	config->writeEntry("execAtStart", execAtStart);
	config->writeEntry("startCommand", startCommand);
	config->writeEntry("execAtEnd", execAtEnd);
	config->writeEntry("endCommand", endCommand);
}

void KTTSettings::loadConfig(KConfig *config)
{
	config->setGroup("settings");
	saveData = config->readBoolEntry("saveData", saveData);
	directory.setPath(config->readEntry("directory", directory.absPath()));
	fileNum = config->readNumEntry("fileNum", fileNum);
	fileStem = config->readEntry("fileStem", fileStem);
	saveType = (FileFormat) config->readNumEntry("saveType", saveType);
	startTrigger = config->readNumEntry("startTrigger", startTrigger);
	scanTrigger = config->readNumEntry("scanTrigger", scanTrigger);
	conversionTrigger = config->readNumEntry("conversionTrigger", conversionTrigger);
	stopTrigger = config->readNumEntry("stopTrigger", stopTrigger);
	scanFrequency = config->readDoubleNumEntry("scanFrequency", scanFrequency);
	conversionFrequency = config->readDoubleNumEntry("conversionFrequency", conversionFrequency);
	numScans = config->readUnsignedLongNumEntry("numScans", numScans);
	numChannels = config->readNumEntry("numChannels", numChannels);
	range = config->readNumEntry("range", range);
	reference = config->readNumEntry("reference", reference);
	realTime = config->readBoolEntry("realTime", realTime);
	lowLatency = config->readBoolEntry("lowLatency", lowLatency);
	execAtStart = config->readBoolEntry("execAtStart", execAtStart);
	startCommand = config->readEntry("startCommand", startCommand);
	execAtEnd = config->readBoolEntry("execAtEnd", execAtEnd);
	endCommand = config->readEntry("endCommand", endCommand);
}

QString KTTSettings::fullFileName() const
{
	return fullFileStem() + fileExtension();
}

QString KTTSettings::fullFileStem() const
{
	QString name;
	name.sprintf("%s/%s%.3i", (const char*) directory.absPath(),
		(const char*) fileStem, fileNum);
	return name;
}

QString KTTSettings::fileExtension() const
{
	QString temp;
	switch(saveType)
	{
		case TEXT:
			temp = ".txt";
			break;
		case GZ_TEXT:
			temp = ".txt.gz";
			break;
		case BINARY:
			temp = "";		// binary format has different extension for each channel
			break;
		default:
			temp = "";
			break;
	}
	return temp;
}

