#include <stdlib.h>
#include <string.h>
#include "file.h"

const int MAX_INPUT_FILE_NAME = 10;
const int MAX_INDEX_FOLDER_NAME = 10;
const int MAX_QUERY_STRING_LENGTH = 20;

char *create_file_name(char *folder, char *file, char *extension){
	char *file_Name = malloc((strlen(folder) + strlen("/") + strlen(file) + strlen(extension))*sizeof(char));
	strcpy(file_Name, folder);
	strcat(strcat(strcat(file_Name, "/"), file),extension);
	return file_Name;
}