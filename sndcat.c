#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "sndcat.h"
#include "ReadFile.h"
#include "getcs229Data.h"
#include "getAIFFData.h"
#include "convertData.h"
#include "concatHelpers.h" 

int main(int argc, char *argv[]) 
{
	int CS229output = 0;
	int AIFFoutput = 0;
	
	/* 1 if CS229, 2 if AIFF */
	int returnType = 0;
	
	int numFiles = argc - 2;
	char *fileNames[numFiles];
	
	int i = 0;
	int fileCount = 0;
	/* start i at 1 so we skip the executable name
		check for flags. */
	for(i = 1; i < argc; i++)
	{
		if(strcmp("-h", argv[i]) == 0)
		{
			displayCatHelp();
			return 0;
		}
		if(strcmp("-c", argv[i]) == 0)
		{
			if(AIFFoutput)
			{
				fprintf(stderr, "You can't specify both CS229 and AIFF. Try again.\n");
				return 1;
			}
			CS229output = 1;
			returnType = 1;
			continue;
		}
		if(strcmp("-a", argv[i]) == 0)	
		{
			if(CS229output)
			{
				fprintf(stderr, "You can't specify both CS229 and AIFF. Try again.\n");
				return 1;
			}
			AIFFoutput = 1;
			returnType = 2;
			continue;
		}	
		else
		{
			fileNames[fileCount] = argv[i];
			fileCount++;
		}
	}
	
	/* They didn't specify how to output the file, so throw an error. */
	if(!(AIFFoutput || CS229output))
	{
		fprintf(stderr, "You must specify whether to output in CS229 or AIFF format!\n");
		fprintf(stderr, "Use the help switch for more information.\n");
		return 1;
	}
	
	/* if we DID concat, tryConcat returns 0. */
	int concat = tryConcat(fileNames, fileCount, returnType);
	if(!tryConcat)
	{
		return -1;
	}
	
	return 0;
}


/* Attempts to concatenate the sample data from a number of files.
	Takes an array of strings that are the file names to be loaded,
	a int specifying the number of files and
	an int specifying what type of file the concatenated result should be. 
	returnType = 1 for CS229, 2 for AIFF */
int tryConcat(char *fileNames[], int numFiles, int returnType)
{
	/* if we aren't supposed to concatenate any files, just print whatever
		we're given out to stdout. */
	if(0 == numFiles)
	{
		int c = getchar();
		while(EOF != c)
		{
			fprintf(stdout, "%c", c);
			c = getchar();
		}
	}
	else
	{
		/* Try to load each of the files. */
		FILE *files[numFiles];
	
		int filesLoaded = 0;
	
		int i = 0;
		for(i = 0; i < numFiles; i++)
		{
			FILE *file = fopen(fileNames[i], "r");
			if(!file)
			{
				return -1;
			}
			files[filesLoaded] = file;
			filesLoaded++;
		}
	
		int filesNotCompatible = compareFileData(files, numFiles);
		
		if(filesNotCompatible)
		{
			return -1;
		}
	
		/* presumably, if we got here, all the data is compatible. ITS CONCATENATION TIME! */
		actuallyConcatenate(files, numFiles, returnType);
	}

	
	return 0;
}

/* Directs the files to the proper concatenation method 
	depending on the file type.
	Takes an array of files to concatenate,
	an int giving the number of files to concatenate,
	and an int telling us which type to return as
	returnType = 1 for CS229, 2 for AIFF */
int actuallyConcatenate(FILE* files[], int numFiles, int returnType)
{
	if(1 == returnType)
	{
		printAsCS229(files, numFiles, stdout);
	}
	else if(2 == returnType)
	{
		printAsAIFF(files, numFiles);
	}
}

/* Does the actual concatenation work. 
	Takes an array of file pointers to concatenate the data from and
	an int specifying the number of files we will concatenate. */
void printAsAIFF(FILE* files[], int numFiles)
{
	/* print AIFF header */
	printAIFFHeader(files, numFiles, stdout);

	/* go through all the files */
	int i =0;
	for(i = 0; i < numFiles; i++)
	{
		rewind(files[i]); 
	
		FILE *thisFile = files[i];
		int fileType = identifyFile(files[i]);
		/* CS229 file  - we DO need to convert data*/
		if(1 == fileType)
		{
			/* get information for bitDepth, samples, channels */
			parse229File(files[i], "Dummy");
			int bitDepth = atoi(get229BitDepth());
			int channels = get229NumChannels();
			int samples = get229NumSamples();
			
			convertCS229Data(files[i], stdout, bitDepth, samples, channels);
		}
		/* AIFF file - we don't need to convert data */
		else if(2 == fileType)
		{
			int c = findAIFFDataChunk(files[i]);
			/* skip those 3 4-byte ints */
			int j = 0;
			for(j = 0; j < 12; j++)
			{
				c = fgetc(files[i]);
			}
			
			/* print out data */
			while(EOF != c)
			{
				fprintf(stdout, "%c", c);
				c = fgetc(files[i]);
			}
		}
	}
}

/* Compares the data between each of the files to make sure they are compatible 
	for concatenation.
	Returns 0 if the file types are compatible for concatenation, -1 otherwise. */
int compareFileData(FILE *files[], int numFiles)
{
	/* get the bitDepth, sampleRate, channels that ALL files should have in common 
		from the first file in the array. */
	int fileType = identifyFile(files[0]);
	unsigned int numChannels = 0;
	long double sampleRate = 0;
	unsigned int bitDepth = 0;
	
	if(1 == fileType)
	{
		/* its a CS229 file */
		parse229File(files[0], "Dummy value");
		numChannels = get229NumChannels();
		sampleRate = atoi(get229SampleRate());
		bitDepth = atoi(get229BitDepth());
	}
	else if(2 == fileType)
	{
		/* its an AIFF file */
		parseAIFFFile(files[0]);		
		numChannels = getNumChannels();
		sampleRate = getFileRate();
		bitDepth = getBitDepth();
	}
	else
	{
		/* ragequit because that's not even a valid file type. 
		HOW AM I SUPPOSED TO CONCATENATE THAT?!? */
		fprintf(stderr, "At least one of those files isn't a valid CS229 or AIFF file.\n");
		fprintf(stderr, "Please try again!\n");
		return -1;
	}
	
	int i = 0;
	for(i = 1; i < numFiles; i++)
	{
		int thisFileType = identifyFile(files[i]);
		if(1 == thisFileType)
		{
			parse229File(files[i], "Dummy value");
			if((numChannels != get229NumChannels()) || (sampleRate != atoi(get229SampleRate())) || (bitDepth != atoi(get229BitDepth())))
			{
				/* ragequit */
				fprintf(stderr, "These files do not all have matching values for: \n");
				fprintf(stderr, "Bit depth\n");
				fprintf(stderr, "Sample rate\n");
				fprintf(stderr, "Channels\n");
				fprintf(stderr, "Try again with files that have matching values for this data.\n");
				
				return -1;
			}
		}
		else if(2 == thisFileType)
		{
			parseAIFFFile(files[i]);
			if((numChannels != getNumChannels()) || (sampleRate != getFileRate()) || (bitDepth != getBitDepth()))
			{
				/* ragequit */
				fprintf(stderr, "These files do not all have matching values for: \n");
				fprintf(stderr, "Bit depth\n");
				fprintf(stderr, "Sample rate\n");
				fprintf(stderr, "Channels\n");
				fprintf(stderr, "Try again with files that have matching values for this data.\n");  
				
				return -1;
			}
		}
	}
	
	return 0;
}

void displayCatHelp() 
{
	fprintf(stderr, "Do you need help?\n");
	fprintf(stderr, "You can type './sndcat -h' for help!\n");
	fprintf(stderr, "Enter files to concatenate in the form:\n");
	fprintf(stderr, "   $ sndcat [switches] [file] [file]\n");
	fprintf(stderr, "If you don't specify files to concatenate, the program reads from stdin.\n");
	fprintf(stderr, "You must use flag '-a' to output in AIFF format\n");
	fprintf(stderr, "or flag '-c' to output in CS229 format.\n");
	fprintf(stderr, "If you don't use one of those flags, the program will terminate.\n");
}