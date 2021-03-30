#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <cuda.h>
#include <cuda_runtime.h>

#define ASCIIMIN 32
#define ASCIIMAX 126

#define CHARS_PER_THREADS 256
#define THREADS_PER_BLOCK 256

void __global__ kernel(int nLines, char* dev_chars, int nChars, int* dev_counts, int nCounts) {
	const unsigned int tidb = threadIdx.x;
	const unsigned int ti = blockIdx.x*blockDim.x + tidb;

	__shared__ int shared_counts[ASCIIMAX - ASCIIMIN + 1];

	if (tidb == 0) {
		for (int i = 0; i < ASCIIMAX - ASCIIMIN - 1; ++i) {
			shared_counts[i] = 0;
		}
	}

	__syncthreads();

	if (ti < nLines) {
		for (int i = 0; i < CHARS_PER_THREADS; ++i) {
			int ascii = (int)dev_chars[CHARS_PER_THREADS * ti + i];
			atomicAdd(&shared_counts[ascii - ASCIIMIN], 1);
		}
	}

	__syncthreads();

	if (tidb == 0) {
		for (int i = 0; i < nCounts; ++i) {
			atomicAdd(&dev_counts[i], shared_counts[i]);
		}
	}
}

int isValid(char* c){
	int asciicode = (int)*c;
	int valid = (asciicode <= 126);
	valid = valid && (asciicode >= 32);
	return valid;
}

int main(int argc, char** argv){

	clock_t t1, t2;
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
	int* counts;

	char* dev_chars;
	int* dev_counts;

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

	//Allocate memory
	counts = (int*) malloc(nCounts * sizeof(int));
	chars = (char*) malloc(nChars * sizeof(char));
	if(chars == NULL) {
		printf("Input file too large!\n");
		return 1;
	}
	printf("%d chars processed\n", nChars);
	cudaMalloc( (void**)&dev_chars, nChars * sizeof(char));
	cudaMalloc( (void**)&dev_counts, nCounts * sizeof(int));

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

	cudaMemcpy(dev_chars, chars, nChars * sizeof(char), cudaMemcpyHostToDevice);
	cudaMemcpy(dev_counts, counts, nCounts * sizeof(int), cudaMemcpyHostToDevice);

	//Count chars
	for (int i = 0; i < nChars; ++i){
		int ascii = (int)chars[i];
		++counts[ascii - ASCIIMIN];
	}

	int nLines = (nChars + CHARS_PER_THREADS - 1) / CHARS_PER_THREADS;

	kernel<<<(nLines + THREADS_PER_BLOCK - 1 ) / THREADS_PER_BLOCK, THREADS_PER_BLOCK>>>(nLines, dev_chars, nChars, dev_counts, nCounts);

	cudaMemcpy(counts, dev_counts, nCounts * sizeof(int), cudaMemcpyDeviceToHost);

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
	cudaFree(dev_chars);
	cudaFree(dev_counts);
	free(chars);
	free(counts);

	printf("Timings:\nInitialisation: %f\nProcess:        %f\n", (float)1000 * t1/CLOCKS_PER_SEC, (float)1000 * t2/CLOCKS_PER_SEC);


	return 0;

}

