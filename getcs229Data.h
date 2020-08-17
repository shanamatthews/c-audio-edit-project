extern int file229Samples; 
extern int file229Channels;
extern char file229BitDepth[3];
extern char file229Rate[50];

int parse229File(FILE* file, char filename[256]);

int countSamples(FILE* file);

int countChannels(FILE* file);

char nextLine(FILE* file);

int get229NumSamples();
int get229NumChannels();
char* get229BitDepth();
char* get229SampleRate();