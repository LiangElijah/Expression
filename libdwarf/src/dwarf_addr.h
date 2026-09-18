#ifndef __DWARF_ADDR_H__
#define __DWARF_ADDR_H__

#include <stdio.h>
#include <stdint.h>

#include "list.h"
#include "dwarf_die.h"
#include "dwarf_str.h"

typedef struct st_addr {
    uint64_t bit_offset;
    uint64_t bit_size;
    uint32_t addr;
    uint32_t num;
    uint32_t numDef;
    uint32_t*deep;
    char *typName;
} st_addr_t;

int dwarf_addr_cal(st_dieNode_t *entry, 
    st_str_t *strBuf, 
    st_addr_t *addrBuf);
int dwarf_print_addr(st_addr_t *addrBuf);

#endif /* __DWARF_ADDR_H__ */