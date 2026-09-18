#include "dwarf_coff.h"

int dwarf_coff_init(const char *path, 
    Dwarf_Obj_Access_Data **dw_accessData_p,
    Dwarf_Debug *ret_dbg, 
    Dwarf_Error *error)
{
    FILE *fp = NULL;
    st_filehdr_t filehdr = {0};
    int32_t i32StrOffset = 0;

    Dwarf_Unsigned file_size = 0;
    Dwarf_Small byte_order = DW_END_little;
    Dwarf_Obj_Access_Data *dw_accessData = NULL;

    int res = 0;

    // 1、判断文件是否存在
    if(access(path, F_OK) != 0) {
        printf("[%s-%s:%d] access() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        return DW_DLV_ERROR;
    }
    
    // 2、以二进制只读打开文件
    fp = fopen(path, "rb");
    if(fp == NULL) {
        printf("[%s-%s:%d] fopen() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        return DW_DLV_ERROR;
    }

    // 3、获取文件大小
    if(fseek(fp, 0, SEEK_END) != 0) {
        printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        goto CLOSE;
    }
    res = ftell(fp);
    if(res == -1) {
        printf("[%s-%s:%d] ftell() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        goto CLOSE;
    }
    if(fseek(fp, 0, SEEK_SET) != 0) {
        printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        goto CLOSE;
    }
    file_size = res;

    // 4、读取文件头
    res = fread(&filehdr, 1, sizeof(st_filehdr_t), fp);
    if(res != sizeof(st_filehdr_t)) {
        printf("[%s-%s:%d] fread() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
        goto CLOSE;
    }

    if(filehdr.u16Version != 0xC2) {
        printf("[%s-%s:%d] Unsupported File Type (%x).\n", __FILE__, __func__, __LINE__, filehdr.u16Version);
        goto CLOSE;
    }

    if(filehdr.u16Flags & 0x0100) {
        byte_order = DW_END_little;
    } else {
        byte_order = DW_END_big;
    }

    // 5、获取字符串表偏移
    i32StrOffset = filehdr.i32SymbolOffset + filehdr.i32NumSymbol * sizeof(st_syment_t);
    if(filehdr.u16OptHdrSZ != 0) {
        // 5.1、跳过可选文件头
        int32_t i32SecOffset = sizeof(st_filehdr_t) + filehdr.u16OptHdrSZ;
        if(fseek(fp, i32SecOffset, SEEK_SET) != 0) {
            printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
            goto CLOSE;
        }
    }

    // 6、遍历字段表
    dw_accessData = malloc(sizeof(Dwarf_Obj_Access_Data));
    dw_accessData->byte_order = byte_order;
    dw_accessData->length_size = 4;
    dw_accessData->pointer_size = 4;
    dw_accessData->file_size = file_size;
    dw_accessData->section_num = 0;
    dw_accessData->section = malloc((filehdr.u16NumSec + 1)*sizeof(Dwarf_Obj));
    //memset(dw_accessData->section, 0, sizeof(Dwarf_Obj)); // 索引0预留为空
    for(int i = 1; i <= filehdr.u16NumSec; i++) {
        st_sechdr_t sechdr = {0};
        char section_name[128] = {0};
        int section_name_length = 0;
        
        res = fread(&sechdr, 1, sizeof(st_sechdr_t), fp);
        if(res != sizeof(st_sechdr_t)) {
            printf("[%s-%s:%d] fread() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
            goto FREE;
        }
        
        if(sechdr.u.s.u32Zero == 0) {
            long pos = ftell(fp);
            if(pos == -1) {
                printf("[%s-%s:%d] ftell() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            int offset = i32StrOffset + sechdr.u.s.u32Offset;
            if(fseek(fp, offset, SEEK_SET) != 0) {
                printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            for(; section_name_length < 127; section_name_length++) {
                char c = 0;
                res = fread(&c, 1, 1, fp);
                if(res != 1) {
                    printf("[%s-%s:%d] fread() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                    goto FREE;
                }
                if((c <= 32) || (c > 127)) break;
                section_name[section_name_length] = c;
            }
            section_name_length++;
            if(fseek(fp, pos, SEEK_SET) != 0) {
                printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
        } else {
            memcpy(section_name, sechdr.u.i8Name, 8);
            section_name_length = 8;
        }

        if(strncmp(section_name, ".debug_", strlen(".debug_")) == 0) {
            dw_accessData->section[dw_accessData->section_num].name = malloc(section_name_length);
            dw_accessData->section[dw_accessData->section_num].addr = 0x0000;
            dw_accessData->section[dw_accessData->section_num].size = sechdr.i32Size;
            dw_accessData->section[dw_accessData->section_num].data = malloc(sechdr.i32Size);
            memcpy(dw_accessData->section[dw_accessData->section_num].name, section_name, section_name_length);
            dw_accessData->section_num++;

            long pos = ftell(fp);
            if(pos == -1) {
                printf("[%s-%s:%d] ftell() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            int offset = sechdr.i32SecOffset;
            if(fseek(fp, offset, SEEK_SET) != 0) {
                printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            res = fread(dw_accessData->section[dw_accessData->section_num - 1].data, 1, sechdr.i32Size, fp);
            if(res != sechdr.i32Size) {
                printf("[%s-%s:%d] fread() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
            if(fseek(fp, pos, SEEK_SET) != 0) {
                printf("[%s-%s:%d] fseek() %s.\n", __FILE__, __func__, __LINE__, strerror(errno));
                goto FREE;
            }
        }
    }

    if(dw_accessData->section_num <= 0) {
        printf("[%s-%s:%d] Unsupported File Type (%x).\n", __FILE__, __func__, __LINE__, filehdr.u16Version);
        goto FREE;
    }

    Dwarf_Obj_Access_Interface_a dw_interface = {dw_accessData, &dw_methods};
    if(dwarf_object_init_b(&dw_interface, NULL, NULL, DW_GROUPNUMBER_ANY, ret_dbg, error) == DW_DLV_OK)
    {
        *dw_accessData_p = dw_accessData;
        fclose(fp);
        
        return DW_DLV_OK;
    }

FREE:
    dwarf_coff_release(dw_accessData);

CLOSE:
    fclose(fp);

    return DW_DLV_ERROR;
}

void dwarf_coff_deinit(Dwarf_Debug dw_dbg, 
    Dwarf_Obj_Access_Data *dw_accessData)
{
    dwarf_finish(dw_dbg);

    if(dw_accessData == NULL) return;

    for(int i = 0; i <= dw_accessData->section_num; i++)
    {
        free(dw_accessData->section[i].name);
        free(dw_accessData->section[i].data);
    }

    free(dw_accessData->section);
    free(dw_accessData);
}

void dwarf_coff_release(Dwarf_Obj_Access_Data *dw_accessData)
{
    if(dw_accessData == NULL) return;

    for(int i = 0; i <= dw_accessData->section_num; i++)
    {
        free(dw_accessData->section[i].name);
        free(dw_accessData->section[i].data);
    }

    free(dw_accessData->section);
    free(dw_accessData);
}
