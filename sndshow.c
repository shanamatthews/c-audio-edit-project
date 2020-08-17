#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "sndshow.h"
#include "ReadFile.h"
#include "getcs229Data.h"
#include "getAIFFData.h"

int main(int argc, char *argv[])
{
	int whichChannel = 0;
	int width = 80;
	int zoom = 1;
	int i = 0;

	/* start i at 1 so we skip the executable name */
	for(i = 1; i < argc; i++)
	{
		if(strcmp("-h", argv[i]) == 0)
		{
			displayShowHelp();
			return 0;
		}
		if(strcmp("-c", argv[i]) == 0)
		{
			whichChannel = atoi(argv[++i]);
			if(whichChannel < 1)
			{
				fprintf(stderr, "Please select a channel between 1 and #channels.\n");
			}
		}
		if(strcmp("-w", argv[i]) == 0)	
		{
			width = atoi(argv[++i]);
			if(20 > width)
			{
				fprintf(stderr, "Widths smaller than 20 are not supported.\n");
				return -1;
			}
			if(1 == width % 2)
			{
				width = width - 1;
			}
		}	
		if(strcmp("-z", argv[i]) == 0)	
		{
			zoom = atoi(argv[++i]);
			if(zoom < 1)
			{
				fprintf(stderr, "You cannot zoom smaller than n = 1.\n");
				return -1;
			}
		}	
	}
	
	/* now we read from stdin to get a file */
	FILE *tempFile = tmpfile();
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
	
	int bitDepth = 0;
	int channels = 0;
	int samples = 0;
	
	int fileType = identifyFile(tempFile);
	if(1 == fileType)
	{
		parse229File(tempFile, "Dummy");
		bitDepth = atoi(get229BitDepth());
		channels = get229NumChannels();
		samples = get229NumSamples();
		
		printCS229Show(tempFile, whichChannel, width, zoom, bitDepth, channels, samples);
	}
	else if(2 == fileType)
	{
		FILE *CS229Temp = tmpfile();
		if(!CS229Temp)
		{
			fprintf(stderr, "We had a problem making a temp file. Sorry. Try again?\n");
			return -1;
		}
		
		rewind(tempFile);
		
		convertAIFFtoCS229(tempFile, CS229Temp);
		
		rewind(CS229Temp);

		parse229File(CS229Temp, "Dummy");
		
		bitDepth = atoi(get229BitDepth());
		channels = get229NumChannels();
		fprintf(stderr, "Channels: %d\n", channels);
		samples = get229NumSamples();
		
		
		printCS229Show(CS229Temp, whichChannel, width, zoom, bitDepth, channels, samples);
	}
	
	
	return 0;
}

int printCS229Show(FILE *file, int channelToShow, int width, int zoom, int bitDepth, int channels, int samples)
{
	/* we already checked that if they entered a channel it was greater than 0 */
	/* if channelToShow = 0 now, it means that they didn't enter anything */
	if(channelToShow > channels)
	{
		fprintf(stderr, "Please select a valid channel.\n");
		return -1;
	}
	
	rewind(file);
	
	/* go to sample data */
	char c = findStartData(file);
	ungetc(c, file);
	fprintf(stderr, "Jumped to start data.\n");
	
	/* we're ignoring the channels stuff & there is no zoom */
	if((!channelToShow) && !(zoom - 1))
	{
		int i = 0;
		for(i = 0; i < samples * channels; i++)
		{
			int dataPoint = 0;
			fscanf(file, "%i", &dataPoint);
			
			char *dataStr = makeString(width, bitDepth, dataPoint);
			if(i % channels == 0)
			{
				fprintf(stdout, "%9i%s|\n", (i/channels), dataStr);
			}
			else
			{
				fprintf(stdout, "         %s|\n", dataStr);
			}
		
			free(dataStr);
		}
	}
	
	/* we have to print off a specific channel but there is no zoom */
	else if(channelToShow && !(zoom - 1))
	{
		/* to make it 0-based */
		channelToShow--;
		
		int i = 0;
		for(i = 0; i < samples * channels; i++)
		{
			int dataPoint = 0;
			fscanf(file, "%i", &dataPoint);
			
			char *dataStr = makeString(width, bitDepth, dataPoint);
			if(i % channels == channelToShow)
			{
				fprintf(stdout, "%9i%s|\n", (i/channels), dataStr);
			}
			else
			{
				continue;
			}
		
			free(dataStr);
		}
	}
	/* ignoring channels but there is zoom! */
	else if((!channelToShow) && (zoom - 1))
	{
		/* we're gonna go through all the samples */
		int i = 0;
		for(i = 0; i < samples; i = i + zoom)
		{
			/* put samples in an array to check through */
			int samples[channels][zoom];
			/* we need to check them in groups based on the zoom level */
			int z= 0;
			for(z = 0; z < zoom; z++)
			{
				int chan = 0;
				for(chan = 0; chan < channels; chan++)
				{
					int sample = 0;
					fscanf(file, "%i", &sample);
					samples[chan][z] = sample;
				}
			}
			/* we just ate <zoom> rows of samples from the file */
			
			/* now we have an array of samples - select the correct data points to print */
			int chan = 0;
			for(chan = 0; chan < channels; chan++)
			{
				int dataPoint = 0;
				for(z = 0; z < zoom; z++)
				{
					if(abs(samples[chan][z]) > dataPoint)
					{
						dataPoint = samples[chan][z];
					}
				}
				
				/* this is the largest dataPoint in the array for that sample */
				char *dataStr = makeString(width, bitDepth, dataPoint);

				if(chan == 0)
				{
					fprintf(stdout, "%9i%s|\n", i, dataStr);
				}
				else
				{
					fprintf(stdout, "         %s|\n", dataStr);
				}

				free(dataStr);
			}
		}
	}
	/* channels is specified and there is zoom */
	else if(channelToShow && (zoom - 1))
	{	
		/* make it 0-based */
		channelToShow--;
		
		/* we're gonna go through all the samples */
		int i = 0;
		for(i = 0; i < samples; i = i + zoom)
		{
			/* put samples in an array to check through */
			int samples[channels][zoom];
			/* we need to check them in groups based on the zoom level */
			int z= 0;
			for(z = 0; z < zoom; z++)
			{
				int chan = 0;
				for(chan = 0; chan < channels; chan++)
				{
					int sample = 0;
					fscanf(file, "%i", &sample);
					samples[chan][z] = sample;
				}
			}
			/* we just ate <zoom> rows of samples from the file */
			
			/* now we have an array of samples - select the correct data points to print */
			int chan = 0;
			for(chan = 0; chan < channels; chan++)
			{
				int dataPoint = 0;
				for(z = 0; z < zoom; z++)
				{
					if(abs(samples[chan][z]) > dataPoint)
					{
						dataPoint = samples[chan][z];
					}
				}
				
				/* this is the largest dataPoint in the array for that sample */
				char *dataStr = makeString(width, bitDepth, dataPoint);

				if(chan == channelToShow)
				{
					fprintf(stdout, "%9i%s|\n", i, dataStr);
				}
				else
				{
					continue;
				}

				free(dataStr);
			}
		}
	}
	
	
	return 0;
}

void displayShowHelp()
{
	fprintf(stderr, "Do you need help?\n");
	fprintf(stderr, "You can type './sndshow -h' for help!\n");
	fprintf(stderr, "This program shows ASCII art for a sound file read from stdin.\n");
	fprintf(stderr, "This program accepts the following switches:\n");
	fprintf(stderr, "   '-h' for help\n");
	fprintf(stderr, "   '-c <numChannels>' to show output only for a certain channel.\n");
	fprintf(stderr, "   '-w <width>' to specify the output with. Default = 80, min = 20.\n");
	fprintf(stderr, "   '-z <n>' to zoom out by a factor of n. Default = 1. \n");
}
