#pragma once
#include "../utils/dsu.h"
#include "../utils/fast_set.h"
#include "move.h"
#include <array>

enum class Player {
    RED, BLUE, NONE
};

template<int Size>
class Board {
public:
    Board();

    Board(const Board &other) = default;

    void make_move(Move move);

    void undo();

    std::vector<Move> get_legal_moves() const;

    bool is_game_over();

    Player get_turn() const {
        return turn_;
    }

private:
    int get_pos(int row, int col) const {
        return row * Size + col;
    }

    bool is_inside(int row, int col) const {
        return 0 <= row && row < Size && 0 <= col && col < Size;
    }

private:
    struct UndoState {
        Move move;
        int op;
    };

    Player turn_;
    std::array<int, 4> edges_;
    std::vector<Player> board_;
    std::vector<UndoState> states_;
    DSU dsu_;
    FastSet<Size*Size> empty_positions_;
};

template<int Size>
Board<Size>::Board() : board_(Size * Size, Player::NONE), dsu_(Size * Size + 4) {
    turn_ = Player::RED;
    states_.clear();

    for (int i = 0; i < Size * Size; i++)
        empty_positions_.add(i);

    // edge nodes for our DSU
    for (int i = 0; i < 4; i++)
        edges_[i] = Size * Size + i;
}

template<int Size>
void Board<Size>::make_move(Move move) {
    constexpr std::array<std::pair<int, int>, 6> directions = {{
        {0, -1}, {0, 1}, {-1, 0}, {1, 0}, {-1, 1}, {1, -1}
    }};

    auto pos = move.get_pos();
    auto row = pos / Size, col = pos % Size;
    int operations = 0;

    assert(board_[pos] == Player::NONE);

    board_[pos] = turn_;

    for (auto &[dx, dy] : directions) {
        int new_row = row + dx, new_col = col + dy;
        auto new_pos = get_pos(new_row, new_col);

        if (is_inside(new_row, new_col)) {
            // join with adjacent cell
            if (board_[new_pos] == board_[pos])
                operations += dsu_.join(pos, new_pos);
        }
        else {
            // join with edge
            if (new_row < 0)
                operations += dsu_.join(pos, edges_[0]);
            if (new_col < 0)
                operations += dsu_.join(pos, edges_[1]);
            if (new_row >= Size)
                operations += dsu_.join(pos, edges_[2]);
            if (new_col >= Size)
                operations += dsu_.join(pos, edges_[3]);
        }
    }

    turn_ = turn_ == Player::RED ? Player::BLUE : Player::RED;
    states_.emplace_back(move, operations);
    
    // remove current position from the list of empty cells
    empty_positions_.remove(pos);
}

template<int Size>
void Board<Size>::undo() {
    auto [move, operations] = states_.back();
    states_.pop_back();

    while (operations--)
        dsu_.undo();

    empty_positions_.add(move.get_pos());
}

template<int Size>
std::vector<Move> Board<Size>::get_legal_moves() const {
    auto raw_positions = empty_positions_.get();
    std::vector<Move> legal_moves;
    legal_moves.reserve(raw_positions.size());
    for (auto &pos : raw_positions)
        legal_moves.emplace_back(pos);

    return legal_moves;
}

template<int Size>
bool Board<Size>::is_game_over() {
    return dsu_.is_connected(edges_[0], edges_[2]) || 
           dsu_.is_connected(edges_[1], edges_[3]);
}