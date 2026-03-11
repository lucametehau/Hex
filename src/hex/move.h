#pragma once
#include <fstream>

class Move {
public:
    Move() = default;

    Move(int row, int col, int size) : pos_(row * size + col) {}

    Move(int pos) :pos_(pos) {}

    int get_pos() const {
        return pos_;
    }

private:
    int pos_;
};