/********************************************************
 * Description : sample of sqlite helper
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#include <cstdio>
#include <string>
#include "sqlite_helper.h"

struct table_row_t
{
    uint64_t        id;
    std::string     name;
    uint16_t        age;
    std::string     address;
    double          salary;
};

static const table_row_t table_row[] =
{
    { 0, "Paul",  32, "California", 20000.00 },
    { 1, "Allen", 25, "Texas",      15000.00 },
    { 2, "Teddy", 23, "Norway",     20000.00 },
    { 3, "Mark",  25, "Rich-Mond",  65000.00 }
};

static bool test_table_create(SQLiteDB & db)
{
    printf("test create table ...\n");

    const char * create_table_sql = 
        "CREATE TABLE IF NOT EXISTS COMPANY ("  \
        "ID BIGINT PRIMARY KEY       NOT NULL," \
        "NAME              TEXT      NOT NULL," \
        "AGE               INT       NOT NULL," \
        "ADDRESS           CHAR(50),"           \
        "SALARY            REAL );";

    return db.execute(create_table_sql);
}

static bool test_table_delete(SQLiteDB & db)
{
    printf("test delete data ...\n");

    const char * delete_sql = "DELETE FROM COMPANY;";
    SQLiteWriter writer(db.create_writer(delete_sql));
    if (!writer.good())
    {
        return false;
    }

    if (!writer.reset())
    {
        return false;
    }

    if (!writer.write())
    {
        return false;
    }

    return true;
}

static bool test_table_insert(SQLiteDB & db)
{
    printf("test insert data ...\n");

    const char * insert_sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) VALUES (?, ?, ?, ?, ?);";
    SQLiteWriter writer(db.create_writer(insert_sql));
    if (!writer.good())
    {
        return false;
    }

    if (!db.transaction_begin())
    {
        return false;
    }

    for (uint32_t index = 0; index < sizeof(table_row) / sizeof(table_row[0]); ++index)
    {
        const table_row_t & row = table_row[index];
        printf("    %d: [" GOOFER_U64_FMT " %10s %6d %12s %12.2f]\n", index, row.id, row.name.c_str(), row.age, row.address.c_str(), row.salary);
        writer.reset();
        writer.set(row.id);
        writer.set(row.name);
        writer.set(row.age);
        writer.set(row.address);
        writer.set(row.salary);
        if (!writer.write())
        {
            break;
        }
    }

    if (!db.transaction_end())
    {
        return false;
    }

    return true;
}

static bool test_table_update(SQLiteDB & db)
{
    printf("test update data ...\n");

    const char * update_sql = "UPDATE COMPANY set SALARY = SALARY+? where ID=?;";
    SQLiteWriter writer(db.create_writer(update_sql));
    if (!writer.good())
    {
        return false;
    }

    if (!db.transaction_begin())
    {
        return false;
    }

    for (uint32_t index = 0; index < sizeof(table_row) / sizeof(table_row[0]); ++index)
    {
        printf("    %d update...\n", index);
        writer.reset();
        writer.set(10000.00 + 1000.00 * index);
        writer.set(index);
        if (!writer.write())
        {
            break;
        }
    }

    if (!db.transaction_end())
    {
        return false;
    }

    return true;
}

static bool test_table_select(SQLiteDB & db)
{
    printf("test select data ...\n");

    const char * select_sql = "SELECT * from COMPANY;";
    SQLiteReader reader(db.create_reader(select_sql));
    if (!reader.good())
    {
        return false;
    }

    while (reader.read())
    {
        table_row_t row;
        reader.get(row.id);
        reader.get(row.name);
        reader.get(row.age);
        reader.get(row.address);
        reader.get(row.salary);
        printf("    [" GOOFER_U64_FMT " %10s %6d %12s %12.2f]\n", row.id, row.name.c_str(), row.age, row.address.c_str(), row.salary);
    }

    return true;
}

static bool test_sqlite()
{
    SQLiteDB db;

    const char * path = "./test.db";
    if (!db.init(path))
    {
        printf("sqlite db init failure\n");
        return false;
    }

    if (!db.is_open())
    {
        printf("sqlite db is not open\n");
        return false;
    }

    if (!test_table_create(db))
    {
        printf("sqlite test table create failure\n");
        return false;
    }

    if (!test_table_delete(db))
    {
        printf("sqlite test table delete failure\n");
        return false;
    }

    if (!test_table_insert(db))
    {
        printf("sqlite test table insert failure\n");
        return false;
    }

    if (!test_table_select(db))
    {
        printf("sqlite test table select failure\n");
        return false;
    }

    if (!test_table_update(db))
    {
        printf("sqlite test table update failure\n");
        return false;
    }

    if (!test_table_select(db))
    {
        printf("sqlite test table select failure\n");
        return false;
    }

    if (!test_table_delete(db))
    {
        printf("sqlite test table delete failure\n");
        return false;
    }

    db.exit();

    return true;
}

int main()
{
    if (test_sqlite())
    {
        printf("sqlite test success\n");
    }
    else
    {
        printf("sqlite test failure\n");
    }
    return 0;
}
