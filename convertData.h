int convertCS229Data(FILE* infile, FILE* outfile, int bitDepth, int samples, int channels);

int convertAIFFData(FILE* infile, FILE* outfile, int bitDepth, int samples, int channels, char c); 

int convertAIFFtoCS229(FILE* infile, FILE* outfile);

int convertCS229toAIFF(FILE* infile, FILE* outfile);

void printCS229Header(FILE* files[], int numFiles, FILE* writeTo);

void printAIFFHeader(FILE* files[], int numFiles, FILE* writeTo);