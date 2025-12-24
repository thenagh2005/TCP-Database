#include "zset.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace kv
{
    ZSet::ZSet(int max_level) : rng_(std::random_device{}())
    {
        max_level_ = max_level;
        current_level_ = 0;
        length_ = 0;
        head_ = new ZSetNode(max_level_, "", -std::numeric_limits<double>::infinity());
        std::random_device rd;
    }

    ZSet::~ZSet()
    {
        ZSetNode *current = head_;

        while (current != nullptr)
        {
            ZSetNode *next = current->forward[0];
            delete current;
            current = next;
        }
    }

    int ZSet::randomLevel() const
    {

        int level = 1;

        std::uniform_real_distribution<double> dist(0.0, 1.0);

        while (dist(const_cast<std::mt19937 &>(rng_)) < 0.5 && level < max_level_)
        {
            level++;
        }
        return level;
    }

    bool ZSet::add(const std::string &member, double score)
    {
        auto it = node_map_.find(member);

        if (it != node_map_.end())
        {
            // Member already exists, update score
            double old_score = it->second->score;

            if (old_score == score)
            {
                return false; // No change needed
            }

            remove(member);
        }

        // New member

        std::vector<ZSetNode *> update(max_level_, nullptr);
        ZSetNode *current = head_;

        for (int i = current_level_; i >= 0; i--)
        {
            while (current->forward[i] != nullptr &&
                   (current->forward[i]->score < score ||
                    (current->forward[i]->score == score && current->forward[i]->member < member)))
            {
                current = current->forward[i];
            }
            update[i] = current;
        }

        int height = randomLevel();

        // Now add the new node to the skip list

        if (height > current_level_)
        {
            for (int i = current_level_ + 1; i < height; i++)
            {
                update[i] = head_;
            }

            current_level_ = height;
        }

        ZSetNode *new_node = new ZSetNode(height, member, score);

        for (int i = 0; i < height; i++)
        {
            new_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = new_node;
        }

        node_map_[member] = new_node;
        length_++;

        return true;
    }

    bool ZSet::remove(const std::string &member)
    {
        // first check if the member exists

        auto it = node_map_.find(member);

        if (it != node_map_.end())
        {
            // It exists, lets remove it

            ZSetNode *node_to_remove = it->second; // get the node to remove
            double score = node_to_remove->score;

            // Now we remove

            std::vector<ZSetNode *> update(max_level_, nullptr);
            ZSetNode *current = head_;

            for (int i = current_level_; i >= 0; i--)
            {
                while (current->forward[i] != nullptr &&
                       (current->forward[i]->score < score ||
                        (current->forward[i]->score == score && current->forward[i]->member < member)))
                {
                    current = current->forward[i];
                }
                update[i] = current;
            }

            current = current->forward[0];

            if (current == nullptr || current->member != member)
            {
                return false;
            }

            // Do stuff here

            for (int i = 0; i < current_level_; i++) {
                if (update[i]->forward[i] != current) {
                    break;
                }

                update[i]->forward[i] = current->forward[i];
            }

            while (current_level_ > 0 && head_->forward[current_level_] == nullptr) {
                current_level_--;
            }

            node_map_.erase(it); // remove from map

            delete current; // free memory
            length_--;
            return true;

            // Now handle the skip list pointers
        }

        // It does not exist
        return false;
    }

    std::optional<double> ZSet::score(const std::string &member) const
    {
        auto it = node_map_.find(member);

        if (it != node_map_.end())
        {
            return it->second->score;
        }
        return std::nullopt;
    }

    std::optional<int> ZSet::rank(const std::string &member) const
    {
        if (node_map_.find(member) == node_map_.end())
        {
            return std::nullopt;
        }

        int rank = 0;
        double member_score = node_map_.at(member)->score;

        ZSetNode *current = head_->forward[0];

        while (current != nullptr)
        {
            if (current->score < member_score)
            {
                rank++;
                current = current->forward[0];
            }
            else if (current->score == member_score && current->member == member)
            {
                return rank;
            }
            else if (current->score == member_score)
            {
                if (current->member < member)
                {
                    rank++;
                }
                current = current->forward[0];
            }
            else
            {
                break;
            }
        }

        return rank;
    }

    std::vector<std::pair<std::string, double>> ZSet::range(int start, int stop) const
    {
        return {};
    }

    ZSetNode *ZSet::findNode(const std::string &member, double score) const
    {
        //Start looking for the node from the head

        ZSetNode *current = head_;

        //Search da skip list whoooooooooo

        for (int i = current_level_; i >= 0; i--)
            {
                while (current->forward[i] != nullptr &&
                       (current->forward[i]->score < score ||
                        (current->forward[i]->score == score && current->forward[i]->member < member)))
                {
                    current = current->forward[i];
                }
                
            }
        
            current = current->forward[0];

            if (current != nullptr && current->score == score &&current->member == member)
            {
                return current;
            }

            return nullptr;

        
        
        


    }
}
