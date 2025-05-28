/********************************************************
 * Description : sample of mongo helper
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#include <cstdio>
#include "mongo_helper.h"

/*
 * uri format
 *     mongodb://[username:password@]host1[:port1[,host2:port2]]/database?safe=true&fsync=true&journal=true&wtimeoutMS=5000&connectTimeoutMS=5000&socketTimeoutMS=5000&replicaSet=myReplicaSet
 */

static bool test()
{
    MongoTable mongo_table;
    if (!mongo_table.init("mongodb://localhost:27017/", "db_test", "tb_test"))
    {
        printf("mongo table init failed\n");
        return false;
    }

    if (0 == mongo_table.count())
    {
        if (!mongo_table.index("user_id"))
        {
            printf("mongo table init failed\n");
            return false;
        }
    }

    if (mongo_table.select())
    {
        std::string element;
        while (mongo_table.read(element))
        {
            printf("%s\n", element.c_str());
        }

        if (!mongo_table.remove())
        {
            printf("mongo table init failed\n");
            return false;
        }
    }

    if (!mongo_table.insert("{\"user_id\": 111, \"user_name\": \"jack\", \"hobbies\": [\"reading\", \"writing\", \"drawing\"], \"birthday\": \"2001-04-07\", \"married\": true}"))
    {
        printf("mongo table insert failed\n");
        return false;
    }

    if (!mongo_table.insert("{\"user_id\": 222, \"user_name\": \"john\", \"hobbies\": [], \"birthday\": \"2002-05-08\", \"married\": true}"))
    {
        printf("mongo table insert failed\n");
        return false;
    }

    if (!mongo_table.insert("{\"user_id\": 333, \"user_name\": \"mary\", \"hobbies\": [\"dancing\"], \"birthday\": \"2003-06-09\", \"married\": true}"))
    {
        printf("mongo table insert failed\n");
        return false;
    }

    if (!mongo_table.update("{\"user_id\": 222}", "{\"user_name\": \"mark\", \"hobbies\": [\"acting\"], \"school\": [\"m1\", \"u2\"], \"birthday\": \"2002-07-09\", \"married\": false}"))
    {
        printf("mongo table update failed\n");
        return false;
    }

    return true;
}

int main()
{
    if (test())
    {
        printf("mongo table test success\n");
    }
    else
    {
        printf("mongo table test failure\n");
    }
    return 0;
}
