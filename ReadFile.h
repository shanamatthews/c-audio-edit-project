int read229File(FILE* file); 

int readAIFFFile(FILE* file);
 
int identifyFile(FILE* file);

int print229FileInfo(FILE* file, char filename[], int samplesCounted, int channelsCounted, char bitDepth[], char sampleRate[], int multiple);

int printAIFFFileInfo(FILE* file, char filename[], int channels, int samples, int bitDepth, long double sampleRate, int multiple);

int print60Dashes();  

char findStartData(FILE* file);

char findAIFFDataChunk(FILE* file);

char* makeString(int width, int bitDepth, int dataPoint);
