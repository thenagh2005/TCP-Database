#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include <shared_mutex>
#include <vector>
#include <chrono>
#include <random>


namespace kv {
    struct ZSetNode {
        std::string member;
        double score;
        std::vector<ZSetNode*> forward;

        ZSetNode(int level, const std::string &member, double score) : member(member), score(score), forward(level + 1, nullptr) {}
    };

    class ZSet {
        public:

            ZSet(int max_level = 16);
            ~ZSet();

            bool add(const std::string &member, double score);
            bool remove(const std::string &member);
            std::optional<double> score(const std::string &member) const;
            std::optional<int> rank(const std::string &member) const;
            std::vector<std::pair<std::string, double>> range(int start, int stop) const;
            size_t size() const;
        
        private:
            ZSetNode *head_;
            int max_level_;
            int current_level_;
            size_t length_;
            std::unordered_map<std::string, ZSetNode*> node_map_;
            std::mt19937 rng_;
            int randomLevel() const;
            ZSetNode* findNode(const std::string &member, double score) const;
    };
}