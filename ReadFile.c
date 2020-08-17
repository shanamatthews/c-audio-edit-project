#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include "sndinfo.h"
#include "ReadFile.h"

/*
**	Contains helper methods for reading CS229 and AIFF files 
*/

/* Reads the first few characters of a file, scanning for the CS229 or FORM 
	flag to identify the file.
	Returns 1 for CS229, 2 for AIFF, 3 for anything else. */
int identifyFile(FILE* file)
{
	/* need to be 6 chars long for some reason? */
	char identifiers[5];
	char file229Identifiers[5] = {"CS22"};
	char AIFFIdentifiers[5] = {"FORM"};
	
	rewind(file);
	fgets(identifiers, 5, file);
	
	
	if(strcmp(identifiers, file229Identifiers) == 0)
	{
		return 1;
	}
	else if(strcmp(identifiers, AIFFIdentifiers) == 0)
	{
		return 2;
	}
	else
	{
		fprintf(stderr, "That's not a file type we can parse.\n"); 
		fprintf(stderr, "%s\n", identifiers); 
		return 3;
	}

	return 0;
}

/* Prints the file information for a CS229 file to stdout
	If multiple = 0, there is only one file to be printed,
	else, there are multiple files to be printed.
	Takes a file pointer from which to take data, 
	a string to print for the file name, 
	an int representing the number of samples in the file, 
	an int representing the number of channels in the file,  
	a string representing the bit depth for the file, 
	a string representing the sample rate for the file, 
	and an int telling us if we are about to print multiple files or just one.*/
int print229FileInfo(FILE* file, char filename[], int samplesCounted, int channelsCounted, char bitDepth[], char sampleRate[], int multiple)
{ 
	/* translates the string representing the sample rate into a float
		that we can use to calculate the length of the sample */
	float sampleRateFloat; 
	sscanf(sampleRate, "%f", &sampleRateFloat);
	
	/* calculate the length in time of the sample */
	float timeInSeconds = (1/sampleRateFloat) * samplesCounted;
	int timeInMinutes = (int)timeInSeconds/60;
	int timeInHours = timeInMinutes/60;
	int remainingMinutes = ((int)timeInSeconds % 3600)/60;
	double remainingSeconds = timeInSeconds - (timeInHours*3600 + timeInMinutes*60);
	
	print60Dashes();
	
	printf("Filename: %s\n", filename);
	printf("Format: CS229\n");
	printf("Sample Rate: %s\n", sampleRate);
	printf("Bit Depth: %s\n", bitDepth);
	printf("Channels: %u\n", channelsCounted);
	printf("Samples: %u\n", (samplesCounted > 0 ? samplesCounted : 0));
	printf("Duration: %d:%d:%.2f\n", timeInHours, remainingMinutes, remainingSeconds); 
	
	if(!multiple)
	{
		print60Dashes();
	}
	
	
	return 0;
}

/* If multiple = 0, there is only one file to be printed,
else, there are multiple files to be printed. */
int printAIFFFileInfo(FILE* file, char filename[], int channels, int samples, int bitDepth, long double sampleRate, int multiple)
{
	float timeInSeconds = (1/sampleRate) * samples;
	int timeInMinutes = (int)timeInSeconds/60;
	int timeInHours = timeInMinutes/60;
	int remainingMinutes = ((int)timeInSeconds % 3600)/60;
	float remainingSeconds = timeInSeconds - (timeInHours*3600 + timeInMinutes*60);
	
	char * name;
	char * name2;
	
	print60Dashes();
	
	printf("Filename: %s\n", filename);
	printf("Format: AIFF\n");
	printf("Sample Rate: %Lg\n", sampleRate);
	printf("Bit Depth: %u\n", bitDepth);
	printf("Channels: %u\n", channels);
	printf("Samples: %u\n", samples);
	printf("Duration: %d:%.2d:%05.2f\n", timeInHours, remainingMinutes, remainingSeconds);
	
	if(!multiple)
	{
		print60Dashes();
	}
}

int print60Dashes()
{
	int i;
	for(i = 0; i < 60; i++)
	{
		printf("-");
	}
	printf("\n");
	return 0;
}

/* finds where the sample data starts in a CS229 file */
char findStartData(FILE* file)
{
	char startData[] = {"StartData"};
	int startDataFound = 0;
	
	char c = getc(file);
	int i = 0;
	
	while(!startDataFound && EOF != c)
	{
		while('x' == c)
		{
			char str[1000];
			fgets(str, 1000, file);
		}
		i = 0;
		while(startData[i] == c && !startDataFound)
		{
			i++;
			if('\0' == startData[i])
			{
				startDataFound = 1;
				break;
			}
			if('#' == c)
			{
				char str[1000];
				fgets(str, 1000, file);
				i = 0;
				break;
			}
			c = fgetc(file);
		}
		c = fgetc(file);
	}
	if(startDataFound)
	{
		c = fgetc(file);
		return c;
	}
	else
	{
		return EOF;
	}
}

/* Finds the beginning of the sample data in an AIFF file
	returns the char RIGHT after SSND is found 
	To get to the data, we still have to skip through 3 4-byte ints */
char findAIFFDataChunk(FILE* file)
{
	char SSND[] = {"SSND"};
	int  SSNDFound = 0;
	
	char c = getc(file);
	int i = 0;
	
	while(!SSNDFound && EOF != c)
	{
		i = 0;
		while(SSND[i] == c && !SSNDFound)
		{
			i++;
			if('\0' == SSND[i])
			{
				SSNDFound = 1;
				break;
			}
			c = fgetc(file); 
		}
		c = fgetc(file);

	}
	
	if(SSNDFound)
	{
		return c;
	}
	else
	{
		return EOF;
	}
}

/* Creates an ASCII representation of a single channel of a 
	a single sample from a CS229 file */
char* makeString(int width, int bitDepth, int dataPoint)
{
	double maxSize = 0; 
	if(8 == bitDepth)
	{
		maxSize = 127; 
	}
	else if(16 == bitDepth)
	{
		maxSize = 32767;
	}
	else if(32 == bitDepth)
	{
		maxSize = 2147483647;
	}
	
	maxSize = maxSize * 2;
	
	/* 12 for formatting stuff */
	int sampleWidth = width - 12;
	
	char *toRet = malloc(width - 10);
	strcpy(toRet, "|");	
	
	/* fill up one string with all spaces. Either use for - or + vals */
	char *blanks = malloc((sampleWidth/2 + 1));
	strcpy(blanks, " ");
	int i = 0;
	for(i = 0; i < (sampleWidth/2 - 1); i++)
	{
		strcat(blanks, " ");
	}
	strcat(blanks, "\0");

	
	int blocksToShow = (dataPoint/maxSize) * (sampleWidth);

	
	/* the data point was negative, so it goes on the left */
	if(blocksToShow < 0)
	{
		for(i = 0; i < sampleWidth/2; i++)
		{
			if(i < (sampleWidth/2 + blocksToShow))
			{
				strcat(toRet, " ");
			}
			else
			{
				strcat(toRet, "-");
			}
		}
		/* negative part put in! Now just add blanks & formatting */
		strcat(toRet, "|");
		strcat(toRet, blanks);
	}
	/* else, the data point was 0 or positive, so it goes on the right */
	else
	{
		strcat(toRet, blanks);
		strcat(toRet, "|");
		
		for(i = 0; i < sampleWidth/2; i++)
		{
			if(i < blocksToShow)
			{
				strcat(toRet, "-");
			}
			else
			{
				strcat(toRet, " ");
			}
		}
		
	}
	free(blanks);
	strcat(toRet, "\0");
	
	return toRet;
}
