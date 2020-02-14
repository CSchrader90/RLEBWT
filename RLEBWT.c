#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "file.h"
#include "BWT.h"

int main(int argc, char *argv[]) {

	if(argc!= 4){
		fprintf(stderr, "usage: rlebwt -X FILENAME INDEX_FOLDER\n");
	}

	char *inputFileName = malloc(MAX_INPUT_FILE_NAME*sizeof(char));
	char *inputFolderName = malloc(MAX_INDEX_FOLDER_NAME*sizeof(char));
	strcpy(inputFileName, argv[2]);
	strcpy(inputFolderName, argv[3]);

	create_BWT(inputFileName, inputFolderName);

	free(inputFileName);
	free(inputFolderName);

	return 0;
}