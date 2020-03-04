#include "dict.h"

// 指示字典是否启用 rehash 的标识
static int dict_can_resize = 1;
// 强制 rehash 的比率
static unsigned int dict_force_resize_ratio = 5;

/* -------------------------- private prototypes ---------------------------- */
static int _dictExpandIfNeeded(dict *ht);
static unsigned long _dictNextPower(unsigned long size);
static int _dictKeyIndex(dict *ht, const void *key);
static int _dictInit(dict *ht, dictType *type, void *privDataPtr);


int _dictExpandIfNeeded(dict *d)
{
    if (dictIsRehashing(d)) {
        return DICT_OK;
    }

    if (d->ht[0].size == 0) {
        return dictExpand(d, DICT_HT_INITIAL_SIZE);
    }

    if (d->ht[0].used >= d->ht[0].size && 
        (dict_can_resize || d->ht[0].used/d->ht[0].size > dict_force_resize_ratio)) {
        return dictExpand(d, d->ht[0].used*2);    
    }

    return DICT_OK;
}

// 重置(或初始化)给定哈希表的各项属性值
static void _dictReset(dictht *ht) 
{
    ht->table = NULL;
    ht->size = 0;
    ht->used = 0;
    ht->sizemask = 0;
}

// 初始化哈希表
int _dictInit(dict *d, dictType *type, void *privDataPtr)
{
    // 初始化两个哈希表的各项属性值
    // 但暂时还不分配内存给哈希表数组
    _dictReset(&d->ht[0]);
    _dictReset(&d->ht[1]);

    // 设置类型特定函数
    d->type = type;
}