#ifndef __DWARF_STR_H__
#define __DWARF_STR_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct st_str {
    int segmentNum;
    char segment[16][32];
    int dimension[16][4];
    int dimensionNum[16];
    char str[128];
} st_str_t;

int dwarf_str_init(const char *str, 
    st_str_t *strBuf);
int dwarf_print_str(st_str_t *strBuf);

#endif /* __DWARF_STR_H__ */