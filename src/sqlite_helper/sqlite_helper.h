/********************************************************
 * Description : sqlite helper which base on sqlite3
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#ifndef SQLITE_HELPER_H
#define SQLITE_HELPER_H


#include <cstdint>
#include <string>
#include "macros.h"

struct sqlite3;
struct sqlite3_stmt;

class SQLiteReader;
class SQLiteWriter;

class GOOFER_API SQLiteDB
{
public:
    SQLiteDB();
    SQLiteDB(const SQLiteDB & other) = delete;
    SQLiteDB(SQLiteDB && other) = delete;
    SQLiteDB & operator = (const SQLiteDB & other) = delete;
    SQLiteDB & operator = (SQLiteDB && other) = delete;
    ~SQLiteDB();

public:
    bool init(const std::string & path);
    void exit();

public:
    bool is_open() const;
    int error() const;
    const char * what() const;

public:
    bool execute(const std::string & sql);
    bool begin_transaction();
    bool end_transaction();

public:
    SQLiteReader create_reader(const std::string & sql);
    SQLiteWriter create_writer(const std::string & sql);

private:
    std::string                             m_path;
    sqlite3                               * m_sqlite;
};

class GOOFER_API SQLiteStatement
{
public:
    SQLiteStatement();
    SQLiteStatement(sqlite3 * sqlite, const std::string & sql, bool writer);
    SQLiteStatement(const SQLiteStatement & other) = delete;
    SQLiteStatement(SQLiteStatement && other);
    SQLiteStatement & operator = (const SQLiteStatement & other) = delete;
    SQLiteStatement & operator = (SQLiteStatement && other);
    ~SQLiteStatement();

public:
    bool good() const;
    void clear();
    bool reset();

public:
    bool set(bool value);
    bool set(int8_t value);
    bool set(uint8_t value);
    bool set(int16_t value);
    bool set(uint16_t value);
    bool set(int32_t value);
    bool set(uint32_t value);
    bool set(int64_t value);
    bool set(uint64_t value);
    bool set(float value);
    bool set(double value);
    bool set(const std::string & value);

protected:
    bool set(int index, int value);
    bool set(int index, int64_t value);
    bool set(int index, double value);
    bool set(int index, const std::string & value);

protected:
    std::string                             m_sql;
    sqlite3                               * m_sqlite;
    bool                                    m_writer;
    sqlite3_stmt                          * m_statement;
    int                                     m_set_index;
    int                                     m_get_index;
};

class GOOFER_API SQLiteReader : public SQLiteStatement
{
public:
    SQLiteReader();
    SQLiteReader(sqlite3 * sqlite, const std::string & sql);

public:
    bool read();

public:
    bool get(bool & value);
    bool get(int8_t & value);
    bool get(uint8_t & value);
    bool get(int16_t & value);
    bool get(uint16_t & value);
    bool get(int32_t & value);
    bool get(uint32_t & value);
    bool get(int64_t & value);
    bool get(uint64_t & value);
    bool get(float & value);
    bool get(double & value);
    bool get(std::string & value);

protected:
    bool get(int index, int & value);
    bool get(int index, int64_t & value);
    bool get(int index, double & value);
    bool get(int index, std::string & value);
};

class GOOFER_API SQLiteWriter : public SQLiteStatement
{
public:
    SQLiteWriter();
    SQLiteWriter(sqlite3 * sqlite, const std::string & sql);

public:
    bool write();
};


#endif // SQLITE_HELPER_H
