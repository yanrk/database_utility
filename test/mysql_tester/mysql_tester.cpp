/********************************************************
 * Description : sample of string codec functions
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 1.0
 * Copyright(C): 2024
 ********************************************************/

#include <list>
#include <string>
#include <cstdio>
#include <cstdint>
#include "macros.h"
#include "mysql_helper.hpp"

struct type_check_t
{
    uint64_t    id;
    bool        b;
    int8_t      i8;
    uint8_t     u8;
    int16_t     i16;
    uint16_t    u16;
    int32_t     i32;
    uint32_t    u32;
    int64_t     i64;
    uint64_t    u64;
    float       f;
    double      d;
    std::string s;
};

static bool operator != (const type_check_t & lhs, const type_check_t & rhs)
{
    if (lhs.id != rhs.id || lhs.b != rhs.b || lhs.i8 != rhs.i8 || lhs.u8 != rhs.u8 ||
        lhs.i16 != rhs.i16 || lhs.u16 != rhs.u16 || lhs.i32 != rhs.i32 || lhs.u32 != rhs.u32 ||
        lhs.i64 != rhs.i64 || lhs.u64 != rhs.u64 || lhs.s != rhs.s)
    {
        return true;
    }

    if (lhs.f + 1e-6 < rhs.f || lhs.f > rhs.f + 1e-6 ||
        lhs.d + 1e-9 < rhs.d || lhs.d > rhs.d + 1e-9)
    {
        return true;
    }

    return false;
}

struct conf_abc_t
{
    uint64_t    id;
    uint64_t    a;
    bool        b;
    char        c;
};

struct data_xyz_t
{
    uint64_t    id;
    std::string x;
    uint32_t    y;
    double      z;
};

class MyqlTester1
{
public:
    MyqlTester1();
    ~MyqlTester1();

public:
    bool init(const std::string & db);
    void exit();

public:
    bool test_type_check();

private:
    bool insert_type_check(type_check_t & type_check);
    bool select_type_check(type_check_t & type_check);
    bool remove_type_check(const type_check_t & type_check);

private:
    MyqlTester1(const MyqlTester1 &) = delete;
    MyqlTester1(MyqlTester1 &&) = delete;
    MyqlTester1 & operator = (const MyqlTester1 &) = delete;
    MyqlTester1 & operator = (MyqlTester1 &&) = delete;

private:
    MysqlxTable                                 m_table;
};

MyqlTester1::MyqlTester1()
    : m_table()
{

}

MyqlTester1::~MyqlTester1()
{

}

bool MyqlTester1::init(const std::string & db)
{
    return m_table.init(db, "t_type_check");
}

void MyqlTester1::exit()
{
    m_table.exit();
}

bool MyqlTester1::test_type_check()
{
    if (!m_table.is_open())
    {
        return false;
    }

    type_check_t src_type_check;
    src_type_check.id = 0;
    src_type_check.b = true;
    src_type_check.i8 = -1;
    src_type_check.u8 = 1;
    src_type_check.i16 = -2;
    src_type_check.u16 = 2;
    src_type_check.i32 = -3;
    src_type_check.u32 = 3;
    src_type_check.i64 = -4;
    src_type_check.u64 = 4;
    src_type_check.f = 1 / 7.0f;
    src_type_check.d = 1 / 7.0;
    src_type_check.s = "test";
    insert_type_check(src_type_check);

    type_check_t dst_type_check;
    select_type_check(dst_type_check);

    if (src_type_check != dst_type_check)
    {
        printf("src_type_check != dst_type_check\n");
        return false;
    }

    remove_type_check(dst_type_check);

    return true;
}

bool MyqlTester1::insert_type_check(type_check_t & type_check)
{
    MYSQL_HELPER_TRY
    {
        type_check.id = 0;

        mysqlx::Result result =
            m_table->insert()
                .values(
                    type_check.id,
                    type_check.b,
                    type_check.i8,
                    type_check.u8,
                    type_check.i16,
                    type_check.u16,
                    type_check.i32,
                    type_check.u32,
                    type_check.i64,
                    type_check.u64,
                    type_check.f,
                    type_check.d,
                    type_check.s
                )
            .execute();

        type_check.id = result.getAutoIncrementValue();

        return true;
    }
    MYSQL_HELPER_CATCH2("insert", "tb_type_check")

    return false;
}

bool MyqlTester1::select_type_check(type_check_t & type_check)
{
    MYSQL_HELPER_TRY
    {
        mysqlx::RowResult type_row_list = m_table->select().limit(1).execute();
        mysqlx::Row type_row = type_row_list.fetchOne();
        if (!type_row)
        {
            return false;
        }

        uint32_t index = 0;

        type_check.id = type_row.get(index++);
        type_check.b = static_cast<bool>(type_row.get(index++));
        type_check.i8 = static_cast<int8_t>(static_cast<int32_t>(type_row.get(index++)));
        type_check.u8 = static_cast<uint8_t>(static_cast<uint32_t>(type_row.get(index++)));
        type_check.i16 = static_cast<int16_t>(static_cast<int32_t>(type_row.get(index++)));
        type_check.u16 = static_cast<uint16_t>(static_cast<uint32_t>(type_row.get(index++)));
        type_check.i32 = type_row.get(index++);
        type_check.u32 = type_row.get(index++);
        type_check.i64 = type_row.get(index++);
        type_check.u64 = type_row.get(index++);
        type_check.f = type_row.get(index++);
        type_check.d = type_row.get(index++);
        type_check.s = static_cast<std::string>(type_row.get(index++));

        return true;
    }
    MYSQL_HELPER_CATCH2("select", "tb_type_check")

    return false;
}

bool MyqlTester1::remove_type_check(const type_check_t & type_check)
{
    MYSQL_HELPER_TRY
    {
        char query[64] = { 0x0 };
        snprintf(query, sizeof(query) - 1, "id = " GOOFER_U64_FMT, type_check.id);

        mysqlx::Result result =
            m_table->remove()
                .where(query)
            .execute();

        return true;
    }
    MYSQL_HELPER_CATCH2("remove", "tb_type_check")

    return false;
}

class MyqlTester2
{
public:
    MyqlTester2();
    ~MyqlTester2();

public:
    bool init(const std::string & db);
    void exit();

public:
    bool test_modify_conf_abc();
    bool test_remove_conf_abc();
    bool test_modify_data_xyz();
    bool test_remove_data_xyz();

private:
    static bool select_conf_abc(mysqlx::Table & conf_abc_table, conf_abc_t & conf_abc);
    static bool insert_conf_abc(mysqlx::Table & conf_abc_table, conf_abc_t & conf_abc);
    static bool update_conf_abc(mysqlx::Table & conf_abc_table, const conf_abc_t & conf_abc);
    static bool remove_conf_abc(mysqlx::Table & conf_abc_table, const conf_abc_t & conf_abc);

private:
    static bool select_data_xyz(mysqlx::Table & data_xyz_table, std::list<data_xyz_t> & data_xyz_list);
    static bool insert_data_xyz(mysqlx::Table & data_xyz_table, data_xyz_t & data_xyz);
    static bool update_data_xyz(mysqlx::Table & data_xyz_table, const data_xyz_t & data_xyz);
    static bool remove_data_xyz(mysqlx::Table & data_xyz_table);

private:
    MyqlTester2(const MyqlTester2 &) = delete;
    MyqlTester2(MyqlTester2 &&) = delete;
    MyqlTester2 & operator = (const MyqlTester2 &) = delete;
    MyqlTester2 & operator = (MyqlTester2 &&) = delete;

private:
    conf_abc_t                                  m_conf_abc;
    std::list<data_xyz_t>                       m_data_xyz_list;
    MysqlxSchema                                m_schema;
};

MyqlTester2::MyqlTester2()
    : m_conf_abc()
    , m_data_xyz_list()
    , m_schema()
{
    memset(&m_conf_abc, 0x0, sizeof(m_conf_abc));
}

MyqlTester2::~MyqlTester2()
{

}

bool MyqlTester2::init(const std::string & db)
{
    return m_schema.init(db);
}

void MyqlTester2::exit()
{
    m_schema.exit();
    memset(&m_conf_abc, 0x0, sizeof(m_conf_abc));
    m_data_xyz_list.clear();
}

bool MyqlTester2::test_modify_conf_abc()
{
    if (!m_schema.is_open())
    {
        return false;
    }

    MYSQL_HELPER_TRY
    {
        mysqlx::Table conf_abc_table = m_schema->getTable("t_conf_abc");

        if (!select_conf_abc(conf_abc_table, m_conf_abc))
        {
            m_conf_abc.id = 0;
            m_conf_abc.a = 1111;
            m_conf_abc.b = true;
            m_conf_abc.c = 0x3d;
            insert_conf_abc(conf_abc_table, m_conf_abc);
        }
        else
        {
            m_conf_abc.a = 2222;
            m_conf_abc.b = false;
            m_conf_abc.c = 0x7f;
            update_conf_abc(conf_abc_table, m_conf_abc);
        }

        return true;
    }
    MYSQL_HELPER_CATCH2("get table", "t_conf_abc")

    return false;
}

bool MyqlTester2::test_remove_conf_abc()
{
    if (!m_schema.is_open())
    {
        return false;
    }

    MYSQL_HELPER_TRY
    {
        mysqlx::Table conf_abc_table = m_schema->getTable("t_conf_abc");
        remove_conf_abc(conf_abc_table, m_conf_abc);
        return true;
    }
    MYSQL_HELPER_CATCH2("get table", "t_conf_abc")

    return false;
}

bool MyqlTester2::test_modify_data_xyz()
{
    if (!m_schema.is_open())
    {
        return false;
    }

    MYSQL_HELPER_TRY
    {
        mysqlx::Table data_xyz_table = m_schema->getTable("t_data_xyz");

        if (!select_data_xyz(data_xyz_table, m_data_xyz_list))
        {
            for (int i = 0; i < 5; ++i)
            {
                data_xyz_t data_xyz;
                data_xyz.id = 0;
                data_xyz.x = std::string(i + 1, 'x');
                data_xyz.y = i;
                data_xyz.z = i / 7.0;
                if (!insert_data_xyz(data_xyz_table, data_xyz))
                {
                    break;
                }
            }
        }
        else
        {
            for (std::list<data_xyz_t>::iterator iter = m_data_xyz_list.begin(); m_data_xyz_list.end() != iter; ++iter)
            {
                data_xyz_t & data_xyz = *iter;
                data_xyz.x += data_xyz.x;
                data_xyz.y += data_xyz.y;
                data_xyz.z += data_xyz.z;
                if (!update_data_xyz(data_xyz_table, data_xyz))
                {
                    break;
                }
            }
        }

        return true;
    }
    MYSQL_HELPER_CATCH2("get table", "t_data_xyz")

    return false;
}

bool MyqlTester2::test_remove_data_xyz()
{
    if (!m_schema.is_open())
    {
        return false;
    }

    MYSQL_HELPER_TRY
    {
        mysqlx::Table data_xyz_table = m_schema->getTable("t_data_xyz");
        remove_data_xyz(data_xyz_table);
        return true;
    }
    MYSQL_HELPER_CATCH2("get table", "t_data_xyz")

    return false;
}

bool MyqlTester2::select_conf_abc(mysqlx::Table & conf_abc_table, conf_abc_t & conf_abc)
{
    MYSQL_HELPER_TRY
    {
        mysqlx::RowResult abc_row_list = conf_abc_table.select().orderBy("id desc").limit(1).execute();
        mysqlx::Row abc_row = abc_row_list.fetchOne();
        if (!abc_row)
        {
            return false;
        }

        uint32_t index = 0;

        conf_abc.id = abc_row.get(index++);
        conf_abc.a = abc_row.get(index++);
        conf_abc.b = static_cast<bool>(abc_row.get(index++));
        conf_abc.c = static_cast<char>(static_cast<uint32_t>(abc_row.get(index++)));

        return true;
    }
    MYSQL_HELPER_CATCH2("select", "t_conf_abc")

    return false;
}

bool MyqlTester2::insert_conf_abc(mysqlx::Table & conf_abc_table, conf_abc_t & conf_abc)
{
    MYSQL_HELPER_TRY
    {
        conf_abc.id = 0;

        mysqlx::Result result =
            conf_abc_table.insert()
                .values(
                    conf_abc.id,
                    conf_abc.a,
                    conf_abc.b,
                    conf_abc.c
                )
            .execute();

        conf_abc.id = result.getAutoIncrementValue();

        return true;
    }
    MYSQL_HELPER_CATCH2("insert", "t_conf_abc")

    return false;
}

bool MyqlTester2::update_conf_abc(mysqlx::Table & conf_abc_table, const conf_abc_t & conf_abc)
{
    MYSQL_HELPER_TRY
    {
        char query[64] = { 0x0 };
        snprintf(query, sizeof(query) - 1, "id = " GOOFER_U64_FMT, conf_abc.id);

        mysqlx::Result result =
            conf_abc_table.update()
                .set("a", conf_abc.a)
                .set("b", conf_abc.b)
                .set("c", conf_abc.c)
                .where(query)
            .execute();

        return true;
    }
    MYSQL_HELPER_CATCH2("update", "t_conf_abc")

    return false;
}

bool MyqlTester2::remove_conf_abc(mysqlx::Table & conf_abc_table, const conf_abc_t & conf_abc)
{
    MYSQL_HELPER_TRY
    {
        char query[64] = { 0x0 };
        snprintf(query, sizeof(query) - 1, "id = " GOOFER_U64_FMT, conf_abc.id);

        mysqlx::Result result =
            conf_abc_table.remove()
                .where(query)
            .execute();

        return true;
    }
    MYSQL_HELPER_CATCH2("remove", "t_conf_abc")

    return false;
}

bool MyqlTester2::select_data_xyz(mysqlx::Table & data_xyz_table, std::list<data_xyz_t> & data_xyz_list)
{
    MYSQL_HELPER_TRY
    {
        mysqlx::RowResult xyz_row_list = data_xyz_table.select().execute();

        while (true)
        {
            mysqlx::Row xyz_row = xyz_row_list.fetchOne();
            if (!xyz_row)
            {
                return !data_xyz_list.empty();
            }

            uint32_t index = 0;
            data_xyz_t data_xyz;

            index = 0;
            data_xyz.id = xyz_row.get(index++);
            data_xyz.x = static_cast<std::string>(xyz_row.get(index++));
            data_xyz.y = xyz_row.get(index++);
            data_xyz.z = xyz_row.get(index++);
            data_xyz_list.push_back(data_xyz);
        }

        return true;
    }
    MYSQL_HELPER_CATCH2("select", "t_data_xyz")

    return false;
}

bool MyqlTester2::insert_data_xyz(mysqlx::Table & data_xyz_table, data_xyz_t & data_xyz)
{
    MYSQL_HELPER_TRY
    {
        data_xyz.id = 0;

        mysqlx::Result result =
            data_xyz_table.insert()
                .values(
                    data_xyz.id,
                    data_xyz.x,
                    data_xyz.y,
                    data_xyz.z
                )
            .execute();

        data_xyz.id = result.getAutoIncrementValue();

        return true;
    }
    MYSQL_HELPER_CATCH2("insert", "t_data_xyz")

    return false;
}

bool MyqlTester2::update_data_xyz(mysqlx::Table & data_xyz_table, const data_xyz_t & data_xyz)
{
    MYSQL_HELPER_TRY
    {
        char query[64] = { 0x0 };
        snprintf(query, sizeof(query) - 1, "id = " GOOFER_U64_FMT, data_xyz.id);

        mysqlx::Result result =
            data_xyz_table.update()
                .set("x", data_xyz.x)
                .set("y", data_xyz.y)
                .set("z", data_xyz.z)
                .where(query)
            .execute();

        return true;
    }
    MYSQL_HELPER_CATCH2("update", "t_data_xyz")

    return false;
}

bool MyqlTester2::remove_data_xyz(mysqlx::Table & data_xyz_table)
{
    MYSQL_HELPER_TRY
    {
        const char * query = "1 = 1";

        mysqlx::Result result =
            data_xyz_table.remove()
                .where(query)
            .execute();

        return true;
    }
    MYSQL_HELPER_CATCH2("remove", "t_data_xyz")

    return false;
}

static int test_1()
{
    MyqlTester1 tester1;
    if (!tester1.init("mysqlx://yanrk:123456@localhost:33060/db_test?ssl-mode=disabled")) // ubuntu need "ssl-mode=required"
    {
        printf("tester1 init failure\n");
        return 1;
    }

    tester1.test_type_check();

    tester1.exit();
    return 0;
}

static int test_2()
{
    MyqlTester2 tester2;
    if (!tester2.init("mysqlx://yanrk:123456@localhost:33060/db_test?ssl-mode=required")) // ubuntu need "ssl-mode=required"
    {
        printf("tester2 init failure\n");
        return 1;
    }

    if (!tester2.test_modify_conf_abc())
    {
        printf("tester2 test modify conf_abc failure 1\n");
        return 2;
    }

    if (!tester2.test_modify_conf_abc())
    {
        printf("tester2 test modify conf_abc failure 2\n");
        return 3;
    }

    if (!tester2.test_modify_data_xyz())
    {
        printf("tester2 test modify data_xyz failure 1\n");
        return 4;
    }

    if (!tester2.test_modify_data_xyz())
    {
        printf("tester2 test modify data_xyz failure 2\n");
        return 4;
    }

    if (!tester2.test_remove_conf_abc())
    {
        printf("tester2 test remove conf_abc failure\n");
        return 4;
    }

    if (!tester2.test_remove_data_xyz())
    {
        printf("tester2 test remove data_xyz failure\n");
        return 4;
    }

    tester2.exit();
    return 0;
}

int main(int argc, char * argv[])
{
    int ret1 = test_1();
    int ret2 = test_2();
    return (ret1 << 8) + ret2;
}
