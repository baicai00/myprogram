#include "tailqueue.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

struct AlarmInfo
{
    TAILQ_ENTRY(AlarmInfo) TQueueField;
    /* 上一句相当于如下语句
    struct {
        struct AlarmInfo *tqe_next;
        struct AlarmInfo **tqe_prev;
    } TQueueField;
    */

    int devId;
    char name[32];
};

TAILQ_HEAD(AlarmInfoTailQueue, AlarmInfo);
/* 上一句相当于如下语句
struct AlarmInfoTailQueue {
    struct AlarmInfo *tqh_first;
    struct AlarmInfo **tqh_last;
};
*/

int main()
{
    struct AlarmInfoTailQueue alarmTQHead;
    TAILQ_INIT(&alarmTQHead);
    /* 上一句相当一如下语句
    (&alarmTQHead)->tqh_first = NULL;
    (&alarmTQHead)->tqh_last = &(&alarmTQHead)->tql_first;
    */

    struct AlarmInfo *obj0 = (struct AlarmInfo*)malloc(sizeof(struct AlarmInfo));
    struct AlarmInfo *obj1 = (struct AlarmInfo*)malloc(sizeof(struct AlarmInfo));
    struct AlarmInfo *obj2 = (struct AlarmInfo*)malloc(sizeof(struct AlarmInfo));
    obj0->devId = 0;
    strcpy(obj0->name, "zhangfei");
    obj1->devId = 1;
    strcpy(obj1->name, "liubei");
    obj2->devId = 2;
    strcpy(obj2->name, "guanyu");

    // 在头部插入
    /* 
    TAILQ_INSERT_HEAD(&alarmTQHead, obj0, TQueueField);
    TAILQ_INSERT_HEAD(&alarmTQHead, obj1, TQueueField);
    TAILQ_INSERT_HEAD(&alarmTQHead, obj2, TQueueField);
    */

    // 在某个元素的后面插入
    /* 
    TAILQ_INSERT_HEAD(&alarmTQHead, obj0, TQueueField);
    TAILQ_INSERT_HEAD(&alarmTQHead, obj1, TQueueField);
    // 将obj2插入到obj1的后面
    TAILQ_INSERT_AFTER(&alarmTQHead, obj1, obj2, TQueueField);
    struct AlarmInfo *p;
    TAILQ_FOREACH(p, &alarmTQHead, TQueueField) {
        printf("%d, %s\n", p->devId, p->name);
    }
    */

    // 在某个元素的前面插入 
    TAILQ_INSERT_HEAD(&alarmTQHead, obj0, TQueueField);
    TAILQ_INSERT_HEAD(&alarmTQHead, obj1, TQueueField);
    // 将obj2插入到obj0的前面
    TAILQ_INSERT_BEFORE(obj0, obj2, TQueueField);
    struct AlarmInfo *p;
    // 打印的顺序是obj1  obj2 obj1
    TAILQ_FOREACH(p, &alarmTQHead, TQueueField) {
        printf("%d, %s\n", p->devId, p->name);
    }

    free(obj0);
    free(obj1);
    free(obj2);

    return 0;
}