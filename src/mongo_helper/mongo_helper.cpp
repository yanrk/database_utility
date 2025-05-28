/**********************************************************
 * Description : mongo helper which base on mongo-c-driver
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 **********************************************************/

#include "base.h"
#include "bson.h"
#include "mongoc.h"
#include "mongo_helper.h"

MongoTable::MongoTable()
    : m_uri()
    , m_db()
    , m_tb()
    , m_client(nullptr, nullptr)
    , m_collection(nullptr, nullptr)
    , m_cursor(nullptr, nullptr)
{

}

MongoTable::~MongoTable()
{
    exit();
}

bool MongoTable::init(const std::string & uri, const std::string & db, const std::string & tb)
{
    exit();

    if (uri.empty())
    {
        RUN_LOG_ERR("mongo table init failure while invalid uri");
        return false;
    }

    if (db.empty())
    {
        RUN_LOG_ERR("mongo table init failure while invalid db");
        return false;
    }

    if (tb.empty())
    {
        RUN_LOG_ERR("mongo table init failure while invalid tb");
        return false;
    }

    do
    {
        mongoc_init();

        m_client = std::unique_ptr<_mongoc_client_t, void (*) (_mongoc_client_t *)>(mongoc_client_new(uri.c_str()), mongoc_client_destroy);
        if (!m_client)
        {
            RUN_LOG_ERR("mongo table (%s, %s, %s) init failure while create mongo client", uri.c_str(), db.c_str(), tb.c_str());
            break;
        }

        m_collection = std::unique_ptr<_mongoc_collection_t, void (*) (_mongoc_collection_t *)>(mongoc_client_get_collection(m_client.get(), db.c_str(), tb.c_str()), mongoc_collection_destroy);
        if (!m_collection)
        {
            RUN_LOG_ERR("mongo table (%s, %s, %s) init failure while create mongo collection", uri.c_str(), db.c_str(), tb.c_str());
            break;
        }

        m_uri = uri;
        m_db = db;
        m_tb = tb;

        return true;
    } while (false);

    exit();

    return false;
}

void MongoTable::exit()
{
    m_cursor.release();
    m_collection.release();
    m_client.release();

    mongoc_cleanup();
}

bool MongoTable::index(const std::string & key, bool asc, bool unique)
{
    if (!m_collection)
    {
        return false;
    }

    const std::string index_json("{\"" + key + "\":" + (asc ? "1" : "-1") + "}");

    bson_error_t error = { 0x0 };
    std::unique_ptr<_bson_t, void (*) (_bson_t *)> index_bson(bson_new_from_json(reinterpret_cast<const uint8_t *>(index_json.c_str()), index_json.size(), &error), bson_destroy);
    if (!index_bson)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) index (%s) failure while json to bson error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), index_json.c_str(), error.message);
        return false;
    }

    mongoc_index_opt_t index_opt;
    mongoc_index_opt_init(&index_opt);
    index_opt.unique = unique;

    if (!mongoc_collection_create_index_with_opts(m_collection.get(), index_bson.get(), &index_opt, nullptr, nullptr, &error))
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) index (%s) failure while create index with options error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), index_json.c_str(), error.message);
        return false;
    }

    return true;
}

int64_t MongoTable::count(const std::string & select_json)
{
    if (!m_collection)
    {
        return -1;
    }

    bson_error_t error = { 0x0 };
    std::unique_ptr<_bson_t, void (*) (_bson_t *)> select_bson(bson_new_from_json(reinterpret_cast<const uint8_t *>(select_json.c_str()), select_json.size(), &error), bson_destroy);
    if (!select_bson)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) count (%s) failure while json to bson error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), select_json.c_str(), error.message);
        return -1;
    }

    int64_t result = mongoc_collection_count_documents(m_collection.get(), select_bson.get(), nullptr, nullptr, nullptr, &error);
    if (result < 0)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) count (%s) failure while count documents error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), select_json.c_str(), error.message);
        return -1;
    }

    return result;
}

int64_t MongoTable::count()
{
    return count("{}");
}

bool MongoTable::select(const std::string & select_json)
{
    if (!m_collection)
    {
        return false;
    }

    bson_error_t error = { 0x0 };
    std::unique_ptr<_bson_t, void (*) (_bson_t *)> select_bson(bson_new_from_json(reinterpret_cast<const uint8_t *>(select_json.c_str()), select_json.size(), &error), bson_destroy);
    if (!select_bson)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) select (%s) failure while json to bson error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), select_json.c_str(), error.message);
        return false;
    }

    m_cursor = std::unique_ptr<mongoc_cursor_t, void (*) (mongoc_cursor_t *)>(mongoc_collection_find_with_opts(m_collection.get(), select_bson.get(), nullptr, nullptr), mongoc_cursor_destroy);
    if (!m_cursor)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) select (%s) failure while find with options error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), select_json.c_str(), error.message);
        return false;
    }

    return true;
}

bool MongoTable::select()
{
    return select("{}");
}

bool MongoTable::read(std::string & select_json)
{
    if (!m_cursor)
    {
        return false;
    }

    const bson_t * select_bson = nullptr;
    if (mongoc_cursor_next(m_cursor.get(), &select_bson))
    {
        select_json = bson_as_json(select_bson, nullptr);
        return true;
    }
    else
    {
        select_json.clear();
        m_cursor.release();
        return false;
    }
}

bool MongoTable::insert(const std::string & insert_json)
{
    if (!m_collection)
    {
        return false;
    }

    bson_error_t error = { 0x0 };
    std::unique_ptr<_bson_t, void (*) (_bson_t *)> insert_bson(bson_new_from_json(reinterpret_cast<const uint8_t *>(insert_json.c_str()), insert_json.size(), &error), bson_destroy);
    if (!insert_bson)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) insert (%s) failure while json to bson error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), insert_json.c_str(), error.message);
        return false;
    }

    if (!mongoc_collection_insert_one(m_collection.get(), insert_bson.get(), nullptr, nullptr, &error))
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) insert (%s) failure while insert one error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), insert_json.c_str(), error.message);
        return false;
    }

    return true;
}

bool MongoTable::update(const std::string & select_json, const std::string & update_json)
{
    if (!m_collection)
    {
        return false;
    }

    bson_error_t error = { 0x0 };
    std::unique_ptr<_bson_t, void (*) (_bson_t *)> select_bson(bson_new_from_json(reinterpret_cast<const uint8_t *>(select_json.c_str()), select_json.size(), &error), bson_destroy);
    if (!select_bson)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) update (key: %s) failure while json to bson error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), select_json.c_str(), error.message);
        return false;
    }

    const std::string update_setting("{\"$set\":" + update_json + "}");
    std::unique_ptr<_bson_t, void (*) (_bson_t *)> update_bson(bson_new_from_json(reinterpret_cast<const uint8_t *>(update_setting.c_str()), update_setting.size(), &error), bson_destroy);
    if (!update_bson)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) update (val: %s) failure while json to bson error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), update_json.c_str(), error.message);
        return false;
    }

    const std::string option_setting("{\"upsert\":true}");
    std::unique_ptr<_bson_t, void (*) (_bson_t *)> option_bson(bson_new_from_json(reinterpret_cast<const uint8_t *>(option_setting.c_str()), option_setting.size(), &error), bson_destroy);
    if (!option_bson)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) update (opt: true) failure while json to bson error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), error.message);
        return false;
    }

    if (!mongoc_collection_update_one(m_collection.get(), select_bson.get(), update_bson.get(), option_bson.get(), nullptr, &error))
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) update (key: %s), (val: %s) (opt: true) failure while update one error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), select_json.c_str(), update_json.c_str(), error.message);
        return false;
    }

    return true;
}

bool MongoTable::remove(const std::string & remove_json)
{
    if (!m_collection)
    {
        return false;
    }

    bson_error_t error = { 0x0 };
    std::unique_ptr<_bson_t, void (*) (_bson_t *)> remove_bson(bson_new_from_json(reinterpret_cast<const uint8_t *>(remove_json.c_str()), remove_json.size(), &error), bson_destroy);
    if (!remove_bson)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) remove (%s) failure while json to bson error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), remove_json.c_str(), error.message);
        return false;
    }

    if (!mongoc_collection_delete_one(m_collection.get(), remove_bson.get(), nullptr, nullptr, &error))
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) remove (%s) failure while delete one error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), remove_json.c_str(), error.message);
        return false;
    }

    return true;
}

bool MongoTable::remove()
{
    if (!m_collection)
    {
        return false;
    }

    const std::string remove_json("{}");

    bson_error_t error = { 0x0 };
    std::unique_ptr<_bson_t, void (*) (_bson_t *)> remove_bson(bson_new_from_json(reinterpret_cast<const uint8_t *>(remove_json.c_str()), remove_json.size(), &error), bson_destroy);
    if (!remove_bson)
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) remove (%s) failure while json to bson error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), remove_json.c_str(), error.message);
        return false;
    }

    if (!mongoc_collection_delete_many(m_collection.get(), remove_bson.get(), nullptr, nullptr, &error))
    {
        RUN_LOG_ERR("mongo table (%s, %s, %s) remove (%s) failure while delete many error (%s)", m_uri.c_str(), m_db.c_str(), m_tb.c_str(), remove_json.c_str(), error.message);
        return false;
    }

    return true;
}

const std::string & MongoTable::get_uri() const
{
    return m_uri;
}

const std::string & MongoTable::get_db() const
{
    return m_db;
}

const std::string & MongoTable::get_tb() const
{
    return m_tb;
}
