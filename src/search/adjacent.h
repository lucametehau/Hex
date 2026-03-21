#pragma once
#include <array>
#include <vector>
#include <cassert>

const int INF = 10'000'000;

class AdjacentValues {
    public:

    AdjacentValues();

    int get_value(int adjacent) {
        assert(adjacent < 729);
        assert(adjacent >= 0);
        return values_[adjacent];
    }

    private:

    std::array<int, 729> values_;

    void set_value(int adjacent, int value) {
        assert(adjacent < 729);
        assert(adjacent >= 0);
        values_[adjacent] = value;
    }

    bool ones_connected(const std::array<int, 6>& s) {
        int starts = 0;
        for (int i = 0; i < 6; i++) {
            int prev = s[(i + 5) % 6];
            if (s[i] == 1 && prev != 1)
                starts++;
        }
        return starts <= 1;
    }

    bool check_with_zero(const std::array<int, 6>& s) {
        std::vector<int> zeros;
        for (int i = 0; i < 6; i++)
            if (s[i] == 0)
                zeros.push_back(i);

        for (int special : zeros) {
            for (int val = 1; val <= 2; val++) {
                bool ok = true;
                std::array<int, 6> base = s;
                base[special] = val;

                int k = zeros.size();
                for (int mask = 0; mask < (1 << k); mask++) {
                    std::array<int, 6> t = base;

                    for (int j = 0; j < k; j++) {
                        int idx = zeros[j];
                        if (idx == special) continue;
                        t[idx] = ((mask >> j) & 1) ? 2 : 1;
                    }

                    if (!ones_connected(t)) {
                        ok = false;
                        break;
                    }
                }
                if(ok)
                    return true;
            }
        }
        return false;
    }

    bool is_useless(const std::array<int, 6>& s) {
        bool has_zero = false, has_one = false;
        for (int x : s) {
            if (x == 0) has_zero = true;
            if (x == 1) has_one = true;
        }

        if (!has_zero)
            return ones_connected(s);
        else
            return check_with_zero(s);
    }

    /*
    generates all adjacent masks that are useless

    a mask is considered useless if either:
    1. there's no 0s and all the 1s are connected
    2. there exists a 0 such that setting it either to 1 or to 2 makes it so that all
       ways of setting the other 0s to 1 or 2 ends up in all the 1s being connected
    

    reasoning:
    1. is obvious
    2. we have 2 cases:
       - if there exists a 0 such that setting it to 2 gives the above property, then
         the opponent can just respond with that 2, making the center move useless
         and gaining territory
       - if there exists a 0 such that setting it to 1 gives the above property, then
         playing in that neighboring position instead of the center gives the same 
         outcome while gaining more territory than the center
    */
    void generate_all_useless() {
        for (int mask = 0; mask < 729; mask++) {
            int x = mask;
            std::array<int, 6> s;

            for (int i = 0; i < 6; i++) {
                s[i] = x % 3;
                x /= 3;
            }

            if (is_useless(s)) {
                set_value(mask, -INF);
            }
        }
    }
};

inline AdjacentValues::AdjacentValues() {
    values_.fill(0);
    generate_all_useless();
}
