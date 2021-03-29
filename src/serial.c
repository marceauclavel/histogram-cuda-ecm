#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define ASCIIMIN 32
#define ASCIIMAX 126

int isValid(char* c){
	int asciicode = (int)*c;
	int valid = (asciicode <= 126);
	valid = valid && (asciicode >= 32);
	return valid;
}

int main(int argc, char** argv){

	clock_t t1, t2, t3;
	printf("Initialisation...\n");
	t1 = clock();

	//Declarations
	FILE* inputFile =  NULL;
	FILE* outputFile =  NULL;
	char* inputFileName = NULL;
	char* outputFileName = NULL;

	int nChars = 0;
	char* chars;

	int nCounts = ASCIIMAX - ASCIIMIN + 1;
	int counts[nCounts];

	int opt;

	//Get comand line options
	while ((opt = getopt (argc, argv, "i:o:")) != -1) {
		switch(opt) {
			case 'i':
				inputFileName = optarg;
				break;
			case 'o':
				outputFileName = optarg;
				break;
		}
	}

	//Count number of chars in inputFile
	inputFile = fopen(inputFileName,"r");
	if (!inputFile) return 1;
	nChars = 0;
	for (char c = getc(inputFile); c != EOF; c = getc(inputFile)){
		if(isValid(&c)) ++nChars;
	}
	fclose(inputFile);

	//Allocate memory to store chars
	chars = (char*) malloc(nChars * sizeof(char));
	if(chars == NULL) {
		printf("Input file too large!\n");
		return 1;
	}

	//Filling chars array
	inputFile = fopen(inputFileName,"r");
	if (!inputFile) return 1;
	int i = 0;
	for (char c = getc(inputFile); c != EOF; c = getc(inputFile)){
		if(isValid(&c)){
			if (c>=65 && c<=90) chars[i] = (char)(c + 32);
			else chars[i] = c;
			++i;
		}
	}
	fclose(inputFile);

	t1 = clock() - t1;
	printf("Process...\n");
	t2 = clock();

	//Initialize counter array
	for (int i = 0; i < nCounts; ++i){
		counts[i] = 0;
	}

	//Count chars
	for (int i = 0; i < nChars; ++i){
		int ascii = (int)chars[i];
		++counts[ascii - ASCIIMIN];
	}

	t2 = clock() - t2;

	//Write in outputFile
	outputFile = fopen(outputFileName, "w+");
	if (!outputFile) return 1;
	for (int i = 0; i < 127 - ASCIIMIN; ++i){
		if (i + ASCIIMIN < 65 || i + ASCIIMIN > 90) {
			fprintf(outputFile, "%c:%d\n", (char)(i + ASCIIMIN), (int)counts[i]);
		}
	}
	fclose(outputFile);

	//Return memory
	free(chars);

	printf("Timings:\nInitialisation: %f\nProcess:        %f\n", (float)1000 * t1/CLOCKS_PER_SEC, (float)1000 * t2/CLOCKS_PER_SEC);


	return 0;

}

