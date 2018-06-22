
#ifndef __FILES
#define __FILES

// TEXT FILES

char *read_text_file(const char *filename);
void write_text_file(const char *filename, const char *str);

char *read_text_file_resources(const char *filename);
void write_text_file_resources(const char *filename, const char *str);


// BINARY FILES

uint8_t *read_bin_file(const char *filename);
void write_bin_file(const char *filename, const uint8_t *buffer, const size_t len);

uint8_t *read_bin_file_resources(const char *filename);
void write_bin_file_resources(const char *filename, const uint8_t *buffer, const size_t len);

#endif
