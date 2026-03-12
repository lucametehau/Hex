#include "uhi.h"
#include "../search/search.h"
#include <iostream>
#include <sstream>

UHI::UHI() {
    // create command map
    commands_["boardsize"] = {};
    commands_["clear_board"] = [&](std::istringstream&) {
        board_ = Board<BOARD_SIZE>();
    };
    commands_["showboard"] = [&](std::istringstream&) {
        std::cout << board_ << "\n";
    };
    commands_["final_score"] = [&](std::istringstream&) {
        if (board_.is_game_over())
            std::cout << (board_.get_turn() == Player::WHITE ? "B+" : "W+") << "\n";
        else
            std::cout << "cannot score\n";
    };
    commands_["play"] = [&](std::istringstream &iss) {
        std::string move;
        iss >> move;
        board_.make_move(Move(move, BOARD_SIZE));
    };
    commands_["reg_genmove"] = [&](std::istringstream &iss) {
        std::string player;
        iss >> player;
        assert ((player == "white" ? Player::WHITE : Player::BLACK) == board_.get_turn());

        const auto [move, score] = searcher_.search(board_, limits_);

        std::cout << move.to_string(BOARD_SIZE) << "\n";

        std::cout << "Playing with score of " << 100.0 * score << "%\n";
    };
    commands_["genmove"] = [&](std::istringstream &iss) {
        std::string player;
        iss >> player;
        assert ((player == "white" ? Player::WHITE : Player::BLACK) == board_.get_turn());

        const auto [move, score] = searcher_.search(board_, limits_);

        std::cout << move.to_string(BOARD_SIZE) << "\n";

        std::cout << "Playing with score of " << 100.0 * score << "%\n";

        board_.make_move(move);
    };
    commands_["undo"] = [&](std::istringstream&) {
        board_.undo();
    };
    commands_["all_legal_moves"] = [&](std::istringstream&) {
        const auto moves = board_.get_legal_moves();
        for (auto &move : moves)
            std::cout << move.to_string(BOARD_SIZE) << " ";
        std::cout << "\n";
    };

    commands_["name"] = [&](std::istringstream&) {
        std::cout << "Abeille by X\n";
    };
    commands_["version"] = [&](std::istringstream&) {
        std::cout << "1.0\n";
    };
    commands_["protocol_version"] = [&](std::istringstream&) {
        std::cout << "2\n";
    };
    commands_["list_commands"] = [&](std::istringstream&) {
        for (auto &[command, _] : commands_)
            std::cout <<  command << "\n";
    };
    commands_["known_command"] = [&](std::istringstream &iss) {
        std::string command;
        iss >> command;
        std::cout << (commands_.find(command) != commands_.end() ? "true" : "false") << "\n";
    };
    commands_["quit"] = [&](std::istringstream&) {
        exit(0);
    };
}

void UHI::uhi_loop() {
    std::cout << "This is a noob Hex engine, welcome!\n";

    std::string input;
    while (getline(std::cin, input)) {
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (commands_.find(command) == commands_.end()) {
            std::cout << std::format(
                "unknown command <{}>\n", command
            );
            continue;
        }

        commands_[command](iss);
    }
}