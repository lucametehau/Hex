#pragma once
#include <vector>
#include <iostream>

// Disjoint set union
class DSU {
public:
    DSU() = default;

    DSU(std::size_t n) : par_(n), sz_(n, 1) {
        for (int i = 0; i < n; i++)
            par_[i] = i;
    }

    bool join(int x, int y) {
        // std::cout << "joining " << x << " with " << y << "\n";
        x = get_par(x);
        y = get_par(y);

        if (x == y)
            return false;

        if (sz_[x] > sz_[y])
            std::swap(x, y);

        states_.emplace_back(x, y, sz_[y], par_[x]);
        par_[x] = y;
        sz_[y] += sz_[x];
        return true;
    }

    // undoes the last operation
    void undo() {
        auto [x, y, old_sz_y, old_par_x] = states_.back();
        states_.pop_back();

        sz_[y] = old_sz_y;
        par_[x] = old_par_x;
    }

    bool is_connected(int x, int y) {
        return get_par(x) == get_par(y);
    }

private:
    int get_par(int x) {
        if (x == par_[x])
            return x;
        return get_par(par_[x]);
    }

private:
    struct UndoState {
        int x, y;
        int old_sz_y, old_par_x;
    };

    std::vector<int> par_;
    std::vector<int> sz_;
    std::vector<UndoState> states_;
};