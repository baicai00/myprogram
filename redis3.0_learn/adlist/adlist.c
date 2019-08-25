#include "adlist.h"
#include <stdlib.h>

/*
 * 创建一个新的链表
 * 创建成功返回链表，失败返回NULL
 */
list* listCreate(void)
{
    struct list *list;

    if ((list = malloc(sizeof(struct list))) == NULL) {
        return NULL;
    }

    // 初始化属性
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;

    return list;
}

/*
 * 释放整个链表，以及链表中所有节点
 */
void listRelease(list *list)
{
    unsigned long len;
    listNode *current, *next;

    current = list->head;
    len = list->len;
    while (len--) {
        next = current->next;
        // 如果有设置值释放函数，那么调用它来释放节点的值
        if (list->free) {
            list->free(current->value);
        }
        // 释放节点
        free(current);
        current = next;
    }

    // 释放链表头
    free(list);
}

/*
 * 将一个包含有给定值指针value的新节点添加到链表的表头
 * 失败返回NULL，成功返回传入的链表指针
 */
list* listAddNodeHead(list *list, void *value)
{
    listNode *node;

    if ((node = malloc(sizeof(listNode))) == NULL) {
        return NULL;
    }

    node->value = value;
    if (list->len == 0) {
        // 添加节点到空链表
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        // 添加节点到非空链表
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }

    // 更新链表的节点数
    list->len++;

    return list;
}

/*
 * 将一个包含有给定值value的新节点添加到链表的表尾
 */
list* listAddNodeTail(list *list, void *value)
{
    listNode *node;

    if ((node = malloc(sizeof(listNode))) == NULL) {
        return NULL;
    }

    node->value = value;
    if (list->len == 0) {
        // 添加节点到空链表
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        // 添加节点到非空链表
        node->next = NULL;
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }

    // 更新链表计数
    list->len++;

    return list;
}

/*
 * 创建一个包含值value的新节点，并将它插入到old_node的前面或后面
 * 如果after为0，将新节点插入到old_node之前
 * 如果after为1，将新节点插入到old_node之后
 */
list* listInsertNode(list *list, listNode *old_node, void *value, int after)
{
    listNode *node;

    if ((node = malloc(sizeof(listNode))) == NULL) {
        return NULL;
    }

    node->value = value;
    if (after) {
        // 插入到old_node之后
        node->next = old_node->next;
        node->prev = old_node;
        if (list->tail == old_node) {
            list->tail = node;
        }
    } else {
        // 插入到old_node之前
        node->next = old_node;
        node->prev = old_node->prev;
        if (list->head == old_node) {
            list->head = node;
        }
    }

    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }

    list->len++;

    return list;
}

/*
 * 从链表list中删除给定节点node
 */
void listDelNode(list *list, listNode *node)
{
    // 调整前置节点的指针
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        // 节点的prev域为NULL,说明该节点为链表的第一个节点
        list->head = node->next;
    }

    // 调整后置节点的指针
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        // 节点的next域为NULL,说明该节点为链表的最后一个节点
        list->tail = node->prev;
    }

    // 释放节点值
    if (list->free) {
        list->free(node->value);
    }
    // 释放节点
    free(node);
    // 链表节点数减一
    list->len--;
}

/*
 * 为给定链表创建一个迭代器，之后每次对这个迭代器调用listNext函数都将返回被迭代到的链表节点
 * 
 * direction参数次定了迭代器的迭代方向
 * AL_START_HEAD：从表头向表尾迭代
 * AL_START_TAIL：从表尾向表头迭代
 */
listIter* listGetIterator(list *list, int direction)
{
    listIter *iter;
    if ((iter = (listIter*)malloc(sizeof(listIter))) == NULL) {
        return NULL;
    }

    if (direction == AL_START_HEAD) {
        iter->next = list->head;
    } else {
        iter->next = list->tail;
    }

    iter->direction = direction;

    return iter;
}

/*
 * 返回迭代器当前所指向的节点。
 *
 * 删除当前节点是允许的，但不能修改链表里的其他节点。
 *
 * 函数要么返回一个节点，要么返回 NULL ，常见的用法是：
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * T = O(1)
 */
listNode* listNext(listIter *iter)
{
    listNode *current = iter->next;
    if (current != NULL) {
        if (iter->direction == AL_START_HEAD) {
            iter->next = current->next;
        } else {
            iter->next = current->prev;
        }
    }

    return current;
}

void listReleaseIterator(listIter *iter)
{
    free(iter);
}

/*
 * 复制整个链表。
 *
 * 复制成功返回输入链表的副本，
 * 如果因为内存不足而造成复制失败，返回 NULL 。
 *
 * 如果链表有设置值复制函数 dup ，那么对值的复制将使用复制函数进行，
 * 否则，新节点将和旧节点共享同一个指针。
 *
 * 无论复制是成功还是失败，输入节点都不会修改。
 *
 * T = O(N)
 */
list* listDup(list *orig)
{
    list *copy;
    listIter *iter;
    listNode *node;

    // 创建新链表
    if ((copy = listCreate()) == NULL) {
        return;
    }

    // 设置节点值处理函数
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;

    // 迭代整个输入链表
    iter = listGetIterator(orig, AL_START_HEAD);
    while ((node = listNext(iter)) != NULL) {
        void *value;

        // 复制节点值到新节点
        if (copy->dup) {
            value = copy->dup(node->value);
            if (value == NULL) {
                listRelease(copy);
                listReleaseIterator(iter);
                return NULL;
            }
        } else {
            value = node->value;
        }

        // 将节点添加到链表
        if (listAddNodeTail(copy, value) == NULL) {
            listRelease(copy);
            listReleaseIterator(iter);
            return NULL;
        }
    }

    // 释放迭代器
    listReleaseIterator(iter);

    // 返回副本
    return copy;
}

/* 
 * 查找链表 list 中值和 key 匹配的节点。
 * 
 * 对比操作由链表的 match 函数负责进行，
 * 如果没有设置 match 函数，
 * 那么直接通过对比值的指针来决定是否匹配。
 *
 * 如果匹配成功，那么第一个匹配的节点会被返回。
 * 如果没有匹配任何节点，那么返回 NULL 。
 *
 * T = O(N)
 */
listNode* listSearchKey(list *list, void *key)
{
    listIter *iter;
    listNode *node;

    // 迭代整个链表
    iter = listGetIterator(list, AL_START_HEAD);
    while ((node = listNext(iter)) != NULL) {
        if (list->match) {
            if (list->match(node->value, key)) {
                listReleaseIterator(iter);
                // 找到
                return node;
            }
        } else {
            if (key == node->value) {
                listReleaseIterator(iter);
                // 找到
                return node;
            }
        }
    }

    listReleaseIterator(iter);

    // 未找到
    return NULL;
}

/*
 * 返回链表在给定索引上的值。
 *
 * 索引以 0 为起始，也可以是负数， -1 表示链表最后一个节点，诸如此类。
 *
 * 如果索引超出范围（out of range），返回 NULL 。
 *
 * T = O(N)
 */
listNode* listIndex(list *list, long index)
{
    listNode *n;

    if (index < 0) {
        // 索引为负数，从表尾开始查找
        index = (-index) - 1;
        n = list->tail;
        while (index-- && n) {
            n = n->prev;
        }
    } else {
        // 索引为正数，从表头开始查找
        n = list->head;
        while (index-- && n) {
            n = n->next;
        }
    }

    return n;
}

/*
 * 将迭代器的方向设置为 AL_START_HEAD ，
 * 并将迭代指针重新指向表头节点。
 *
 * T = O(1)
 */
void listRewind(list *list, listIter *li)
{
    li->direction = AL_START_HEAD;
    li->next = list->head;
}

/*
 * 将迭代器的方向设置为 AL_START_TAIL ，
 * 并将迭代指针重新指向表尾节点。
 *
 * T = O(1)
 */
void listRewindTail(list *list, listIter *li)
{
    li->direction = AL_START_TAIL;
    li->next = list->tail;
}

/* Rotate the list removing the tail node and inserting it to the head. */
/*
 * 取出链表的表尾节点，并将它移动到表头，成为新的表头节点。
 *
 * T = O(1)
 */
void listRotate(list *list)
{
    listNode *tail = list->tail;

    if (listLength(list) <= 1) {
        return;
    }

    // 取出表尾节点
    list->tail = tail->prev;
    tail->prev->next = NULL;

    // 插入到表头
    tail->next = list->head;
    tail->prev = NULL;
    list->head->prev = tail;
    list->head = tail;
}