/***************************************************************************
                          adc.h  -  description
                             -------------------

	The adc class represents the analog-to-digital converter card being
	used to collect data.  It needs a driver for the card provided by
	the 'comedi' project (see http://stm.lbl.gov/comedi/ ).

    begin                : Fri Dec 15 2000
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

#ifndef ADC_H
#define ADC_H

#include <qstring.h>

#include <unistd.h>
#include <pthread.h>

#include <comedilib.h>

/**
  *@author Frank Mori Hess
  */


class adc
{
public:
	adc();
	~adc();
	/* sets which device file /dev/comediX we are going to use */
	int setDevice(unsigned int number);
	/* Take data.  If numConversions is zero, data will be taken forever
	 * (until stop() is called.)  aRef is the low reference the card is to use,
	 * it may be used or ignored by the comedi driver depending on which card/driver is used.
	 * This function has more arguments than I like, will probably be broken up into smaller
	 * functions at some time in the future.
	 */	
	int start(unsigned int numScans = 0, unsigned int endChannel = 0, unsigned int startChannel = 0);
	/* stop taking data*/
	void stop();
	/* read into buffer from /dev/comediX */
	ssize_t read(void *buffer, size_t size);
	// poll card for data sitting in fifo
	int poll();
	/* converts comedi's native data format sampl_t (unsigned 16 bit) to a
	 * a floating-point voltage */
	inline float toPhys(sampl_t dataPoint);
	/* converts comedi's native data format sampl_t (unsigned 16 bit) to
	 * true binary, for backwards compatibility with our old timetrace data files */
	inline int toTrueBinary(sampl_t dataPoint);
	/* returns the input range the card is currently set to */
	comedi_range* getRange(int i);
	/* returns the number of ranges */
	int getNumRanges(unsigned int chan = 0);
	/* set card to a new input range */
	void setRange(int rng);
	/* set analog input reference (ground, common, differential) */
	void setARef(int reference);
	/* Utility function, returns a trigger that is consistent with mask,
	 * trys suggestedTrig first */
	static int defaultTrigger(int mask, int suggestedTrig = 0);
	/* Utility function, returns a analog reference that is consistent with subdevice flags,
	 * trys suggestedRef first */
	static unsigned int defaultReference(unsigned int subdevFlags, unsigned int suggestedRef = AREF_GROUND);
	/* Sets and return triggers for aquisition */
	int startTrigger(int trig = 0);
	int scanTrigger(int trig = 0);
	int conversionTrigger(int trig = 0);
	int stopTrigger(int trig = 0);
	/* Return masks of supported triggers */
	int startTriggerMask();
	int scanTriggerMask();
	int conversionTriggerMask();
	int stopTriggerMask();
	/* set/return frequencies */
	float scanFrequency(float freq = -1.0);
	float conversionFrequency(float freq = -1.0);
	/* ask comedi to use real-time interrupt handler (or not) */
	void useRT(bool rt = true);
	/* ask comedi to wake on end-of-scan (or not) */
	void useLowLatency(bool lowLat = true);
	/* returns maximum value the card can return (4095 for a 12-bit card) */
	int maxData();
	/* returns true if everything is a-ok */
	bool good();
	/* return name of board */
	QString boardName();
	/* returns path of device file */
	QString devicePath();
	/* query/set comedi buffer size in bytes*/
	int bufferSize(unsigned int size = 0);
	/* query/set comedi maximum buffer size in bytes*/
	int maxBufferSize(unsigned int size = 0);
	/* returns number of analog input channels */
	int numChannels();
	// returns subdevice flags for open subdevice
	unsigned int subdevFlags();
	/* some text-to-blah/blah-to-text functions required because stupid qt combobox doesn't
	 * let you attach an id number to elements of combobox */
	/* converts comedi trigger numbers into a text description*/
	static QString trigToText(int trig);
	/* inverse of trigToText */
	static int textToTrig(const QString&);
	/* converts comedi analog reference numbers into a text description*/
	static QString refToText(int aref);
	/* inverse of refToText */
	static int textToRef(const QString&);
	// mutex to prevent simultaneous access
	pthread_mutex_t mutex;
private:
	/* represents /dev/comediX file we have open */
	comedi_t *dev;
	/* the subdevice number for the card's analog input subdevice */
	int subDev;
	/* the input range */
	int range;
	/* stores the actual voltages that 'range' corresponds to */
	comedi_range *rangePtr;
	/* triggers */
	int startTrig;
	int scanTrig;
	int conversionTrig;
	int stopTrig;
	/* masks of supported triggers */
	int startTrigMask;
	int scanTrigMask;
	int conversionTrigMask;
	int stopTrigMask;
	/* timing periods in nanoseconds */
	unsigned int scanPeriod;
	unsigned int conversionPeriod;
	/* comedi_cmd flags */
	int flags;
	/* maximum value the card can return (4095 for a 12-bit card) */
	int maximum;
	/* analog input reference (ground, common, differential) */
	int aRef;
	/* good flag indicates everything is a-ok */
	bool goodFlag;
	/* board name */
	QString bName;
	/* device file path */
	QString devPath;
	/* string constants used in trigToText() and textToTrig */
	static const QString noneText;
	static const QString nowText;
	static const QString followText;
	static const QString timeText;
	static const QString timerText;
	static const QString countText;
	static const QString extText;
	static const QString intText;
	static const QString invalText;
	// string constants used in refToText() and textToRef() */
	static const QString groundRefText;
	static const QString commonRefText;
	static const QString diffRefText;
	static const QString otherRefText;
};

// inline function definitions
inline float adc::toPhys(sampl_t dataPoint)
{
	return comedi_to_phys(dataPoint, rangePtr, maximum);
}

inline int adc::toTrueBinary(sampl_t dataPoint)
{
	int ret = dataPoint;

	// if range is bipolar, subtract offset
	if(rangePtr->min < 0.0)
		ret -=  (maximum + 1) / 2;

	return ret;
}

#endif
