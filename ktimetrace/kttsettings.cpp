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

#include <QDir>
#include <QSettings>

#include "kttsettings.h"

KTTSettings::KTTSettings()
{
	directory = QDir::homePath();
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

void KTTSettings::saveConfig()
{
	QSettings settings("KDE", "KTimeTrace");

	settings.setValue("settings/saveData", saveData);
	settings.setValue("settings/directory", directory.absolutePath());
	settings.setValue("settings/fileNum", fileNum);
	settings.setValue("settings/saveType", saveType);
	settings.setValue("settings/fileStem", fileStem);
	settings.setValue("settings/startTrigger", startTrigger);
	settings.setValue("settings/scanTrigger", scanTrigger);
	settings.setValue("settings/conversionTrigger", conversionTrigger);
	settings.setValue("settings/stopTrigger", stopTrigger);
	settings.setValue("settings/scanFrequency", scanFrequency);
	settings.setValue("settings/conversionFrequency", conversionFrequency);
	settings.setValue("settings/numScans", (int) numScans);
	settings.setValue("settings/numChannels", (int) numChannels);
	settings.setValue("settings/range", range);
	settings.setValue("settings/reference", reference);
	settings.setValue("settings/realTime", realTime);
	settings.setValue("settings/lowLatency", lowLatency);
	settings.setValue("settings/execAtStart", execAtStart);
	settings.setValue("settings/startCommand", startCommand);
	settings.setValue("settings/execAtEnd", execAtEnd);
	settings.setValue("settings/endCommand", endCommand);
}

void KTTSettings::loadConfig()
{
	QSettings settings("KDE", "KTimeTrace");

	saveData = settings.value("settings/saveData", saveData).toBool();
	directory.setPath(settings.value("settings/directory", directory.absolutePath()).toString());
	fileNum = settings.value("settings/fileNum", fileNum).toInt();
	fileStem = settings.value("settings/fileStem", fileStem).toString();
	saveType = (FileFormat) settings.value("settings/saveType", saveType).toInt();
	startTrigger = settings.value("settings/startTrigger", startTrigger).toInt();
	scanTrigger = settings.value("settings/scanTrigger", scanTrigger).toInt();
	conversionTrigger = settings.value("settings/conversionTrigger", conversionTrigger).toInt();
	stopTrigger = settings.value("settings/stopTrigger", stopTrigger).toInt();
	scanFrequency = settings.value("settings/scanFrequency", scanFrequency).toInt();
	conversionFrequency = settings.value("settings/conversionFrequency", conversionFrequency).toInt();
	numScans = settings.value("settings/numScans", (int) numScans).toInt();
	numChannels = settings.value("settings/numChannels", numChannels).toInt();
	range = settings.value("settings/range", range).toInt();
	reference = settings.value("settings/reference", reference).toInt();
	realTime = settings.value("settings/realTime", realTime).toBool();
	lowLatency = settings.value("settings/lowLatency", lowLatency).toBool();
	execAtStart = settings.value("settings/execAtStart", execAtStart).toBool();
	startCommand = settings.value("settings/startCommand", startCommand).toString();
	execAtEnd = settings.value("settings/execAtEnd", execAtEnd).toBool();
	endCommand = settings.value("settings/endCommand", endCommand).toString();
}

QString KTTSettings::fullFileName() const
{
	return fullFileStem() + fileExtension();
}

QString KTTSettings::fullFileStem() const
{
	QString name;
	name.sprintf("%s/%s%.3i", directory.absolutePath(), fileStem, fileNum);
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

