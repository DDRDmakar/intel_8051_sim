
#ifndef __FILES
#define __FILES


char *read_text_file_cwd         (const char *filename);
char *read_text_file_resources   (const char *filename);
uint8_t *read_bin_file_cwd       (uint8_t *destination, const size_t maxlength, const char *filename);
uint8_t *read_bin_file_resources (uint8_t *destination, const size_t maxlength, const char *filename);
void write_text_file_cwd         (const char *filename, const char *str);
void write_text_file_resources   (const char *filename, const char *str);
void write_bin_file_cwd          (const char *filename, const uint8_t *buffer, const size_t len);
void write_bin_file_resources    (const char *filename, const uint8_t *buffer, const size_t len);


#endif
