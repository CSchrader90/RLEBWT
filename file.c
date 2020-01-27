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

char *remove_file_name_extension(char *file_Name){
	int dot_idx = -1;
	char *output_String;
	for(int i = 0; i < strlen(file_Name) && dot_idx < 0; i++){
		if(file_Name[i] == '.')
			dot_idx = i;
	}
	if(dot_idx > -1){
		output_String = malloc(sizeof(char)*dot_idx);
		memcpy(output_String, file_Name, dot_idx);
		return output_String;
	}
	else
		return file_Name;
}