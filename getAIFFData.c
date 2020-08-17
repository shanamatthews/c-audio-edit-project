#include <stdio.h>  
#include "ReadFile.h"
#include "getAIFFData.h"

static int fileSamples = -1;
static int fileChannels = -1;
static int fileBitDepth = -1;
static long double fileRate = -1;

/* helper method to switch endian-ness */
int swap(char toSwap[256], int swap1, int swap2)
{
	char temp = toSwap[swap1];
	toSwap[swap1] = toSwap[swap2];
	toSwap[swap2] = temp; 
}

/* search through the header of an AIFF file to gather information 
	Takes a file pointer to the file to parse, and the filename,
	which is basically superfluous code right now. */
int parseAIFFFile(FILE* file) 
{
	/* the string we're searching for */
	char comm[5] = {"COMM"};
	
	int commLength = 0;
	
	int commFound = 0;
	char c = fgetc(file);
	
	while(!commFound && EOF != c)
	{
		int i = 0;
		/* we hit something that looks like the start of the file info */
		while(c == comm[i] && !commFound && EOF != c)
		{
			/* we definitely got to the string signifying the start of the file info */
			i++;
			if('\0' == comm[i])
			{
				i = 0;
				commFound = 1;
				break;
			}
			c = fgetc(file);
		}
		c = fgetc(file); 
	}
	/* we found the beginning of the header information */
	if(commFound)
	{
		char commLengthChars[5];
		int commLength;
	
		int i;
		for(i = 0; i < 4; i++)
		{
			commLengthChars[i] = c;
			c = fgetc(file);
		}
		
		swap(commLengthChars, 0, 3);
		swap(commLengthChars, 1, 2);
		
		/* the number of channels specified */
		char numChannels[3];
		
		for(i = 0; i < 2; i++)
		{
			numChannels[i] = c;			
			c = fgetc(file);
		}
		
		/* switch endian-ness so we can read */
		swap(numChannels, 0, 1);
		int channels = *(int*) numChannels;
		
		/* not really sure why I need to do this or why it works */
		fileChannels = numChannels[0];
		
		
		/* same procedure for sample frames */
		char numSampleFrames[5];
		
		for(i = 0; i < 4; i++)
		{
			numSampleFrames[i] = c;
			c = fgetc(file);
		}
		
		swap(numSampleFrames, 0, 3);
		swap(numSampleFrames, 1, 2);
		
		int samples = *(int*)numSampleFrames;
		fileSamples = samples;
		
		/* and again for sample size */
		char sampleSize[3];
		
		for(i = 0; i < 2; i++)
		{
			sampleSize[i] = c;
			c = fgetc(file);
		}
		
		/* This value will be 8, 16, or 32! */
		swap(sampleSize, 0, 1);
		
		
		int bitDepth = *(int*) sampleSize;
		
		/* again, not sure why this is neccessary, or even works. Sad. */
		fileBitDepth = sampleSize[0];
		
		
		/* This is a IEEE Standard 754 80-bit floating point value 
		representing the sample rate in sample frames/s */
		char sampleRate[11];
		
		for(i = 0; i < 10; i++)
		{
			sampleRate[i] = c;
			c = fgetc(file);
		}
		
		ungetc(c, file);
		
		swap(sampleRate, 0, 9);
		swap(sampleRate, 1, 8);
		swap(sampleRate, 2, 7);
		swap(sampleRate, 3, 6);
		swap(sampleRate, 4, 5);
	
		long double rate = *(long double*)sampleRate;
		fileRate = rate;
	}
	
	return 0;
}

/* returns the number of samples specified by the header 
	If any of these are called before the  parse file method
	is called, then the values given will not be correct */
int getNumSamples()
{
	return fileSamples;
}

/* returns the number of channels specified by the header */
int getNumChannels()
{
	return fileChannels;
}

/* returns the bit depth specified by the header */
int getBitDepth()
{
	return fileBitDepth;
}

/* returns the sample rate specified by the header */
long double getFileRate()
{
	return fileRate;
}



