#include "MysqlClient.h"
#include <stdio.h>
#include <time.h>

int main()
{
    MysqlClient mysql;
    bool ret = mysql.mysql_connect("192.168.182.129:3306", "root", "123", "dk_test");
    if (!ret)
    {
        printf("connect failed.\n");
        return 0;
    }

    time_t start_t = time(NULL);
    /*printf("begin insert \n");
    for (int i = 0; i <= 100000; i++)
    {
        char sql[256];
        sprintf(sql, "insert into t_user (name, age) value ('test%d', %d)", i, i);
        if (!mysql.mysql_exec(sql))
        {
            printf("exec sql failed. sql: %s\n", sql);
        }
    }
    printf("end insert time: %d\n", time(NULL) - start_t);*/

    start_t = time(NULL);
    printf("begin select\n");
    char buff[1024];
    sprintf(buff, "select * from t_user where name = 'test100000'");
    mysql.mysql_exec(buff);
    printf("begin select time: %d\n", time(NULL) - start_t);
    
    MysqlResult result(mysql.handle());
    MysqlRow row;
    result.fetch_row(row);
    printf("uid=%d, age=%d\n", row.get_int("uid"), row.get_int("age"));

    return 0;
}