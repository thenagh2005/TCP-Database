#include "kv/kvstore.h"
#include <functional>

namespace kv {
    KVStore::KVStore(size_t shards) : num_shards_(shards), shards_(shards) {}

    size_t KVStore::getShard(const std::string& key) const {
        return std::hash<std::string>{}(key) % num_shards_;
    }

    void KVStore::set(const std::string& key, const std::string& value) {
        size_t shard_index = getShard(key);
        auto& shard = shards_[shard_index];
        std::unique_lock lock(shard.mutex);
        shard.data[key] = value;
    }

    std::optional<std::string> KVStore::get(const std::string& key) const {
        size_t shard_index = getShard(key);
        const auto& shard = shards_[shard_index];
        std::shared_lock lock(shard.mutex);
        auto it = shard.data.find(key);
        if (it != shard.data.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool KVStore::del(const std::string& key) {
        size_t shard_index = getShard(key);
        auto& shard = shards_[shard_index];
        std::unique_lock lock(shard.mutex);
        return shard.data.erase(key) > 0;
    }

    bool KVStore::exists(const std::string& key) const {
        size_t shard_index = getShard(key);
        const auto& shard = shards_[shard_index];
        std::shared_lock lock(shard.mutex);
        return shard.data.find(key) != shard.data.end();
    }
    
}