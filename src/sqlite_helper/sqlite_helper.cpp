/********************************************************
 * Description : sqlite helper which base on sqlite3
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#include <utility>
#include "sqlite_helper.h"
#include "sqlite3.h"
#include "base.h"

SQLiteDB::SQLiteDB()
    : m_path()
    , m_sqlite(nullptr)
{

}

SQLiteDB::~SQLiteDB()
{
    exit();
}

bool SQLiteDB::init(const std::string & path)
{
    exit();

    if (path.empty())
    {
        RUN_LOG_ERR("sqlite db open failure while path is invalid");
        return false;
    }

    int result = sqlite3_open(path.c_str(), &m_sqlite);
    if (SQLITE_OK != result)
    {
        RUN_LOG_ERR("sqlite db open failure while open (%s) failed, error (%d: %s)", path.c_str(), result, sqlite3_errstr(result));
        return false;
    }

    m_path = path;

    return true;
}

void SQLiteDB::exit()
{
    if (nullptr != m_sqlite)
    {
        int result = sqlite3_close(m_sqlite);
        if (SQLITE_OK != result)
        {
            RUN_LOG_ERR("sqlite db close failure while close (%s) failed, error (%d: %s)", m_path.c_str(), result, sqlite3_errstr(result));
        }
        m_sqlite = nullptr;
        m_path.clear();
    }
}

bool SQLiteDB::is_open() const
{
    return nullptr != m_sqlite;
}

int SQLiteDB::error() const
{
    return nullptr != m_sqlite ? sqlite3_errcode(m_sqlite) : 999;
}

const char * SQLiteDB::what() const
{
    return nullptr != m_sqlite ? sqlite3_errmsg(m_sqlite) : "sqlite db is closed";
}

bool SQLiteDB::execute(const std::string & sql)
{
    if (!is_open() || sql.empty())
    {
        return false;
    }

    char * error_message = nullptr;
    int result = sqlite3_exec(m_sqlite, sql.c_str(), nullptr, nullptr, &error_message);
    if (SQLITE_OK != result)
    {
        RUN_LOG_ERR("sqlite db execute failure while exec (%s) failed, error (%d: %s) message (%s)", sql.c_str(), result, sqlite3_errstr(result), nullptr != error_message ? error_message : "unknown");
    }

    if (nullptr != error_message)
    {
        sqlite3_free(error_message);
        error_message = nullptr;
    }

    return SQLITE_OK == result;
}

bool SQLiteDB::begin_transaction()
{
    return execute("BEGIN TRANSACTION;");
}

bool SQLiteDB::end_transaction()
{
    return execute("END TRANSACTION;");
}

SQLiteReader SQLiteDB::create_reader(const std::string & sql)
{
    return SQLiteReader(m_sqlite, sql);
}

SQLiteWriter SQLiteDB::create_writer(const std::string & sql)
{
    return SQLiteWriter(m_sqlite, sql);
}

SQLiteStatement::SQLiteStatement()
    : m_sql()
    , m_sqlite(nullptr)
    , m_writer(false)
    , m_statement(nullptr)
    , m_set_index(0)
    , m_get_index(0)
{

}

SQLiteStatement::SQLiteStatement(sqlite3 * sqlite, const std::string & sql, bool writer)
    : m_sql()
    , m_sqlite(nullptr)
    , m_writer(false)
    , m_statement(nullptr)
    , m_set_index(0)
    , m_get_index(0)
{
    if (nullptr != sqlite && !sql.empty())
    {
        sqlite3_stmt * statement = nullptr;
        int result = sqlite3_prepare_v2(sqlite, sql.c_str(), static_cast<int>(sql.size()), &statement, nullptr);
        if (SQLITE_OK == result)
        {
            m_sql = sql;
            m_sqlite = sqlite;
            m_writer = writer;
            m_statement = statement;
        }
        else
        {
            RUN_LOG_ERR("sqlite %s (%s) create failure while prepare failed, error (%d: %s)", writer ? "writer" : "reader", sql.c_str(), result, sqlite3_errstr(result));
        }
    }
}

SQLiteStatement::SQLiteStatement(SQLiteStatement && other)
    : m_sql()
    , m_sqlite(nullptr)
    , m_writer(false)
    , m_statement(nullptr)
    , m_set_index(0)
    , m_get_index(0)
{
    std::swap(m_sql, other.m_sql);
    std::swap(m_sqlite, other.m_sqlite);
    std::swap(m_writer, other.m_writer);
    std::swap(m_statement, other.m_statement);
    std::swap(m_set_index, other.m_set_index);
    std::swap(m_get_index, other.m_get_index);
}

SQLiteStatement & SQLiteStatement::operator = (SQLiteStatement && other)
{
    if (&other != this)
    {
        clear();
        std::swap(m_sql, other.m_sql);
        std::swap(m_sqlite, other.m_sqlite);
        std::swap(m_writer, other.m_writer);
        std::swap(m_statement, other.m_statement);
        std::swap(m_set_index, other.m_set_index);
        std::swap(m_get_index, other.m_get_index);
    }
    return *this;
}

SQLiteStatement::~SQLiteStatement()
{
    clear();
}

bool SQLiteStatement::good() const
{
    return nullptr != m_statement;
}

void SQLiteStatement::clear()
{
    if (nullptr != m_statement)
    {
        int result = sqlite3_finalize(m_statement);
        if (SQLITE_OK != result)
        {
            RUN_LOG_ERR("sqlite %s (%s) clear failure while finalize failed, error (%d: %s)", m_writer ? "writer" : "reader", m_sql.c_str(), result, sqlite3_errstr(result));
        }
        m_sql.clear();
        m_sqlite = nullptr;
        m_writer = false;
        m_statement = nullptr;
        m_set_index = 0;
        m_get_index = 0;
    }
}

bool SQLiteStatement::reset()
{
    if (nullptr == m_statement)
    {
        return false;
    }

    m_set_index = 0;

    int result = sqlite3_reset(m_statement);
    if (SQLITE_OK != result)
    {
        RUN_LOG_ERR("sqlite %s (%s) reset failure while reset failed, error (%d: %s)", m_writer ? "writer" : "reader", m_sql.c_str(), result, sqlite3_errstr(result));
        return false;
    }

    return true;
}

bool SQLiteStatement::set(int index, int value)
{
    if (nullptr == m_statement)
    {
        return false;
    }

    int result = sqlite3_bind_int(m_statement, index + 1, value);
    if (SQLITE_OK != result)
    {
        RUN_LOG_ERR("sqlite %s (%s) set failure while bind failed, error (%d: %s)", m_writer ? "writer" : "reader", m_sql.c_str(), result, sqlite3_errstr(result));
        return false;
    }

    return true;
}

bool SQLiteStatement::set(int index, int64_t value)
{
    if (nullptr == m_statement)
    {
        return false;
    }

    int result = sqlite3_bind_int64(m_statement, index + 1, value);
    if (SQLITE_OK != result)
    {
        RUN_LOG_ERR("sqlite %s (%s) set failure while bind failed, error (%d: %s)", m_writer ? "writer" : "reader", m_sql.c_str(), result, sqlite3_errstr(result));
        return false;
    }

    return true;
}

bool SQLiteStatement::set(int index, double value)
{
    if (nullptr == m_statement)
    {
        return false;
    }

    int result = sqlite3_bind_double(m_statement, index + 1, value);
    if (SQLITE_OK != result)
    {
        RUN_LOG_ERR("sqlite %s (%s) set failure while bind failed, error (%d: %s)", m_writer ? "writer" : "reader", m_sql.c_str(), result, sqlite3_errstr(result));
        return false;
    }

    return true;
}

bool SQLiteStatement::set(int index, const std::string & value)
{
    if (nullptr == m_statement)
    {
        return false;
    }

    int result = sqlite3_bind_text(m_statement, index + 1, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
    if (SQLITE_OK != result)
    {
        RUN_LOG_ERR("sqlite %s (%s) set failure while bind failed, error (%d: %s)", m_writer ? "writer" : "reader", m_sql.c_str(), result, sqlite3_errstr(result));
        return false;
    }

    return true;
}

bool SQLiteStatement::set(bool value)
{
    return set(m_set_index++, static_cast<int>(value));
}
bool SQLiteStatement::set(int8_t value)
{
    return set(m_set_index++, static_cast<int>(value));
}

bool SQLiteStatement::set(uint8_t value)
{
    return set(m_set_index++, static_cast<int>(value));
}

bool SQLiteStatement::set(int16_t value)
{
    return set(m_set_index++, static_cast<int>(value));
}

bool SQLiteStatement::set(uint16_t value)
{
    return set(m_set_index++, static_cast<int>(value));
}

bool SQLiteStatement::set(int32_t value)
{
    return set(m_set_index++, static_cast<int>(value));
}

bool SQLiteStatement::set(uint32_t value)
{
    return set(m_set_index++, static_cast<int>(value));
}

bool SQLiteStatement::set(int64_t value)
{
    return set(m_set_index++, value);
}

bool SQLiteStatement::set(uint64_t value)
{
    return set(m_set_index++, static_cast<int64_t>(value));
}

bool SQLiteStatement::set(float value)
{
    return set(m_set_index++, static_cast<double>(value));
}

bool SQLiteStatement::set(double value)
{
    return set(m_set_index++, value);
}

bool SQLiteStatement::set(const std::string & value)
{
    return set(m_set_index++, value);
}

SQLiteReader::SQLiteReader()
    : SQLiteStatement()
{

}

SQLiteReader::SQLiteReader(sqlite3 * sqlite, const std::string & sql)
    : SQLiteStatement(sqlite, sql, false)
{

}

bool SQLiteReader::read()
{
    if (nullptr == m_statement)
    {
        return false;
    }

    m_get_index = 0;

    int result = sqlite3_step(m_statement);
    if (SQLITE_ROW != result && SQLITE_DONE != result)
    {
        RUN_LOG_ERR("sqlite reader (%s) read failure while step failed, error (%d: %s, %d: %s)", m_sql.c_str(), result, sqlite3_errstr(result), sqlite3_errcode(m_sqlite), sqlite3_errmsg(m_sqlite));
    }

    return SQLITE_ROW == result;
}

bool SQLiteReader::get(int index, int & value)
{
    if (nullptr == m_statement)
    {
        return false;
    }

    if (index >= sqlite3_column_count(m_statement))
    {
        RUN_LOG_ERR("sqlite reader (%s) get failure while get index is overflow (%d >= %d)", m_sql.c_str(), index, sqlite3_column_count(m_statement));
        return false;
    }

    value = sqlite3_column_int(m_statement, index);

    return true;
}

bool SQLiteReader::get(int index, int64_t & value)
{
    if (nullptr == m_statement)
    {
        return false;
    }

    if (index >= sqlite3_column_count(m_statement))
    {
        RUN_LOG_ERR("sqlite reader (%s) get failure while get index is overflow (%d >= %d)", m_sql.c_str(), index, sqlite3_column_count(m_statement));
        return false;
    }

    value = sqlite3_column_int64(m_statement, index);

    return true;
}

bool SQLiteReader::get(int index, double & value)
{
    if (nullptr == m_statement)
    {
        return false;
    }

    if (index >= sqlite3_column_count(m_statement))
    {
        RUN_LOG_ERR("sqlite reader (%s) get failure while get index is overflow (%d >= %d)", m_sql.c_str(), index, sqlite3_column_count(m_statement));
        return false;
    }

    value = sqlite3_column_double(m_statement, index);

    return true;
}

bool SQLiteReader::get(int index, std::string & value)
{
    if (nullptr == m_statement)
    {
        return false;
    }

    if (index >= sqlite3_column_count(m_statement))
    {
        RUN_LOG_ERR("sqlite reader (%s) get failure while get index is overflow (%d >= %d)", m_sql.c_str(), index, sqlite3_column_count(m_statement));
        return false;
    }

    const char * column_text = reinterpret_cast<const char *>(sqlite3_column_text(m_statement, index));
    int column_size = sqlite3_column_bytes(m_statement, index);
    value.assign(column_text, column_size);

    return true;
}

bool SQLiteReader::get(bool & value)
{
    int dummy = 0;
    bool result = get(m_get_index++, dummy);
    value = static_cast<bool>(dummy);
    return result;
}

bool SQLiteReader::get(int8_t & value)
{
    int dummy = 0;
    bool result = get(m_get_index++, dummy);
    value = static_cast<int8_t>(dummy);
    return result;
}

bool SQLiteReader::get(uint8_t & value)
{
    int dummy = 0;
    bool result = get(m_get_index++, dummy);
    value = static_cast<uint8_t>(dummy);
    return result;
}

bool SQLiteReader::get(int16_t & value)
{
    int dummy = 0;
    bool result = get(m_get_index++, dummy);
    value = static_cast<int16_t>(dummy);
    return result;
}

bool SQLiteReader::get(uint16_t & value)
{
    int dummy = 0;
    bool result = get(m_get_index++, dummy);
    value = static_cast<uint16_t>(dummy);
    return result;
}

bool SQLiteReader::get(int32_t & value)
{
    int dummy = 0;
    bool result = get(m_get_index++, dummy);
    value = static_cast<int32_t>(dummy);
    return result;
}

bool SQLiteReader::get(uint32_t & value)
{
    int dummy = 0;
    bool result = get(m_get_index++, dummy);
    value = static_cast<uint32_t>(dummy);
    return result;
}

bool SQLiteReader::get(int64_t & value)
{
    return get(m_get_index++, value);
}

bool SQLiteReader::get(uint64_t & value)
{
    int64_t dummy = 0;
    bool result = get(m_get_index++, dummy);
    value = static_cast<uint64_t>(dummy);
    return result;
}

bool SQLiteReader::get(float & value)
{
    double dummy = 0;
    bool result = get(m_get_index++, dummy);
    value = static_cast<float>(dummy);
    return result;
}

bool SQLiteReader::get(double & value)
{
    return get(m_get_index++, value);
}

bool SQLiteReader::get(std::string & value)
{
    return get(m_get_index++, value);
}

SQLiteWriter::SQLiteWriter()
    : SQLiteStatement()
{

}

SQLiteWriter::SQLiteWriter(sqlite3 * sqlite, const std::string & sql)
    : SQLiteStatement(sqlite, sql, true)
{

}

bool SQLiteWriter::write()
{
    if (nullptr == m_statement)
    {
        return false;
    }

    int result = sqlite3_step(m_statement);
    if (SQLITE_ERROR == result || SQLITE_MISUSE == result)
    {
        RUN_LOG_ERR("sqlite writer (%s) write failure while step failed, error (%d: %s, %d: %s)", m_sql.c_str(), result, sqlite3_errstr(result), sqlite3_errcode(m_sqlite), sqlite3_errmsg(m_sqlite));
        return false;
    }

    return true;
}
