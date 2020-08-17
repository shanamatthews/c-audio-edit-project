int main(int argc, char *argv[]);

void displayCatHelp();

int tryConcat(char *fileNames[], int numFiles, int returnType);

int compareFileData(FILE *files[], int numFiles); 

int actuallyConcatenate(FILE* files[], int numFiles, int returnType);

void printAsAIFF(FILE* files[], int numFiles);