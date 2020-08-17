int main(int argc, char *argv[]);

void cutDownFile(FILE *file, char *samplesToRemove[], int numSamplePairs);

int cutCS229File(FILE *file, int samplesToRemove[], int sampleNums, int bitDepth, int samples, int channels, long double sampleRate); 

int cutAIFFFile(FILE *file, int samplesToRemove[], int numSampleVals, int bitDepth, int samples, int channels, long double sampleRate);

void displayCutHelp();