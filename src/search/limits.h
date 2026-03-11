#pragma once
#include <iostream>

class SearchLimits {
public:
    SearchLimits() : max_nodes_(1'000'000), max_time_(1'000) {}

    void set_max_nodes(std::size_t max_nodes) {
        max_nodes_ = max_nodes;
    }

    void set_max_time(std::size_t max_time) {
        max_time_ = max_time;
    }

    std::size_t get_max_nodes() const {
        return max_nodes_;
    }

    std::size_t get_max_time() const {
        return max_time_;
    }
private:
    std::size_t max_nodes_;
    std::size_t max_time_;
};