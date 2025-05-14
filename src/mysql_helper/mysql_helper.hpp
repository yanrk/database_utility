/********************************************************
 * Description : mysql helper
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#ifndef MYSQL_HELPER_HPP
#define MYSQL_HELPER_HPP


#include <string>
#include "mysqlx/xdevapi.h"
#include "base.h"

class MysqlSchema
{
public:
    MysqlSchema();
    ~MysqlSchema();

public:
    bool init(const std::string & db);
    void exit();

public:
    const std::string & get_name() const;

public:
    mysqlx::Schema * operator -> () const;
    mysqlx::Schema * operator & () const;

private:
    MysqlSchema(const MysqlSchema &) = delete;
    MysqlSchema(MysqlSchema &&) = delete;
    MysqlSchema & operator = (const MysqlSchema &) = delete;
    MysqlSchema & operator = (MysqlSchema &&) = delete;

private:
    mysqlx::Session                   * m_session;
    mysqlx::Schema                    * m_schema;
    std::string                         m_db;
};

class MysqlTable
{
public:
    MysqlTable();
    ~MysqlTable();

public:
    bool init(const std::string & db, const std::string & tb);
    void exit();

public:
    const std::string & get_name() const;

public:
    mysqlx::Table * operator -> () const;

private:
    MysqlTable(const MysqlTable &) = delete;
    MysqlTable(MysqlTable &&) = delete;
    MysqlTable & operator = (const MysqlTable &) = delete;
    MysqlTable & operator = (MysqlTable &&) = delete;

private:
    MysqlSchema                         m_schema;
    mysqlx::Table                     * m_table;
    std::string                         m_tb;
};

inline MysqlSchema::MysqlSchema()
    : m_session(nullptr)
    , m_schema(nullptr)
    , m_db()
{

}

inline MysqlSchema::~MysqlSchema()
{
    exit();
}

inline bool MysqlSchema::init(const std::string & db)
{
    if (db.empty())
    {
        RUN_LOG_ERR("mysql schema init failure while invalid db name");
        return false;
    }

    try
    {
        m_db = db;

        m_session = new mysqlx::Session(m_db.c_str());
        if (nullptr == m_session)
        {
            RUN_LOG_ERR("mysql schema (%s) init failure while create session", m_db.c_str());
            return false;
        }

        m_schema = new mysqlx::Schema(m_session->getDefaultSchema());
        if (nullptr == m_schema)
        {
            RUN_LOG_ERR("mysql schema (%s) init failure while create schema", m_db.c_str());
            return false;
        }

        return true;
    }
    catch (const mysqlx::Error & err)
    {
        RUN_LOG_CRI("mysql schema (%s) init exception (%s)", m_db.c_str(), err.what());
    }
    catch (std::exception & err)
    {
        RUN_LOG_CRI("mysql schema (%s) init exception (%s)", m_db.c_str(), err.what());
    }
    catch (const char * err)
    {
        RUN_LOG_CRI("mysql schema (%s) init exception (%s)", m_db.c_str(), err);
    }
    catch (...)
    {
        RUN_LOG_CRI("mysql schema (%s) init exception", m_db.c_str());
    }

    return false;
}

inline void MysqlSchema::exit()
{
    try
    {
        if (nullptr != m_schema)
        {
            delete m_schema;
            m_schema = nullptr;
        }

        if (nullptr != m_session)
        {
            delete m_session;
            m_session = nullptr;
        }
    }
    catch (const mysqlx::Error & err)
    {
        RUN_LOG_CRI("mysql schema (%s) exit exception (%s)", m_db.c_str(), err.what());
    }
    catch (std::exception & err)
    {
        RUN_LOG_CRI("mysql schema (%s) exit exception (%s)", m_db.c_str(), err.what());
    }
    catch (const char * err)
    {
        RUN_LOG_CRI("mysql schema (%s) exit exception (%s)", m_db.c_str(), err);
    }
    catch (...)
    {
        RUN_LOG_CRI("mysql schema (%s) exit exception", m_db.c_str());
    }
}

inline const std::string & MysqlSchema::get_name() const
{
    return m_db;
}

inline mysqlx::Schema * MysqlSchema::operator -> () const
{
    return m_schema;
}

inline mysqlx::Schema * MysqlSchema::operator & () const
{
    return m_schema;
}

inline MysqlTable::MysqlTable()
    : m_schema()
    , m_table(nullptr)
    , m_tb()
{

}

inline MysqlTable::~MysqlTable()
{
    exit();
}

inline bool MysqlTable::init(const std::string & db, const std::string & tb)
{
    if (db.empty())
    {
        RUN_LOG_ERR("mysql table init failure while invalid db name");
        return false;
    }

    if (tb.empty())
    {
        RUN_LOG_ERR("mysql table init failure while invalid tb name");
        return false;
    }

    m_tb = db + "?table=" + tb;

    if (!m_schema.init(db))
    {
        RUN_LOG_ERR("mysql table (%s) init failure while init schema", m_tb.c_str());
        return false;
    }

    try
    {
        m_table = new mysqlx::Table(*&m_schema, tb.c_str(), true);
        if (nullptr == m_table)
        {
            RUN_LOG_ERR("mysql table (%s) init failure while create table", m_tb.c_str());
            return false;
        }

        return true;
    }
    catch (const mysqlx::Error & err)
    {
        RUN_LOG_CRI("mysql table (%s) init exception (%s)", m_tb.c_str(), err.what());
    }
    catch (std::exception & err)
    {
        RUN_LOG_CRI("mysql table (%s) init exception (%s)", m_tb.c_str(), err.what());
    }
    catch (const char * err)
    {
        RUN_LOG_CRI("mysql table (%s) init exception (%s)", m_tb.c_str(), err);
    }
    catch (...)
    {
        RUN_LOG_CRI("mysql table (%s) init exception", m_tb.c_str());
    }

    return false;
}

inline void MysqlTable::exit()
{
    try
    {
        if (nullptr != m_table)
        {
            delete m_table;
            m_table = nullptr;
        }
    }
    catch (const mysqlx::Error & err)
    {
        RUN_LOG_CRI("mysql table (%s) exit exception (%s)", m_tb.c_str(), err.what());
    }
    catch (std::exception & err)
    {
        RUN_LOG_CRI("mysql table (%s) exit exception (%s)", m_tb.c_str(), err.what());
    }
    catch (const char * err)
    {
        RUN_LOG_CRI("mysql table (%s) exit exception (%s)", m_tb.c_str(), err);
    }
    catch (...)
    {
        RUN_LOG_CRI("mysql table (%s) exit exception", m_tb.c_str());
    }
    m_schema.exit();
}

inline const std::string & MysqlTable::get_name() const
{
    return m_tb;
}

inline mysqlx::Table * MysqlTable::operator -> () const
{
    return m_table;
}


#endif // MYSQL_HELPER_HPP
