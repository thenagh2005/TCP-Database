#pragma once
#include <string>
#include <optional>
#include <unordered_map>
#include <shared_mutex>
#include <vector>

namespace kv
{
    class KVStore
    {
    public:
        KVStore(size_t shards = 16);

        void set(const std::string &key, const std::string &value);
        std::optional<std::string> get(const std::string &key) const;
        bool del(const std::string &key);
        bool exists(const std::string &key) const;

    private:
        struct Shard
        {
            mutable std::shared_mutex mutex;
            std::unordered_map<std::string, std::string> data;

        };

        std::vector<Shard> shards_;
        size_t num_shards_;
        size_t getShard(const std::string &key) const;
    };

}