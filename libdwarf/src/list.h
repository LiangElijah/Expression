/**
 * @file      : kernel_list.h
 * @brief     : 内核链表头文件(从Linux内核中提取出来)
 * @author    : huenrong (huenrong1028@outlook.com)
 * @date      : 2023-01-30 14:20:37
 *
 * @copyright : Copyright (c) 2023 huenrong
 *
 * @history   : date       author          description
 *              2023-01-30 huenrong        创建文件
 *
 */

#ifndef __KERNEL_LIST_H
#define __KERNEL_LIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

// 链表删除后, 指向的位置(这两个位置在系统中不能被访问, 访问会报错)
#if 0
#define POISON_POINTER_DELTA 0

#define LIST_POISON1 ((void *)0x00100100 + POISON_POINTER_DELTA)
#define LIST_POISON2 ((void *)0x00200200 + POISON_POINTER_DELTA)
// 为了在多平台兼容, 可以将其定义为NULL
#else
#include <stddef.h>

#define LIST_POISON1 NULL
#define LIST_POISON2 NULL
#endif

#ifndef offsetof
/**
 * @brief  计算成员变量member在type(结构体)中的偏移地址
 * @param  type  : 输入参数, 结构体类型
 * @param  member: 输入参数, 结构体中的成员变量名
 */
#define offsetof(type, member) (size_t)(&((type *)0)->member)
#endif

// typeof是GNU中获取变量类型的关键字
/**
 * @brief  根据成员变量member的地址获取type(结构体)的起始地址
 * @param  ptr   : 输入参数, 指向成员变量member的指针
 * @param  type  : 输入参数, 结构体类型
 * @param  member: 输入参数, 结构体中的成员变量名
 */
#define container_of(ptr, type, member) ({ \
    const typeof(((type *)0)->member)*__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); })

// 链表结构
typedef struct list_head
{
    struct list_head *next;
    struct list_head *prev;
} list_head_t;

#define LIST_HEAD_INIT(name) \
    {                        \
        &(name), &(name)     \
    }

// 初始化链表头节点
#define LIST_HEAD(name) \
    list_head_t name = LIST_HEAD_INIT(name)

// 初始化链表头节点
static inline void INIT_LIST_HEAD(list_head_t *list)
{
    list->next = list;
    list->prev = list;
}

// 初始化链表头节点（清零）
static inline void RESET_LIST_HEAD(list_head_t *list)
{
    list->next = NULL;
    list->prev = NULL;
}

/**
 * @brief  在两个已知的连续节点之间插入新的节点(仅限内部使用)
 * @param  new_node : 输出参数, 待插入的新节点
 * @param  prev_node: 输出参数, 已知的前一个节点
 * @param  next_node: 输出参数, 已知的后一个节点
 */
static inline void __list_add(list_head_t *new_node, list_head_t *prev_node, list_head_t *next_node)
{
    if(next_node != NULL) next_node->prev = new_node;
    new_node->next = next_node;
    new_node->prev = prev_node;
    if(prev_node != NULL) prev_node->next = new_node;
}

/**
 * @brief  插入节点(头插法)
 * @param  new_node : 输出参数, 待插入的新节点
 * @param  head_node: 输出参数, 链表头节点
 */
static inline void list_add(list_head_t *new_node, list_head_t *head_node)
{
    __list_add(new_node, head_node, head_node->next);
}

/**
 * @brief  插入节点(尾插法)
 * @param  new_node : 输出参数, 待插入的新节点
 * @param  head_node: 输出参数, 链表头节点
 */
static inline void list_add_tail(list_head_t *new_node, list_head_t *head_node)
{
    __list_add(new_node, head_node->prev, head_node);
}

/**
 * @brief  删除两个已知的连续节点之间的节点(仅限内部使用)
 * @param  prev_node: 输出参数, 已知的前一个节点
 * @param  next_node: 输出参数, 已知的后一个节点
 */
static inline void __list_del(list_head_t *prev_node, list_head_t *next_node)
{
    next_node->prev = prev_node;
    prev_node->next = next_node;
}

/**
 * @brief  删除指定节点
 * @param  entry_node: 输出参数, 待删除的节点
 */
static inline void list_del(list_head_t *entry_node)
{
    __list_del(entry_node->prev, entry_node->next);
    entry_node->next = LIST_POISON1;
    entry_node->prev = LIST_POISON2;
}

/**
 * @brief  判断链表是否为空
 * @param  head_node: 输入参数, 带判断的链表
 * @return 0: 链表非空
 *         1: 链表为空
 */
static inline int list_empty(const list_head_t *head_node)
{
    return head_node->next == head_node;
}

/**
 * @brief  根据成员变量member的地址获取type(结构体)的起始地址
 * @param  ptr   : 输入参数, 指向成员变量member的指针(struct list_head指针)
 * @param  type  : 输入参数, 结构体类型
 * @param  member: 输入参数, 结构体中的成员变量名(结构体中list_head的名称)
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * @brief  从链表中获取第一个元素
 * @param  ptr   : 输入参数, 从中获取元素的链表头
 * @param  type  : 输入参数, 结构体类型
 * @param  member: 输入参数, 结构体中的成员变量名(结构体中list_head的名称)
 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

/**
 * @brief  获取链表中的下一个元素
 * @param  ptr   : 输入参数, 从中获取元素的链表头
 * @param  type  : 输入参数, 结构体类型
 * @param  member: 输入参数, 结构体中的成员变量名(结构体中list_head的名称)
 */
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * @brief  遍历链表
 * @param  pos : 输出参数, 遍历得到的链表节点(list_head_t *类型指针)
 * @param  head: 输入参数, 链表头(list_head_t *类型指针)
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief  遍历给定类型的链表
 * @param  pos   : 输出参数, 遍历得到的链表节点(用户层定义结构体指针)
 * @param  head  : 输入参数, 链表头(list_head_t *类型指针)
 * @param  member: 输入参数, 结构体中list_head的名称
 */
#define list_for_each_entry(pos, head, member)               \
    for (pos = list_first_entry(head, typeof(*pos), member); \
         &pos->member != (head);                             \
         pos = list_next_entry(pos, member))

/**
 * @brief  遍历给定类型的链表, 以防删除链表项
 * @param  pos   : 输出参数, 遍历得到的链表节点(用户层定义结构体指针)
 * @param  n     : 输入参数, 临时变量(用户层定义结构体指针)
 * @param  head  : 输入参数, 链表头(list_head_t *类型指针)
 * @param  member: 输入参数, 结构体中list_head的名称
 */
#define list_for_each_entry_safe(pos, n, head, member)       \
    for (pos = list_first_entry(head, typeof(*pos), member), \
        n = list_next_entry(pos, member);                    \
         &pos->member != (head);                             \
         pos = n, n = list_next_entry(n, member))

/**
 * @brief  替换节点(未断开old的前驱和后继)
 * @param  old_node: 输出参数, 待替换的节点
 * @param  new_node: 输出参数, 待插入的节点
 */
static inline void list_replace(list_head_t *old_node, list_head_t *new_node)
{
    new_node->next = old_node->next;
    new_node->next->prev = new_node;
    new_node->prev = old_node->prev;
    new_node->prev->next = new_node;
}

/**
 * @brief  替换节点(断开old的前驱和后继)
 * @param  old: 输出参数, 待替换的节点
 * @param  new: 输出参数, 待插入的节点
 */
static inline void list_replace_init(list_head_t *old_node, list_head_t *new_node)
{
    list_replace(old_node, new_node);
    INIT_LIST_HEAD(old_node);
}

#ifdef __cplusplus
}
#endif

#endif // __KERNEL_LIST_H
