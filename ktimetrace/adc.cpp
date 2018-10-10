/***************************************************************************
                          adc.cpp  -  description
                             -------------------
    begin                : Fri Dec 15 2000
    copyright            : (C) 2000 by Frank Hess
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

#include "adc.h"
#include "resource.h"

#include <comedilib.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include <qstring.h>

using namespace std;

adc::adc()
{
	goodFlag = false;
	dev = 0;
	subDev = -1;
	flags = 0;
	pthread_mutex_init(&mutex, NULL);
}

int adc::setDevice(unsigned int devNum)
{
	if(devNum > 3)
	{
		cerr << "Invalid comedi device number" << std::endl;
		return -1;
	}

	goodFlag = false;

	// close and cleanup open comedi device file
	if(dev)
	{
		if(subDev >= 0)
			comedi_unlock(dev, subDev);
		comedi_close(dev);
		dev = 0;
		subDev = -1;
	}

	devPath.setNum(devNum);
	devPath = "/dev/comedi" + devPath;

	dev = comedi_open(devPath);
	if(!dev)
	{
		perror(devPath);
		return -1;
	}

	subDev = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_AI, 0);
	if(subDev < 0)
	{
		cerr << "No analog input subdevice found on " << devPath << std::endl;
		return -1;
	}

	// lock analog input so no other program messes us up
	if(comedi_lock(dev, subDev) < 0)
	{
		cerr << "Failed to obtain lock on analog input subdevice" << std::endl;
		return -1;
	}

	// set non-blocking mode flag
	fcntl(comedi_fileno(dev), F_SETFL, O_NONBLOCK);

	// figure out what triggers this driver accepts
	comedi_cmd cmd;
	memset(&cmd, 0, sizeof(cmd));

	int ret;
	ret = comedi_get_cmd_src_mask(dev, subDev, &cmd);
	if(ret < 0)
	{
		cerr << "Comedi command test failed." << std::endl;
		perror("ioctl");
		return -1;
	}

	startTrigMask = cmd.start_src;
	scanTrigMask = cmd.scan_begin_src;
	conversionTrigMask = cmd.convert_src;
	stopTrigMask = cmd.stop_src;
	startTrig = defaultTrigger(startTrigMask, TRIG_NOW);
	scanTrig = defaultTrigger(scanTrigMask, TRIG_FOLLOW);
	conversionTrig = defaultTrigger(conversionTrigMask, TRIG_TIMER);
	stopTrig = defaultTrigger(stopTrigMask, TRIG_COUNT);
	if(!startTrigMask || !scanTrigMask || !conversionTrig || !stopTrig)
	{
		cerr << "Failed to set default triggers!" << std::endl;
		return -1;
	}

	range = 0;
	rangePtr = comedi_get_range(dev, subDev, 0, range);
	aRef = AREF_DIFF;
	maximum = comedi_get_maxdata(dev, subDev, 0);
	bName = comedi_get_board_name(dev);

	goodFlag = true;

	return 0;
}

adc::~adc()
{
}

int adc::start(unsigned int numScans, unsigned int endChannel, unsigned int startChannel)
{
	comedi_cmd cmd;
	unsigned int *chanList;
	unsigned int numChannels = endChannel - startChannel + 1;

	if(numChannels < 1)
		return -1;

	memset(&cmd, 0, sizeof(comedi_cmd));

	cmd.subdev = subDev;
	cmd.start_src = startTrig;
	cmd.scan_begin_src = scanTrig;
	if(cmd.scan_begin_src == TRIG_TIMER)
		cmd.scan_begin_arg = scanPeriod;
	cmd.convert_src = conversionTrig;
	if(cmd.convert_src == TRIG_TIMER)
		cmd.convert_arg = conversionPeriod;
	cmd.scan_end_src = TRIG_COUNT;
	cmd.scan_end_arg = numChannels;		/* number of channels */
	cmd.stop_src = stopTrig;
	if(stopTrig == TRIG_COUNT)
	{
		cmd.stop_arg = numScans;
	}
	else
	{
		cmd.stop_arg = 0;
	}
	cmd.flags = flags;	

	// set up channel / gain list
	chanList = new unsigned int[numChannels];
	cmd.chanlist = chanList;
	cmd.chanlist_len = numChannels;
	for(unsigned int i = 0; i < numChannels; i++)
		chanList[i] = CR_PACK(startChannel + i, range, aRef);

	// send command
	int ret;
	ret = comedi_command_test(dev, &cmd);
	if(ret < 0)
	{
		cerr << "Comedi command test failed." << std::endl;
		perror("ioctl");
		delete [] chanList;
		return -1;
	}
	// if triggers are not supported or incompatible
	if(ret == 1 || ret == 2)
	{
		cerr << "cmd used unsupported or incompatible triggers" << std::endl;
		return ret;
	}

	// give it another test pass to fix up arguments if necessary	
	comedi_command_test(dev, &cmd);

	ret = comedi_command(dev, &cmd);
	if(ret < 0)
	{
		cerr << "Comedi command failed." << std::endl;
		perror("ioctl");
		delete [] chanList;
		return -1;
	}
	
	delete [] chanList;

	// read back actual timings used
	if(cmd.scan_begin_src == TRIG_TIMER)
		scanPeriod = cmd.scan_begin_arg;
	if(cmd.convert_src == TRIG_TIMER)
	{
		conversionPeriod = cmd.convert_arg;
		if(cmd.scan_begin_src == TRIG_FOLLOW)
			scanPeriod = cmd.convert_arg * cmd.chanlist_len;
	}

	return 0;
}

void adc::stop()
{
	comedi_cancel(dev, subDev);
}

ssize_t adc::read(void *buffer, size_t size)
{
	return ::read(comedi_fileno(dev), buffer, size);
}

int adc::poll()
{
	if(goodFlag)	
		return comedi_poll(dev, subDev);

	return -1;
}

comedi_range* adc::getRange(int i)
{
	return comedi_get_range(dev, subDev, 0, i);
}

int adc::getNumRanges(unsigned int chan)
{
	return comedi_get_n_ranges(dev, subDev, chan);
}

void adc::setRange(int rng)
{
	range = rng;
	rangePtr = comedi_get_range(dev, subDev, 0, rng);
}

void adc::setARef(int reference)
{
	aRef = reference;
}

int adc::maxData()
{
	return maximum;
}

bool adc::good()
{
	return goodFlag;
}

QString adc::boardName()
{
	if(good())
		return bName;
	return "";
}

QString adc::devicePath()
{
	if(good())
		return devPath;
	return "";
}

// need to check comedilib version to see if this is supported
int adc::bufferSize(unsigned int size)
{
	if(good() == false)
		return -1;

	if(size)
		return comedi_set_buffer_size(dev, subDev, size);

	return comedi_get_buffer_size(dev, subDev);
}

int adc::maxBufferSize(unsigned int size)
{
	if(good() == false)
		return -1;

	if(size)
		return comedi_set_max_buffer_size(dev, subDev, size);

	return comedi_get_max_buffer_size(dev, subDev);
}

int adc::numChannels()
{
	return comedi_get_n_channels(dev, subDev);
}

int adc::defaultTrigger(int mask, int suggestion)
{
	int trigger = mask;

	// use the suggested trigger mask if any are supported
	if(suggestion & mask)
		trigger &= suggestion;

	// find a valid trigger
	if(trigger & TRIG_NOW)
		return TRIG_NOW;
	if(trigger & TRIG_FOLLOW)
		return TRIG_FOLLOW;
	if(trigger & TRIG_TIMER)
		return TRIG_TIMER;
	if(trigger & TRIG_COUNT)
		return TRIG_COUNT;
	if(trigger & TRIG_EXT)
		return TRIG_EXT;
	if(trigger & TRIG_NONE)
		return TRIG_NONE;
	if(trigger & TRIG_INT)
		return TRIG_INT;
	if(trigger & TRIG_TIME)
		return TRIG_TIME;

	// failed to find good trigger!
	return 0;
}

unsigned int adc::defaultReference(unsigned int subdevFlags, unsigned int suggestion)
{
	
	// use the suggested trigger mask if any are supported
	switch(suggestion)
	{
		case AREF_GROUND:
			if(subdevFlags & SDF_GROUND)
				return suggestion;
			break;
		case AREF_COMMON:
			if(subdevFlags & SDF_COMMON)
				return suggestion;
			break;
		case AREF_DIFF:
			if(subdevFlags & SDF_DIFF)
				return suggestion;
			break;
		case AREF_OTHER:
			if(subdevFlags & SDF_OTHER)
				return suggestion;
			break;
		default:
			break;
	}

	// find a valid reference
	if(subdevFlags & SDF_GROUND)
		return AREF_GROUND;
	else if(subdevFlags & SDF_COMMON)
		return AREF_COMMON;
	else if(subdevFlags & SDF_DIFF)
		return AREF_DIFF;
	else if(subdevFlags & SDF_OTHER)
		return AREF_OTHER;

	// failed to find good trigger!
	return 0;
}

int adc::startTrigger(int trig)
{
	if(trig & startTrigMask)
		startTrig = trig;

	return startTrig;
}

int adc::startTriggerMask()
{
	return startTrigMask;
}

int adc::scanTrigger(int trig)
{
	if(trig & scanTrigMask)
		scanTrig = trig;

	return scanTrig;
}

int adc::scanTriggerMask()
{
	return scanTrigMask;
}

int adc::conversionTrigger(int trig)
{
	if(trig & conversionTrigMask)
		conversionTrig = trig;

	return conversionTrig;
}

int adc::conversionTriggerMask()
{
	return conversionTrigMask;
}

int adc::stopTrigger(int trig)
{
	if(trig & stopTrigMask)
		stopTrig = trig;

	return stopTrig;
}

int adc::stopTriggerMask()
{
	return stopTrigMask;
}

float adc::scanFrequency(float freq)
{
	double temp;

	if(freq > 0.0)
	{
		temp = rint(1e9 / freq);
		// make sure period is not too large
		if( temp > INT_MAX)
			temp = INT_MAX;
		scanPeriod = (unsigned int) temp;
	}

	return 1e9 / scanPeriod;
}

float adc::conversionFrequency(float freq)
{
	double temp;

	if(freq > 0.0)
	{
		temp = rint(1e9 / freq);
		// make sure period is not too large
		if( temp > INT_MAX)
			temp = INT_MAX;
		conversionPeriod = (unsigned int) temp;
	}

	return 1e9 / conversionPeriod;
}

void adc::useRT(bool rt)
{
	if(rt)
		flags |= TRIG_RT;
	else
		flags &= ~TRIG_RT;
}

void adc::useLowLatency(bool lowLat)
{
	if(lowLat)
		flags |= TRIG_WAKE_EOS;
	else
		flags &= ~TRIG_WAKE_EOS;
}

const QString adc::noneText = "None";
const QString adc::nowText = "Immediate";
const QString adc::followText = "Automatic";
const QString adc::timeText = "Time";
const QString adc::timerText = "Timer";
const QString adc::countText = "Count";
const QString adc::extText = "External";
const QString adc::intText = "Internal";
const QString adc::invalText = "Invalid";

QString adc::trigToText(int trig)
{
	QString text;
	switch(trig)
	{
		case TRIG_NONE:
			text = noneText;
			break;
		case TRIG_NOW:
			text = nowText;
			break;
		case TRIG_FOLLOW:
			text = followText;
			break;
		case TRIG_TIME:
			text = timeText;
			break;
		case TRIG_TIMER:
			text = timerText;
			break;
		case TRIG_COUNT:
			text = countText;
			break;
		case TRIG_EXT:
			text = extText;
			break;
		case TRIG_INT:
			text = intText;
			break;
		case 0:
			text = invalText;
			break;
		default:
			text = "Unknown";
			break;
	}
	return text;
}

int adc::textToTrig(const QString &text)
{
	if(text == noneText)
		return TRIG_NONE;
	else if(text == nowText)
		return TRIG_NOW;
	else if(text == followText)
		return TRIG_FOLLOW;
	else if(text == timeText)
		return TRIG_TIME;
	else if(text == timerText)
		return TRIG_TIMER;
	else if(text == countText)
		return TRIG_COUNT;
 	else if(text == extText)
		return TRIG_EXT;
 	else if(text == intText)
		return TRIG_INT;

	return 0;
}

const QString adc::groundRefText = "ground";
const QString adc::commonRefText = "common";
const QString adc::diffRefText = "differential";
const QString adc::otherRefText = "other";

QString adc::refToText(int aref)
{
	QString text;
	switch(aref)
	{
		case AREF_GROUND:
			text = groundRefText;
			break;
		case AREF_COMMON:
			text = commonRefText;
			break;
		case AREF_DIFF:
			text = diffRefText;
			break;
		case AREF_OTHER:
			text = otherRefText;
			break;
		default:
			text = "Unknown";
			break;
	}
	return text;
}

int adc::textToRef(const QString &text)
{
	if(text == groundRefText)
		return AREF_GROUND;
	else if(text == commonRefText)
		return AREF_COMMON;
	else if(text == diffRefText)
		return AREF_DIFF;
	else if(text == otherRefText)
		return AREF_OTHER;

	return 0;
}

unsigned int adc::subdevFlags()
{
	if(good() == false)
		return 0;

	return comedi_get_subdevice_flags(dev, subDev);
}
