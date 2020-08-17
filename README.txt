Welcome to my Project 1 Part 3!!!!
I hope you enjoy grading it EVEN MORE than parts 1 & 2! :) 

Executables produced:
	sndinfo
		Usage: $ sndinfo [switch] [fileToParse] [fileToParse] ...
		Switches accepted:
			-h: Displays a help message to stderr
			-1: Prompts user for a file to generate information for
		If no switches are used and no files are given, the program reads file from stdin.	
		File types accepted are: AIFF, cs229
		If the file is not one of the two types, the program terminates with an error message.
		If the file is one of those types, the program parse the files,
		then prints a formatted printout to stndout with information for the file.
		Example:
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
	
	sndconv
		Usage: $ sndconv [switch] 
		Switches accepted: 
			-h: Displays a help message to stderr
			-1: Prompts the user for a file to convert, then another file to store in
			-a: Always outputs a file in AIFF format
			-c: Always outputs a file in CS229 format
		Unless the "-1" flag is specified, reads a file from stdin and writes to stdout.
		Files types accepted are: AIFF, cs229.
		If the file is not one of those two types, the program terminates with an error message.
		If the file is one of those types, the program will parse the file, 
		saving the result in the "opposite" (i.e. AIFF parsed to cs229, cs229 parsed to AIFF)
		file type, unless the "-a" or "-c" flag s specified.
		The file it is saved at is given whatever name and extension the user specifies.
		
	sndcat
		Usage: $ sndcat [switch] [fileToParse] [fileToParse] ...
		Switches accepted:
			-h: Displays a help message to stderr
			-a: Outputs in AIFF
			-c: Outputs in CS229
		If a switch to determine output type isn't specified, the program terminates.
		If the files do not have matching sample rates, bit depths, or number of channels, 
		the program terminates.
		Otherwise, the program concatenates the sound data of all the files regardless of 
		type, and writes the resulting sound file to stdout.
		If no files are passed in as arguments, the program reads from stdin, but does 
		not concatenate anything. It just spits out that file again.
	
	sndcut
		Usage: $ sndcut [switches] [low..high] [low..high] ...
		Switches accepted:
			-h: Displays a help message to stderr
		Reads a file from stdin and prints out a modified file to stdout.
		The file is modified by removing the samples (inclusive, 0-based) specified by low..high
		where low is the min sample to remove and high is the max. 
		If the low...high arguments are passed out of order (e.g. low1 > low2) my program
		will probably totally break.
		PLEASE DON'T TEST IT :( I TRIED SO HARD.	
	
	sndshow
		Usage: $ sndshow [switches]
		Switches accepted:
			-h: Displays a help message to stderr
			-c <n>: Show the output only for channel n, for 1 <= n <= #channels.
			-w <n>: Specify the total output width, in number of characters. 
				If not specified, the default is n = 80. 
				Note that n must be even.
			-z <n>: Zoom out by a factor of n. If not specified, the default is n = 1. 
		Displays a beautiful ASCII art representation of the sound file.
		If the width given is less than 20, the program terminates. Otherwise the 
		program prints the ASCII art with the width given.
		If the channel specified is not in range, the program terminates. Otherwise the
		program prints the ASCII art only for the data in that channel.
		If the zoom given is < 1, the program terminates. Otherwise the ASCII art
		"zooms" out, skipping over samples.
	
	sndedit
		Usage: $ sndedit [fileToShow]
		Switches accepted:
			None. Don't try to do any. It won't work.
		Displays a beautiful ASCII art representation of the sound file THAT YOU CAN EDIT!
		What? It's awesome. 
		Start it up for more on-screen instructions. You can do such amazing editing as:
			Cut samples
			Copy & paste samples
			Save the file
			Stare in awe at the file's ASCII beauty.
		
				
Files:
	concatHelpers.c
		Contains the shared code needed to concatenate multiple files into one
		CS229 file.
	concatHelpers.h
	
	convertData.c
		Contains the code that actually parses the samples and converts from either 
		cs229 to AIFF or vice versa.
	convertData.h
		Header file.
		
	getAIFFData.c
		This parses the "header" of and AIFF file, moving the pointer to where the
		sample data starts.
	getAIFFData.h
		Header file.
	
	getcs229Data.c
		This is equivalent to getAIFFData.c, but for cs229 files types.
	getcs229Data.h
	
	ReadFile.c
		This has general functions to identify which file we are reading, and print
		formatted file information to stdout.
	ReadFile.h
	
	sndconv.c
		Contains the main() method for sndconv. Promps the user for file to convert
		and file to store in.
	sndconv.h
	
	sndinfo.c
		Contains the main() method for sndinfo. Prompts user for a file to get info about.
		Most of the parsing work for sndinfo is broken into getcs229Data.c
		and getAIFFData.c
	sndinfo.h
	
	sndcat.c
		Contains main() for sndcat and related helper methods. Contains methods to print
		a file as CS229 or AIFF and print the headers for each type of file, and the 
		logic for concatenating files.
	sndcat.h
	
	sndcut.c
		Contains main() for sndcut and related helper methods. Contains the logic for 
		cutting out samples.
	sndcat.h
	
	sndedit.c
		Contains main() for sndedit and related helper methods. Uses ncurses to display
		an editable ASCII representation of the file given.
	sndedit.h
		
	sndshow.c
		Contains main() for sndshow and related helper methods. Contains the logic
		for converting sample data into bars, and printing those bars according
		to the flags specified.
	sndshow.h
	
	

Strategy:
	sndinfo
		AIFF
			Parse through "header" of AIFF file until samples, channels, bit depth,
			and file rate are all found.
			We grab read these values into char arrays, then cast them to their 
			respective types (int, long double) for later use.
		cs229
			Parse through header of cs229 file until StartData is found.
			Read all characters into strings.
			Also counts the number of samples and channels.
	
	sndconv
		cs229 -> AIFF
			Take the values for samples, channels, bit depth and file rate
			from getAIFFData.c functions, convert into char arrays (using atoi function)
			then perform byte swapping to convert to big endian. 
			Write to file using fwrite();
			Copy all samples based on their bit depth, flip bytes to convert to big
			endian, then copies to the outfile using fwrite();
		AIFF -> cs229
			Take the sample values from the AIFF byte by byte, writing them to a char
			array. We perform byte swapping to convert to big endian, then print 
			to the cs229 outfile using fprintf();
			
	sndcat
		First determines if the two files are compatible, if not terminates.
		Also checks if a valid output format is specified. 
		Then concatenates the files based on whether they should be output type AIFF or CS229.
		AIFF
			For each file given, check the type. If it is AIFF, we simply skip to 
			the data and tack it on.
			If it is CS229, we skip to the data, convert it to AIFF format 
			using helper methods from sndconv, then tack it on.
		CS229
			For each file given, check the type. If it is CS229, we simply skip to 
			the data and tack it on.
			If it is AIFF, we skip to the data, convert it to CS229 format
			using helper methods from sndconv, then tack it on.
		If no files are given as inputs, the program just prints back out whatever
		you put in to stdin. I mean, what were you expecting?
		
	sndcut
		Reads stdin into a temp file, then cuts it down based on whether it is CS229 or AIFF
		CS229
			Prints a CS229 header, then goes through the temp file, sample by sample, 
			writing every sample except for the ones specified to be skipped.
		AIFF	
			Same as CS229, but with AIFF.	
			
	sndshow
		Reads file in from stdin into a temp file, then identifies it as CS229 or AIFF.
		CS229
			Parses file to get bitdepth, num samples, channels. Calculates the 
			number of "bars" to display in helper method. Prints
		AIFF
			Converts file to CS229 format, then proceed.
	
	sndedit
		Reads in the given file. If it is 
			AIFF: we convert it to CS229
			CS229: no extra step
		Then copy to a temp file and use helper methods from sndshow as well as the ncurses
		library to print an ASCII representation of that file on screen.
		We then use helper methods from sndcut and sndcat to enable cutting/pasting of data.
		If the user chooses to save the changes to the file, we convert back to AIFF if necessary,
		then copy that temp file back into the original file.
		


Here's some cool ASCII art to make your day better:
       
       
                 ________________
                |                |_____    __
                |    I Love C!   |     |__|  |_________
                |________________|     |::|  |        /
   /\**/\       |                \.____|::|__|      <
  ( o_o  )_     |                      \::/  \._______\
   (u--u   \_)  |
    (||___   )==\
  ,dP"/b/=( /P"/b\
  |8 || 8\=== || 8
  `b,  ,P  `b,  ,P
    """`     """`


                    .............                .""".             .""".    
            ..."""""             """""...       $   . ".         ." .   $    
        ..""        .   .   .   .   .    ..    $   $$$. ". ... ." .$$$   $    
      ."    . " . " . " . " . " . " . " .  "" ."  $$$"""  "   "  """$$$  ".   
    ."      . " . " . " . " . " . " . " .     $  "                    "   $   
   ."   . " . " . "           "   " . " . "  ."      ...          ...     ".  
  ."    . " . "    .."""""""""...     " . "  $     .$"              "$.    $  
 ."     . " . " .""     .   .    ""..   . " $ ".      .""$     .""$      ." $ 
."    " . " .       . " . " . " .    $    " $ "      "  $$    "  $$       " $ 
$     " . " . " . " . " . " . " . "   $     $             $$.$$             $ 
$     " . " . " . " . " . " . " . " .  $  " $  " .        $$$$$        . "  $ 
$     " . " . " . " . " . " . " . " .  $    $      "  ..   "$"   ..  "      $ 
".    " . " . " . " . " . " . " . "   ."  "  $  . . . $  . .". .  $ . . .  $  
 $    " . " . " . " . " . " . " . "  ."   "            ".."   ".."       
  $     . " . " . " . " . " . "   .."   . " . "..    "             "    .."   
  ".      " . " . " . " . " .  .""    " . " .    """$...         ...$"""      
   ". "..     " . " . " . " .  "........  "    .....  ."""....."""            
     ". ."$".....                       $..."$"$"."   $".$"... `":....        
       "".."    $"$"$"$"""$........$"$"$"  ."."."  ...""      ."".    `"".    
           """.$.$." ."  ."."."    ."."." $.$.$"""".......  ". ". $ ". ". $   
                  """.$.$.$.$.....$.$.""""               ""..$..$."..$..$."
                  
                  
                  
          ,.                 .,
         ,: ':.    .,.    .:' :,
         ,',   '.:'   ':.'   ,',
         : '.  '         '  .' :
         ', : '           ' : ,'
         '.' .,:,.   .,:,. '.'
          ,:    V '. .' V    :,
         ,:        / '        :,
         ,:                   :,
          ,:       =:=       :,
           ,: ,     :     , :,
            :' ',.,' ',.,:' ':
           :'      ':WW::'   '.
          .:'       '::::'   ':
          ,:        '::::'    :,
          :'         ':::'    ':
         ,:           ':''     :.
        .:'             '.     ',.
       ,:'               ''     '.
       .:'               .',    ':
      .:'               .'.,     :
      .:                .,''     :
      ::                .,''    ,:
      ::              .,'','   .:'
    .,::'.           .,','     ::::.
  .:'     ',.       ,:,       ,WWWWW,
  :'        :       :W:'     :WWWWWWW,          .,.
  :         ',      WWW      WWWWWWWWW          '::,
  '.         ',     WWW     :WWWWWWWWW            '::,
   '.         :     WWW     :WWWWWWWW'             :::
    '.       ,:     WWW     :WWWWWWW'             .:::
     '.     .W:     WWW     :WWWWWW'           .,:::'
      '.   :WW:     WWW     :WWWWW'      .,,:::::''
     .,'   ''::     :W:     :WWWWW.  .,::::''
  ,'        ''','',',','','''WWWWW::::''
   ':,,,,,,,':  :  : : :  :  :WWWW'''
   
   
