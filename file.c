#include <stdlib.h>
#include <string.h>
#include "file.h"

const int MAX_INPUT_FILE_NAME = 20;
const int MAX_INDEX_FOLDER_NAME = 20;
const int MAX_QUERY_STRING_LENGTH = 20;

char *create_file_name(char *folder, char *file, char *extension){
	int num_char_to_use = -1;
	for(int i = 0; i < strlen(file) && num_char_to_use < 0; i++){
		if(file[i] == '.')
			num_char_to_use = i;
	}

	if(num_char_to_use == - 1)
		num_char_to_use = strlen(file);

	char *file_Name = malloc((strlen(folder) + strlen("/") + num_char_to_use + strlen(extension) + 1)*sizeof(char));
	strcpy(file_Name, folder);
	strcat(strncat(strcat(file_Name, "/"), file, num_char_to_use), extension);
	return file_Name;
}