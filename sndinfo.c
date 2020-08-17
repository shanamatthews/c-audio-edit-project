#include <stdio.h> 
#include <string.h>
#include "sndinfo.h"
#include "getAIFFData.h"
#include "getcs229Data.h"
#include "ReadFile.h"


char filename[2000]; 

int main(int argc, char *argv[]) 
{	
	int i = 0;
	/* check for flags on the command line */
	for(i = 0; i < argc; i++)
	{
		if(strcmp("-h", argv[i]) == 0)
		{
			displayInfoHelp();
			return 0;
		}
		if(strcmp("-1", argv[i]) == 0)
		{
			openFile();
			return 0;
		}
		if(strcmp("-i", argv[i]) == 0)	
		{
			fprintf(stderr, "size of short = %i\n", sizeof(short));
			fprintf(stderr, "size of signed short = %i\n", sizeof(signed short));
			return 0;
		}	
	}


	if(1 == argc)
	{
		/* Now we have grab the file from stdin and put into a temp file*/
		FILE* tempFile = tmpfile();
		if(!tempFile)
		{
			fprintf(stderr, "We had a problem making a temp file. Sorry. Try again?\n");
			return -1;
		}
		
		char c = getchar();
		while(EOF != c)
		{
			putc(c, tempFile);
			c = getchar();
		}
		
		rewind(tempFile);

		
		int fileType = identifyFile(tempFile);
		if(1 == fileType)
		{
			parse229File(tempFile, filename);
		
			int samplesCounted = get229NumSamples();		
			
			int channelsCounted = get229NumChannels();

			char bitDepth[50];
			strcpy(bitDepth, file229BitDepth); 
			char sampleRate[50];
			strcpy(sampleRate, file229Rate);
					
			print229FileInfo(tempFile, "(standard input)", samplesCounted, channelsCounted, bitDepth, sampleRate, 0);
		}
		else if (2 == fileType)
		{
			parseAIFFFile(tempFile);
		
			int samples = getNumSamples();
			int channels = getNumChannels();
			int bitDepth = getBitDepth();
			long double rate = getFileRate();
	
			printAIFFFileInfo(tempFile, "(standard input)", channels, samples, bitDepth, rate, 0);
			
		}
	
		fclose(tempFile);
		remove("temp.txt");
		return 0;
	} 
	
	else
	{
		/* This is 1 because we won't have a switch then,
		so the 1 is from the exe name. */
		int numFiles = argc - 1;
		
		openMultipleFiles(numFiles, argv);
		
	}
	
	return 0; 
}

void displayInfoHelp()
{
	fprintf(stderr, "Do you need help?\n");
	fprintf(stderr, "You can type './sndinfo -h' for help!\n");
	fprintf(stderr, "Or add flag -1 to make the program run in 'classic mode'\n");
	fprintf(stderr, "You can also type arguments in the format:\n");
	fprintf(stderr, "   $ sndinfo [switches] [file] [file]\n");
	fprintf(stderr, "where the files are what you want information about.\n");
}

/* opens multiple files.
	File names given in the array of strings, the number of files given in int numFiles */
int openMultipleFiles(int numFiles, char *fileNames[])
{
	int i = 0;
	/* Try to open each of the files */
	for(i = 0; i < numFiles; i++)
	{
		FILE* infile;
		infile = fopen(fileNames[i+1], "r");
		if(!infile)
		{
			fprintf(stderr, "Something went wrong opening %s. Skipping to the next.\n", fileNames[i+1]);
			/* Actually skip to the next... */
			continue;
		}
		
		int fileType = identifyFile(infile);
		
		/* identify the file and get the data from it, then print it off using appropriate helper method */
		if(1 == fileType)
		{
			parse229File(infile, filename);
			int samplesCounted = get229NumSamples();
			int channelsCounted = get229NumChannels();
			char bitDepth[50];
			strcpy(bitDepth, file229BitDepth); 
			char sampleRate[50];
			strcpy(sampleRate, file229Rate);
		
			print229FileInfo(infile, filename, samplesCounted, channelsCounted, bitDepth, sampleRate, 1);
		}
		else if (2 == fileType)
		{
			parseAIFFFile(infile);
		
			int samples = getNumSamples();
			int channels = getNumChannels();
			int bitDepth = getBitDepth();
			long double rate = getFileRate();
	
			printAIFFFileInfo(infile, filename, channels, samples, bitDepth, rate, 1);
			
		}
	}
	
	print60Dashes();
}

/* opens just one file, gets the path from prompting the user */
int openFile()
{
	FILE* infile;
	
	fprintf(stderr, "What's the file path to the file you'd like to open?\n");
	scanf("%s", &filename);
	
	infile = fopen(filename, "r");
	if(!infile)
	{
		fprintf(stderr, "Something went wrong. We couldn't read file %s\n", &filename);
		return 1;
	}
	
	
	int fileType = identifyFile(infile);
	
	
	if(1 == fileType)
	{
		parse229File(infile, filename);
		int samplesCounted = get229NumSamples();
		int channelsCounted = get229NumChannels();
		char bitDepth[50];
		strcpy(bitDepth, file229BitDepth); 
		char sampleRate[50];
		strcpy(sampleRate, file229Rate);
		
		print229FileInfo(infile, filename, samplesCounted, channelsCounted, bitDepth, sampleRate, 0);
	}
	else if (2 == fileType)
	{
		parseAIFFFile(infile);
		
		int samples = getNumSamples();
		int channels = getNumChannels();
		int bitDepth = getBitDepth();
		long double rate = getFileRate();
	
		printAIFFFileInfo(infile, filename, channels, samples, bitDepth, rate, 0);
	}
	
	return 0;
	
}



/* 
Enter the pathname of a sound file:
<read from standard input>
------------------------------------------------------------ <60 dashes>
Filename: <filename>
Format: <CS229 or AIFF>
Sample Rate: <sample rate>
Bit Depth: <bit depth>
Channels: <number of channels>
Samples: <number of samples>
Duration: <h>:<mm>:<ss.ff>
------------------------------------------------------------ <60 dashes>
*/

/*
CS229
<header>
StartData
<samples>
<EOF>
*/

/*
CS229
# A really short sound to illustrate this file format

Samples 4
# We are doing something bizarre with the channels?
Channels 3
BitDepth 8
SampleRate 11025

# From here on things are more rigid
StartData
0 -127 127
10 45 -103
20 83 -4
30 0 99
*/