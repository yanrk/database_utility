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

#define MYSQL_HELPER_TRY                                                \
    try                                                                 \
    {

#define MYSQL_HELPER_CATCH1(description)                                \
    }                                                                   \
    catch (const mysqlx::Error & err)                                   \
    {                                                                   \
        RUN_LOG_CRI("%s exception (%s)", description, err.what());      \
    }                                                                   \
    catch (std::exception & err)                                        \
    {                                                                   \
        RUN_LOG_CRI("%s exception (%s)", description, err.what());      \
    }                                                                   \
    catch (const char * err)                                            \
    {                                                                   \
        RUN_LOG_CRI("%s exception (%s)", description, err);             \
    }                                                                   \
    catch (...)                                                         \
    {                                                                   \
        RUN_LOG_CRI("%s exception (unknown)", description);             \
    }

#define MYSQL_HELPER_CATCH2(operate, tb)                                \
    }                                                                   \
    catch (const mysqlx::Error & err)                                   \
    {                                                                   \
        RUN_LOG_CRI("%s %s exception (%s)", operate, tb, err.what());   \
    }                                                                   \
    catch (std::exception & err)                                        \
    {                                                                   \
        RUN_LOG_CRI("%s %s exception (%s)", operate, tb, err.what());   \
    }                                                                   \
    catch (const char * err)                                            \
    {                                                                   \
        RUN_LOG_CRI("%s %s exception (%s)", operate, tb, err);          \
    }                                                                   \
    catch (...)                                                         \
    {                                                                   \
        RUN_LOG_CRI("%s %s exception (unknown)", operate, tb);          \
    }

class MysqlxSchema
{
public:
    MysqlxSchema();
    ~MysqlxSchema();

public:
    bool init(const std::string & uri); // mysqlx://user:pass@host1:port1,host2:port2/db?option1=value1&option2=value2
    void exit();

public:
    bool is_open() const;
    const std::string & get_name() const;
    bool execute_sql(const std::string & statement);

public:
    bool transaction_start();
    bool transaction_create_snapshot(std::string & save_point_name);
    bool transaction_remove_snapshot(const std::string & save_point_name);
    bool transaction_rollback(const std::string & save_point_name);
    bool transaction_rollback();
    bool transaction_commit();

public:
    mysqlx::Schema * operator -> () const;
    mysqlx::Schema * operator & () const;

private:
    MysqlxSchema(const MysqlxSchema &) = delete;
    MysqlxSchema(MysqlxSchema &&) = delete;
    MysqlxSchema & operator = (const MysqlxSchema &) = delete;
    MysqlxSchema & operator = (MysqlxSchema &&) = delete;

private:
    bool                                m_transaction;
    mysqlx::Session                   * m_session;
    mysqlx::Schema                    * m_schema;
    std::string                         m_uri;
};

class MysqlxTable
{
public:
    MysqlxTable();
    ~MysqlxTable();

public:
    bool init(const std::string & uri, const std::string & tb);
    void exit();

public:
    bool is_open() const;
    const std::string & get_name() const;
    bool execute_sql(const std::string & statement);

public:
    bool transaction_start();
    bool transaction_create_snapshot(std::string & save_point_name);
    bool transaction_remove_snapshot(const std::string & save_point_name);
    bool transaction_rollback(const std::string & save_point_name);
    bool transaction_rollback();
    bool transaction_commit();

public:
    mysqlx::Table * operator -> () const;

private:
    MysqlxTable(const MysqlxTable &) = delete;
    MysqlxTable(MysqlxTable &&) = delete;
    MysqlxTable & operator = (const MysqlxTable &) = delete;
    MysqlxTable & operator = (MysqlxTable &&) = delete;

private:
    MysqlxSchema                        m_schema;
    mysqlx::Table                     * m_table;
    std::string                         m_tb;
};

inline MysqlxSchema::MysqlxSchema()
    : m_transaction(false)
    , m_session(nullptr)
    , m_schema(nullptr)
    , m_uri()
{

}

inline MysqlxSchema::~MysqlxSchema()
{
    exit();
}

inline bool MysqlxSchema::init(const std::string & uri)
{
    exit();

    if (uri.empty())
    {
        RUN_LOG_ERR("mysql schema init failure while invalid uri");
        return false;
    }

    try
    {
        m_session = new mysqlx::Session(m_uri.c_str());
        if (nullptr == m_session)
        {
            RUN_LOG_ERR("mysql schema (%s) init failure while create session", m_uri.c_str());
            return false;
        }

        m_schema = new mysqlx::Schema(m_session->getDefaultSchema());
        if (nullptr == m_schema)
        {
            RUN_LOG_ERR("mysql schema (%s) init failure while create schema", m_uri.c_str());
            return false;
        }

        m_transaction = false;
        m_uri = uri;

        return true;
    }
    catch (const mysqlx::Error & err)
    {
        RUN_LOG_CRI("mysql schema (%s) init exception (%s)", uri.c_str(), err.what());
    }
    catch (std::exception & err)
    {
        RUN_LOG_CRI("mysql schema (%s) init exception (%s)", uri.c_str(), err.what());
    }
    catch (const char * err)
    {
        RUN_LOG_CRI("mysql schema (%s) init exception (%s)", uri.c_str(), err);
    }
    catch (...)
    {
        RUN_LOG_CRI("mysql schema (%s) init exception", uri.c_str());
    }

    return false;
}

inline void MysqlxSchema::exit()
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

        m_uri.clear();
    }
    catch (const mysqlx::Error & err)
    {
        RUN_LOG_CRI("mysql schema (%s) exit exception (%s)", m_uri.c_str(), err.what());
    }
    catch (std::exception & err)
    {
        RUN_LOG_CRI("mysql schema (%s) exit exception (%s)", m_uri.c_str(), err.what());
    }
    catch (const char * err)
    {
        RUN_LOG_CRI("mysql schema (%s) exit exception (%s)", m_uri.c_str(), err);
    }
    catch (...)
    {
        RUN_LOG_CRI("mysql schema (%s) exit exception", m_uri.c_str());
    }
}

inline bool MysqlxSchema::is_open() const
{
    return !m_uri.empty();
}

inline const std::string & MysqlxSchema::get_name() const
{
    return m_uri;
}

inline bool MysqlxSchema::execute_sql(const std::string & statement)
{
    if (!is_open() || statement.empty())
    {
        return false;
    }

    m_session->sql(statement).execute();
    return true;
}

inline bool MysqlxSchema::transaction_start()
{
    if (!is_open() || m_transaction)
    {
        return false;
    }

    m_session->startTransaction();
    m_transaction = true;
    return true;
}

inline bool MysqlxSchema::transaction_create_snapshot(std::string & save_point_name)
{
    if (!is_open() || !m_transaction)
    {
        return false;
    }

    if (save_point_name.empty())
    {
        save_point_name = m_session->setSavepoint();
    }
    else
    {
        m_session->setSavepoint(save_point_name);
    }

    return true;
}

inline bool MysqlxSchema::transaction_remove_snapshot(const std::string & save_point_name)
{
    if (!is_open() || !m_transaction || save_point_name.empty())
    {
        return false;
    }

    m_session->releaseSavepoint(save_point_name);
    return true;
}

inline bool MysqlxSchema::transaction_rollback(const std::string & save_point_name)
{
    if (!is_open() || !m_transaction || save_point_name.empty())
    {
        return false;
    }

    m_session->rollbackTo(save_point_name);
    return true;
}

inline bool MysqlxSchema::transaction_rollback()
{
    if (!is_open() || !m_transaction)
    {
        return false;
    }

    m_transaction = false;
    m_session->rollback();
    return true;
}

inline bool MysqlxSchema::transaction_commit()
{
    if (!is_open() || !m_transaction)
    {
        return false;
    }

    m_transaction = false;
    m_session->commit();
    return true;
}

inline mysqlx::Schema * MysqlxSchema::operator -> () const
{
    return m_schema;
}

inline mysqlx::Schema * MysqlxSchema::operator & () const
{
    return m_schema;
}

inline MysqlxTable::MysqlxTable()
    : m_schema()
    , m_table(nullptr)
    , m_tb()
{

}

inline MysqlxTable::~MysqlxTable()
{
    exit();
}

inline bool MysqlxTable::init(const std::string & uri, const std::string & tb)
{
    exit();

    if (uri.empty())
    {
        RUN_LOG_ERR("mysql table init failure while invalid uri");
        return false;
    }

    if (tb.empty())
    {
        RUN_LOG_ERR("mysql table init failure while invalid tb");
        return false;
    }

    const std::string name(uri + (std::string::npos == uri.find('?') ? "?table=" : "&table=") + tb);

    if (!m_schema.init(uri))
    {
        RUN_LOG_ERR("mysql table (%s) init failure while init schema", name.c_str());
        return false;
    }

    try
    {
        m_table = new mysqlx::Table(*&m_schema, tb.c_str(), true);
        if (nullptr == m_table)
        {
            RUN_LOG_ERR("mysql table (%s) init failure while create table", name.c_str());
            return false;
        }

        m_tb = name;

        return true;
    }
    catch (const mysqlx::Error & err)
    {
        RUN_LOG_CRI("mysql table (%s) init exception (%s)", name.c_str(), err.what());
    }
    catch (std::exception & err)
    {
        RUN_LOG_CRI("mysql table (%s) init exception (%s)", name.c_str(), err.what());
    }
    catch (const char * err)
    {
        RUN_LOG_CRI("mysql table (%s) init exception (%s)", name.c_str(), err);
    }
    catch (...)
    {
        RUN_LOG_CRI("mysql table (%s) init exception", name.c_str());
    }

    return false;
}

inline void MysqlxTable::exit()
{
    try
    {
        if (nullptr != m_table)
        {
            delete m_table;
            m_table = nullptr;
        }

        m_tb.clear();
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

inline bool MysqlxTable::is_open() const
{
    return !m_tb.empty();
}

inline const std::string & MysqlxTable::get_name() const
{
    return m_tb;
}

inline bool MysqlxTable::execute_sql(const std::string & statement)
{
    return m_schema.execute_sql(statement);
}

inline bool MysqlxTable::transaction_start()
{
    return m_schema.transaction_start();
}

inline bool MysqlxTable::transaction_create_snapshot(std::string & save_point_name)
{
    return m_schema.transaction_create_snapshot(save_point_name);
}

inline bool MysqlxTable::transaction_remove_snapshot(const std::string & save_point_name)
{
    return m_schema.transaction_remove_snapshot(save_point_name);
}

inline bool MysqlxTable::transaction_rollback(const std::string & save_point_name)
{
    return m_schema.transaction_rollback(save_point_name);
}

inline bool MysqlxTable::transaction_rollback()
{
    return m_schema.transaction_rollback();
}

inline bool MysqlxTable::transaction_commit()
{
    return m_schema.transaction_commit();
}

inline mysqlx::Table * MysqlxTable::operator -> () const
{
    return m_table;
}


#endif // MYSQL_HELPER_HPP
