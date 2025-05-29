/********************************************************
 * Description : sample of leveldb helper
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#include "leveldb_helper.h"


struct user_storage_t
{
    int32_t     user_id;
    int32_t     storage_id;
    uint64_t    file_count;
    uint64_t    file_bytes;
};

static bool operator != (const user_storage_t & lhs, const user_storage_t & rhs)
{
    return (0 != memcmp(&lhs, &rhs, sizeof(user_storage_t)));
}

static bool test_1()
{
    const std::string path("./db1");

    if (!LevelDB::destroy(path))
    {
        printf("level db destroy test failed\n");
        return false;
    }

    LevelDB db;
    if (!db.init(path))
    {
        printf("level db init test failed\n");
        return false;
    }

    if (!db.erase("not exist"))
    {
        printf("level db erase test failed\n");
        return false;
    }

    user_storage_t data_in = { 0x0 };
    data_in.user_id = 10001;
    data_in.storage_id = 34411;
    data_in.file_count = 11111;
    data_in.file_bytes = 123456789;
    if (!db.set("test", &data_in, sizeof(data_in)))
    {
        printf("level db set test failed\n");
        return false;
    }

    user_storage_t data_out = { 0x0 };
    if (!db.get("test", &data_out, sizeof(data_out)))
    {
        printf("level db get test failed\n");
        return false;
    }

    if (data_in != data_out)
    {
        printf("level db get data size is incorrect\n");
        return false;
    }

    if (!db.erase("test"))
    {
        printf("level db erase test failed\n");
        return false;
    }

    if (db.get("test", &data_out, sizeof(data_out)))
    {
        printf("level db erase test failed\n");
        return false;
    }

    db.exit();

    if (!LevelDB::destroy(path))
    {
        printf("level db destroy test failed\n");
        return false;
    }

    return true;
}

static bool test_2()
{
    const std::string path("./db2");

    if (!LevelDB::destroy(path))
    {
        printf("level db destroy test failed\n");
        return false;
    }

    LevelDB db;
    if (!db.init(path))
    {
        printf("level db init test failed\n");
        return false;
    }

    // release db reference when leave the scope
    {
        LevelIter iter1(db);
        if (iter1.good())
        {
            printf("level iter good test failed\n");
            return false;
        }
    }

    if (!db.set("a", "app"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    if (!db.set("b", "bak"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    if (!db.set("c", "cfg"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    if (!db.set("d", "dev"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    if (!db.set("e", "etc"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    // release db reference when leave the scope
    {
        LevelIter iter2(db);
        while (iter2.good())
        {
            printf("(%s) -> (%s)\n", iter2.get_key().c_str(), iter2.get_value().c_str());
            iter2.next();
        }
        printf("\n");
    }

    if (!db.set("c", "1cfg"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    if (!db.set("d", "2dev"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    if (!db.set("e", "3etc"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    if (!db.set("f", "4fee"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    if (!db.set("a", "5app"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    if (!db.set("b", "6bak"))
    {
        printf("level iter set test failed\n");
        return false;
    }

    // release db reference when leave the scope
    {
        LevelIter iter3(db);
        while (iter3.good())
        {
            printf("(%s) -> (%s)\n", iter3.get_key().c_str(), iter3.get_value().c_str());
            iter3.next();
        }
        printf("\n");
    }

    db.exit();

    if (!LevelDB::destroy(path))
    {
        printf("level db destroy test failed\n");
        return false;
    }

    return true;
}

int main()
{
    if (test_1())
    {
        printf("level db test 1 success\n");
    }
    else
    {
        printf("level db test 1 failure\n");
    }

    if (test_2())
    {
        printf("level db test 2 success\n");
    }
    else
    {
        printf("level db test 2 failure\n");
    }

    return 0;
}

