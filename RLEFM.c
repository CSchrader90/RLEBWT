#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include <string.h>
#include "file.h"
#include "BWT.h"
#include "buckets.h"

int main(int argc, char *argv[]) {

	if(argc!= 5){
		fprintf(stderr, "usage: rlebwt -X FILENAME INDEX_FOLDER QUERY_STRING\n");
	}

	char *inputFileName = malloc(MAX_INPUT_FILE_NAME*sizeof(char));
	char *inputFolderName = malloc(MAX_INDEX_FOLDER_NAME*sizeof(char));
	char *QUERY_STRING = malloc(MAX_QUERY_STRING_LENGTH*sizeof(char));
	strcpy(inputFileName, argv[2]);
	strcpy(inputFolderName, argv[3]);
	strcpy(QUERY_STRING, argv[4]);

	char *BBFile = create_file_name(inputFolderName, inputFileName, ".bb");
	char *CountFile = create_file_name(inputFolderName, inputFileName, ".count");
	char *Count_S_File = create_file_name(inputFolderName, inputFileName, ".Count_S");
	char *Rank_S_File = create_file_name(inputFolderName, inputFileName, ".Rank_S");

	create_BWT(inputFileName, inputFolderName);

	free(inputFileName);
	free(inputFolderName);
	free(QUERY_STRING);
	free(BBFile);
	free(CountFile);
	free(Count_S_File);
	free(Rank_S_File);

	return 0;
}