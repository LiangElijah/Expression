#ifndef __DWARF_DIE_H__
#define __DWARF_DIE_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "config.h"
#include "list.h"

typedef enum {
    NodeTyp_CU,
    NodeTyp_VAR,
    NodeTyp_TYPE,
    NodeTyp_MEM,
} en_nodeType_t;

typedef struct st_dieNode {
    list_head_t row; // 行
    list_head_t column; // 列

    en_nodeType_t nodeType;
    union {
        struct {
            char *name;
        } cu;
        struct {
            char *name;
            int32_t operation[4];
            uint32_t num;
            uint64_t size;
            uint32_t deep[4];
        } var;
        struct {
            char *name;
        } type;
        struct {
            char *name;
            int32_t operation[4];
            uint32_t num;
            uint64_t size;
            uint32_t deep[4];
            uint64_t bit_offset;
            uint64_t bit_size;
        } mem;
    } un;
} st_dieNode_t;

int dwarf_die_init(Dwarf_Debug dw_dbg, 
    st_dieNode_t **entry, 
    Dwarf_Error *error);
int dwarf_print_die(Dwarf_Debug dw_dbg, 
    st_dieNode_t *entry, 
    Dwarf_Error *error);

#endif /* __DWARF_DIE_H__ */