#ifndef __DWARF_ELF_H__
#define __DWARF_ELF_H__

#include <stdio.h>
#include <stdint.h>

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "config.h"

int dwarf_elf_init(const char *path,
    Dwarf_Debug *ret_dbg, 
    Dwarf_Error *error);

#endif /* __DWARF_ELF_H__ */