#ifndef __DWARF_COFF_H__
#define __DWARF_COFF_H__

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "dwarf.h"
#include "libdwarf.h"
#include "libdwarf_private.h"
#include "config.h"

#include "dwarf_method.h"

// 文件头
typedef struct st_filehdr {
    uint16_t u16Version;        // 版本
    uint16_t u16NumSec;         // 段落数
    int32_t  i32Time;           // 时间戳
    int32_t  i32SymbolOffset;   // 符号表偏移
    int32_t  i32NumSymbol;      // 符号数
    uint16_t u16OptHdrSZ;       // 可选头长度
    uint16_t u16Flags;          // 标记
    uint16_t u16Magic;          // 魔数
} __attribute__((packed)) st_filehdr_t;

// 可选头
typedef struct st_opthdr {
    int16_t i16Magic;           // 魔数
    int16_t i16Version;         // 版本
    int32_t i32TextSize;        // 代码段大小
    int32_t i32InitDataSZ;      // 已初始化数据段大小
    int32_t i32UninitDataSZ;    // 未初始化数据段大小
    int32_t i32Entry;           // 程序入口点
    int32_t i32TextBase;        // 代码段基址
    int32_t i32DataBase;        // 数据段基址（在PE32中才有）
} __attribute__((packed)) st_opthdr_t;

// 段落头
typedef struct st_sechdr {
    union {
        int8_t i8Name[8];       // 段名
        struct {
        uint32_t u32Zero;       // 字符串表标识
        uint32_t u32Offset;     // 字符串偏移
        } __attribute__((packed)) s;
    } u;
    int32_t  i32PAddr;          // 物理地址
    int32_t  i32VAddr;          // 虚拟地址
    int32_t  i32Size;           // 段长度
    int32_t  i32SecOffset;      // 段数据偏移
    int32_t  i32RelOffset;      // 段重定位表偏移
    int32_t  i32Reserved;       // 预留
    uint32_t u32NumRel;         // 重定位表数量
    uint32_t i32Reserved2;      // 预留
    uint32_t u32Flags;          // 段标记
    uint16_t u16Reserved3;      // 预留
    uint16_t u16NumMP;          // 内存页数量
} __attribute__((packed)) st_sechdr_t;

// 符号头
typedef struct st_syment {
  union {
    int8_t i8Name[8];           // 符号名
    struct {
      uint32_t u32Zero;         // 字符串表标识
      uint32_t u32Offset;       // 字符串偏移
    } __attribute__((packed)) s;
  } u;
  int32_t  i32Value;            // 符号值
  int16_t  i16Section;          // 符号所在段
  uint16_t u16Reserved;         // 预留
  int8_t   i8Class;             // 符号存储类型
  int8_t   i8NumAux;            // 符号附加记录数
} __attribute__((packed)) st_syment_t;

int dwarf_coff_init(const char *path,
    Dwarf_Obj_Access_Data **dw_accessData_p, 
    Dwarf_Debug *ret_dbg, 
    Dwarf_Error *error);
void dwarf_coff_deinit(Dwarf_Obj_Access_Data **dw_accessData_p);

#endif /* __DWARF_COFF_H__ */