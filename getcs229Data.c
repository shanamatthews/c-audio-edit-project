#include <stdio.h>
#include <string.h>
#include "ReadFile.h"
#include "getcs229Data.h"

int file229Samples = -1;
int file229Channels = -1;
char file229BitDepth[3];
char file229Rate[50]; 

char numChannels[50];

/* parses a 229 file to get it's header data */
int parse229File(FILE* file, char filename[256])
{
	/* The strings we're searching for */
	char samples[8] = {"Samples"};
	char channels[9] = {"Channels"};
	char bitDepth[9] = {"BitDepth"};
	char sampleRate[11] = {"SampleRate"};
	char startData[10] = {"StartData"};
	
	char numSamples[50];

	char numBitDepth[3];
	char numSampleRate[50];
	
	int samplesFound = 0;
	int channelsFound = 0;
	int bitDepthFound = 0;
	int sampleRateFound = 0;
	int startDataFound = 0;
	
	int samplesCounted = 0;
	int channelsCounted = 0;
	
	
	/* Move to next line, past the CS229 header to get data */
	nextLine(file);
	
	/* Now we're into the rest of the file. Time to parse. */
	
	/* Get the first char of the new line to see if we care about that line. */
	char c = fgetc(file);
	int i = 0;

	while(!startDataFound && EOF != c)
	{
		/* while('\n' == c && EOF != c) */
		{
			/* c = fgetc(file); */
			
			/* skip if this line is a comment */
			if('#' == c)
			{
				char str[1000];
				fgets(str, 1000, file);
			}
		}
		
		/* looking for the samples flag */
		while(c == samples[i] && !samplesFound && EOF != c)
		{
			c = fgetc(file);
			i++;
			/* we found all the characters in the samples flag,
				so the next data on the line should be */
			if('\0' == samples[i])
			{
				/* reset i so we can get the next category of data */
				i = 0;
				samplesFound = 1;
				
				/* ignore all whitespace before ints */
				while(!(c > 47 && c < 58) && EOF != c)
				{
					c = fgetc(file);
				}
				int j = 0;
				/* grab the digits of the int into a char array */
				while((c > 47 && c < 58) && EOF != c)
				{
					numSamples[j] = c;
					c = fgetc(file);
					j++;			
				}
				/* put a null terminator so we can tell where our digits end */
				numSamples[j] = '\0';
			}
		}
		while(c == sampleRate[i] && !sampleRateFound && EOF != c)
		{
			c = fgetc(file);
			i++;
			/* if we got to the end of this string,
			then this line contains the # samples we need to store */
			if('\0' == samples[i])
			{
				/* reset i so we can get the next category of data */
				i = 0;
				sampleRateFound = 1;
				
				/* ignore all whitespace before ints */
				while(!(c > 47 && c < 58) && EOF != c)
				{
					c = fgetc(file);
				}
				
				int j = 0;
				while((c > 47 && c < 58) && EOF != c)
				{
					numSampleRate[j] = c;
					c = fgetc(file);
					j++;			
				}
				/* put a null terminator so we can tell where our digits end */
				numSampleRate[j] = '\0';
				
				strcpy(file229Rate, numSampleRate);
			}
		}
		while(c == channels[i] && !channelsFound && EOF != c)
		{
			c = fgetc(file);
			i++;
			/* if we got to the end of this string,
			then this line contains the # of samples we need to store */
			if('\0' == samples[i])
			{
				/* reset i so we can get the next category of data */
				i = 0;
				channelsFound = 1;
				
				/* ignore all whitespace before ints */
				while(!(c > 47 && c < 58) && EOF != c)
				{
					c = fgetc(file);
				}
				/* now we are dealing with an int representing the # samples */
				int j = 0;
				while((c > 47 && c < 58) && EOF != c)
				{
					/* This is quite a bad way to do this. */
					numChannels[j] = c;
					c = fgetc(file);
					j++;			
				}
				/* put a null terminator so we can tell where our digits end */
				numChannels[j] = '\0';
			}
		}
		while(c == bitDepth[i]&& !bitDepthFound && EOF != c)
		{
			c = fgetc(file);
			i++;
			/* if we got to the end of this string,
			then this line contains the # samples we need to store */
			if('\0' == samples[i])
			{
				/* reset i so we can get the next category of data */
				i = 0;
				bitDepthFound = 1;
				
				/* ignore all whitespace before ints */
				while(!(c > 47 && c < 58) && EOF != c)
				{
					c = fgetc(file);
				}
				/* now we are dealing with an int representing the # samples */
				int j = 0;
				while((c > 47 && c < 58) && EOF != c)
				{
					/* This is quite a bad way to do this. */
					numBitDepth[j] = c;
					c = fgetc(file);
					j++;			
				}
				/* put a null terminator so we can tell where our digits end */
				numBitDepth[j] = '\0';
				
				strcpy(file229BitDepth, numBitDepth);
			}
		}
		while(c == startData[i]&& !startDataFound && EOF != c)
		{
			c = fgetc(file);
			i++;
			/* if we got to the end of this string,
			then this line contains the # samples we need to store */
			if('\0' == samples[i])
			{
				/* reset i so we can get the next category of data */
				i = 0;
				startDataFound = 1;
			}
		}
		c = fgetc(file);
	}
	
	nextLine(file);
	
	samplesCounted = countSamples(file);
	rewind(file);
	channelsCounted = countChannels(file);
	
	/* set pointer back to the beginning for file parsing */
	rewind(file);
	
	return 0;
}

/* runs through the file counting the number of samples in the specified file */
int countSamples(FILE* file)
{
	rewind(file);
	int c = findStartData(file);
	
	int sampleCount = 0; 
	while(c != EOF)
	{
		if(((c > 47 && c < 58) || (45 == c)) && EOF != c)
		{
			sampleCount++;
			char sample[1000];
			fgets(sample, 1000, file);
		}
		else
		{
			fprintf(stderr, "%c\n", c);
			break;
		}
		c = fgetc(file);
	}

	file229Samples = sampleCount;

	return sampleCount;
}

/* counts the number of channels in the first sample of the file specified */
int countChannels(FILE* file)
{
	rewind(file);
	int c = findStartData(file);
	if(EOF == c)
	{
		return -1;
	}
	/* find start data takes one char too many for my taste. */
	ungetc(c, file);
	
	int channelCount = 0;
	char prev = fgetc(file);
	char curr = fgetc(file);
	while(prev != '\n' && EOF != prev && EOF != curr)
	{
		/* if the previous character was a digit and this one is now white space, 
			that means that we passed a channel */
		if((prev > 47 && prev < 58) && (curr < 46 || curr > 57))
		{
			channelCount++;
		}
		prev = curr;
		curr = fgetc(file);
	}
	
	file229Channels = channelCount;
	
	return channelCount;
}

/* skips a line in the file given. I made this method before I figured out what fgets(...) was... */
char nextLine(FILE* file)
{
	char c;
	for(c = fgetc(file); '\n' != c; c = fgetc(file))
	{
	}
	
	return c;
}

int get229NumSamples()
{
	return file229Samples;
}

int get229NumChannels()
{
	/* if we couldn't count the number of channels, we use the number specified in the header */
	if(0 == file229Samples)
	{
		file229Channels = atoi(numChannels);
	}
	return file229Channels;
}

/* returns a string representing the bit depth according to the header data */
char* get229BitDepth()
{
	return file229BitDepth;
}

/* returns a string representing the sample rate according to the header data */
char* get229SampleRate()
{
	return file229Rate;
}