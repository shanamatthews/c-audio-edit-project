int main(int argc, char *argv[]);

void printShow(FILE *file, int width, int bitDepth, int channels, int samples, int startSample, int rowsToShow);

void initSideScreen(FILE *file, int rows, int cols);

void copySamples(FILE* tempForSamples, FILE* existing, int firstSample, int lastSample);

void removeSamples(FILE* smallerFile, FILE* original, int firstToCut, int lastToCut);