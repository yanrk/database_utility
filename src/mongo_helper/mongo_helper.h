/**********************************************************
 * Description : mongo helper which base on mongo-c-driver
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 **********************************************************/

#ifndef MONGO_HELPER_H
#define MONGO_HELPER_H


#include <cstdint>
#include <string>
#include <memory>
#include "macros.h"

struct _mongoc_client_t;
struct _mongoc_collection_t;
struct _mongoc_cursor_t;

class GOOFER_API MongoTable
{
public:
    MongoTable();
    MongoTable(const MongoTable &) = delete;
    MongoTable(MongoTable &&) = delete;
    MongoTable & operator = (const MongoTable &) = delete;
    MongoTable & operator = (MongoTable &&) = delete;
    ~MongoTable();

public:
    bool init(const std::string & uri, const std::string & db, const std::string & tb);
    void exit();

public:
    bool index(const std::string & key, bool asc = true, bool unique = true);

public:
    int64_t count(const std::string & select_json);
    int64_t count();

public:
    bool select(const std::string & select_json);
    bool select();
    bool read(std::string & select_json);

public:
    bool insert(const std::string & insert_json);

public:
    bool update(const std::string & select_json, const std::string & update_json);

public:
    bool remove(const std::string & remove_json);
    bool remove();

public:
    const std::string & get_uri() const;
    const std::string & get_db() const;
    const std::string & get_tb() const;

private:
    std::string                                                                 m_uri;
    std::string                                                                 m_db;
    std::string                                                                 m_tb;
    std::unique_ptr<_mongoc_client_t, void (*) (_mongoc_client_t *)>            m_client;
    std::unique_ptr<_mongoc_collection_t, void (*) (_mongoc_collection_t *)>    m_collection;
    std::unique_ptr<_mongoc_cursor_t, void (*) (_mongoc_cursor_t *)>            m_cursor;
};


#endif // MONGO_HELPER_H
