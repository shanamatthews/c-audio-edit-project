#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "sndconv.h"
#include "ReadFile.h"
#include "getAIFFData.h"
#include "getcs229Data.h"
#include "convertData.h"


char infilename[2000];
char outfilename[2000];

int main(int argc, char *argv[])
{
	int AIFFoutput = 0;
	int CS229output = 0;

	int i = 0;
	/* searches for flags */
	for(i = 0; i < argc; i++)
	{
		if(strcmp("-h", argv[i]) == 0)
		{
			displayConvHelp();
			return 0;
		}
		if(strcmp("-1", argv[i]) == 0)
		{
			promptForFiles();
			return 0;
		}
		
		if(strcmp("-a", argv[i]) == 0)
		{
			if(CS229output)
			{
				fprintf(stderr, "You can't specify both CS229 and AIFF. Try again.\n");
				return 1;
			}	
			AIFFoutput = 1;
		}
		if(strcmp("-c", argv[i]) == 0)
		{
			if(AIFFoutput)
			{
				fprintf(stderr, "You can't specify both CS229 and AIFF. Try again.\n");
				return 1;
			}	
			CS229output = 1;
		}

	}
	
	/* if we didn't get the "-1" flag then we're
	going to be reading a file from stdin */
	
	/* Now we have grab from sndin */
	FILE *temp = tmpfile();
	if(!temp)
	{
		fprintf(stderr, "We couldn't make a temporary file.\n");
		return -1;
	}

	
	int c = getchar();
	while(EOF != c)
	{	
		fwrite(&c, sizeof(char), 1, temp);
		c = getchar();
	}
	
	rewind(temp);
	
	int fileType = identifyFile(temp);
	
	/* if they didn't specify to output in either format 
		then we can go about the normal process */
	if(!(AIFFoutput || CS229output))
	{
		if(1 == fileType)
		{
			/* CHANGE OUTPUT BACK TO STDOUT */
			convertCS229toAIFF(temp, stdout);
		}
		else if(2 == fileType)
		{
			convertAIFFtoCS229(temp, stdout);
		}
	}
	/* Its a CS229 file */
	else if(1 == fileType)
	{
		/* and we want output in CS229 */
		if(CS229output)
		{	
			/* so legit just print everything back out to stdout */
			rewind(temp);
			c = getc(temp);
			while(EOF != c)
			{
				fprintf(stdout, "%c", c);
				c = getc(temp);
			}
		}
		else if(AIFFoutput)
		{
			convertAIFFtoCS229(temp, stdout);
		}
	}
	/* Its an AIFF file */
	else if(2 == fileType)
	{
		/* and we want output in AIFF! */
		if(AIFFoutput)
		{
			rewind(temp);
			c = getc(temp);
			while(EOF != c)
			{
				fprintf(stdout, "%c", c);
				c = getc(temp);
			}
		}
		else if(CS229output)
		{
			convertAIFFtoCS229(temp, stdout); 
		}
	}
	
	return 0;

}

void displayConvHelp()
{
	fprintf(stderr, "Do you need help?\n");
	fprintf(stderr, "You can type './sndconv -h' for help!\n");
	fprintf(stderr, "Or type -1 to make the program run in 'classic mode'.\n");
	fprintf(stderr, "If you just type './sndconv' the program reads from stdin\n");
	fprintf(stderr, "and writes the converted file to stdout.\n");
	fprintf(stderr, "To make the program always output in AIFF, type './sndconv -a'\n");
	fprintf(stderr, "And to make the program always output in CS229, type './sndconv -c'\n");
}

/* prompts to std out for a path to a file to convert. 
	Converts the file and prints to the second file specified by  
	the user. */
int promptForFiles()
{
	FILE* infile;
	FILE* outfile; 
	
	fprintf(stderr, "What is the file path to the file you'd like to convert?\n");
	scanf("%s", &infilename);
	
	infile = fopen(infilename, "r");
	if(!infile)
	{
		fprintf(stderr, "We couldn't read from that file.\n");
		return 1;
	}
	
	
	fprintf(stderr, "What is the file path to the file you'd like to store the results in? \n");
	scanf("%s", &outfilename);
	
	outfile = fopen(outfilename, "wb");
	if(!outfile)
	{
		fprintf(stderr, "We couldn't write to that file.\n");
	}
	
	int fileType = identifyFile(infile);
	 
	
	if(1 == fileType)
	{
		convertCS229toAIFF(infile, outfile);
	}
	else if(2 == fileType)
	{
		convertAIFFtoCS229(infile, outfile);
	}
	
}









