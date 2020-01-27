extern const int MAX_INPUT_FILE_NAME;
extern const int MAX_INDEX_FOLDER_NAME;
extern const int MAX_QUERY_STRING_LENGTH;

char *create_file_name(char *folder, char *file, char *extension);
char *remove_file_name_extension(char *file_Name);