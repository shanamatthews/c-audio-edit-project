#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "ReadFile.h"
#include "getcs229Data.h"
#include "getAIFFData.h"
#include "sndconv.h"
#include "convertData.h"

/* Accepts an array of file pointers, an int giving the number of files
	in that array, and a file pointer indicating where to print to.
	Prints a CS229 header and concatenates all the data from the files in 
	the array, regardless of file type. */
void printAsCS229(FILE* files[], int numFiles, FILE* printTo) 
{
	/* Print CS229 header using data from the first file. */
	printCS229Header(files, numFiles, printTo);
	/* go through all the files */
	int i =0;
	for(i = 0; i < numFiles; i++)
	{
		rewind(files[i]);
		
		FILE *thisFile = files[i];
		int fileType = identifyFile(files[i]);
		
		rewind(files[i]); 
		
		/* CS229 file - we don't need to convert data */
		if(1 == fileType)
		{
			char c = findStartData(files[i]);
			/* just print the data */
			while(EOF != c)
			{
				fprintf(printTo, "%c", c);
				c = fgetc(files[i]);
			}
		}
		/* AIFF file - we DO need to convert data */
		else if(2 == fileType)
		{
			/* get information on bitDepth, samples, channels */
			parseAIFFFile(files[i]);
			int bitDepth = getBitDepth();
			int channels = getNumChannels();
			int samples = getNumSamples(); 
		
			char c = findAIFFDataChunk(files[i]);
			/* skip 3 4-byte integers */
			int j = 0;
			for(j = 0; j < 12; j++)
			{
				c = fgetc(files[i]);
			}
			 
			/* convert the data to CS229 */
			convertAIFFData(files[i], printTo, bitDepth, samples, channels, c); 
		}
	}
}