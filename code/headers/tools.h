
#ifndef __TOOLS
#define __TOOLS

#include <stdint.h>
#include <string.h>

int is_udec_num(char *line);
int is_ubin_num(char *line);
int is_uhex_num(char *line);

char *memory_to_str(uint8_t *storage, size_t size);

#endif
