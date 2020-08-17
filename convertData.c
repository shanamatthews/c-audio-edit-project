#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "ReadFile.h"
#include "getcs229Data.h"
#include "getAIFFData.h"


/* Accepts a file pointer indicating where the data should be taken from,
	a file pointer indicating where the data should be written to,
	integers giving the file's bit depth, number of samples, number of
	channels, and a character which marks the place we are at in the infile.
	char c is a placeholder for where we were in the file. 
	For this function to work correctly, it should be on the very first piece of 
	sound data in the file */
int convertAIFFData(FILE* infile, FILE* outfile, int bitDepth, int samples, int channels, char c)
{
	if(8 == bitDepth)
	{
		int i = 0;
		int j = 0;
		
		for(i = 0; i < (samples * channels); i++)
		{
			/* This loop is unnecessary, but helps keep consistency between bit depths */
			char sample[1];
			/* Runs through all the samples, converting them from big endian binary
				to little endian integers, then writing them to the outfile. */
			for(j = 0; j < 1; j++)
			{
				sample[j] = c;
				c = fgetc(infile);
			}
			
			/* no swapping necessary, since just 1 byte! */
			
			char dataSample = *(char*)sample;
			fprintf(outfile, "%d ", dataSample);
			
			if(0 == (i+1) % channels)
			{
				fputs("\n", outfile);
			}
		}

	}
	else if(16 == bitDepth) 
	{	
		int i = 0; 
		int j = 0;
		
		/* same, but for larger samples */
		for(i = 0; i < (samples * channels); i++)
		{
			char sample[2];
			for(j = 0; j < 2; j++)
			{
				sample[j] = c;
				c = fgetc(infile);
			}
			
			/* change endian-ness by swapping bytes around */
			swap(sample, 0, 1);
			
			short dataSample = *(short*)sample;
		
			fprintf(outfile, "%i ", dataSample);
			
			/* if we finished a row of channels, move to new line for a new sample. */
			if(0 == (i+1) % channels)
			{
				fputs("\n", outfile);
			}	
		}
	}
	else if(32 == bitDepth)
	{	
		int i = 0;
		int j = 0;
		
		for(i = 0; i < (samples * channels); i++)
		{
			char sample[4];
			for(j = 0; j < 4; j++)
			{
				sample[j] = c;
				c = fgetc(infile);
			}
			
			swap(sample, 0, 3);
			swap(sample, 1, 2);
			
			int dataSample = *(int*)sample;
			
			fprintf(outfile, "%i ", dataSample);
			
			if(0 == (i+1) % channels)
			{
				fputs("\n", outfile);
			}
		}
	}
	else
	{
		fprintf(stderr, "Something went wrong when trying to parse this file.");
		fprintf(stderr, "Perhaps it uses an unsupported bit depth? Supported bit depths are:\n");
		fprintf(stderr, "8, 16, 32\n");
		fclose(outfile);
		fclose(infile);
		return -1;
	}
}

/* Accepts a file pointer indicating where the data should be read from, 
	a file pointer indicating where the data should be written to,
	and ints representing the file's bit depth, number of samples, and
	number of channels. 
	Converts the data from a CS229 file to AIFF format. */
int convertCS229Data(FILE* infile, FILE* outfile, int bitDepth, int samples, int channels)
{
	/* start writing data! */
	char c = findStartData(infile);
	if(EOF == c)
	{
		fprintf(stderr, "We had a problem reading this file.\n");
		return 1;
	}
	
	if(8 == bitDepth)
	{
		int i = 0;
		/* runs through all the samples converting them from little endian ints to big endian binary */
		for(i = 0; i < samples * channels; i++)
		{
			int dataPoint = 0;
			fscanf(infile, "%i", &dataPoint);
		
			char dataBytes[1];
			dataBytes[0] = dataPoint;
		
			fwrite(dataBytes, sizeof(char), 1, outfile);
		}
	}
	else if(16 == bitDepth)
	{
		int i = 0;
		for(i = 0; i < samples * channels; i++)
		{
			int dataPoint = 0;
			fscanf(infile, "%i", &dataPoint);
		
			/* change endian- ness by bitshifting */
			char dataBytes[3];
			dataBytes[0] = (dataPoint >> 8) & 0xFF;
			dataBytes[1] = dataPoint & 0xFF;
		
			fwrite(dataBytes, sizeof(char), 2, outfile);
		}
	}
	else if(32 == bitDepth)
	{
		int i = 0;
		for(i = 0; i < samples * channels; i++)
		{
			int dataPoint = 0;

			fscanf(infile, "%d", &dataPoint);
		
			char dataBytes[4];
			dataBytes[0] = (dataPoint >> 24) & 0xFF;
			dataBytes[1] = (dataPoint >> 16) & 0xFF;
			dataBytes[2] = (dataPoint >> 8) & 0xFF;
			dataBytes[3] = dataPoint & 0xFF;
		
			fwrite(dataBytes, sizeof(char), 4, outfile);
		}
	}
	else
	{
		fprintf(stderr, "Something went wrong when trying to parse this file.");
		fprintf(stderr, "Perhaps it uses an unsupported bit depth? Supported bit depths are:\n");
		fprintf(stderr, "8, 16, 32\n");
		fclose(outfile);
		fclose(infile);
		return -1;
	}
}

/* Converts an entire AIFF file to a CS229 file.
	Accepts a file pointer for the AIFF file to convert and a 
	file pointer for the CS229 file which to write to. */
int convertAIFFtoCS229(FILE* infile, FILE* outfile)
{
	char c;
	
	/* Run through the infile and collect data */
	parseAIFFFile(infile);
	
	/* fetch the data */
	int samples;
	samples = getNumSamples();
	int bitDepth;
	bitDepth = getBitDepth();
	int channels;
	channels = getNumChannels();
	long double sampleRate;
	sampleRate = getFileRate();
	
	/* increment the file pointer to the point where the AIFF data begins */ 
	c = findAIFFDataChunk(infile);
	if(EOF == c)
	{
		fprintf(stderr, "This file did not contain any usable data.\n");
		return 1;
	}
	
	/* skip c through a 4-byte int */
	char bytesLeftInChunk[5];
	int z = 0;
	for(z = 0; z < 4; z++)
	{
		bytesLeftInChunk[z] = c;
		c = fgetc(infile);
	}
	
	swap(bytesLeftInChunk, 0, 3);
	swap(bytesLeftInChunk, 1, 2);
	
	int bytesLeft = *(int*) bytesLeftInChunk;
	
	/* check num samples. If it doesn't match the value given in the header, terminate. */
	if((bytesLeft - 8)/(bitDepth/8) != (samples * channels))
	{
		fprintf(stderr, "This file is improperly formatted. The number of samples or channels specified is probably incorrect. Terminating.\n");
		return -1;
	}
	
	/* print cs229 header */
	fputs("CS229\n", outfile);
	fputs("\n", outfile);
	fputs("\n", outfile);
	
	fprintf(outfile, "SampleRate  %Lg\n", sampleRate);
	fprintf(outfile, "Channels  %i\n", channels);
	fprintf(outfile, "BitDepth  %i\n", bitDepth);
	fprintf(outfile, "Samples  %i\n", samples);
	fprintf(outfile, "\n\n");
	
	/* skip c through another 4-byte int */
	char offset[5];
	for(z = 0; z < 4; z++)
	{
		offset[z] = c;
		c = fgetc(infile); 
	}
	
	/* skip c through the last 4-byte int */
	char blockSize[5];
	for(z = 0; z < 4; z++)
	{
		blockSize[z] = c;
		c = fgetc(infile);
	}
	
	fputs("StartData\n", outfile);
	
	/* finished with the header, now convert data */
	convertAIFFData(infile, outfile, bitDepth, samples, channels, c);
	
	return 0;
}

/* converts a CS229-formatted file into an AIFF-formatted file
	Takes a file pointer to the CS229 file we want to convert and a 
	file pointer to the file we want to write to. */
int convertCS229toAIFF(FILE* infile, FILE* outfile)
{	
	/* searches through the header of the CS229 file to find information */
	parse229File(infile, "DUMMYVAL");
	
	/* grab that information */
	unsigned int samples;
	samples = get229NumSamples();
	
	unsigned int channels;
	channels = get229NumChannels();
	
	char bitDepthArray[4];
	strcpy(bitDepthArray, file229BitDepth);
	unsigned int bitDepth = 0;
	
	char sampleRateArray[11];
	strcpy(sampleRateArray, file229Rate);
	long double sampleRate = 0; 
	
	
	bitDepth = atoi(bitDepthArray);
	sampleRate = atoi(sampleRateArray);
	
	
	/* put in FORM and remaining bytes in the file */
	fputs("FORM", outfile);
	
	unsigned int totalRemainingBytes = 46 + (samples * bitDepth/8 * channels);
	char tRB[5];

	tRB[0] = (totalRemainingBytes >> 24) & 0xFF;
	tRB[1] = (totalRemainingBytes >> 16) & 0xFF;
	tRB[2] = (totalRemainingBytes >> 8) & 0xFF;
	tRB[3] = totalRemainingBytes & 0xFF; 

	
	fwrite(tRB, sizeof(char), 4, outfile);
	
	/* put AIFF in there */
	fputs("AIFF", outfile);
	
	/* start the COMM chunk and size = 18 */
	fputs("COMM", outfile);
	
	unsigned int commRemainingBytes = 18;
	char cRB[5];

	cRB[0] = (commRemainingBytes >> 24) & 0xFF;
	cRB[1] = (commRemainingBytes >> 16) & 0xFF;
	cRB[2] = (commRemainingBytes >> 8) & 0xFF;
	cRB[3] = commRemainingBytes & 0xFF;
	
	fwrite(cRB, sizeof(char), 4, outfile);
	
	/* put in numChannels */
	char channelBytes[3];
	
	channelBytes[0] = (channels >> 8) & 0xFF;
	channelBytes[1] = channels & 0xFF;
	
	fwrite(channelBytes, sizeof(char), 2, outfile);
	
	/* put in numSampleFrames (samples)*/
	char numSamplesBytes[5];
	
	numSamplesBytes[0] = (samples >> 24) & 0xFF;
	numSamplesBytes[1] = (samples >> 16) & 0xFF;
	numSamplesBytes[2] = (samples >> 8) & 0xFF;
	numSamplesBytes[3] = samples & 0xFF;
	
	fwrite(numSamplesBytes, sizeof(char), 4, outfile);
	
	/* put in bitDepth */
	char bitDepthBytes[3];
	
	bitDepthBytes[0] = (bitDepth >> 8) & 0xFF;
	bitDepthBytes[1] = bitDepth & 0xFF;
	
	fwrite(bitDepthBytes, sizeof(char), 2, outfile);
	
	/* put in sampleRate */
	char sampleRateBytes[sizeof(long double)];
	memcpy(sampleRateBytes, &sampleRate, sizeof(sampleRateBytes));
	
	swap(sampleRateBytes, 0, 9);
	swap(sampleRateBytes, 1, 8);
	swap(sampleRateBytes, 2, 7);
	swap(sampleRateBytes, 3, 6);
	swap(sampleRateBytes, 4, 5);
	
	fwrite(sampleRateBytes, sizeof(char), 10, outfile);
	
	/* start the SSND chunk and size! */
	fputs("SSND", outfile);
	
	unsigned int ssndRemainingBytes = 8 + (samples * bitDepth/8 * channels);
	
	char sRB[5];
	sRB[0] = (ssndRemainingBytes >> 24) & 0xFF;
	sRB[1] = (ssndRemainingBytes >> 16) & 0xFF;
	sRB[2] = (ssndRemainingBytes >> 8) & 0xFF;
	sRB[3] = ssndRemainingBytes & 0xFF;
	
	fwrite(sRB, sizeof(char), 4, outfile);
	
	/* put in offset */
	unsigned int offset = 0;
	char offsetBytes[5];

	offsetBytes[0] = (offset >> 24) & 0xFF;
	offsetBytes[1] = (offset >> 16) & 0xFF;
	offsetBytes[2] = (offset >> 8) & 0xFF;
	offsetBytes[3] = offset & 0xFF; 
	
	fwrite(offsetBytes, sizeof(char), 4, outfile);
	
	/* put in blocksize */
	unsigned int blockSize = 0;
	char blockSizeBytes[5];

	blockSizeBytes[0] = (blockSize >> 24) & 0xFF;
	blockSizeBytes[1] = (blockSize >> 16) & 0xFF;
	blockSizeBytes[2] = (blockSize >> 8) & 0xFF;
	blockSizeBytes[3] = blockSize & 0xFF; 
	
	fwrite(blockSizeBytes, sizeof(char), 4, outfile);
	
	/* finished with header, now convert data */
	convertCS229Data(infile, outfile, bitDepth, samples, channels);
	
	return 0;	
}

/* a helper method to print the header for a CS229 file. 
	Ideally, I would use this inside of the previous method for converting an AIFF file to CS229,
	but time constraints are rough. 
	This prints the header, taking into account that we will be concatenating
	multiple files together.
	Takes an array of file pointers which we will be concatenating,
	an int giving the number of files in that array, and a 
	file pointer that we want to write the output to. */
void printCS229Header(FILE* files[], int numFiles, FILE* writeTo)
{
	int totalBitDepth = 0;
	int totalSamples = 0;
	int totalChannels = 0;
	long double totalSampleRate = 0; 
	
	FILE *firstFile = files[0];
	
	rewind(firstFile);
	
	/* Get information from the first file that needs to be consistent
	throughout all the files (bit depth, channels, sample rate) */
	int firstFileType = identifyFile(firstFile);
	if(1 == firstFileType)
	{
		parse229File(firstFile, "dummy");
		totalBitDepth = atoi(get229BitDepth());
		totalSamples = get229NumSamples();
		totalChannels = get229NumChannels();
		totalSampleRate = atoi(get229SampleRate());
	}
	else if(2 == firstFileType)
	{
		parseAIFFFile(firstFile);
		totalBitDepth = getBitDepth();
		totalSamples = getNumSamples();
		totalChannels = getNumChannels();
		totalSampleRate = getFileRate();
	}
	
	/* loop to count the total number of samples */
	int i = 0;
	for(i = 1; i < numFiles; i++)
	{
		rewind(files[i]);
		
		int fileType = identifyFile(files[i]);
		if(1 == fileType)
		{
			parse229File(files[i], "dummy");
			totalSamples = totalSamples + get229NumSamples();
		}
		else if (2 == fileType)
		{
			parseAIFFFile(files[i]);
			totalSamples = totalSamples + getNumSamples();
		}
	}
	
	/* print header */
	fputs("CS229\n", writeTo);
	fputs("\n", writeTo);
	fputs("\n", writeTo);
	
	fprintf(writeTo, "SampleRate  %Lg\n", totalSampleRate);
	fprintf(writeTo, "Channels  %i\n", totalChannels);
	fprintf(writeTo, "BitDepth  %i\n", totalBitDepth);
	fprintf(writeTo, "Samples  %i\n", totalSamples);
	fprintf(writeTo, "\n\n");

	fputs("StartData\n", writeTo);
}

/* A helper method to print the header for an AIFF file. 
	Analogous to above method, but now we have to convert
	everything we're putting in into little endian */
void printAIFFHeader(FILE* files[], int numFiles, FILE* writeTo)
{
	int totalBitDepth = 0;
	int totalSamples = 0;
	int totalChannels = 0;
	long double totalSampleRate = 0; 
	
	FILE *firstFile = files[0];
	
	rewind(firstFile);
	
	int firstFileType = identifyFile(firstFile);
	if(1 == firstFileType)
	{
		parse229File(firstFile, "dummy");
		totalBitDepth = atoi(get229BitDepth());
		totalSamples = get229NumSamples();
		totalChannels = get229NumChannels();
		totalSampleRate = atoi(get229SampleRate());
	}
	else if(2 == firstFileType)
	{
		parseAIFFFile(firstFile);
		totalBitDepth = getBitDepth();
		totalSamples = getNumSamples();
		totalChannels = getNumChannels();
		totalSampleRate = getFileRate();
	}
	
	/* loop to count the total number of samples */
	int i = 0;
	for(i = 1; i < numFiles; i++)
	{
		rewind(files[i]);
		
		int fileType = identifyFile(files[i]);
		if(1 == fileType)
		{
			parse229File(files[i], "dummy");
			totalSamples = totalSamples + get229NumSamples();
		}
		else if (2 == fileType)
		{
			parseAIFFFile(files[i]);
			totalSamples = totalSamples + getNumSamples();
		}
	}
	
	/* put in FORM and remaining bytes in the file */
	fputs("FORM", writeTo);
	
	unsigned int totalRemainingBytes = 46 + (totalSamples * totalBitDepth/8 * totalChannels);
	char tRB[5];

	tRB[0] = (totalRemainingBytes >> 24) & 0xFF;
	tRB[1] = (totalRemainingBytes >> 16) & 0xFF;
	tRB[2] = (totalRemainingBytes >> 8) & 0xFF;
	tRB[3] = totalRemainingBytes & 0xFF; 

	
	fwrite(tRB, sizeof(char), 4, writeTo);
	
	/* put AIFF in there */
	fputs("AIFF", writeTo);
	
	/* start the COMM chunk and size = 18 */
	fputs("COMM", writeTo);
	
	unsigned int commRemainingBytes = 18;
	char cRB[5];

	cRB[0] = (commRemainingBytes >> 24) & 0xFF;
	cRB[1] = (commRemainingBytes >> 16) & 0xFF;
	cRB[2] = (commRemainingBytes >> 8) & 0xFF;
	cRB[3] = commRemainingBytes & 0xFF;
	
	fwrite(cRB, sizeof(char), 4, writeTo);
	
	/* put in numChannels */
	char channelBytes[3];
	
	channelBytes[0] = (totalChannels >> 8) & 0xFF;
	channelBytes[1] = totalChannels & 0xFF;
	
	fwrite(channelBytes, sizeof(char), 2, writeTo);
	
	/* put in numSampleFrames (samples)*/
	char numSamplesBytes[5];
	
	numSamplesBytes[0] = (totalSamples >> 24) & 0xFF;
	numSamplesBytes[1] = (totalSamples >> 16) & 0xFF;
	numSamplesBytes[2] = (totalSamples >> 8) & 0xFF;
	numSamplesBytes[3] = totalSamples & 0xFF;
	
	fwrite(numSamplesBytes, sizeof(char), 4, writeTo);
	
	/* put in bitDepth */
	char bitDepthBytes[3];
	
	bitDepthBytes[0] = (totalBitDepth >> 8) & 0xFF;
	bitDepthBytes[1] = totalBitDepth & 0xFF;
	
	fwrite(bitDepthBytes, sizeof(char), 2, writeTo);
	
	/* put in sampleRate */
	char sampleRateBytes[sizeof(long double)];
	memcpy(sampleRateBytes, &totalSampleRate, sizeof(sampleRateBytes));
	
	swap(sampleRateBytes, 0, 9);
	swap(sampleRateBytes, 1, 8);
	swap(sampleRateBytes, 2, 7);
	swap(sampleRateBytes, 3, 6);
	swap(sampleRateBytes, 4, 5);
	
	fwrite(sampleRateBytes, sizeof(char), 10, writeTo);
	
	/* start the SSND chunk and size! */
	fputs("SSND", writeTo);
	
	unsigned int ssndRemainingBytes = 8 + (totalSamples * totalBitDepth/8 * totalChannels);
	
	char sRB[5];
	sRB[0] = (ssndRemainingBytes >> 24) & 0xFF;
	sRB[1] = (ssndRemainingBytes >> 16) & 0xFF;
	sRB[2] = (ssndRemainingBytes >> 8) & 0xFF;
	sRB[3] = ssndRemainingBytes & 0xFF;
	
	fwrite(sRB, sizeof(char), 4, writeTo);
	
	/* put in offset */
	unsigned int offset = 0;
	char offsetBytes[5];

	offsetBytes[0] = (offset >> 24) & 0xFF;
	offsetBytes[1] = (offset >> 16) & 0xFF;
	offsetBytes[2] = (offset >> 8) & 0xFF;
	offsetBytes[3] = offset & 0xFF; 
	
	fwrite(offsetBytes, sizeof(char), 4, writeTo);
	
	/* put in blocksize */
	unsigned int blockSize = 0;
	char blockSizeBytes[5];

	blockSizeBytes[0] = (blockSize >> 24) & 0xFF;
	blockSizeBytes[1] = (blockSize >> 16) & 0xFF;
	blockSizeBytes[2] = (blockSize >> 8) & 0xFF;
	blockSizeBytes[3] = blockSize & 0xFF; 
	
	fwrite(blockSizeBytes, sizeof(char), 4, writeTo);
}