#pragma once
#include <iostream>
#include <chrono>
#include <ratio>

class SearchLimits {
public:
    SearchLimits() : time_set_(false), max_nodes_(1'000'000), max_time_(0) {}

    void set_max_nodes(std::size_t max_nodes) {
        max_nodes_ = max_nodes;
    }

    void set_max_time(std::size_t max_time) {
        time_set_ = true;
        max_time_ = max_time;
    }

    std::size_t get_max_nodes() const {
        return max_nodes_;
    }

    std::size_t get_max_time() const {
        return max_time_;
    }

    void set_start_time() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    std::size_t get_time_elapsed() const {
        const auto now =  std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
    }

    bool check_time_elapsed() const {
        return time_set_ && get_time_elapsed() > max_time_;
    }
private:
    bool time_set_;
    std::size_t max_nodes_;
    std::size_t max_time_;
    std::chrono::high_resolution_clock::time_point start_time_;
};