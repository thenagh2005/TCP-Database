#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <chrono>
#include "zset.h"



namespace kv
{
    class KVStore
    {
    public:
        KVStore(size_t shards = 16);

        //Regular operations

        void set(const std::string &key, const std::string &value);
        std::optional<std::string> get(const std::string &key) const;
        bool del(const std::string &key);
        bool exists(const std::string &key) const;

        //Sorted set operations

        bool zadd(const std::string &key, const std::string &member, double score);
        std::optional<double> zscore(const std::string &key, const std::string &member) const;
        std::optional<int> zrank(const std::string &key, const std::string &member) const;
        std::vector<std::pair<std::string, double>> zrange(const std::string &key, int start, int stop) const;
        bool zrem(const std::string &key, const std::string &member);
        size_t zsize(const std::string &key) const;





        void setWithTTL(const std::string &key, const std::string &value, std::chrono::seconds ttl);
        std::optional<std::string> getWithTTL(const std::string &key) const;



    private:
        struct Shard
        {
            mutable std::shared_mutex mutex;
            std::unordered_map<std::string, std::string> data;
            std::unordered_map<std::string, ZSet> sorted_sets;

        };

        std::vector<Shard> shards_;
        size_t num_shards_;
        size_t getShard(const std::string &key) const;
    };

}