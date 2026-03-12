#pragma once
#include "../hex/move.h"
#include <vector>

class Node {
public:
    Node() = default;
    Node(std::size_t parent_index, Move move) : parent_index_(parent_index), move_(move), num_children_(0) {
        visits_ = wins_ = 0;
    }

    std::size_t get_parent() const {
        return parent_index_;
    }

    bool is_expanded() const {
        return num_children_;
    }

    Move get_move() const {
        return move_;
    }

    void add_first_child(std::size_t idx) {
        first_children_index_ = idx;
    }

    void add_child() {
        num_children_++;
    }

    std::size_t size() const {
        return num_children_;
    }

    std::size_t at(std::size_t idx) const {
        return first_children_index_ + idx;
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

    // we assume that the children are consecutive indices
    std::size_t first_children_index_;
    std::size_t num_children_;
};