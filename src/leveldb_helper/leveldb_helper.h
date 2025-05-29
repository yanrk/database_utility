/********************************************************
 * Description : leveldb helper which base on leveldb
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#ifndef LEVELDB_HELPER_H
#define LEVELDB_HELPER_H


#include <cstdint>
#include <string>
#include "macros.h"

namespace leveldb
{
    class Iterator;
    class DB;
}

class LevelDB;

class GOOFER_API LevelIter
{
public:
    LevelIter(LevelDB & db, bool forward = true);
    LevelIter(const LevelIter &) = delete;
    LevelIter(LevelIter &&) = delete;
    LevelIter & operator = (const LevelIter &) = delete;
    LevelIter & operator = (LevelIter &&) = delete;
    ~LevelIter();

public:
    bool good();
    void next();
    std::string get_key();
    std::string get_value();

private:
    leveldb::Iterator     * m_iter;
    bool                    m_forward;
};

class GOOFER_API LevelDB
{
public:
    LevelDB();
    LevelDB(const LevelDB &) = delete;
    LevelDB(LevelDB &&) = delete;
    LevelDB & operator = (const LevelDB &) = delete;
    LevelDB & operator = (LevelDB &&) = delete;
    ~LevelDB();

public:
    static bool repair(const std::string & path);
    static bool destroy(const std::string & path);

public:
    bool init(const std::string & path);
    void exit();

public:
    bool set(const std::string & key, const std::string & value);
    bool get(const std::string & key, std::string & value);
    bool erase(const std::string & key);

public:
    bool set(const std::string & key, const void * value_ptr, size_t value_len);
    bool get(const std::string & key, void * value_ptr, size_t value_len);

public:
    bool set(const std::string & key, const char * value);
    bool set(const std::string & key, bool value);
    bool set(const std::string & key, int8_t value);
    bool set(const std::string & key, uint8_t value);
    bool set(const std::string & key, int16_t value);
    bool set(const std::string & key, uint16_t value);
    bool set(const std::string & key, int32_t value);
    bool set(const std::string & key, uint32_t value);
    bool set(const std::string & key, int64_t value);
    bool set(const std::string & key, uint64_t value);
    bool set(const std::string & key, float value);
    bool set(const std::string & key, double value);

public:
    bool get(const std::string & key, bool & value);
    bool get(const std::string & key, int8_t & value);
    bool get(const std::string & key, uint8_t & value);
    bool get(const std::string & key, int16_t & value);
    bool get(const std::string & key, uint16_t & value);
    bool get(const std::string & key, int32_t & value);
    bool get(const std::string & key, uint32_t & value);
    bool get(const std::string & key, int64_t & value);
    bool get(const std::string & key, uint64_t & value);
    bool get(const std::string & key, float & value);
    bool get(const std::string & key, double & value);

private:
    friend class LevelIter;

private:
    leveldb::DB           * m_db;
    std::string             m_path;
};


#endif // LEVELDB_HELPER_H
