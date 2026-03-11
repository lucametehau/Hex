#pragma once
#include "node.h"
#include "limits.h"
#include "../hex/board.h"
#include <limits>

constexpr std::size_t inf = std::numeric_limits<std::size_t>::max();
constexpr int BOARD_SIZE = 7;

class Searcher {
public:
    Searcher() = default;

    std::pair<Move, float> search(Board<BOARD_SIZE> &board, SearchLimits &limits);

private:
    std::size_t select();

    bool expand(std::size_t node_idx);

    float play(std::size_t node_idx);

    void backprop(std::size_t node_idx, float score);

    void iteration();

    std::size_t push_node(std::size_t parent_index, Move move);


private:
    Board<BOARD_SIZE> board_, root_board_;
    std::vector<Node> tree_;
    std::size_t nodes_;
};