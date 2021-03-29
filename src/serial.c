#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ASCIIMIN 32
#define ASCIIMAX 126

int isValid(char* c){
	int asciicode = (int)*c;
	int valid = (asciicode <= 126);
	valid = valid && (asciicode >= 32);
	return valid;
}

int main(int argc, char **argv){

	FILE* inputFile =  NULL;
	FILE* outputFile =  NULL;
	char c;
	char* inputFileName = NULL;
	char* outputFileName = NULL;
	char* chars;
	int nChars = 0;
	int nCounts = ASCIIMAX - ASCIIMIN + 1;
	int counts[nCounts];

	int opt;
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

	inputFile = fopen(inputFileName,"r");
	if (!inputFile) return 1;
	nChars = 0;
	for (c = getc(inputFile); c != EOF; c = getc(inputFile)){
		if(isValid(&c)) ++nChars;
	}
	fclose(inputFile);

	chars = (char*) malloc(nChars * sizeof(char));
	if(chars == NULL) {
		printf("file too large!\n");
		return 1;
	}

	inputFile = fopen(inputFileName,"r");
	if (!inputFile) return 1;
	int i = 0;
	for (c = getc(inputFile); c != EOF; c = getc(inputFile)){
		if(isValid(&c)){
			if (c>=65 && c<=90) chars[i] = (char)(c + 32);
			else chars[i] = c;
			++i;
		}
	}
	fclose(inputFile);

	for (int i = 0; i < nCounts; ++i){
		counts[i] = 0;
	}

	for (int i = 0; i < nChars; ++i){
		int ascii = (int)chars[i];
		++counts[ascii - ASCIIMIN];
	}

	outputFile = fopen(outputFileName, "w+");
	if (!outputFile) return 1;
	for (int i = 0; i < 127 - ASCIIMIN; ++i){
		if (i + ASCIIMIN < 65 || i + ASCIIMIN > 90) {
			fprintf(outputFile, "%c:%d\n", (char)(i + ASCIIMIN), (int)counts[i]);
		}
	}
	fclose(outputFile);

	free(chars);

	return 0;

}

