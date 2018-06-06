
#ifndef __FILES__
#define __FILES__

char* read_text_file(const char *filename);
void write_text_file(const char *filename, const char *str);

char* read_text_file_resources(const char *filename);
void write_text_file_resources(const char *filename, const char *str);

#endif
