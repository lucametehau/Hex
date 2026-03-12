#include "search.h"
#include "limits.h"
#include <limits>
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

    if (!board_.is_game_over() && !expand(node_idx))
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
        const auto node = tree_[node_idx];
        if (board_.is_game_over() || !node.is_expanded())
            return node_idx;

        // pick children with best exploration score?
        // random for now
        std::size_t best_child = 0;
        float best_score = -1;
        auto parent_visits = tree_[node_idx].get_visits();
        for (std::size_t i = 0; i < node.size(); i++) {
            auto child_node_idx = node.at(i);
            auto visits = tree_[child_node_idx].get_visits();
            auto wins = tree_[child_node_idx].get_wins();

            if (!parent_visits) {
                best_child = child_node_idx;
                break;
            }

            // UCT
            float score = !visits ? std::numeric_limits<float>::infinity() : 1.414f * std::sqrtf(std::log(parent_visits) / visits) + wins / visits;

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
    /*
    Add sons of selected leaf to the search tree.
    */
    auto &node = tree_[node_idx];

    auto moves = board_.get_legal_moves();

    if (nodes_ + moves.size() >= tree_.size())
        return false;

    bool set_first = false;
    for (auto &move : moves) {
        const auto child_node_idx = push_node(node_idx, moves[0]);
        if (!set_first) {
            node.add_first_child(child_node_idx);
            set_first = true;
        }
        node.add_child();
    }

    return true;
}

float Searcher::play(std::size_t node_idx) {
    /*
    Play out random game starting from selected leaf.
    */
    auto cur = board_.get_turn();
    while (!board_.is_game_over()) {
        auto moves = board_.get_legal_moves();
        board_.make_move(moves[rng(sed) % moves.size()]);
    }
    return cur == board_.get_turn() ? 1.0f : 0.0f;
}

void Searcher::backprop(std::size_t node_idx, float score) {
    /*
    Backpropagate result of play out on the move chain.
    */
    while (node_idx != inf) {
        tree_[node_idx].update_stats(score);
        node_idx = tree_[node_idx].get_parent();
        score = 1.0 - score;
    }
}

std::pair<Move, float> Searcher::search(Board<BOARD_SIZE> &board, SearchLimits &limits) {
    limits.set_start_time();
    nodes_ = 0;
    board_ = root_board_ = board;
    tree_.resize(limits.get_max_nodes());

    // root node
    push_node(inf, Move(0));

    int iterations = 0;
    constexpr int max_iterations = 1'000'000;

    while (iterations < max_iterations && !limits.check_time_elapsed()) {
        iteration();
        iterations++;
    }

    std::size_t most_visits = 0;
    std::size_t best_child = 0;
    const auto &root_node = tree_[0];
    for (std::size_t i = 0; i < root_node.size(); i++) {
        auto child_node_idx = root_node.at(i);
        auto visits = tree_[child_node_idx].get_visits();

        if (visits > most_visits) {
            most_visits = visits;
            best_child = child_node_idx;
        }
    }

    std::cout << std::format(
        "Searched {} nodes and {} iterations for {} seconds\n", nodes_, iterations, limits.get_time_elapsed() / 1000.0
    );
    
    return std::make_pair(tree_[best_child].get_move(), 1.0 * tree_[best_child].get_wins() / tree_[best_child].get_visits());
}