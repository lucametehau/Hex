#pragma once
#include "../utils/dsu.h"
#include "../utils/fast_set.h"
#include "move.h"
#include <array>
#include <fstream>
#include <ostream>

enum class Player {
    WHITE, BLACK, NONE
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


    // ngl I'm too lazy to do this myself, credits to Gemini
    friend std::ostream& operator<<(std::ostream &os, const Board &board) {
        // 1. Print column headers (Numbers, based on your Move::to_string)
        os << "  ";
        for (int col = 0; col < Size; ++col) {
            if (col + 1 < 10) os << " "; // Padding for single digits
            os << (col + 1) << " ";
        }
        os << "\n";

        // 2. Print each row
        for (int row = 0; row < Size; ++row) {
            // Indent to create the slanted Hex shape
            for (int i = 0; i < row; ++i) {
                os << " ";
            }

            // Print row label (Letters, based on your Move::to_string)
            os << char('a' + row) << "  ";

            // Print the cells
            for (int col = 0; col < Size; ++col) {
                Player p = board.get_cell(row, col);
                
                if (p == Player::WHITE) {
                    os << "W  ";
                } else if (p == Player::BLACK) {
                    os << "B  ";
                } else {
                    os << ".  "; // Empty space
                }
            }
            os << "\n";
        }
        return os;
    }

private:
    int get_pos(int row, int col) const {
        return row * Size + col;
    }

    bool is_inside(int row, int col) const {
        return 0 <= row && row < Size && 0 <= col && col < Size;
    }

    Player get_cell(int row, int col) const {
        return board_[get_pos(row, col)];
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
    turn_ = Player::BLACK;
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

    // std::cout << move.to_string(Size) << " " << pos << "\n";

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
            if (new_row < 0 && turn_ == Player::WHITE)
                operations += dsu_.join(pos, edges_[0]);
            if (new_col < 0 && turn_ == Player::BLACK)
                operations += dsu_.join(pos, edges_[1]);
            if (new_row >= Size && turn_ == Player::WHITE)
                operations += dsu_.join(pos, edges_[2]);
            if (new_col >= Size && turn_ == Player::BLACK)
                operations += dsu_.join(pos, edges_[3]);
        }
    }

    turn_ = turn_ == Player::WHITE ? Player::BLACK : Player::WHITE;
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

    auto pos = move.get_pos();
    empty_positions_.add(pos);
    board_[pos] = Player::NONE;
    turn_ = turn_ == Player::WHITE ? Player::BLACK : Player::WHITE;
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