#pragma once
#include <fstream>
#include <string>

class Move {
public:
    Move() = default;

    Move(int row, int col, int size) : pos_(row * size + col) {}

    Move(int pos) :pos_(pos) {}

    int get_pos() const {
        return pos_;
    }

    std::string to_string(const int size) const {
        std::string cell;
        cell += char('a' + pos_ / size);
        cell += std::to_string(1 + pos_ % size);
        return cell;
    }

private:
    int pos_;
};