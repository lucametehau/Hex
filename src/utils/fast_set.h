#pragma once
#include <vector>
#include <cassert>

// keeps track of a set of elements from {0, 1, ... ,MaxValue-1}
template<int MaxValue>
class FastSet {
public:
    FastSet() {
        std::fill(index_.begin(), index_.end(), -1);
    }

    void add(int x) {
        assert (x < MaxValue);
        if (index_[x] != -1)
            return;

        index_[x] = data_.size();
        data_.push_back(x);
    }

    void remove(int x) {
        assert (x < MaxValue);
        if (index_[x] == -1)
            return;

        auto pos = index_[x];
        std::swap(data_.back(), data_[pos]);
        data_.pop_back();
        index_[data_[pos]] = pos;
        index_[x] = -1;
    }

    std::vector<int> get() const {
        return data_;
    }

private:
    std::vector<int> data_;
    std::array<int, MaxValue> index_;
};