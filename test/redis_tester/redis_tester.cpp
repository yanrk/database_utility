/********************************************************
 * Description : sample of redis helper
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Version     : 2.0
 * Copyright(C): 2025
 ********************************************************/

#include "macros.h"

#ifdef GOOFER_OS_IS_WIN
    #include <io.h>
    #include <direct.h>
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <errno.h>
#endif // GOOFER_OS_IS_WIN

#include <ctime>
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <list>
#include <string>
#include <iostream>
#include "redis_helper.h"

#ifdef TEST_CLUSTER
    #define SERVER      "172.16.7.25:6379,172.16.7.25:6380,172.16.7.25:6381,172.16.7.25:6382,172.16.7.25:6383,172.16.7.25:6384"
    #define USERNAME    ""
    #define PASSWORD    "abc123"
#else
    #define SERVER      "127.0.0.1:6379"
    #define USERNAME    ""
    #define PASSWORD    ""
#endif // TEST_CLUSTER

class Directory
{
public:
    Directory();
    ~Directory();

public:
    bool open(const char * dirname);
    bool is_open() const;
    bool read();
    void close();

public:
    const std::string & sub_path_name() const;
    const std::string & sub_path_short_name() const;
    bool sub_path_is_directory() const;

private:
    std::string          m_dir_name;
    std::string          m_current_sub_path_name;
    std::string          m_current_sub_path_short_name;
    bool                 m_current_sub_path_is_directory;

#ifdef GOOFER_OS_IS_WIN
    HANDLE               m_dir;
    WIN32_FIND_DATA      m_file;
    bool                 m_eof;
#else
    DIR                * m_dir;
    struct dirent      * m_file;
#endif // GOOFER_OS_IS_WIN
};

#ifdef GOOFER_OS_IS_WIN
    #define g_directory_separator   '\\'
#else
    #define g_directory_separator   '/'
#endif // GOOFER_OS_IS_WIN

Directory::Directory()
    : m_dir_name()
    , m_current_sub_path_name()
    , m_current_sub_path_short_name()
    , m_current_sub_path_is_directory(false)
#ifdef GOOFER_OS_IS_WIN
    , m_dir(INVALID_HANDLE_VALUE)
    , m_file()
    , m_eof(true)
#else
    , m_dir(nullptr)
    , m_file(nullptr)
#endif // GOOFER_OS_IS_WIN
{

}

Directory::~Directory()
{
    close();
}

bool Directory::open(const char * dirname)
{
    close();

    if (nullptr == dirname || '\0' == dirname[0])
    {
        return false;
    }

    m_dir_name = dirname;
    if ('/' != *m_dir_name.rbegin() && '\\' != *m_dir_name.rbegin())
    {
        m_dir_name += g_directory_separator;
    }

#ifdef GOOFER_OS_IS_WIN
    std::string pattern(m_dir_name + "*");
    m_dir = FindFirstFileA(pattern.c_str(), &m_file);
    if (INVALID_HANDLE_VALUE == m_dir)
    {
        return false;
    }
    m_eof = false;
#else
    m_dir = opendir(m_dir_name.c_str());
    if (nullptr == m_dir)
    {
        return false;
    }
    m_file = readdir(m_dir);
#endif // GOOFER_OS_IS_WIN

    return true;
}

bool Directory::is_open() const
{
#ifdef GOOFER_OS_IS_WIN
    return INVALID_HANDLE_VALUE != m_dir;
#else
    return nullptr != m_dir;
#endif // GOOFER_OS_IS_WIN
}

bool Directory::read()
{
    if (!is_open())
    {
        return false;
    }

#ifdef GOOFER_OS_IS_WIN
    while (!m_eof)
    {
        const std::string file_name(m_file.cFileName);
        const DWORD file_type = m_file.dwFileAttributes;
        m_eof = !FindNextFileA(m_dir, &m_file);

        if ("." == file_name || ".." == file_name)
        {
            continue;
        }

        if (FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & file_type))
        {
            m_current_sub_path_short_name = file_name;
            m_current_sub_path_name = m_dir_name + m_current_sub_path_short_name + g_directory_separator;
            m_current_sub_path_is_directory = true;
        }
        else
        {
            m_current_sub_path_short_name = file_name;
            m_current_sub_path_name = m_dir_name + m_current_sub_path_short_name;
            m_current_sub_path_is_directory = false;
        }

        return true;
    }
#else
    while (nullptr != m_file)
    {
        /*
         * do not do like this:
         *     struct dirent * file = m_file;
         *     m_file = readdir(m_dir);
         *     operate_function(file);
         * the behavior is undefined, the result is not expected
         */
        const std::string d_name(m_file->d_name);
#if 0
        const size_t d_type = m_file->d_type;
#endif // 0
        m_file = readdir(m_dir);

        if ("." == d_name || ".." == d_name)
        {
            continue;
        }

#if 0
        /*
         * d_type: not supported by all filesystem
         */
        if (DT_DIR == (DT_DIR & d_type))
        {
            m_current_sub_path_short_name = d_name;
            m_current_sub_path_name = m_dir_name + m_current_sub_path_short_name + g_directory_separator;
            m_current_sub_path_is_directory = true;
            return true;
        }
        else if (DT_REG == (DT_REG & d_type))
        {
            m_current_sub_path_short_name = d_name;
            m_current_sub_path_name = m_dir_name + m_current_sub_path_short_name;
            m_current_sub_path_is_directory = false;
            return true;
        }
#else
        struct stat64 stat_buf = { 0x00 };
        const std::string file_name(m_dir_name + d_name);
        if (0 != stat64(file_name.c_str(), &stat_buf))
        {
            continue;
        }

        if (S_IFDIR == (S_IFDIR & stat_buf.st_mode))
        {
            m_current_sub_path_short_name = d_name;
            m_current_sub_path_name = m_dir_name + m_current_sub_path_short_name + g_directory_separator;
            m_current_sub_path_is_directory = true;
            return true;
        }
        else if (S_IFREG == (S_IFREG & stat_buf.st_mode))
        {
            m_current_sub_path_short_name = d_name;
            m_current_sub_path_name = m_dir_name + m_current_sub_path_short_name;
            m_current_sub_path_is_directory = false;
            return true;
        }
#endif // 0
    }
#endif // GOOFER_OS_IS_WIN

    m_current_sub_path_short_name.clear();
    m_current_sub_path_name.clear();
    m_current_sub_path_is_directory = false;

    return false;
}

void Directory::close()
{
    if (!is_open())
    {
        return;
    }

    m_current_sub_path_short_name.clear();
    m_current_sub_path_name.clear();
    m_current_sub_path_is_directory = false;

#ifdef GOOFER_OS_IS_WIN
    FindClose(m_dir);
    m_dir = INVALID_HANDLE_VALUE;
#else
    closedir(m_dir);
    m_dir = nullptr;
#endif // GOOFER_OS_IS_WIN
}

const std::string & Directory::sub_path_name() const
{
    return m_current_sub_path_name;
}

const std::string & Directory::sub_path_short_name() const
{
    return m_current_sub_path_short_name;
}

bool Directory::sub_path_is_directory() const
{
    return m_current_sub_path_is_directory;
}

static bool get_folder_files(const std::string & folder, std::list<std::string> & file_list)
{
    std::list<std::string> dirname_list;
    dirname_list.push_back(folder);

    while (!dirname_list.empty())
    {
        const std::string dirname = dirname_list.front();
        dirname_list.pop_front();

        Directory dir;
        if (!dir.open(dirname.c_str()))
        {
            continue;
        }

        while (dir.read())
        {
            if (dir.sub_path_is_directory())
            {
                dirname_list.push_back(dir.sub_path_name());
            }
            else
            {
                file_list.push_front(dir.sub_path_name());
            }
        }

        dir.close();
    }

    return true;
}

static std::string get_file_key(const std::string & filename)
{
    const std::string platform_id("2");
    const std::string storage_id("10201");
    const std::string user_prefix("/100000/100001/");
    return platform_id + "|" + storage_id + "|" + user_prefix + "|" + filename + "|" + user_prefix + filename;
}

static std::string get_file_value(const std::string & filename)
{
    const std::string filetime("2019-06-01 00:00:00");
    const std::string filesize("123,456,789");
    return filename + "|" + filetime + "|" + filesize;
}

static bool set_file_to_redis(const std::list<std::string> & file_list, uint16_t table, int64_t expire_seconds)
{
    RedisClient redis_client;
    if (!redis_client.init(SERVER, USERNAME, PASSWORD, table, 5000))
    {
        printf("redis client init failed\n");
        return false;
    }
    for (std::list<std::string>::const_iterator iter = file_list.begin(); file_list.end() != iter; ++iter)
    {
        const std::string & filename = *iter;
        const std::string key(get_file_key(filename));
        const std::string val(get_file_value(filename));
        if (!redis_client.set(key, val))
        {
            printf("redis client set failed\n");
        }
        else if (0 != expire_seconds && !redis_client.expire(key, expire_seconds))
        {
            printf("redis client expire failed\n");
        }
    }
    redis_client.exit();
    return true;
}

static bool get_file_from_redis(const std::list<std::string> & file_list, uint16_t table, int64_t expire_seconds)
{
    RedisClient redis_client;
    if (!redis_client.init(SERVER, USERNAME, PASSWORD, table, 5000))
    {
        printf("redis client init failed\n");
        return false;
    }
    for (std::list<std::string>::const_iterator iter = file_list.begin(); file_list.end() != iter; ++iter)
    {
        const std::string & filename = *iter;
        const std::string key(get_file_key(filename));
        const std::string dst_val(get_file_value(filename));
        std::string src_val;
        if (!redis_client.get(key, src_val))
        {
            printf("redis client get failed\n");
        }
        else if (dst_val != src_val)
        {
            printf("redis client get exception\n");
        }
        else if (0 != expire_seconds && !redis_client.expire(key, expire_seconds))
        {
            printf("redis client expire failed\n");
        }
    }
    redis_client.exit();
    return true;
}

static bool find_file_from_redis(const std::list<std::string> & file_list, uint16_t table)
{
    RedisClient redis_client;
    if (!redis_client.init(SERVER, USERNAME, PASSWORD, table, 5000))
    {
        printf("redis client init failed\n");
        return false;
    }
    for (std::list<std::string>::const_iterator iter = file_list.begin(); file_list.end() != iter; ++iter)
    {
        const std::string & filename = *iter;
        const std::string key(get_file_key(filename));
        if (!redis_client.find(key))
        {
            printf("redis client find failed\n");
        }
    }
    redis_client.exit();
    return true;
}

static bool erase_file_from_redis(const std::list<std::string> & file_list, uint16_t table)
{
    RedisClient redis_client;
    if (!redis_client.init(SERVER, USERNAME, PASSWORD, table, 5000))
    {
        printf("redis client init failed\n");
        return false;
    }
    for (std::list<std::string>::const_iterator iter = file_list.begin(); file_list.end() != iter; ++iter)
    {
        const std::string & filename = *iter;
        const std::string key(get_file_key(filename));
        if (!redis_client.erase(key))
        {
            printf("redis client erase failed\n");
        }
    }
    redis_client.exit();
    return true;
}

static bool clear_file_from_redis(const std::list<std::string> & file_list, uint16_t table)
{
    RedisClient redis_client;
    if (!redis_client.init(SERVER, USERNAME, PASSWORD, table, 5000))
    {
        printf("redis client init failed\n");
        return false;
    }
    if (!redis_client.clear())
    {
        printf("redis client clear failed\n");
        return false;
    }
    return true;
}

static struct timeval get_time()
{
    struct timeval tv_now;

#ifdef GOOFER_OS_IS_WIN
    SYSTEMTIME sys_now;
    GetLocalTime(&sys_now);
    tv_now.tv_sec = static_cast<long>(time(nullptr));
    tv_now.tv_usec = sys_now.wMilliseconds * 1000L;
#else
    gettimeofday(&tv_now, nullptr);
#endif // GOOFER_OS_IS_WIN

    return tv_now;
}

static uint64_t get_time_delta(const struct timeval & lhs, const struct timeval & rhs)
{
    uint64_t delta = 0;
    delta = (lhs.tv_sec - rhs.tv_sec) * 1000 + (lhs.tv_usec - rhs.tv_usec) / 1000;
    return delta;
}

bool test_correctness()
{
    RedisClient redis_client;
    if (!redis_client.init(SERVER, USERNAME, PASSWORD, 0, 5000))
    {
        printf("redis client init failed\n");
        return false;
    }

    if (!redis_client.push_back("test-queue-1", "111"))
    {
        printf("redis client push back failed\n");
        return false;
    }

    if (!redis_client.push_back("test-queue-1", "222"))
    {
        printf("redis client push back failed\n");
        return false;
    }

    if (!redis_client.push_back("test-queue-1", "333"))
    {
        printf("redis client push back failed\n");
        return false;
    }

    if (!redis_client.push_back("test-queue-1", "444"))
    {
        printf("redis client push back failed\n");
        return false;
    }

    if (!redis_client.push_back("test-queue-1", "555"))
    {
        printf("redis client push back failed\n");
        return false;
    }

    std::string str;

    if (!redis_client.pop_front("test-queue-1", str))
    {
        printf("redis client pop front failed\n");
        return false;
    }
    else if ("111" != str)
    {
        printf("redis client pop front exception\n");
        return false;
    }

    if (!redis_client.pop_front("test-queue-1", str))
    {
        printf("redis client pop front failed\n");
        return false;
    }
    else if ("222" != str)
    {
        printf("redis client pop front exception\n");
        return false;
    }

    if (!redis_client.pop_front("test-queue-1", str))
    {
        printf("redis client pop front failed\n");
        return false;
    }
    else if ("333" != str)
    {
        printf("redis client pop front exception\n");
        return false;
    }

    if (!redis_client.pop_front("test-queue-1", str))
    {
        printf("redis client pop front failed\n");
        return false;
    }
    else if ("444" != str)
    {
        printf("redis client pop front exception\n");
        return false;
    }

    if (!redis_client.pop_front("test-queue-1", str))
    {
        printf("redis client pop front failed\n");
        return false;
    }
    else if ("555" != str)
    {
        printf("redis client pop front exception\n");
        return false;
    }

    if (redis_client.pop_front("test-queue-1", str))
    {
        printf("redis client pop front failed\n");
        return false;
    }

    if (!redis_client.set("c:/abc 123 xyz/111", "test data 1"))
    {
        printf("redis client set failed\n");
        return false;
    }

    if (!redis_client.set("c:/abc 123 xyz/222", "test data 2"))
    {
        printf("redis client set failed\n");
        return false;
    }

    if (!redis_client.set("c:/abc 123 xyz/333", "test data 3"))
    {
        printf("redis client set failed\n");
        return false;
    }

    if (!redis_client.set("c:/abc 123 xyz/111", "test data 4"))
    {
        printf("redis client set failed\n");
        return false;
    }

    if (!redis_client.expire("c:/abc 123 xyz/222", 3600))
    {
        printf("redis client expire failed\n");
        return false;
    }

    if (!redis_client.persist("c:/abc 123 xyz/222"))
    {
        printf("redis client persist failed\n");
        return false;
    }

    if (!redis_client.get("c:/abc 123 xyz/111", str))
    {
        printf("redis client get failed\n");
        return false;
    }

    if (!redis_client.find("c:/abc 123 xyz/222"))
    {
        printf("redis client find failed\n");
        return false;
    }

    if (redis_client.find("c:/abc 123 xyz/888"))
    {
        printf("redis client find failed\n");
        return false;
    }

    if (!redis_client.erase("c:/abc 123 xyz/333"))
    {
        printf("redis client erase failed\n");
        return false;
    }

    if (redis_client.erase("c:/abc 123 xyz/999"))
    {
        printf("redis client erase failed\n");
        return false;
    }

#ifdef TEST_CLUSTER
    std::list<std::string> keys;
    if (!redis_client.find("c:/abc 124 xyz/*", keys) || !keys.empty())
    {
        printf("redis client find failed\n");
        return false;
    }

    keys.clear();
    if (!redis_client.find("c:/abc 123 xyz/*", keys) || keys.empty())
    {
        printf("redis client find failed\n");
        return false;
    }

    if (!redis_client.expire(keys, 3600))
    {
        printf("redis client expire failed\n");
        return false;
    }

    if (!redis_client.persist(keys))
    {
        printf("redis client persist failed\n");
        return false;
    }

    if (!redis_client.erase(keys))
    {
        printf("redis client erase failed\n");
        return false;
    }

    if (redis_client.erase(keys))
    {
        printf("redis client erase exception\n");
        return false;
    }

    if (redis_client.expire("not exist", 3600))
    {
        printf("redis client erase exception\n");
        return false;
    }

    if (redis_client.persist("not exist"))
    {
        printf("redis client persist exception\n");
        return false;
    }
#endif // TEST_CLUSTER

    redis_client.exit();

    return true;
}

void test_performance()
{
    const std::string folder("../..");
    std::list<std::string> file_list;

    {
        struct timeval time_beg = get_time();
        get_folder_files(folder, file_list);
        struct timeval time_end = get_time();
        printf("init folder (%s) file count (%u) use time (%u) ms\n", folder.c_str(), static_cast<uint32_t>(file_list.size()), static_cast<uint32_t>(get_time_delta(time_end, time_beg)));
    }

    {
        struct timeval time_beg = get_time();
        set_file_to_redis(file_list, 1, 0);
        struct timeval time_end = get_time();
        printf("set folder (%s) file count (%u) (without expire) use time (%u) ms\n", folder.c_str(), static_cast<uint32_t>(file_list.size()), static_cast<uint32_t>(get_time_delta(time_end, time_beg)));
    }

    {
        struct timeval time_beg = get_time();
        set_file_to_redis(file_list, 2, 36000);
        struct timeval time_end = get_time();
        printf("set folder (%s) file count (%u) (with expire) use time (%u) ms\n", folder.c_str(), static_cast<uint32_t>(file_list.size()), static_cast<uint32_t>(get_time_delta(time_end, time_beg)));
    }

    {
        struct timeval time_beg = get_time();
        get_file_from_redis(file_list, 1, 0);
        struct timeval time_end = get_time();
        printf("get folder (%s) file count (%u) (without expire) use time (%u) ms\n", folder.c_str(), static_cast<uint32_t>(file_list.size()), static_cast<uint32_t>(get_time_delta(time_end, time_beg)));
    }

    {
        struct timeval time_beg = get_time();
        get_file_from_redis(file_list, 2, 36000);
        struct timeval time_end = get_time();
        printf("get folder (%s) file count (%u) (with expire) use time (%u) ms\n", folder.c_str(), static_cast<uint32_t>(file_list.size()), static_cast<uint32_t>(get_time_delta(time_end, time_beg)));
    }

    {
        struct timeval time_beg = get_time();
        find_file_from_redis(file_list, 1);
        struct timeval time_end = get_time();
        printf("find folder (%s) file count (%u) use time (%u) ms\n", folder.c_str(), static_cast<uint32_t>(file_list.size()), static_cast<uint32_t>(get_time_delta(time_end, time_beg)));
    }

    {
        struct timeval time_beg = get_time();
        erase_file_from_redis(file_list, 1);
        struct timeval time_end = get_time();
        printf("erase folder (%s) file count (%u) use time (%u) ms\n", folder.c_str(), static_cast<uint32_t>(file_list.size()), static_cast<uint32_t>(get_time_delta(time_end, time_beg)));
    }

    {
        struct timeval time_beg = get_time();
        clear_file_from_redis(file_list, 2);
        struct timeval time_end = get_time();
        printf("clear folder (%s) file count (%u) use time (%u) ms\n", folder.c_str(), static_cast<uint32_t>(file_list.size()), static_cast<uint32_t>(get_time_delta(time_end, time_beg)));
    }
}

int main(int argc, char * argv[])
{
    if (test_correctness())
    {
        printf("redis client test correctness success\n");
    }
    else
    {
        printf("redis client test correctness failure\n");
    }

    printf("redis client test performance begin\n");
    test_performance();
    printf("redis client test performance end\n");

    return 0;
}
