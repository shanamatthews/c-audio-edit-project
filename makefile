CFLAGS = "-Wall -g -ANSI" 

all: sndinfo sndconv sndcat sndcut sndshow sndedit

tarball: project1.tar.gz

project1.tar.gz: makefile getAIFFData.c getAIFFData.h getcs229Data.c getcs229Data.h ReadFile.c ReadFile.h sndconv.c sndconv.h sndinfo.c sndinfo.h README.txt sndcat.c sndcat.h sndcut.c sndcut.h sndshow.c sndshow.h convertData.c convertData.h sndedit.c sndedit.h concatHelpers.c concatHelpers.h 
	tar -czf proj1.tar.gz makefile makefile getAIFFData.c getAIFFData.h getcs229Data.c getcs229Data.h ReadFile.c ReadFile.h sndconv.c sndconv.h sndinfo.c sndinfo.h README.txt sndcat.c sndcat.h sndcut.c sndcut.h sndshow.c sndshow.h convertData.c convertData.h sndedit.c sndedit.h concatHelpers.c concatHelpers.h 

sndinfo: ReadFile.o sndinfo.o getAIFFData.o getcs229Data.o
	gcc ReadFile.o sndinfo.o getAIFFData.o getcs229Data.o -o sndinfo 
	
sndconv: ReadFile.o sndconv.o getAIFFData.o getcs229Data.o convertData.o 
	gcc ReadFile.o sndconv.o getAIFFData.o getcs229Data.o convertData.o -o sndconv
	
sndcat: sndcat.o ReadFile.o getAIFFData.o getcs229Data.o convertData.o concatHelpers.o
	gcc sndcat.o ReadFile.o getcs229Data.o getAIFFData.o convertData.o concatHelpers.o -o sndcat
	
sndcut: sndcut.o ReadFile.o getcs229Data.o getAIFFData.o
	gcc sndcut.o ReadFile.o getcs229Data.o getAIFFData.o -o sndcut
	
sndshow: sndshow.o getcs229Data.o getAIFFData.o ReadFile.o convertData.o
	gcc sndshow.o getcs229Data.o getAIFFData.o ReadFile.o convertData.o -o sndshow
	
sndedit: sndedit.o ReadFile.o getcs229Data.o getAIFFData.o convertData.o concatHelpers.o
	gcc sndedit.o ReadFile.o getcs229Data.o getAIFFData.o convertData.o concatHelpers.o -lncurses -o sndedit
	
ReadFile.o: ReadFile.c ReadFile.h sndinfo.h
	gcc -c ReadFile.c

sndinfo.o: sndinfo.c sndinfo.h ReadFile.h getAIFFData.h getcs229Data.h
	gcc -c sndinfo.c
	
sndconv.o: sndconv.c sndconv.h ReadFile.h getAIFFData.h getcs229Data.h convertData.h
	gcc -c sndconv.c

sndcat.o: sndcat.c sndcat.h ReadFile.h getcs229Data.h getAIFFData.h convertData.h concatHelpers.h
	gcc -c sndcat.c
	
sndcut.o: sndcut.c sndcut.h getcs229Data.h getAIFFData.h ReadFile.h
	gcc -c sndcut.c
	
sndshow.o: sndshow.c sndshow.h getcs229Data.h getAIFFData.h ReadFile.h 
	gcc -c sndshow.c
	
sndedit.o: sndedit.c sndedit.h ReadFile.h getcs229Data.h getAIFFData.h convertData.h concatHelpers.h
	gcc -c sndedit.c
	
convertData.o: convertData.c ReadFile.h getcs229Data.h getAIFFData.h
	gcc -c convertData.c

getAIFFData.o: getAIFFData.c getAIFFData.h ReadFile.h
	gcc -c getAIFFData.c
	
getcs229Data.o: getcs229Data.c getcs229Data.h ReadFile.h
	gcc -c getcs229Data.c
	
concatHelpers.o: concatHelpers.c concatHelpers.h
	gcc -c concatHelpers.c
	
	
clean:
	rm -f *.o sndinfo sndconv sndcat sndcut sndshow sndedit