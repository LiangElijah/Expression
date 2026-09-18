#ifndef __DWARF_METHOD_H__
#define __DWARF_METHOD_H__

#include <stdio.h>
#include <stdint.h>

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "config.h"

typedef struct {
    char*name;
    Dwarf_Addr addr;
    Dwarf_Unsigned size;
    Dwarf_Small *data;
} Dwarf_Obj;

typedef struct {
    Dwarf_Small byte_order;
    Dwarf_Small length_size;
    Dwarf_Small pointer_size;
    Dwarf_Unsigned file_size;
    Dwarf_Unsigned section_num;
    Dwarf_Obj *section;
} Dwarf_Obj_Access_Data;

extern const Dwarf_Obj_Access_Methods_a dw_methods;

#endif /* __DWARF_METHOD_H__ */