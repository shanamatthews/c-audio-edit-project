#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>
#include "sndedit.h"
#include "ReadFile.h"
#include "getcs229Data.h"
#include "getAIFFData.h"
#include "convertData.h"
#include "concatHelpers.h"

static int samples = 0;
static int channels = 0;
static int bitDepth = 0;
static long double sampleRate = 2;

static int savable = 0;
static int numSamplesInBuffer = 0;
static int numSamplesMarked = 0;
static int firstMarked = -1;
static int lastMarked = 0;
static int originalMark = -1;
static int marking = 0;

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "You must pass in just one parameter, a file name.\n");
	}
	
	/* open the file that we want to display and save to */
	FILE* final = fopen(argv[1], "r+");
	if(!final)
	{
		endwin();
		fprintf(stderr, "We couldn't open that file\n");
		return -1;
	}
	
	char path[50]; 
	strcpy(path, argv[1]);

	/* make a temp file that we can modify without changing the original */
	FILE *file = tmpfile();
	if(!file)
	{
		endwin();
		fprintf(stderr, "We had a problem making a temp file. Sorry. Try again?\n");
		return -1;
	}
	
	/* copy our final file into file, a temporary copy that we will edit */
	int c = getc(final);
	while(EOF != c)
	{
		putc(c, file);
		c = getc(final);
	}
	
	rewind(file);
	
	int fileType = identifyFile(file);
	/* CS229 file */
	if(1 == fileType)
	{
		strcat(path, "(CS229)");
	}
	/* AIFF file */
	else if(2 == fileType)
	{
		strcat(path, "(AIFF)");
		
		FILE *CS229Temp = tmpfile();
		if(!CS229Temp)
		{
			endwin();
			fprintf(stderr, "We had a problem making a temp file. Sorry. Try again?\n");
			return -1;
		}
		
		/* if the conversion process failed, we terminate */
		int fail = convertAIFFtoCS229(file, CS229Temp);
		if(fail)
		{
			endwin();
			fprintf(stderr, "That file could not be read.\n");
			return -1;
		}
		rewind(CS229Temp);
		file = CS229Temp;
	}
	/* not a file type we can parse */
	else
	{
		endwin();
		fprintf(stderr, "That was not a filetype that we could read.\n");
		return -1;
	}
	
	/* make a temp file for copying data into */
	FILE* copied = tmpfile();
	if(!copied)
	{
		endwin();
		fprintf(stderr, "We had a problem making a temp file. Sorry. Try again?\n");
		return -1;
	}

	/* start up the screen! */
	initscr();
	noecho();
	keypad(stdscr, 1);
	
	int rows = 0;
	int cols = 0;
	getmaxyx(stdscr, rows, cols);
	
	int cursorX = (cols - 20 - 9)/2 + 9;
	if(1 == cols%2)
	{
		cursorX--;
	}
	int cursorY = 2;
	
	move(cursorY, cursorX);
	

	if(cols < 40 || rows < 24)
	{
		endwin();
		fprintf(stderr, "Sorry, your window is too small.\n");
		return -1;
	}
	
	/* initialize some more screen formatting */
	char separator[cols];
	strcpy(separator, "=");
	int i = 0;
	for(i=1; i<cols; i++)
	{
		strcat(separator, "=");
	}

	
	int currentStartSample = 0;
	int currentSample = currentStartSample;
	int channel = 1;
	
	
	mvprintw(0, (cols - strlen(path))/2, "%s", path);
	mvprintw(1, 0, "%s", separator);
	
	initSideScreen(file, rows, cols);
	
	printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows -2);
	
	for(;;)
	{
		/* make sure the data displayed on screen is the most current. */
		mvprintw(rows - 2, cols - 20, "  Marked: %d, %d", firstMarked, lastMarked);
		initSideScreen(file, rows, cols);
		move(cursorY, cursorX);
		refresh();
		int c = getch();
		
		/* quit */
		if(('q' == c) || ('Q' == c))
		{
			break;
		}
		/* goto */
		else if(('g' == c) || ('G' == c))
		{
			char mesg[] = "  Enter the sample you want to jump to:  ";
			char sampleNum[10]  = "0";
			mvprintw(rows/2 -1 , (cols - strlen(mesg))/2, "                                           ");
			mvprintw(rows/2, (cols - strlen(mesg))/2, "%s", mesg);
			mvprintw(rows/2 +1 , (cols - strlen(mesg))/2, "                                           ");
			int j = 0;
			/* read in the digits the user is typing one by one */
			char digit = getch();
			while(digit < 58 && digit > 47 && j < 10)
			{
				sampleNum[j] = digit;
				j++;
				digit = getch();
			}

			/* convert that string to an int so we can use it to reprint the screen starting at the sample they specified */
			int sampleToGoTo = atoi(sampleNum);
			mvprintw(rows/2, (cols + (strlen(mesg) - strlen(sampleNum)))/2, "%d\n", sampleToGoTo);
			
			/* check that the sample they tried to go to is within a valid range */
			if(sampleToGoTo >= 0 && sampleToGoTo < samples)
			{
				currentStartSample = sampleToGoTo;
				currentSample = sampleToGoTo;
				channel = 1;
				cursorY = 2;
			}
			if(marking)
			{
				/* marking upwards from original */
				if(currentSample < originalMark)
				{
					firstMarked = currentSample;
					lastMarked = originalMark;
				}
				/* marking down from the original */
				else
				{
					lastMarked = currentSample;
					firstMarked = originalMark;
				}
			}
			printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows -2);
		}
		else if(('m' == c) || ('M' == c))
		{
			/* toggle marking */
			if(marking)
			{
				marking = 0;
				printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows-2);
				mvprintw(rows - 2, cols - 20, "                     ", firstMarked);

				
				firstMarked = -1;
				lastMarked = 0;
				originalMark = -1;
			}
			else
			{
				marking++;
				attron(A_REVERSE);
				
				int x = 0;
				int y = 0;
				/* reprint everything that should now be marked */
				for(x = 0; x < (cols - 20); x++)
				{
					for(y = 0; y < ((channels - channel) + 1); y++)
					{
						char toMark = mvwinch(stdscr, cursorY + y, x);
						mvprintw(cursorY + y, x, "%c", toMark);
					}
				}

				firstMarked = currentSample;
				lastMarked = currentSample;
				originalMark = currentSample;
			}
			attroff(A_REVERSE);
			if(-1 == firstMarked)
			{
				mvprintw(rows - 2, cols - 20, "                    ");
			}
			else
			{
				mvprintw(rows - 2, cols - 20, "  Marked: %d, %d", firstMarked, lastMarked);
			}
		}
		else if(marking && (('c'  == c) || ('C' == c)))
		{
			/* clear out the temp file because we're now copying something new into it */
			copied = tmpfile();
			copySamples(copied, file, firstMarked, lastMarked);
			
			/* update the buffered value */
			numSamplesInBuffer = (lastMarked - firstMarked) + 1;
			
			/* turn off marking, because it feels more intuitive in terms of UX */
			marking = 0;
			firstMarked = -1;
			lastMarked = 0;
			originalMark = -1;
			
			printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows -2);
		}
		else if(marking && (('x' == c) || ('X' == c)))
		{
			/* clear out temp file for new samples */
			copied = tmpfile();
			copySamples(copied, file, firstMarked, lastMarked);
			
			/* remove samples from current file */
			rewind(file);
			FILE* smallerFile = tmpfile();
			removeSamples(smallerFile, file, firstMarked, lastMarked);
			
			/* copy the newly cut down file into the file we're editing */
			file = smallerFile;
			/* update the number of samples to be correct */
			samples = samples - ((lastMarked - firstMarked) + 1);
			
			numSamplesInBuffer = (lastMarked - firstMarked) + 1;
			
			/* turn off marking, because it feels more intuitive in terms of UX */
			marking = 0;
			firstMarked = -1;
			lastMarked = 0;
			originalMark = -1;
			
			/* move cursor to the top of the page */
			currentSample = currentStartSample;
			cursorY = 2;
			
			printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows -2); 
			
			savable = 1;
		}
		else if(numSamplesInBuffer && ('^' == c))
		{
			/* split the file into 3 parts: before the insert, the part to insert, and after the insert */
			FILE* beginning = tmpfile();
			removeSamples(beginning, file, currentSample, samples);
			FILE* end = tmpfile();
			removeSamples(end, file, 0, currentSample-1);
			
			FILE* storeIn = tmpfile();
			
			/* concatenate beginning, copied, end files */
			FILE* toConcat[3];
			toConcat[0] = beginning;
			toConcat[1] = copied;
			toConcat[2] = end;
			printAsCS229(toConcat, 3, storeIn);
			
			/* display newly edited file */
			file = storeIn;
			rewind(file);
			parse229File(file, "Name");
			samples = get229NumSamples();
			printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows -2);
			
			savable = 1;
		}
		else if(numSamplesInBuffer && (('v' == c) || ('V' == c)))
		{
			/* same method as previous */
			FILE* beginning = tmpfile();
			removeSamples(beginning, file, currentSample + 1, samples);
			FILE* end = tmpfile();
			removeSamples(end, file, 0, currentSample);
			
			FILE* storeIn = tmpfile();
			
			/* concatenate beginning, copied, end files */
			FILE* toConcat[3];
			toConcat[0] = beginning;
			toConcat[1] = copied;
			toConcat[2] = end;
			printAsCS229(toConcat, 3, storeIn);
			
			/* display newly edited file */
			file = storeIn;
			rewind(file);
			parse229File(file, "Name");
			samples = get229NumSamples();
			printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows -2);
			
			savable = 1;
		}
		if(savable && (('s' == c) || ('S' == c)))
		{
			/* create a NEW empty file, overwriting the one we were originally given.*/
			final = fopen(argv[1], "r+");
			/* if it was originally CS229 we can just copy our tempfile back into the original file */
			if(1 == fileType)
			{
				char saveChar = fgetc(file);
				while(EOF != saveChar)
				{
					fputc(saveChar, final);
					saveChar = fgetc(file);
				}
			}
			/* if it was originally AIFF, we have to convert our temp file back to AIFF then save to original file */
			else if(2 == fileType)
			{
				FILE* AIFFTemp = tmpfile();
				if(!AIFFTemp)
				{
					fprintf(stderr, "We had a problem making a temp file.\n");
					endwin();
					return -1;
				}
				convertCS229toAIFF(file, AIFFTemp);
				rewind(AIFFTemp);
				char saveChar = fgetc(AIFFTemp);
				while(EOF != saveChar)
				{
					fputc(saveChar, final);
					saveChar = fgetc(AIFFTemp);
				}
			}
			savable = 0;
		}
		else if(KEY_UP == c)
		{
			/* if we're within bounds */
			if(cursorY > 2)
			{
				cursorY = cursorY - channels;
				currentSample--;
				
				if(marking)
				{
					/* unhighlights */
					if((currentSample + 1) == lastMarked && (currentSample + 1) != originalMark)
					{
						int x = 0;
						int y = 0;
						for(x = 0; x < (cols - 20); x++)
						{
							for(y = 0; y < channels; y++)
							{
								char toMark = mvwinch(stdscr, (cursorY + channels) + y, x);
								mvprintw((cursorY + channels) + y, x, "%c", toMark);
							}
						}
						lastMarked--;
					}
					/* highlights */
					if(currentSample < originalMark)
					{
						attron(A_REVERSE);
						int x = 0;
						int y = 0;
						for(x = 0; x < (cols - 20); x++)
						{
							for(y = 0; y < channels; y++)
							{
								char toMark = mvwinch(stdscr, cursorY + y, x);
								mvprintw(cursorY + y, x, "%c", toMark);
							}
						}
						attroff(A_REVERSE);
						firstMarked = currentSample;
					}
				}
				move(cursorY, cursorX);
				refresh();
			}
			else if((channel - 1) > 0 || (currentSample -1) >= 0)
			{
				/* scroll */
				if(marking)
				{
					if(currentSample-1 < originalMark)
					{
						firstMarked--;
					}
					else if(currentSample-1 < lastMarked)
					{
						lastMarked--;
					}
					mvprintw(rows - 2, cols - 20, "  Marked: %d, %d", firstMarked, lastMarked);
				}
				printShow(file, cols-20, bitDepth, channels, samples, --currentStartSample, rows-2);
				refresh();
				currentSample--;
			}
			move(cursorY, cursorX);
			refresh();
		}
		else if(KEY_DOWN == c && (currentStartSample < samples))
		{
			if(cursorY <= (rows-2))
			{
				cursorY = cursorY + channels;
				currentSample++;
				if(marking)
				{
					/* highlight */
					if(currentSample < firstMarked || ((currentSample > lastMarked) && (currentSample < samples)))
					{
						attron(A_REVERSE);
						int x = 0;
						int y = 0;
						for(x = 0; x < (cols - 20); x++)
						{
							for(y = 0; y < channels; y++)
							{
								char toMark = mvwinch(stdscr, cursorY + y, x);
								mvprintw(cursorY + y, x, "%c", toMark);
							}
						}
						lastMarked = currentSample; /*(currentSample <= samples) ? currentSample : samples ; */
						attroff(A_REVERSE);
					}
					/* unhighlight */
					else if(currentSample < samples)
					{
						int x = 0;
						int y = 0;
						for(x = 0; x < (cols - 20); x++)
						{
							for(y = 0; y < channels; y++)
							{
								char toMark = mvwinch(stdscr, (cursorY - channels)+ y, x);
								mvprintw((cursorY - channels) + y, x, "%c", toMark);
							}
						}
						firstMarked++;
					}
				}
				move(cursorY, cursorX);
				refresh();
			}
			else if((currentSample + 1) <= samples || (channel < channels))
			{
				/* scroll */
				if(marking)
				{
					if((currentSample +1) > originalMark)
					{
						lastMarked++;
					}
					else if((currentSample +1) > firstMarked)
					{
						firstMarked++;
					}
					mvprintw(rows - 2, cols - 20, "  Marked: %d, %d", firstMarked, lastMarked);
				}
				printShow(file, cols-20, bitDepth, channels, samples, ++currentStartSample, rows-2);
				refresh();
				currentSample++;
			}
		}
		/* page up*/
		else if(KEY_PPAGE == c)
		{
			/* if going a page up won't put us in negative samples, do it */
			if((currentStartSample - ((rows/channels) - 2)) > 0)
			{
				if(marking)
				{
					/* we're crossing the original marked by unmarking upwards then marking upwards */
					if((currentSample >= firstMarked) && (currentStartSample - ((rows -2)/channels) < originalMark))
					{
						firstMarked = currentStartSample - ((rows -2)/channels);
						lastMarked = originalMark;
					}
					/* we're marking upwards from the start */
					else if(firstMarked >= currentSample)
					{
						firstMarked = currentStartSample - ((rows - 2)/channels);
						/* lastMarked = (originalMark < lastMarked) ? originalMark : lastMarked; */
					}
					/* we're unmarking upwards from the end */
					else
					{
						lastMarked = currentStartSample - ((rows -2)/channels);
					}
					
					
					mvprintw(rows - 2, cols - 20, "  Marked: %d, %d", firstMarked, lastMarked);
				}
				currentStartSample -= ((rows -2)/channels);
				printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows-2);
				
				move(cursorY, cursorX);
				refresh();
			}
			/* if it would put us in negative samples, we just hop to the beginning */
			else
			{
				if(marking)
				{	
					firstMarked = 0;
					lastMarked = (originalMark < lastMarked) ? originalMark : lastMarked;

					mvprintw(rows - 2, cols - 20, "  Marked: %d, %d", firstMarked, lastMarked);
				}
				currentStartSample = 0;
				printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows-2);			
			}
			
			cursorY = 2;
			currentSample = currentStartSample;
			channel = 1;
		}
		/* page down */
		else if(KEY_NPAGE == c && ((currentStartSample + ((rows - 2)/channels)) < samples))
		{
			if(marking)
			{
				/* we're crossing the originalMark by unhighlighting down past it */
				if(currentSample <= originalMark && (currentStartSample + ((rows - 2)/channels)) > originalMark)
				{
					firstMarked = originalMark;
					lastMarked = currentStartSample + ((rows - 2)/channels);
				}
				/* marking down from the originalMark */
				else if(currentSample >= originalMark)
				{
					lastMarked = currentStartSample + ((rows -2) /channels);
				}
				/* else unmarking down from the firstMarked & not going to cross originalMark*/
				else
				{
					firstMarked = currentStartSample + ((rows - 2)/channels); /*(originalMark > firstMarked) ? originalMark : firstMarked; */
				}
				
				mvprintw(rows - 2, cols - 20, "  Marked: %d, %d", firstMarked, lastMarked);
			}
			currentStartSample += ((rows - 2)/channels);
			printShow(file, cols-20, bitDepth, channels, samples, currentStartSample, rows-2);
			
			cursorY = 2; 
			currentSample = currentStartSample;
			channel = 1;
		}
		move(cursorY, cursorX);
		refresh();
	}
	endwin();
	fclose(copied);
	fclose(file);
	
	return 0;
}

/* helper method to copy selected samples,
	takes a file pointer to the temp file we should store the copied samples in,
	a file pointer to the file we're copying samples from,
	an int giving the first sample to copy, and 
	an int giving the last sample to copy */
void copySamples(FILE* tempForSamples, FILE* existing, int firstSample, int lastSample)
{
	/* print a CS229 header with the num samples changed accordingly */
	fputs("CS229\n", tempForSamples);
	fputs("\n", tempForSamples);
	
	fprintf(tempForSamples, "SampleRate  %Lg\n", sampleRate);
	fprintf(tempForSamples, "Channels  %i\n", channels);
	fprintf(tempForSamples, "BitDepth  %i\n", bitDepth);
	fprintf(tempForSamples, "Samples  %i\n", (lastMarked - firstMarked) + 1);
	fprintf(tempForSamples, "\n");
	fputs("StartData\n", tempForSamples);
	
	/* navigate file to the beginning of the data */
	rewind(existing);
	char startChar = findStartData(existing);
	ungetc(startChar, existing);
	if(EOF == startChar)
	{
		return;
	}
	
	/* copy the samples in the buffer into this temp file */
	int thisSample = 0;
	for(thisSample = 0; thisSample <= lastMarked; thisSample++)
	{
		if(thisSample < firstMarked)
		{
			/* skip this sample */
			nextLine(existing);
			continue;
		}
		char str[1000];
		fgets(str, 1000, existing);
		fprintf(tempForSamples, "%s", str);
	}
}

/* cuts out samples using a similar method as the previous */
void removeSamples(FILE* smallerFile, FILE* original, int firstToCut, int lastToCut)
{
	int samplesLeft = samples - ((lastToCut - firstToCut) + 1);
	/* print a CS229 header with the num samples changed accordingly */
	fputs("CS229\n", smallerFile);
	fputs("\n", smallerFile);
	
	fprintf(smallerFile, "SampleRate  %Lg\n", sampleRate);
	fprintf(smallerFile, "Channels  %i\n", channels);
	fprintf(smallerFile, "BitDepth  %i\n", bitDepth);
	fprintf(smallerFile, "Samples  %i\n", samplesLeft);
	fprintf(smallerFile, "\n");
	fputs("StartData\n", smallerFile);
	
	rewind(original);
	char startChar = findStartData(original);
	ungetc(startChar, original);
	if(EOF == startChar)
	{
		return;
	}
	int thisSample = 0;
	for(thisSample = 0; thisSample < samples; thisSample++)
	{
		/* if the sample exists within the samples to cut, we don't want to copy it */
		if((thisSample >= firstToCut) && (thisSample <= lastToCut))
		{
			nextLine(original);
			continue;
		}
		char str[1000];
		fgets(str, 1000,original);
		fprintf(smallerFile, "%s", str);
	}
}

/* initializes the side of the screen, giving information about the file */
void initSideScreen(FILE *file, int rows, int cols)
{
	rewind(file);
	parse229File(file, "Name");
	
	samples = get229NumSamples();
	channels = get229NumChannels();
	bitDepth = atoi(get229BitDepth());
	sampleRate = atoi(get229SampleRate());
	
	float timeInSeconds = (1/sampleRate) * samples;
	int timeInMinutes = (int)timeInSeconds/60;
	int timeInHours = timeInMinutes/60;
	int remainingMinutes = ((int)timeInSeconds % 3600)/60;
	float remainingSeconds = timeInSeconds - (timeInHours*3600 + timeInMinutes*60);
	
	mvprintw(2, cols - 20, "Sample Rate: %Lg", sampleRate);
	mvprintw(3, cols - 20, "  Bit Depth: %d", bitDepth);
	mvprintw(4, cols - 20, "   Channels: %d", channels);
	mvprintw(5, cols - 20, "Samples: %d    ", samples);
	mvprintw(6, cols - 20, " Length: %d:%.2d:%05.2f", timeInHours, remainingMinutes, remainingSeconds);
	mvprintw(7, cols - 20, "====================");
	
	if(marking)
	{
		mvprintw(8, cols - 20, "   m: unmark");
		mvprintw(9, cols - 20, "   c: copy");
		mvprintw(10, cols - 20, "   x: cut");
	}
	else
	{
		mvprintw(8, cols - 20, "   m: mark  ");
		mvprintw(9, cols - 20, "          ");
		mvprintw(10, cols - 20, "          ");
	}
	if(numSamplesInBuffer)
	{
		mvprintw(11, cols -20, "   ^: insert before");
		mvprintw(12, cols -20, "   v: insert after");
	}
	else
	{
		mvprintw(11, cols -20, "                  ");
		mvprintw(12, cols -20, "                  ");
	}
	if(savable)
	{
		mvprintw(13, cols -20, "   s: save");	
	}
	else
	{
		mvprintw(13, cols -20, "             ");
	}
	mvprintw(14, cols - 20, "   q: quit");
	mvprintw(16, cols - 20, " Movement:");
	mvprintw(17, cols - 20, "  up/dn");
	mvprintw(18, cols - 20, "  pgup/pgdn");
	mvprintw(19, cols - 20, "  g: goto sample");
	
	mvprintw(rows - 3, cols - 20, "====================");
	if(-1 == firstMarked)
	{
		mvprintw(rows - 2, cols - 20, "                    ", firstMarked);
	}
	else
	{
		mvprintw(rows - 2, cols - 20, "  Marked: %d", firstMarked);
	}
	if(numSamplesInBuffer)
	{
		mvprintw(rows - 1, cols - 20, "Buffered: %d", numSamplesInBuffer);
	}
	else
	{
		mvprintw(rows - 1, cols - 20, "                              ", numSamplesInBuffer);
	}

}

/* code borrowed from my sndshow printing method.
	Takes a file pointer to the file to represent,
	int giving the width of the window we can use to print to,
	int giving bit depth,
	int giving number of channels,
	ing giving number of samples,
	int giving the sample at which to start printing,
	int giving number of rows to display. */
void printShow(FILE *file, int width, int bitDepth, int channels, int samples, int startSample, int rowsToShow)
{
	/* if there is nothing to print, just skip */
	if(0 == samples)
	{
		fprintf(stderr, "There aren't any samples!\n");
	}
	rewind(file);
	char c = findStartData(file);
	ungetc(c, file);
	
	int i = 0;
	/* skip through samples that aren't being displayed on screen */
	for(i = 0; i < startSample * channels; i++)
	{
		int dataPoint = 0; 
		fscanf(file, "%i", &dataPoint);
	}
	
	rowsToShow = rowsToShow + i;
	for(; i < rowsToShow; i++)
	{
		/* if we went past the number of available samples, just make the screen blank */
		if(i >= (samples * channels))
		{
			char blankLine[width];
			int j = 0;
			for(j=0; j < width; j++)
			{
				blankLine[j] = ' ';
			}
			mvprintw((i+2 - (startSample * channels)), 0, "%s", blankLine);
		}
		/* print each sample! */
		else
		{
			int dataPoint = 0;
			fscanf(file, "%i", &dataPoint);
			
			/* helper method from another file */
			char *dataStr = makeString(width, bitDepth, dataPoint);
			if(marking && ((i/channels) <= lastMarked) && ((i/channels) >= firstMarked))
			{
				attron(A_REVERSE);
			}			
			if(i % channels == 0)
			{
				mvprintw((i+2 - (startSample * channels)), 0, "%9i%s|", (i/channels), dataStr);
			}
			else
			{
				mvprintw((i+2 - (startSample * channels)), 0, "         %s|", dataStr);
			}
			if(marking && ((i/channels) <= lastMarked) && ((i/channels) >= firstMarked))
			{
				attroff(A_REVERSE);
			}	
		
			free(dataStr);
		}
	}
	refresh();
}