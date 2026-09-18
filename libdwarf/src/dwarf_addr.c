#include "dwarf_addr.h"

int dwarf_addr_cal(st_dieNode_t *entry, 
    st_str_t *strBuf, 
    st_addr_t *addrBuf)
{
    if((entry == NULL) || (strBuf == NULL)) return -1;

    uint32_t found = 0, address = 0;
    uint32_t num = 0, size = 0, *deep = NULL;
    struct list_head *mem_head = NULL;
    st_dieNode_t *var_node = NULL;
    st_dieNode_t *mem_node = NULL;

    for(int n = 0; n < strBuf->segmentNum; n++) {
        if(n == 0) {
            found = 0;
            if(entry->row.next != NULL) {
                var_node = list_entry(entry->row.next, st_dieNode_t, row);
                if(strcmp((char *)var_node->un.var.name, strBuf->segment[n]) == 0) {
                    found = 1;
                } else {
                    st_dieNode_t *var_entry = var_node;
                    list_for_each_entry(var_node, &var_entry->column, column) {
                        if(strcmp((char *)var_node->un.var.name, strBuf->segment[n]) == 0) {
                            found = 1; break;
                        }
                    }
                }
            }
            if(found == 0) {
                st_dieNode_t *cu_node = NULL;
                list_for_each_entry(cu_node, &entry->column, column) {
                    if(cu_node->row.next != NULL)
                    {
                        var_node = list_entry(cu_node->row.next, st_dieNode_t, row);
                        if(strcmp((char *)var_node->un.var.name, strBuf->segment[n]) == 0) {
                            found = 1; break;
                        } else {
                            st_dieNode_t *var_entry = var_node;
                            list_for_each_entry(var_node, &var_entry->column, column) {
                                if(strcmp((char *)var_node->un.var.name, strBuf->segment[n]) == 0) {
                                    found = 1; break;
                                }
                            }
                            if(found == 1) break;
                            else return -1;
                        }
                    }
                }
            }
            if(var_node->un.var.operation[0] == DW_OP_addr) {
                address = var_node->un.var.operation[1];
            } else {
                printf("[%s-%s:%d] Unsupported DW_OP Type (%d 0x%02x).\n", 
                    __FILE__, __func__, __LINE__, 
                    n, var_node->un.var.operation[1]);
                return -1;
            }

            num = var_node->un.var.num;
            deep = var_node->un.var.deep;
            size = var_node->un.var.size;
        } else /*if(n > 0)*/ {
            found = 0;
            if(n == 1) {
                if((var_node->row.next != NULL) && (var_node->row.next->next != NULL))
                    mem_head = var_node->row.next->next;
                else return -1;
            } else {
                if((mem_head->next != NULL) && (mem_head->next->next != NULL))
                    mem_head = mem_head->next->next;
                else return -1;
            }
            mem_node = list_entry(mem_head, st_dieNode_t, row);
            if(strcmp((char *)mem_node->un.mem.name, strBuf->segment[n]) == 0) {
                found = 1;
            } else {
                st_dieNode_t *mem_entry = mem_node;
                list_for_each_entry(mem_node, &mem_entry->column, column) {
                    if(strcmp((char *)mem_node->un.mem.name, strBuf->segment[n]) == 0) {
                        found = 1; break;
                    }
                }
                if(found == 0) return -1;
            }
            if(mem_node->un.mem.operation[0] == DW_OP_plus_uconst) {
                address += mem_node->un.mem.operation[1];
            } else {
                printf("[%s-%s:%d] Unsupported DW_OP Type (%d 0x%02x).\n", 
                    __FILE__, __func__, __LINE__, 
                    n, mem_node->un.mem.operation[1]);
                return -1;
            }

            num = mem_node->un.mem.num;
            deep = mem_node->un.mem.deep;
            size = mem_node->un.mem.size;
        }

        if((strBuf->dimensionNum[n] > 0) && (strBuf->dimensionNum[n] <= num)) {
            int offset = 0;
            if(((n + 1) < strBuf->segmentNum) && (strBuf->dimensionNum[n] != num)) {
                printf("[%s-%s:%d] Illegal Varient Of Array (%d %d %d %d).\n", 
                    __FILE__, __func__, __LINE__, 
                    (n + 1), strBuf->segmentNum,
                    strBuf->dimensionNum[n], num);
                return -1;
            }
            for(int d = 0; d < strBuf->dimensionNum[n]; d++) {
                int count = 0;
                if(strBuf->dimension[n][d] >= deep[d]) {
                    printf("[%s-%s:%d] Array Out Of Range (%d %d).\n", 
                        __FILE__, __func__, __LINE__, 
                        strBuf->dimension[n][d], deep[d]);
                    return -1;
                }
                for(int g = d + 1; g < num; g++) {
                    if(count == 0) {
                        count = deep[d];
                    } else {
                        count *= deep[d];
                    }
                }
                if(count == 0) count = 1;
                offset += (count * strBuf->dimension[n][d]);
            }
            offset *= size;
            address += offset;
        } else if(strBuf->dimensionNum[n] > 0) {
            printf("[%s-%s:%d] Illegal Array Target (%d %d).\n", 
                __FILE__, __func__, __LINE__, 
                strBuf->dimensionNum[n], num);
            return -1;
        }
    }

    memset(addrBuf, 0, sizeof(st_addr_t));
    addrBuf->addr = address;
    addrBuf->num = strBuf->dimensionNum[strBuf->segmentNum-1];
    addrBuf->numDef = num;
    addrBuf->deep = deep;
    if(mem_node != NULL) {
        addrBuf->bit_offset = mem_node->un.mem.bit_offset;
        addrBuf->bit_size = mem_node->un.mem.bit_size;
        if(mem_node->row.next != NULL) {
            st_dieNode_t *type_node = list_entry(mem_node->row.next, st_dieNode_t, row);
            addrBuf->typName = type_node->un.type.name;
        } else return -1;
    } else {
        if(var_node->row.next != NULL) {
            st_dieNode_t *type_node = list_entry(var_node->row.next, st_dieNode_t, row);
            addrBuf->typName = type_node->un.type.name;
        } else return -1;
    }

    return 0;
}

int dwarf_print_addr(st_addr_t *addrBuf)
{
    if(addrBuf->num < addrBuf->numDef)
    {
        printf("ADDR:0x%X Type:%s", addrBuf->addr, addrBuf->typName);
        for(int d = addrBuf->num; d < addrBuf->numDef; d++)
        {
            printf("[%d]", addrBuf->deep[d]);
        }
        printf("\n");
    }
    else if(addrBuf->bit_size > 0)
    {
        if(addrBuf->bit_size == 1)
        {
            printf("Addr:0x%X | Type:%s | Bit:%d(%d)\n", addrBuf->addr, addrBuf->typName,
                15-addrBuf->bit_offset, addrBuf->bit_size);
        }
        else
        {
            printf("Addr:0x%X | Type:%s | Bit:%d-%d(%d)\n", addrBuf->addr, addrBuf->typName, 
                15 - addrBuf->bit_offset - addrBuf->bit_size + 1, 15 - addrBuf->bit_offset, addrBuf->bit_size);
        }
    }
    else
    {
        printf("ADDR:0x%X | Type:%s\n", addrBuf->addr, addrBuf->typName);
    }

    return 0;
}