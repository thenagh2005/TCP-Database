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

    bool KVStore::zadd(const std::string& key, const std::string& member, double score) {
        size_t shard_index = getShard(key);
        auto& shard = shards_[shard_index];
        std::unique_lock lock(shard.mutex);
        return shard.sorted_sets[key].add(member, score);
    }

    bool KVStore::zrem(const std::string& key, const std::string& member) {
        size_t shard_index = getShard(key);
        auto& shard = shards_[shard_index];
        std::unique_lock lock(shard.mutex);
        return shard.sorted_sets[key].remove(member);
    }

    std::optional<double> KVStore::zscore(const std::string& key, const std::string& member) const {
        size_t shard_index = getShard(key);
        const auto& shard = shards_[shard_index];
        std::shared_lock lock(shard.mutex);
        auto it = shard.sorted_sets.find(key);
        if (it != shard.sorted_sets.end()) {
            return it->second.score(member);
        }
        return std::nullopt;
    }

    //Now zrank, zrange, and zsize

    size_t KVStore::zsize(const std::string& key) const {
        size_t shard_index = getShard(key);
        const auto& shard = shards_[shard_index];
        std::shared_lock lock(shard.mutex);
        auto it = shard.sorted_sets.find(key);
        if (it != shard.sorted_sets.end()) {
            return it->second.size();
        }
        return 0;
    }

    std::vector<std::pair<std::string, double>> KVStore::zrange(const std::string& key, int start, int stop) const {
        size_t shard_index = getShard(key);
        const auto& shard = shards_[shard_index];
        std::shared_lock lock(shard.mutex);

        auto it = shard.sorted_sets.find(key);
        if (it != shard.sorted_sets.end()) {
            return it->second.range(start, stop); 
        }
        return {};

    }

    std::optional<int> KVStore::zrank(const std::string& key, const std::string& member) const {
        size_t shard_index = getShard(key);
        const auto& shard = shards_[shard_index];
        std::shared_lock lock(shard.mutex);
        auto it = shard.sorted_sets.find(key);
        if (it != shard.sorted_sets.end()) {
            return it->second.rank(member);
        }
        return std::nullopt;
    }

    std::vector<std::pair<std::string, std::string>> KVStore::all_entries() const {
        std::vector<std::pair<std::string, std::string>> entries;

        for (const auto& shard : shards_) {
            std::shared_lock lock(shard.mutex);
            
            // String key-value pairs
            for (const auto& [key, value] : shard.data) {
                entries.emplace_back("STRING:" + key, value);
            }
            
            // Sorted set entries
            for (const auto& [key, zset] : shard.sorted_sets) {
                auto members = zset.all(); // Get all members
                for (const auto& [member, score] : members) {
                    entries.emplace_back("ZSET:" + key + ":" + member, std::to_string(score));
                }
            }
        }

        return entries;
    }

    

}