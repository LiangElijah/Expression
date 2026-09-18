#include "dwarf_method.h"

static int om_get_section_info(void *obj, Dwarf_Unsigned section_index, Dwarf_Obj_Access_Section_a *return_section, int *error);
static Dwarf_Small om_get_byte_order(void *obj);
static Dwarf_Small om_get_length_size(void *obj);
static Dwarf_Small om_get_pointer_size(void *obj);
static Dwarf_Unsigned om_get_filesize(void *obj);
static Dwarf_Unsigned om_get_section_count(void *obj);
static int om_load_section(void *obj, Dwarf_Unsigned section_index, Dwarf_Small **return_data, int *error);

const Dwarf_Obj_Access_Methods_a dw_methods = {
    om_get_section_info,
    om_get_byte_order,
    om_get_length_size,
    om_get_pointer_size,
    om_get_filesize,
    om_get_section_count,
    om_load_section,
    NULL
};

static int om_get_section_info(void *obj, Dwarf_Unsigned section_index, Dwarf_Obj_Access_Section_a *return_section, int *error)
{
    Dwarf_Obj_Access_Data *obj_p = (Dwarf_Obj_Access_Data *)(obj);

    *error = 0; /* No error. Avoids unused arg */
    if (section_index >= obj_p->section_num) {
        return DW_DLV_NO_ENTRY;
    }

    return_section->as_name   = obj_p->section[section_index].name;
    return_section->as_type   = 0;
    return_section->as_flags  = 0;
    return_section->as_addr   = obj_p->section[section_index].addr;
    return_section->as_offset = 0;
    return_section->as_size   = obj_p->section[section_index].size;
    return_section->as_link   = 0;
    return_section->as_info   = 0;
    return_section->as_addralign = 0;
    return_section->as_entrysize = 1;

    return DW_DLV_OK;
}

static Dwarf_Small om_get_byte_order(void *obj)
{
    Dwarf_Obj_Access_Data *obj_p = (Dwarf_Obj_Access_Data *)(obj);

    return obj_p->byte_order;
}

static Dwarf_Small om_get_length_size(void *obj)
{
    Dwarf_Obj_Access_Data *obj_p = (Dwarf_Obj_Access_Data *)(obj);

    return obj_p->length_size;
}

static Dwarf_Small om_get_pointer_size(void *obj)
{
    Dwarf_Obj_Access_Data *obj_p = (Dwarf_Obj_Access_Data *)(obj);

    return obj_p->pointer_size;
}

static Dwarf_Unsigned om_get_filesize(void *obj)
{
    Dwarf_Obj_Access_Data *obj_p = (Dwarf_Obj_Access_Data *)(obj);

    return obj_p->file_size;
}

static Dwarf_Unsigned om_get_section_count(void *obj)
{
    Dwarf_Obj_Access_Data *obj_p = (Dwarf_Obj_Access_Data *)(obj);

    return obj_p->section_num;
}

static int om_load_section(void *obj, Dwarf_Unsigned section_index, Dwarf_Small **return_data, int *error)
{
    Dwarf_Obj_Access_Data *obj_p = (Dwarf_Obj_Access_Data *)(obj);

    *error = 0; /* No error. Avoids unused arg */
    if (section_index >= obj_p->section_num) {
        return DW_DLV_NO_ENTRY;
    }

    *return_data = obj_p->section[section_index].data;

    return DW_DLV_OK;
}
