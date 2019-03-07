
#ifndef __TOOLS
#define __TOOLS

#include <stdint.h>
#include <string.h>

int is_udec_num(const char *line);
int is_ubin_num(const char *line);
int is_uhex_num(const char *line);

char *memory_to_str(uint8_t *storage, size_t size);
char *program_memory_to_str(Memory *mem, uint8_t *storage, size_t size);
char *uint32_to_hex_str(uint32_t value);
uint32_t hex_str_to_uint32(char *str);

#endif
