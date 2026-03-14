#include "search.h"
#include "limits.h"
#include <limits>
#include <random>
#include <iostream>

std::uniform_int_distribution<> rng;
const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count() ^ (uintptr_t)&rng;
std::mt19937 sed(seed);

std::size_t Searcher::push_node(std::size_t parent_index, Move move) {
    auto idx = nodes_++;
    tree_[idx] = Node(parent_index, move);
    return idx;
}

void Searcher::iteration() {
    board_ = root_board_;
    std::fill(playout_played_by_.begin(), playout_played_by_.end(), Player::NONE);

    const auto node_idx = select();

    if (!board_.is_game_over() && tree_[node_idx].get_visits() && !expand(node_idx))
        return;

    const auto node_turn = board_.get_turn();

    auto score = play(node_idx);

    backprop(node_idx, node_turn, score);
}

float Searcher::get_score(
    std::size_t parent_visits, std::size_t visits, float wins, std::size_t visits_amaf, float wins_amaf, 
    float policy,  float parent_exploit
) {
    // UCT + RAVE + FPU
    const float fpu_constant = !parent_visits ? FPU_CONSTANT : 1.0f - parent_exploit;
    float exploit = fpu_constant;

    if (visits || visits_amaf) {
        const float exploit_normal = !visits ? fpu_constant : wins / visits;
        const float exploit_amaf = !visits_amaf ? fpu_constant : wins_amaf / visits_amaf;

        const float beta = visits_amaf / (visits_amaf + visits + visits_amaf * visits / 10000.0);

        exploit = (1.0 - beta) * exploit_normal + beta * exploit_amaf;
    }

    const float exploration = EXPLORATION_CONSTANT * policy * std::sqrtf(std::log(parent_visits) / (visits + 1));

    return exploit + exploration;
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

        // pick next best children
        std::size_t best_child = 0;
        float best_score = -1;
        auto parent_visits = node.get_visits();
        for (std::size_t i = 0; i < node.size(); i++) {
            const auto child_node_idx = node.at(i);
            const auto child_node = tree_[child_node_idx];
            const auto visits = child_node.get_visits();
            const auto wins = child_node.get_wins();
            const auto visits_amaf = child_node.get_visits_amaf();
            const auto wins_amaf = child_node.get_wins_amaf();
            const auto policy = 1.0f / node.size();

            if (!parent_visits) {
                best_child = child_node_idx;
                break;
            }

            const float score = get_score(parent_visits, visits, wins, visits_amaf, wins_amaf, policy, 1.0 * node.get_wins() / node.get_visits());

            if (score > best_score) {
                best_score = score;
                best_child = child_node_idx;
            }
        }

        const auto new_node_idx = best_child;
        const auto move = tree_[new_node_idx].get_move();
        
        playout_played_by_[move.get_pos()] = board_.get_turn();
        board_.make_move(move);
        node_idx = new_node_idx;
    }
}

bool Searcher::expand(std::size_t node_idx) {
    /*
    Add sons of selected leaf to the search tree.
    */
    auto &node = tree_[node_idx];

    const auto moves = board_.get_legal_moves();

    if (nodes_ + moves.size() >= tree_.size())
        return false;

    bool set_first = false;
    for (auto &move : moves) {
        const auto child_node_idx = push_node(node_idx, move);
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
    const auto cur = board_.get_turn();
    while (!board_.is_game_over()) {
        const auto moves = board_.get_legal_moves();
        const auto move = moves[rng(sed) % moves.size()];
        playout_played_by_[move.get_pos()] = board_.get_turn();
        board_.make_move(move);
    }
    return cur == board_.get_turn() ? 1.0f : 0.0f;
}

void Searcher::backprop(std::size_t node_idx, Player turn, float score) {
    /*
    Backpropagate result of playout on the move chain.
    */
    while (node_idx != inf) {
        tree_[node_idx].update_stats(score);
        const auto parent_idx = tree_[node_idx].get_parent();

        // RAVE updates / AMAF updates
        // look at siblings which play a move we played in the playout
        // and update the move's stats
        // kind of like history heuristic in chess, but more local, not global
        if (parent_idx != inf) {
            turn = turn == Player::WHITE ? Player::BLACK : Player::WHITE;

            auto &parent_node = tree_[parent_idx];
            for (std::size_t i = 0; i < parent_node.size(); i++) {
                const auto child_node_idx = parent_node.at(i);
                const auto pos = tree_[child_node_idx].get_move().get_pos();

                if (playout_played_by_[pos] == turn)
                    tree_[child_node_idx].update_amaf_stats(score);
            }
        }

        node_idx = parent_idx;
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

    std::cerr << std::format(
        "Searched {} nodes and {} iterations for {} seconds\n", nodes_, iterations, limits.get_time_elapsed() / 1000.0
    );
    
    return std::make_pair(tree_[best_child].get_move(), 1.0 * tree_[best_child].get_wins() / tree_[best_child].get_visits());
}