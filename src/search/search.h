#pragma once
#include "node.h"
#include "limits.h"
#include "../hex/board.h"
#include <limits>

constexpr std::size_t inf = std::numeric_limits<std::size_t>::max();

constexpr int BOARD_SIZE = 13;

constexpr float FPU_CONSTANT = 1.0f;
constexpr float EXPLORATION_CONSTANT = 1.414f;

class Searcher {
public:
    Searcher() = default;

    std::pair<Move, float> search(Board<BOARD_SIZE> &board, SearchLimits &limits);

private:
    float get_score(std::size_t parent_visits, std::size_t visits, float wins, std::size_t visits_amaf, float wins_amaf) const;

    std::size_t select();

    bool expand(std::size_t node_idx);

    float play(std::size_t node_idx);

    void backprop(std::size_t node_idx, Player turn, float score);

    void iteration();

    std::size_t push_node(std::size_t parent_index, Move move);


private:
    Board<BOARD_SIZE> board_, root_board_;
    std::vector<Node> tree_;
    std::size_t nodes_;

    std::array<Player, BOARD_SIZE * BOARD_SIZE> playout_played_by_;
};