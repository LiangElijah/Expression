#include "dwarf_elf.h"

int dwarf_elf_init(const char *path, 
    Dwarf_Debug *ret_dbg, 
    Dwarf_Error *error)
{
    return dwarf_init_path(path, 
        NULL, 0, 
        DW_GROUPNUMBER_ANY, 
        NULL, NULL, 
        ret_dbg, error);
}