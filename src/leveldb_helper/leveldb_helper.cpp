/********************************************************
 * Description : leveldb helper which base on leveldb
 * Author      : baoc, yanrk
 * Email       : yanrkchina@163.com
 * Version     : 3.0
 * History     :
 * Copyright(C): 2023
 ********************************************************/

#include <cstring>
#include "base.h"
#include "leveldb/db.h"
#include "leveldb_helper.h"

LevelIter::LevelIter(LevelDB & db)
    : m_iter(nullptr)
{
    if (nullptr != db.m_db)
    {
        m_iter = db.m_db->NewIterator(leveldb::ReadOptions());
        if (nullptr != m_iter)
        {
            m_iter->SeekToFirst();
        }
    }
}

LevelIter::~LevelIter()
{
    if (nullptr != m_iter)
    {
        delete m_iter;
        m_iter = nullptr;
    }
}

bool LevelIter::good()
{
    return nullptr != m_iter && m_iter->Valid();
}

void LevelIter::next()
{
    if (good())
    {
        m_iter->Next();
    }
}

std::string LevelIter::get_key()
{
    return good() ? m_iter->key().ToString() : "";
}

std::string LevelIter::get_value()
{
    return good() ? m_iter->value().ToString() : "";
}

LevelDB::LevelDB()
    : m_db(nullptr)
    , m_path()
{

}

LevelDB::~LevelDB()
{
    exit();
}

bool LevelDB::repair(const std::string & path)
{
    if (!RepairDB(path, leveldb::Options()).ok())
    {
        RUN_LOG_ERR("level db (%s) repair failure", path.c_str());
        return false;
    }
    return true;
}

bool LevelDB::destroy(const std::string & path)
{
    if (!DestroyDB(path, leveldb::Options()).ok())
    {
        RUN_LOG_ERR("level db (%s) destroy failure", path.c_str());
        return false;
    }
    return true;
}

bool LevelDB::init(const std::string & path)
{
    exit();

    leveldb::Options options;
    options.create_if_missing = true;
    if (!leveldb::DB::Open(options, path.c_str(), &m_db).ok())
    {
        RUN_LOG_ERR("level db (%s) init failure while leveldb open", path.c_str());
        return false;
    }

    m_path = path;

    return true;
}

void LevelDB::exit()
{
    if (nullptr != m_db)
    {
        delete m_db;
        m_db = nullptr;
    }
    m_path.clear();
}

bool LevelDB::set(const std::string & key, const std::string & value)
{
    if (nullptr == m_db)
    {
        return false;
    }

    leveldb::WriteOptions write_options;
    write_options.sync = false;
    if (!m_db->Put(write_options, key, value).ok())
    {
        RUN_LOG_ERR("level db (%s) set (%s) failure", m_path.c_str(), key.c_str());
        return false;
    }

    return true;
}

bool LevelDB::get(const std::string & key, std::string & value)
{
    if (nullptr == m_db)
    {
        return false;
    }

    leveldb::ReadOptions read_options;
    read_options.fill_cache = true;
    if (!m_db->Get(read_options, key, &value).ok())
    {
        RUN_LOG_TRK("level db (%s) get (%s) failure", m_path.c_str(), key.c_str());
        return false;
    }

    return true;
}

bool LevelDB::erase(const std::string & key)
{
    if (nullptr == m_db)
    {
        return false;
    }

    leveldb::WriteOptions write_options;
    write_options.sync = false;

    if (!m_db->Delete(write_options, key).ok())
    {
        RUN_LOG_ERR("level db (%s) erase (%s) failure", m_path.c_str(), key.c_str());
        return false;
    }

    return true;
}

bool LevelDB::set(const std::string & key, const void * value_ptr, size_t value_len)
{
    if (nullptr == value_ptr || 0 == value_len)
    {
        return false;
    }

    const std::string value(reinterpret_cast<const char *>(value_ptr), reinterpret_cast<const char *>(value_ptr) + value_len);

    return set(key, value);
}

bool LevelDB::get(const std::string & key, void * value_ptr, size_t value_len)
{
    if (nullptr == value_ptr || 0 == value_len)
    {
        return false;
    }

    std::string value;

    if (!get(key, value))
    {
        return false;
    }

    if (value.size() != value_len)
    {
        RUN_LOG_ERR("level db (%s) get (%s) failure while unexpected value size (%u != %u)", m_path.c_str(), key.c_str(), static_cast<uint32_t>(value.size()), static_cast<uint32_t>(value_len));
        return false;
    }

    memcpy(value_ptr, value.data(), value_len);

    return true;
}

bool LevelDB::set(const std::string & key, const char * value)
{
    return nullptr != value && set(key, std::string(value));
}

bool LevelDB::set(const std::string & key, bool value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, int8_t value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, uint8_t value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, int16_t value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, uint16_t value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, int32_t value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, uint32_t value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, int64_t value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, uint64_t value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, float value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::set(const std::string & key, double value)
{
    return set(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, bool & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, int8_t & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, uint8_t & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, int16_t & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, uint16_t & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, int32_t & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, uint32_t & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, int64_t & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, uint64_t & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, float & value)
{
    return get(key, &value, sizeof(value));
}

bool LevelDB::get(const std::string & key, double & value)
{
    return get(key, &value, sizeof(value));
}
