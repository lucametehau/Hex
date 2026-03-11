#include "search.h"
#include <random>
#include <iostream>


std::mt19937 sed(time(0));
std::uniform_int_distribution<> rng;

std::size_t Searcher::push_node(std::size_t parent_index, Move move) {
    auto idx = nodes_++;
    tree_[idx] = Node(parent_index, move);
    return idx;
}

void Searcher::iteration() {
    board_ = root_board_;
    const auto node_idx = select();

    if (!expand(node_idx))
        return;

    auto score = play(node_idx);

    backprop(node_idx, score);
}

std::size_t Searcher::select() {
    /*
    Explore a non-terminal path and select it for expansion.
    */

    std::size_t node_idx = 0;

    while (true) {
        if (tree_[node_idx].is_terminal() || !tree_[node_idx].is_expanded())
            return node_idx;

        // pick children with best exploration score?
        // random for now
        std::size_t best_child = 0;
        float best_score = -1;
        auto parent_visits = tree_[node_idx].get_visits();
        for (std::size_t i = 0; i < tree_[node_idx].size(); i++) {
            auto child_node_idx = tree_[node_idx].at(i);
            auto visits = tree_[child_node_idx].get_visits();
            auto wins = tree_[child_node_idx].get_wins();

            if (!parent_visits) {
                best_child = child_node_idx;
                break;
            }

            // UCT
            float score = !visits ? 2 : 1.414f * std::sqrtf(std::log(parent_visits) / visits) + wins / visits;

            if (score > best_score) {
                best_score = score;
                best_child = child_node_idx;
            }
        }

        auto new_node_idx = best_child;
        
        board_.make_move(tree_[new_node_idx].get_move());
        node_idx = new_node_idx;
    }
}

bool Searcher::expand(std::size_t node_idx) {
    auto &node = tree_[node_idx];

    if (node.is_terminal())
        return false;

    auto moves = board_.get_legal_moves();

    if (nodes_ + moves.size() >= max_nodes_)
        return false;

    for (auto &move : moves) {
        node.add_child(push_node(node_idx, move));
    }

    return true;
}

float Searcher::play(std::size_t node_idx) {
    auto cur = board_.get_turn();
    while (!board_.is_game_over()) {
        auto moves = board_.get_legal_moves();
        board_.make_move(moves[rng(sed) % moves.size()]);
    }
    return cur == board_.get_turn() ? 0.0f : 1.0f;
}

void Searcher::backprop(std::size_t node_idx, float score) {
    while (node_idx != inf) {
        tree_[node_idx].update_stats(score);
        node_idx = tree_[node_idx].get_parent();
        score = 1.0 - score;
    }
}

void Searcher::search() {
    int iterations = 0;
    constexpr int max_iterations = 1'000;

    while (iterations < max_iterations) {
        iteration();
        iterations++;
    }

    std::size_t most_visits = 0;
    std::size_t best_child = 0;
    for (std::size_t i = 0; i < tree_[0].size(); i++) {
        auto child_node_idx = tree_[0].at(i);
        auto visits = tree_[child_node_idx].get_visits();

        std::cout << tree_[child_node_idx].get_move().get_pos() << " has " << visits << "  visits\n";

        if (visits > most_visits) {
            most_visits = visits;
            best_child = child_node_idx;
        }
    }

    auto move = tree_[best_child].get_move().get_pos();

    std::cout << "Playing " << move / BOARD_SIZE << " " << move % BOARD_SIZE << "\n";
}