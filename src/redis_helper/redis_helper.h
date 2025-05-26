/********************************************************
 * Description : redis helper which base on hiredis
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#ifndef REDIS_HELPER_H
#define REDIS_HELPER_H


#include <cstdint>
#include <string>
#include <list>
#include "macros.h"

struct redisContext;
struct redisClusterContext;

class GOOFER_API RedisClient
{
public:
    RedisClient();
    RedisClient(const RedisClient &) = delete;
    RedisClient(RedisClient &&) = delete;
    RedisClient & operator = (const RedisClient &) = delete;
    RedisClient & operator = (RedisClient &&) = delete;
    ~RedisClient();

public:
    bool init(const std::string & address, const std::string & username, const std::string & password, uint16_t table_index = 0, uint32_t timeout_ms = 5000);
    void exit();

public:
    bool find(const std::string & key);
    bool find(const std::string & pattern, std::list<std::string> & keys);

public:
    bool erase(const std::string & key);
    bool erase(const std::list<std::string> & keys);

public:
    bool persist(const std::string & key);
    bool persist(const std::list<std::string> & keys);

public:
    bool expire(const std::string & key, int64_t seconds);
    bool expire(const std::list<std::string> & keys, int64_t seconds);

public:
    bool set(const std::string & key, const char * value);
    bool set(const std::string & key, const std::string & value);
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
    bool get(const std::string & key, std::string & value);
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

public:
    bool push_back(const std::string & queue, const char * value);
    bool push_back(const std::string & queue, const std::string & value);
    bool push_back(const std::string & queue, bool value);
    bool push_back(const std::string & queue, int8_t value);
    bool push_back(const std::string & queue, uint8_t value);
    bool push_back(const std::string & queue, int16_t value);
    bool push_back(const std::string & queue, uint16_t value);
    bool push_back(const std::string & queue, int32_t value);
    bool push_back(const std::string & queue, uint32_t value);
    bool push_back(const std::string & queue, int64_t value);
    bool push_back(const std::string & queue, uint64_t value);
    bool push_back(const std::string & queue, float value);
    bool push_back(const std::string & queue, double value);

public:
    bool pop_front(const std::string & queue, std::string & value);
    bool pop_front(const std::string & queue, bool & value);
    bool pop_front(const std::string & queue, int8_t & value);
    bool pop_front(const std::string & queue, uint8_t & value);
    bool pop_front(const std::string & queue, int16_t & value);
    bool pop_front(const std::string & queue, uint16_t & value);
    bool pop_front(const std::string & queue, int32_t & value);
    bool pop_front(const std::string & queue, uint32_t & value);
    bool pop_front(const std::string & queue, int64_t & value);
    bool pop_front(const std::string & queue, uint64_t & value);
    bool pop_front(const std::string & queue, float & value);
    bool pop_front(const std::string & queue, double & value);

public:
    bool clear(const std::string & queue);
    bool clear();

private:
    bool login();
    void logoff();
    bool authenticate();
    bool select_table();
    bool flush_db();

private:
    bool execute(const std::list<std::string> & command_line, int reply_type, void * reply_value);
    bool expire(const std::string & key, const std::string & seconds);

private:
    bool                            m_running;
    std::string                     m_redis_address;
    std::string                     m_redis_username;
    std::string                     m_redis_password;
    std::string                     m_redis_table;
    uint32_t                        m_redis_timeout;
    redisContext                  * m_redis_context;
    redisClusterContext           * m_redis_cluster_context;
};


#endif // REDIS_HELPER_H
