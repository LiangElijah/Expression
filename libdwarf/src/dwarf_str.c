#include "dwarf_str.h"

int dwarf_str_init(const char *str, 
    st_str_t *strBuf)
{
    if(str == NULL) return -1;
    
    memset(strBuf, 0, sizeof(st_str_t));
    for(int index = 0, count = 0; index < (strlen(str) + 1); index++) {
        if(str[index] == '\0') {   // 字符串以 '\0' 结尾
            if(count == 0) {        // 以 . 结尾或空字符串
                printf("[%s-%s:%d] End With \'.\' or Empty String. (%s)\n", __FILE__, __func__, __LINE__, str);
                return -1;
            }

            strBuf->segmentNum++;
            break;
        } else if(str[index] != '.') { // 成员字符
            if(str[index] == '[') {    // 数组起始符
                if(count == 0) {        // 变量以 [ 开头
                    printf("[%s-%s:%d] Begin With \'[\'. (%s)\n", __FILE__, __func__, __LINE__, str);
                    return -1;
                }

                char number[16] = {0};
                int pos = 0;
                while(str[++index] != ']') {   // 数组结束符
                    if(str[index] == '\0') {   // 无 ']' 结尾 
                        printf("[%s-%s:%d] No \']\'. (%s)\n", __FILE__, __func__, __LINE__, str);
                        return -1;
                    } else if((str[index] > 57) || ((str[index] < 48))) { // 非数字
                        printf("[%s-%s:%d] Not Num. (%s)\n", __FILE__, __func__, __LINE__, str);
                        return -1;
                    }

                    number[pos] = str[index];
                    pos++;
                }
                if(pos == 0) {  // '[' 和 ']' 之间无数字 
                    printf("[%s-%s:%d] No Num. (%s)\n", __FILE__, __func__, __LINE__, str);
                    return -1;
                }
                strBuf->dimension[strBuf->segmentNum][strBuf->dimensionNum[strBuf->segmentNum]] = atoi(number);
                strBuf->dimensionNum[strBuf->segmentNum]++;
            } else {    // 非数组成员
                if(strBuf->dimensionNum[strBuf->segmentNum] != 0) {   // 数组后接成员字符
                    printf("[%s-%s:%d] Unknown Format. (%s)\n", __FILE__, __func__, __LINE__, str);
                    return -1;
                }

                strBuf->segment[strBuf->segmentNum][count] = str[index];
                count++;
            }
        } else {    // 成员间隔符
            if(count == 0) {    // 以 . 开头或只有一个 .
                printf("[%s-%s:%d] Begin With \'.\' or Only \'.\'. (%s)\n", __FILE__, __func__, __LINE__, str);
                return -1;
            }

            strBuf->segmentNum++;
            count = 0;
        }
    }
    memcpy(strBuf->str, str, strlen(str));

    return 0;
}

int dwarf_print_str(st_str_t *strBuf)
{
    if(strBuf != NULL)
    {
        printf("origin:%s\r\n", strBuf->str);
        for(int i = 0; i < strBuf->segmentNum; i++)
        {
            printf("%s ", strBuf->segment[i]);
            for(int j = 0; j < strBuf->dimensionNum[i]; j++)
            {
                printf("%d ", strBuf->dimension[i][j]);
            }
            printf("\r\n");
        }
    }

    return 0;
}
