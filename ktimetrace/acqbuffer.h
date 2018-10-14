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

#ifndef ACQBUFFER_H
#define ACQBUFFER_H

#include <cstdio>
#include <cstdlib>
#include <iostream>

QT_BEGIN_NAMESPACE
class QFile;
QT_END_NAMESPACE


#include "adc.h"

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
	inline int16_t swap_bytes_16(int16_t word, bool swap) {
		if(swap)
			return (word & 0xff) << 8 | ((word >> 8) & 0xff);
		else
			return word;
	}
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

#endif
