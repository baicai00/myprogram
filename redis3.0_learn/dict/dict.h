#ifndef _DICT_H_
#define _DICT_H_

#include <stdint.h>

/*
 * 字典的操作状态
 */
// 操作成功
#define DICT_OK 0
// 操作失败（或出错）
#define DICT_ERR 1

/* Unused arguments generate annoying warnings... */
// 如果字典的私有数据不使用时
// 用这个宏来避免编译器错误
#define DICT_NOTUSED(V) ((void) V)

/*
 * 哈希表节点
 */
typedef struct dictEntry {
    // 键
    void *key;

    // 值
    union {
        void *val;
        uint64_t u64;
        int64_t s64;
    } v;

    // 指向下一个哈希表节点，形成链表
    struct dictEntry *next;

} dictEntry;

/*
 * 字典类型特定函数
 */
typedef struct dictType {
    // 计算哈希值的函数
    unsigned int (*hashFunction)(const void *key);

    // 复制键的函数
    void *(*keyDup)(void *privdata, const void *key);

    // 复制值的函数
    void *(*valueDup)(void *privdata, const void *obj);

    // 对比键的函数
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);

    // 销毁键的函数
    void (*keyDestructor)(void *privdata, void *key);

    // 销毁值的函数
    void (*valDestructor)(void *privdata, void *obj);
} dictType;

/*
 * 哈希表
 * 每个字典都使用两个哈希表，从而实现渐进式rehash
 */
typedef struct dictht {
    // 哈希表数组
    dictEntry **table;

    // 哈希表大小
    unsigned long size;

    // 哈希表大小掩码，用于计算索引值
    // 总是等于size-1
    unsigned long sizemask;

    // 该哈希表已有节点的数量
    unsigned long used;
} dictht;

/*
 * 字典
 */
typedef struct dict {
    // 类型特定函数
    dictType *type;

    // 私有数据
    void *privdata;

    // 哈希表
    dictht ht[2];

    // rehash索引
    // 当rehash不在进行时，值为-1
    int rehashidx;

    // 目前正在运行的安全迭代器的数量
    int iterators;
} dict;

/*
 * 哈希表的初始大小
 */
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/
// 释放给定字典节点的值,d为dict类型的字典,entry为哈希表节点(dictEntry类型)
#define dictFreeVal(d, entry) \
    if ((d)->type->valDestructor) \
        (d)->type->valDestructor((d)->privdata, (entry)->v.val)

// 设置给定字典节点的值
#define dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        entry->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        entry->v.val = (_val_); \
} while (0)

// 将一个有符号整数设置为节点的值
#define dictSetSignedIntegerVal(entry, _val_) \
    do { entry->v.s64 = _val_; } while (0)

// 将一个无符号整数设置为节点的值
#define dictSetUnsignedIntegerVal(entry, _val_) \
    do { entry->v.u64 = _val_; } while (0)

// 释放给定字典节点的键
#define dictFreeKey(d, entry) \
    if ((d)->type->keyDestructor) \
        (d)->type->keyDestructor((d)->privdata, (entry)->key)

// 设置给定字典节点的键
#define dictSetKey(d, entry, _key_) do { \
    if ((d)->type->keyDup) \
        (entry)->key = (d)->type->keyDup((d)->privdata, _key_); \
    else \
        (entry)->key = _key_;
} while (0)

// 对比两个键
#define dictCompareKeys(d, key1, key2) \
    (((d)->type->keyCompare) ? \
        (d)->type->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))

// 计算给定键的哈希值
#define dictHashKey(d, key) (d)->type->hashFunction(key)
// 获取给定节点的键
#define dictGetKey(he) ((he)->key)
// 获取给定节点的值
#define dictGetVal(he) ((he)->v.val)
// 获取给定节点的有符号整数值
#define dictGetSignedIntegerVal(he) ((he)->v.s64)
// 返回给定节点的无符号整数值
#define dictGetUnsignedIntegerVal(he) ((he)->v.u64)
// 返回给定字典的大小
#define dictSlots(d) ((d)->ht[0].size + (d)->ht[1].size)
// 返回给定字典已有节点数量
#define dictSize(d) ((d)->ht[0].used + (d)->ht[1].used)
// 返回字典是否正在rehash,返回true表示正在rehash
#define dictIsRehashing(ht) ((ht)->rehashidx != -1)

/* API */
dict* dictCreate(dictType *type, void *privDataPtr);
int dictExpand(dict *d, unsigned long size);
int dictAdd(dict *d, void *key, void *val);
dictEntry* dictAddRaw(dict *d, void *key);
int dictReplace(dict *d, void *key, void *val);
dictEntry *dictReplaceRaw(dict *d, void *key);
int dictDelete(dict *d, const void *key);
int dictDeleteNoFree(dict *d, const void *key);
void dictRelease(dict *d);
dictEntry * dictFind(dict *d, const void *key);
void *dictFetchValue(dict *d, const void *key);
int dictResize(dict *d);
dictIterator *dictGetIterator(dict *d);
dictIterator *dictGetSafeIterator(dict *d);
dictEntry *dictNext(dictIterator *iter);
void dictReleaseIterator(dictIterator *iter);
dictEntry *dictGetRandomKey(dict *d);
int dictGetRandomKeys(dict *d, dictEntry **des, int count);
void dictPrintStats(dict *d);
unsigned int dictGenHashFunction(const void *key, int len);
unsigned int dictGenCaseHashFunction(const unsigned char *buf, int len);
void dictEmpty(dict *d, void(callback)(void*));
void dictEnableResize(void);
void dictDisableResize(void);
int dictRehash(dict *d, int n);
int dictRehashMilliseconds(dict *d, int ms);
void dictSetHashFunctionSeed(unsigned int initval);
unsigned int dictGetHashFunctionSeed(void);
unsigned long dictScan(dict *d, unsigned long v, dictScanFunction *fn, void *privdata);

#endif