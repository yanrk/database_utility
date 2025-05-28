/********************************************************
 * Description : redis helper which base on hiredis
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#include "macros.h"
#ifdef GOOFER_OS_IS_WIN
    #include <winsock2.h>
#else
    #include <sys/time.h>
#endif // GOOFER_OS_IS_WIN
#include <cstring>
#include <cstdio>
#include <string>
#include <list>
#include <vector>
#include "base.h"
#include "hiredis.h"
#include "hircluster.h"
#include "redis_helper.h"

#ifdef GOOFER_OS_IS_WIN
    #define strcmp_ignore_case stricmp
#else
    #define strcmp_ignore_case strcasecmp
#endif // _MSC_VER

RedisClient::RedisClient()
    : m_running(false)
    , m_redis_address()
    , m_redis_username()
    , m_redis_password()
    , m_redis_table("0")
    , m_redis_timeout(0)
    , m_redis_context(nullptr)
    , m_redis_cluster_context(nullptr)
{

}

RedisClient::~RedisClient()
{
    exit();
}

bool RedisClient::init(const std::string & address, const std::string & username, const std::string & password, uint16_t table_index, uint32_t timeout_ms)
{
    exit();

    do
    {
        m_running = true;
        m_redis_address = address;
        m_redis_username = username;
        m_redis_password = password;
        m_redis_table = std::to_string(table_index);
        m_redis_timeout = timeout_ms;

        if (!login())
        {
            RUN_LOG_ERR("redis client init failure while login to redis server");
            break;
        }

        return true;
    } while (false);

    exit();

    return false;
}

void RedisClient::exit()
{
    if (m_running)
    {
        m_running = false;
        logoff();
    }
}

bool RedisClient::login()
{
    if (nullptr != m_redis_context || nullptr != m_redis_cluster_context)
    {
        return true;
    }

    timeval redis_timeout = { m_redis_timeout / 1000, m_redis_timeout % 1000 * 1000 };

    if (std::string::npos == m_redis_address.find(','))
    {
        std::string redis_host;
        uint16_t redis_port = 6379;
        std::string::size_type pos = m_redis_address.find(':');
        if (std::string::npos == pos)
        {
            redis_host = m_redis_address;
        }
        else
        {
            redis_host = m_redis_address.substr(0, pos);
            redis_port = static_cast<uint16_t>(std::stoi(m_redis_address.substr(pos + 1)));
        }

        do
        {
            m_redis_context = redisConnectWithTimeout(redis_host.c_str(), redis_port, redis_timeout);
            if (nullptr == m_redis_context || 0 != m_redis_context->err)
            {
                RUN_LOG_ERR("redis client login redis server [%s] failure while connect error (%s)", m_redis_address.c_str(), nullptr != m_redis_context ? m_redis_context->errstr : "unknown");
                break;
            }

            if (!authenticate())
            {
                RUN_LOG_ERR("redis client login redis server [%s] failure while authenticate error (%s)", m_redis_address.c_str(), nullptr != m_redis_context ? m_redis_context->errstr : "unknown");
                break;
            }

            if (!select_table())
            {
                RUN_LOG_ERR("redis client login redis server [%s] failure while select table error (%s)", m_redis_address.c_str(), nullptr != m_redis_context ? m_redis_context->errstr : "unknown");
                break;
            }

            return true;
        } while (false);
    }
    else
    {
        do
        {
            m_redis_cluster_context = redisClusterContextInit();
            if (nullptr == m_redis_cluster_context)
            {
                RUN_LOG_ERR("redis client login redis server [%s] failure while init redis cluster context", m_redis_address.c_str());
                break;
            }

            int reply_value = redisClusterSetOptionAddNodes(m_redis_cluster_context, m_redis_address.c_str());
            if (REDIS_OK != reply_value)
            {
                RUN_LOG_ERR("redis client login redis server [%s] failure while set redis cluster option (add nodes) error (%s)", m_redis_address.c_str(), m_redis_cluster_context->errstr);
                break;
            }

            if (!m_redis_username.empty())
            {
                reply_value = redisClusterSetOptionUsername(m_redis_cluster_context, m_redis_username.c_str());
                if (REDIS_OK != reply_value)
                {
                    RUN_LOG_ERR("redis client login redis server [%s] failure while set redis cluster option (username) error (%s)", m_redis_address.c_str(), m_redis_cluster_context->errstr);
                    break;
                }
            }

            if (!m_redis_password.empty())
            {
                reply_value = redisClusterSetOptionPassword(m_redis_cluster_context, m_redis_password.c_str());
                if (REDIS_OK != reply_value)
                {
                    RUN_LOG_ERR("redis client login redis server [%s] failure while set redis cluster option (password) error (%s)", m_redis_address.c_str(), m_redis_cluster_context->errstr);
                    break;
                }
            }

            reply_value = redisClusterSetOptionConnectTimeout(m_redis_cluster_context, redis_timeout);
            if (REDIS_OK != reply_value)
            {
                RUN_LOG_ERR("redis client login redis server [%s] failure while set redis cluster option (connect timeout) error (%s)", m_redis_address.c_str(), m_redis_cluster_context->errstr);
                break;
            }

            reply_value = redisClusterSetOptionRouteUseSlots(m_redis_cluster_context);
            if (REDIS_OK != reply_value)
            {
                RUN_LOG_ERR("redis client login redis server [%s] failure while set redis cluster option (route use slots) error (%s)", m_redis_address.c_str(), m_redis_cluster_context->errstr);
                break;
            }

            reply_value = redisClusterConnect2(m_redis_cluster_context);
            if (REDIS_OK != reply_value)
            {
                RUN_LOG_ERR("redis client login redis server [%s] failure while connect redis cluster error (%s)", m_redis_address.c_str(), m_redis_cluster_context->errstr);
                break;
            }

            return true;
        } while (false);
    }

    logoff();

    return false;
}

void RedisClient::logoff()
{
    if (nullptr != m_redis_context)
    {
        redisFree(m_redis_context);
        m_redis_context = nullptr;
    }

    if (nullptr != m_redis_cluster_context)
    {
        redisClusterFree(m_redis_cluster_context);
        m_redis_cluster_context = nullptr;
    }
}

bool RedisClient::execute(const std::list<std::string> & command_line, int reply_type, void * reply_value)
{
    if (!m_running || command_line.empty() || !login())
    {
        return false;
    }

    std::string command;
    std::vector<const char *> arg_ptr;
    std::vector<size_t> arg_len;
    for (std::list<std::string>::const_iterator iter = command_line.begin(); command_line.end() != iter; ++iter)
    {
        const std::string & arg = *iter;
        if (command.empty())
        {
            command = arg;
        }
        else
        {
            command += " \"" + arg + "\"";
        }
        arg_ptr.push_back(arg.c_str());
        arg_len.push_back(arg.size());
    }

    redisReply * redis_reply = nullptr;
    if (nullptr != m_redis_context)
    {
        redis_reply = reinterpret_cast<redisReply *>(redisCommandArgv(m_redis_context, static_cast<int>(command_line.size()), &arg_ptr[0], &arg_len[0]));
    }
    else
    {
        redis_reply = reinterpret_cast<redisReply *>(redisClusterCommandArgv(m_redis_cluster_context, static_cast<int>(command_line.size()), &arg_ptr[0], &arg_len[0]));
    }

    if (nullptr == redis_reply)
    {
        RUN_LOG_ERR("redis client execute command [%s] failure", command.c_str());
        logoff();
        return false;
    }

    bool result = false;
    bool good = true;

    if (reply_type == redis_reply->type)
    {
        switch (reply_type)
        {
            case REDIS_REPLY_STRING:
            {
                std::string & value = *reinterpret_cast<std::string *>(reply_value);
                value = redis_reply->str;
                result = true;
                break;
            }
            case REDIS_REPLY_ARRAY:
            {
                std::list<std::string> & values = *reinterpret_cast<std::list<std::string> *>(reply_value);
                for (size_t index = 0; index < redis_reply->elements; ++index)
                {
                    values.push_back(redis_reply->element[index]->str);
                }
                result = true;
                break;
            }
            case REDIS_REPLY_INTEGER:
            {
                result = (redis_reply->integer > 0);
                break;
            }
            case REDIS_REPLY_NIL:
            {
                break;
            }
            case REDIS_REPLY_STATUS:
            {
                result = (0 == strcmp_ignore_case(redis_reply->str, "ok"));
                break;
            }
            case REDIS_REPLY_ERROR:
            {
                good = false;
                break;
            }
            default:
            {
                good = false;
                break;
            }
        }

        if (!result)
        {
            if (good)
            {
                RUN_LOG_TRK("redis client execute command [%s] failure (%s)", command.c_str(), REDIS_REPLY_ERROR == redis_reply->type ? redis_reply->str : "unknown");
            }
            else
            {
                RUN_LOG_ERR("redis client execute command [%s] exception (%s)", command.c_str(), REDIS_REPLY_ERROR == redis_reply->type ? redis_reply->str : "unknown");
            }
        }
    }
    else
    {
        RUN_LOG_TRK("redis client execute command [%s] failure while unexpected reply type (%d != %d)", command.c_str(), reply_type, redis_reply->type);
    }

    freeReplyObject(redis_reply);

    if (!good)
    {
        logoff();
    }

    return result;
}

bool RedisClient::authenticate()
{
    if (m_redis_password.empty())
    {
        return true;
    }
    std::list<std::string> command_line;
    command_line.push_back("auth");
    command_line.push_back(m_redis_password);
    return execute(command_line, REDIS_REPLY_STATUS, nullptr);
}

bool RedisClient::select_table()
{
    std::list<std::string> command_line;
    command_line.push_back("select");
    command_line.push_back(m_redis_table);
    return execute(command_line, REDIS_REPLY_STATUS, nullptr);
}

bool RedisClient::flush_db()
{
    std::list<std::string> command_line;
    command_line.push_back("flushdb");
    return execute(command_line, REDIS_REPLY_STATUS, nullptr);
}

bool RedisClient::find(const std::string & key)
{
    std::list<std::string> command_line;
    command_line.push_back("exists");
    command_line.push_back(key);
    return execute(command_line, REDIS_REPLY_INTEGER, nullptr);
}

bool RedisClient::find(const std::string & pattern, std::list<std::string> & keys)
{
    if (nullptr == m_redis_context && nullptr != m_redis_cluster_context)
    {
        RUN_LOG_ERR("redis client find pattern failure while cluster not support");
        return false;
    }
    std::list<std::string> command_line;
    command_line.push_back("keys");
    command_line.push_back(pattern);
    return execute(command_line, REDIS_REPLY_ARRAY, &keys);
}

bool RedisClient::erase(const std::string & key)
{
    std::list<std::string> command_line;
    command_line.push_back("del");
    command_line.push_back(key);
    return execute(command_line, REDIS_REPLY_INTEGER, nullptr);
}

bool RedisClient::erase(const std::list<std::string> & keys)
{
    bool result = true;
    for (std::list<std::string>::const_iterator iter = keys.begin(); keys.end() != iter; ++iter)
    {
        if (!erase(*iter))
        {
            result = false;
        }
    }
    return result;
}

bool RedisClient::persist(const std::string & key)
{
    std::list<std::string> command_line;
    command_line.push_back("persist");
    command_line.push_back(key);
    return execute(command_line, REDIS_REPLY_INTEGER, nullptr);
}

bool RedisClient::persist(const std::list<std::string> & keys)
{
    bool result = true;
    for (std::list<std::string>::const_iterator iter = keys.begin(); keys.end() != iter; ++iter)
    {
        if (!persist(*iter))
        {
            result = false;
        }
    }
    return result;
}

bool RedisClient::expire(const std::string & key, const std::string & seconds)
{
    std::list<std::string> command_line;
    command_line.push_back("expire");
    command_line.push_back(key);
    command_line.push_back(seconds);
    return execute(command_line, REDIS_REPLY_INTEGER, nullptr);
}

bool RedisClient::expire(const std::string & key, int64_t seconds)
{
    return expire(key, std::to_string(seconds));
}

bool RedisClient::expire(const std::list<std::string> & keys, int64_t seconds)
{
    bool result = true;
    std::string timeout = std::to_string(seconds);
    for (std::list<std::string>::const_iterator iter = keys.begin(); keys.end() != iter; ++iter)
    {
        if (!expire(*iter, timeout))
        {
            result = false;
        }
    }
    return result;
}

bool RedisClient::set(const std::string & key, const std::string & value)
{
    std::list<std::string> command_line;
    command_line.push_back("set");
    command_line.push_back(key);
    command_line.push_back(value);
    return execute(command_line, REDIS_REPLY_STATUS, nullptr);
}

bool RedisClient::get(const std::string & key, std::string & value)
{
    std::list<std::string> command_line;
    command_line.push_back("get");
    command_line.push_back(key);
    return execute(command_line, REDIS_REPLY_STRING, &value);
}

bool RedisClient::push_back(const std::string & queue, const std::string & value)
{
    std::list<std::string> command_line;
    command_line.push_back("rpush");
    command_line.push_back(queue);
    command_line.push_back(value);
    return execute(command_line, REDIS_REPLY_INTEGER, nullptr);
}

bool RedisClient::pop_front(const std::string & queue, std::string & value)
{
    std::list<std::string> command_line;
    command_line.push_back("lpop");
    command_line.push_back(queue);
    return execute(command_line, REDIS_REPLY_STRING, &value);
}

bool RedisClient::set(const std::string & key, const char * value)
{
    return nullptr != value && set(key, std::string(value));
}

bool RedisClient::set(const std::string & key, bool value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, int8_t value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, uint8_t value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, int16_t value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, uint16_t value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, int32_t value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, uint32_t value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, int64_t value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, uint64_t value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, float value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::set(const std::string & key, double value)
{
    return set(key, std::to_string(value));
}

bool RedisClient::get(const std::string & key, bool & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = ("true" == dummy);
    return result;
}

bool RedisClient::get(const std::string & key, int8_t & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = static_cast<int8_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::get(const std::string & key, uint8_t & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = static_cast<uint8_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::get(const std::string & key, int16_t & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = static_cast<int16_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::get(const std::string & key, uint16_t & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = static_cast<uint16_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::get(const std::string & key, int32_t & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = static_cast<int32_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::get(const std::string & key, uint32_t & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = static_cast<uint32_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::get(const std::string & key, int64_t & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = static_cast<int64_t>(std::stoll(dummy));
    return result;
}

bool RedisClient::get(const std::string & key, uint64_t & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = static_cast<uint64_t>(std::stoull(dummy));
    return result;
}

bool RedisClient::get(const std::string & key, float & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = std::stof(dummy);
    return result;
}

bool RedisClient::get(const std::string & key, double & value)
{
    std::string dummy;
    bool result = get(key, dummy);
    value = std::stod(dummy);
    return result;
}

bool RedisClient::push_back(const std::string & queue, const char * value)
{
    return nullptr != value && push_back(queue, std::string(value));
}

bool RedisClient::push_back(const std::string & queue, bool value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, int8_t value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, uint8_t value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, int16_t value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, uint16_t value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, int32_t value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, uint32_t value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, int64_t value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, uint64_t value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, float value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::push_back(const std::string & queue, double value)
{
    return push_back(queue, std::to_string(value));
}

bool RedisClient::pop_front(const std::string & queue, bool & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = ("true" == dummy);
    return result;
}

bool RedisClient::pop_front(const std::string & queue, int8_t & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = static_cast<int8_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::pop_front(const std::string & queue, uint8_t & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = static_cast<uint8_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::pop_front(const std::string & queue, int16_t & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = static_cast<int16_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::pop_front(const std::string & queue, uint16_t & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = static_cast<uint16_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::pop_front(const std::string & queue, int32_t & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = static_cast<int32_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::pop_front(const std::string & queue, uint32_t & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = static_cast<uint32_t>(std::stoi(dummy));
    return result;
}

bool RedisClient::pop_front(const std::string & queue, int64_t & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = static_cast<int64_t>(std::stoll(dummy));
    return result;
}

bool RedisClient::pop_front(const std::string & queue, uint64_t & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = static_cast<uint64_t>(std::stoull(dummy));
    return result;
}

bool RedisClient::pop_front(const std::string & queue, float & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = std::stof(dummy);
    return result;
}

bool RedisClient::pop_front(const std::string & queue, double & value)
{
    std::string dummy;
    bool result = pop_front(queue, dummy);
    value = std::stod(dummy);
    return result;
}

bool RedisClient::clear(const std::string & queue)
{
    return erase(queue);
}

bool RedisClient::clear()
{
    return flush_db();
}
