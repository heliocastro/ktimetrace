/***************************************************************************
                          acqbuffer.h  -  description
                             -------------------
AcqBuffer copies input data from card to a temporary file, which is later
processed into the final save file for the data.

    begin                : Fri Jan 12 2001
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

#ifndef ACQBUFFER_H
#define ACQBUFFER_H


/**
  *@author Frank Hess
  */

class AcqBuffer;

#include <qfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <comedilib.h>
#include <zlib.h>
#include <iostream>

#include "adc.h"
#include "data.h"

class AcqBuffer {
public: 
	AcqBuffer(adc*);
	~AcqBuffer();
	// reads from board, and also copies data into temporary file if appropriate
	inline ssize_t read(void *buffer, size_t size);
	/* when data collection is done, the following functions are used to process the data
	 * that has been stored in the tempFile and write it to various formats */
	int writeTextFile(const char *fileName, unsigned int numChannels);
	int writeGZTextFile(const char *fileName, unsigned int numChannels);
	int writeBinaryFiles(const char *fileStem, unsigned int numChannels);
	// turn on / off storage of data on hard drive
	int saveToDisk(bool);
private:
	// returns true on a little endian cpu
	bool cpuIsLittleEndian();
	// optionally swaps bytes
	inline int16_t swap_bytes_16(int16_t word, bool swap);
	/* card we are taking data from */
	adc *myADC;
	/* temporary file to store data while we are aquiring it */
	FILE *tempFile;
};

inline ssize_t AcqBuffer::read(void *buffer, size_t size)
{
	size_t writeCount, readCount;
	ssize_t bytes;
	// read data from board
	bytes = myADC->read(buffer, size);
	// write data to temporary file
	if(tempFile && bytes > 0)
	{
		readCount = bytes / sizeof(sampl_t);
		writeCount = fwrite(buffer, sizeof(sampl_t), readCount, tempFile);
		if(writeCount != readCount)
		{
			std::cerr << "error writing data to temporary file, check for sufficient disk space" << std::endl;
			exit(1);
		}
	}
	return bytes;
}

extern inline int16_t AcqBuffer::swap_bytes_16(int16_t word, bool swap)
{
	if(swap)
		return (word & 0xff) << 8 | ((word >> 8) & 0xff);
	else
		return word;
}

#endif
