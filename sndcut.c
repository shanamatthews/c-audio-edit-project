#include <stdio.h> 
#include <string.h>
#include "sndcut.h"
#include "ReadFile.h"
#include "getcs229Data.h"
#include "getAIFFData.h"

int main(int argc, char *argv[])
{
	char *samplesToRemove[argc];
	int numSampleChunks = 0;
	
	int i = 0;
	/* load the samples to remove into another array of strings */
	for(i = 1; i < argc; i++)
	{
		if(strcmp("-h", argv[i]) == 0)
		{
			displayCutHelp();
			return 0;
		}
		else
		{
			samplesToRemove[numSampleChunks] = argv[i];
			numSampleChunks++;
		}
	}
	
	/* read from stdin */
	FILE* tempFile = tmpfile();
	if(!tempFile)
	{
		fprintf(stderr, "We had a problem making a temp file. Sorry. Try again?\n");
		return -1;
	}
		
	int c = getchar();
	while(EOF != c)
	{
		putc(c, tempFile);
		c = getchar();
	}
		
	rewind(tempFile);
	
	cutDownFile(tempFile, samplesToRemove, numSampleChunks);

	return 0;
}

/* cuts out the specified samples from the file.
	Takes a file pointer to the file to be cut down,
	an array of strings representing the samples to remove, and 
	an int giving the number of strings in the previous array. */
void cutDownFile(FILE *file, char *samplesToRemove[], int numSamplePairs)
{
	/* first, translate the strings into something more usable */
	int numSampleVals = 0;
	int sampleValues[numSamplePairs * 2];
	int n = 0;
	for(n = 0; n < numSamplePairs; n++)
	{
		char *sampleNums;
		char str[100];
		strcpy(str, samplesToRemove[n]);
		sampleNums = strtok(str, "..");
		while(sampleNums != NULL)
		{
			sampleValues[numSampleVals] = atoi(sampleNums);
			numSampleVals++;
			sampleNums = strtok(NULL, "..");
		}
	}

	
	int fileType = identifyFile(file);
	
	/* CS229 */
	if(1 == fileType)
	{
		parse229File(file, "Dummy val");
		int bitDepth = atoi(get229BitDepth());
		int samples = get229NumSamples();
		int channels = get229NumChannels();
		long double sampleRate = atoi(get229SampleRate());
		cutCS229File(file, sampleValues, numSampleVals, bitDepth, samples, channels, sampleRate);
	}
	/* AIFF */
	if(2 == fileType)
	{
		parseAIFFFile(file);
		int bitDepth = getBitDepth();
		int samples = getNumSamples();
		int channels = getNumChannels();
		long double sampleRate = getFileRate();
		cutAIFFFile(file, sampleValues, numSampleVals, bitDepth, samples, channels, sampleRate);
	}

}

/* removes samples from a CS229-formatted file.
	Takes a file pointer to remove samples from, 
	an int array containing pairs of samples to remove,
	an int giving the number of ints in the previous array
	an int giving the bit depth
	an int giving the number of samples
	an int giving the number of channels
	and a long couble giving the sample rate */
int cutCS229File(FILE *file, int samplesToRemove[], int numSampleVals, int bitDepth, int samples, int channels, long double sampleRate)
{	
	/* calculate the number of samples that we're removing */
	int samplesRemoved = 0;
	int i = 0;
	for(i = 0; i < numSampleVals; i = i+2)
	{
		int change = (samplesToRemove[i+1] - samplesToRemove[i]) +1;
		if(change < 0)
		{
			fprintf(stderr, "Please select a valid range of samples.\n");
			return -1;
		}
		samplesRemoved = samplesRemoved + change;
	}
	for(i = 0; i < numSampleVals; i++)
	{
		if(samplesToRemove[i] < 0 || samplesToRemove[i] > samples)
		{
			fprintf(stderr, "Please select a valid range of samples.\n");
			return -1;
		}
	}
	
	
	int fullNumSamples = samples;
	samples = samples - samplesRemoved;
	
	/* print a CS229 header with the num samples changed accordingly */
	fputs("CS229\n", stdout);
	fputs("\n", stdout);
	
	fprintf(stdout, "SampleRate  %Lg\n", sampleRate);
	fprintf(stdout, "Channels  %i\n", channels);
	fprintf(stdout, "BitDepth  %i\n", bitDepth);
	fprintf(stdout, "Samples  %i\n", samples);
	fprintf(stdout, "\n");
	
	/* this is 0-based */
	int thisSample = 0;
	i = 0;
	char c = findStartData(file);
	/* We're making this 0-based! */
	for(thisSample = 0; thisSample < fullNumSamples ; thisSample++)
	{
		if((i < numSampleVals) && (samplesToRemove[i] == thisSample))
		{  
			int samplesToSkip = (samplesToRemove[i+1] - samplesToRemove[i]) + 1;
			int j = 0;
			/* inclusive */
			for(j = 0; j < samplesToSkip; j++)
			{
				thisSample++;
				nextLine(file);
			}
			i = i + 2;
		}
		char str[1000];
		fgets(str, 1000, file);
		fprintf(stdout, "%s", str);
	}
	
	return 0;
}

/* cuts down an AIFF file by removing the samples specified in the array.
	Takes a file pointer to the file to be cut down,
	an int array giving the index of the samples to be removed,
	an int giving the size of that array,
	an int specifying the file's bit depth,
	an int specifying the number of samples in the file,
	an int specifying thhe number of channels in the file,
	and a long double giving the file's sample rate. */
int cutAIFFFile(FILE *file, int samplesToRemove[], int numSampleVals, int bitDepth, int samples, int channels, long double sampleRate)
{
	/* calculate the number of samples that we're removing */
	int samplesRemoved = 0;
	int i = 0;
	for(i = 0; i < numSampleVals; i = i+2)
	{
		int change = (samplesToRemove[i+1] - samplesToRemove[i]) +1;
		if(change < 0)
		{
			fprintf(stderr, "Please select a valid range of samples.\n");
			return -1;
		}
		samplesRemoved = samplesRemoved + change;
	}
	
	/* if the samples specified to be removed aren't valid. */
	for(i = 0; i < numSampleVals; i++)
	{
		if(samplesToRemove[i] < 0 || samplesToRemove[i] > samples)
		{
			fprintf(stderr, "Please select a valid range of samples.\n");
			return -1;
		}
	}
	
	int fullNumSamples = samples;
	samples = samples - samplesRemoved;
	
	/* Print an AIFF header */
	{
	
	/* put in FORM and remaining bytes in the file */
	fputs("FORM", stdout);
	
	unsigned int totalRemainingBytes = 46 + (samples * bitDepth/8 * channels);
	char tRB[5];

	tRB[0] = (totalRemainingBytes >> 24) & 0xFF;
	tRB[1] = (totalRemainingBytes >> 16) & 0xFF;
	tRB[2] = (totalRemainingBytes >> 8) & 0xFF;
	tRB[3] = totalRemainingBytes & 0xFF; 

	
	fwrite(tRB, sizeof(char), 4, stdout);
	
	/* put AIFF in there */
	fputs("AIFF", stdout);
	
	/* start the COMM chunk and size = 18 */
	fputs("COMM", stdout);
	
	unsigned int commRemainingBytes = 18;
	char cRB[5];

	cRB[0] = (commRemainingBytes >> 24) & 0xFF;
	cRB[1] = (commRemainingBytes >> 16) & 0xFF;
	cRB[2] = (commRemainingBytes >> 8) & 0xFF;
	cRB[3] = commRemainingBytes & 0xFF;
	
	fwrite(cRB, sizeof(char), 4, stdout);
	
	/* put in numChannels */
	char channelBytes[3];
	
	channelBytes[0] = (channels >> 8) & 0xFF;
	channelBytes[1] = channels & 0xFF;
	
	fwrite(channelBytes, sizeof(char), 2, stdout);
	
	/* put in numSampleFrames (samples)*/
	char numSamplesBytes[5];
	
	numSamplesBytes[0] = (samples >> 24) & 0xFF;
	numSamplesBytes[1] = (samples >> 16) & 0xFF;
	numSamplesBytes[2] = (samples >> 8) & 0xFF;
	numSamplesBytes[3] = samples & 0xFF;
	
	fwrite(numSamplesBytes, sizeof(char), 4, stdout);
	
	/* put in bitDepth */
	char bitDepthBytes[3];
	
	bitDepthBytes[0] = (bitDepth >> 8) & 0xFF;
	bitDepthBytes[1] = bitDepth & 0xFF;
	
	fwrite(bitDepthBytes, sizeof(char), 2, stdout);
	
	/* put in sampleRate */
	char sampleRateBytes[sizeof(long double)];
	memcpy(sampleRateBytes, &sampleRate, sizeof(sampleRateBytes));
	
	swap(sampleRateBytes, 0, 9);
	swap(sampleRateBytes, 1, 8);
	swap(sampleRateBytes, 2, 7);
	swap(sampleRateBytes, 3, 6);
	swap(sampleRateBytes, 4, 5);
	
	fwrite(sampleRateBytes, sizeof(char), 10, stdout);
	
	/* start the SSND chunk and size! */
	fputs("SSND", stdout);
	
	unsigned int ssndRemainingBytes = 8 + (samples * bitDepth/8 * channels);
	
	char sRB[5];
	sRB[0] = (ssndRemainingBytes >> 24) & 0xFF;
	sRB[1] = (ssndRemainingBytes >> 16) & 0xFF;
	sRB[2] = (ssndRemainingBytes >> 8) & 0xFF;
	sRB[3] = ssndRemainingBytes & 0xFF;
	
	fwrite(sRB, sizeof(char), 4, stdout);
	
	/* put in offset */
	unsigned int offset = 0;
	char offsetBytes[5];

	offsetBytes[0] = (offset >> 24) & 0xFF;
	offsetBytes[1] = (offset >> 16) & 0xFF;
	offsetBytes[2] = (offset >> 8) & 0xFF;
	offsetBytes[3] = offset & 0xFF; 
	
	fwrite(offsetBytes, sizeof(char), 4, stdout);
	
	/* put in blocksize */
	unsigned int blockSize = 0;
	char blockSizeBytes[5];

	blockSizeBytes[0] = (blockSize >> 24) & 0xFF;
	blockSizeBytes[1] = (blockSize >> 16) & 0xFF;
	blockSizeBytes[2] = (blockSize >> 8) & 0xFF;
	blockSizeBytes[3] = blockSize & 0xFF; 
	
	fwrite(blockSizeBytes, sizeof(char), 4, stdout);
	
	/* get char c pointer in the right spot */
	char c = findAIFFDataChunk(file);
	int s = 0;
	for(s = 0; s < 12; s++)
	{
		c = fgetc(file);
	}
	
	ungetc(c, file); 
	}
	
	int sampleSize = bitDepth / 8;
	
	/* ACTUALLY PRINT SOME DATA  */
	{
		fprintf(stderr, "Num sample vals: %i\n", numSampleVals);
		/* Sample points are 1 byte each */
		int i = 0;
		int thisSample = 0;
		for(thisSample = 0; thisSample < fullNumSamples; thisSample++)
		{
			char sample[sampleSize * channels];
			fread(sample, sampleSize * sizeof(char), channels, file);
			
			if((i < numSampleVals) && (samplesToRemove[i] == thisSample))
			{
				fprintf(stderr, "i: %i\n", i);
				fprintf(stderr, "Skip samples starting at: %i\n", samplesToRemove[i]);
				int samplesToSkip = (samplesToRemove[i+1] - samplesToRemove[i]) + 1;
				fprintf(stderr, "Samples to skip: %i\n", samplesToSkip);
				int j = 0;
				for(j = 0; j < samplesToSkip; j++)
				{
					fread(sample, sampleSize * sizeof(char), channels, file);
					thisSample++;
				}
				i = i + 2;
			}
			fwrite(sample, sampleSize * sizeof(char), channels, stdout);
			
		}
	}
	
	return 0;
}

void displayCutHelp()
{
	fprintf(stderr, "Do you need help?\n");
	fprintf(stderr, "You can type './sndcut -h' for help!\n");
	fprintf(stderr, "This program reads from stdin and eliminates some samples.\n");
	fprintf(stderr, "Specify samples to be removed in this format:\n");
	fprintf(stderr, "$ sndcut [switches] [low..high] [low..high] ...\n");
	fprintf(stderr, "The resulting file will be printed to stdout.\n");
}