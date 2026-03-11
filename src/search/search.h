#pragma once
#include "node.h"
#include "../hex/board.h"
#include <limits>

constexpr std::size_t inf = std::numeric_limits<std::size_t>::max();
constexpr int BOARD_SIZE = 11;

class Searcher {
public:
    Searcher() = delete;
    Searcher(Board<BOARD_SIZE>& board, std::size_t max_nodes = 30'000'000) : board_(board), root_board_(board), tree_(max_nodes), nodes_(0), max_nodes_(max_nodes) {
        // root node
        push_node(inf, Move(0));
    }

    std::pair<Move, float> search();

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
    std::size_t nodes_, max_nodes_;
};