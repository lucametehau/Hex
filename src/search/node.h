#pragma once
#include "../hex/move.h"
#include <vector>

class Node {
public:
    Node() = default;
    Node(std::size_t parent_index, Move move) : parent_index_(parent_index), move_(move) {
        visits_ = wins_ = 0;
        children_indexes_.clear();
    }

    std::size_t get_parent() const {
        return parent_index_;
    }

    bool is_expanded() const {
        return !children_indexes_.empty();
    }

    bool is_terminal() const {
        return false;
    }

    Move get_move() const {
        return move_;
    }

    void add_child(std::size_t idx) {
        children_indexes_.push_back(idx);
    }

    std::size_t size() const {
        return children_indexes_.size();
    }

    std::size_t at(std::size_t idx) const {
        return children_indexes_[idx];
    }

    float get_wins() const {
        return wins_;
    }

    std::size_t get_visits() const {
        return visits_;
    }

    void update_stats(float score) {
        visits_++;
        wins_ += score;
    }

private:
    float wins_;
    std::size_t visits_;
    std::size_t parent_index_;
    Move move_;
    std::vector<std::size_t> children_indexes_;
};