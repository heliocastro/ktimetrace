/***************************************************************************
                          acqbuffer.cpp  -  description
                             -------------------
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

#include "acqbuffer.h"
#include <kapp.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <qdir.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

AcqBuffer::AcqBuffer(adc *adcPtr)
{
	myADC = adcPtr;
	tempFile = NULL;
}

AcqBuffer::~AcqBuffer()
{
	if(tempFile)
		fclose(tempFile);
}

int AcqBuffer::writeTextFile(const char *fileName, unsigned int numChannels)
{
	FILE *file = 0;
	const int arraySize = 4096;
	sampl_t *array;
	size_t count;
	unsigned int channel;
	int ret;

	if(tempFile == NULL)
		return -1;

	rewind(tempFile);

	file = fopen(fileName, "w");
	if(file == 0)
		return -1;

	array = new sampl_t[arraySize];

	channel = 0;
	// loop while there is data
	while((count = fread(array, sizeof(array[0]), arraySize, tempFile)))
	{
		// write data to text file
		for(unsigned int i = 0; i < count; i++)
		{
			// if scan is finished terminate with newline
			if(channel == numChannels - 1)
				ret = fprintf(file, "%.5e\n", myADC->toPhys(array[i]));
			else	// otherwise tab
				ret = fprintf(file, "%.5e\t", myADC->toPhys(array[i]));
			if(ret < 0)
			{
				fclose(file);
				delete [] array;
				return -1;
			}
			channel = (channel + 1) % numChannels;
		}
	}

	delete [] array;
	ret = fclose(file);
	if(ret != 0)
		return -1;

	return 0;
}

int AcqBuffer::writeGZTextFile(const char *fileName, unsigned int numChannels)
{
	gzFile file = 0;
	const int arraySize = 4096;
	sampl_t *array;
	size_t count;
	unsigned int channel;
	int ret;

	if(tempFile == NULL)
		return -1;

	rewind(tempFile);

	file = gzopen(fileName, "w");
	if(file == 0)
		return -1;

	array = new sampl_t[arraySize];

	channel = 0;
	// loop while there is data
	while((count = fread(array, sizeof(array[0]), arraySize, tempFile)))
	{
		// write data to gzipped text file
		for(unsigned int i = 0; i < count; i++)
		{
			// if scan is finished terminate with newline
			if(channel == numChannels - 1)
				ret = gzprintf(file, "%.5e\n", myADC->toPhys(array[i]));
			else	// otherwise tab
				ret = gzprintf(file, "%.5e\t", myADC->toPhys(array[i]));
			if(ret < 0)
			{
				gzclose(file);
				delete [] array;
				return -1;
			}
			channel = (channel + 1) % numChannels;
		}
	}

	delete [] array;
	ret = gzclose(file);
	if(ret != 0)
		return -1;

	return 0;
}

bool AcqBuffer::cpuIsLittleEndian()
{
	uint16_t test_var = 1;
	uint8_t *test_ptr = (uint8_t*) &test_var;

	if(*test_ptr == test_var) return true;
	else return false;
}

int AcqBuffer::writeBinaryFiles(const char *fileStem, unsigned int numChannels)
{
	FILE **file;
	QString temp;
	const int arraySize = 4096;
	sampl_t *array;
	size_t readCount, writeCount;
	int ret;
	int16_t trueBinary;
	unsigned int channel;
	bool swap;

	if(tempFile == NULL)
		return -1;

	rewind(tempFile);

	file = new FILE*[numChannels];

	for(unsigned int i = 0; i < numChannels; i++)
	{
		temp.sprintf("%s.ts%i", fileStem, i + 1);
		file[i] = fopen(temp, "w");
		if(file[i] == 0)
		{
			for(unsigned int j = 0; j < i; j++)
				fclose(file[j]);
			delete [] file;
			return -1;
		}
	}

	array = new sampl_t[arraySize];

	channel = 0;
	swap = !cpuIsLittleEndian();
	// loop while there is data
	while((readCount = fread(array, sizeof(array[0]), arraySize, tempFile)))
	{
		// write data to binary file (array of 16 bit signed, little-endian integers)
		for(unsigned int i = 0; i < readCount; i++)
		{
			trueBinary = swap_bytes_16(myADC->toTrueBinary(array[i]), swap);
			writeCount = fwrite(&trueBinary, sizeof(trueBinary), 1, file[channel]);
			if(writeCount != 1)
			{
				for(unsigned int j = 0; j < numChannels; j++)
					fclose(file[j]);
				delete [] file;
				delete [] array;
				return -1;
			}
			channel = (channel + 1) % numChannels;
		}
	}

	delete [] array;
	ret = 0;
	for(unsigned int j = 0; j < numChannels; j++)
	{
		if(fclose(file[j]) != 0)
		{
			ret = -1;
		}
	}
	delete [] file;

	return ret;
}

int AcqBuffer::saveToDisk(bool save)
{
	// open tempFile if appropriate
	if(save && tempFile == NULL)
	{
		tempFile = tmpfile();
		if(tempFile == NULL)
		{
			std::cerr << "unable to open temporary file " << std::endl;
			return -1;
		}
	}
	// close tempFile if appropriate
	if(save == false && tempFile != NULL)
	{
		fclose(tempFile);
		tempFile = NULL;	
	}

	return 0;
}

