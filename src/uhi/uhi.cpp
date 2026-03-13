#include "uhi.h"
#include "../search/search.h"
#include <iostream>
#include <sstream>

constexpr std::string_view VERSION = "1.1";

UHI::UHI() {
    // create command map
    commands_["boardsize"] = [&](std::istringstream &iss){
        int n;
        iss >> n;
        if (iss >> n);
    };
    commands_["clear_board"] = [&](std::istringstream&) {
        board_ = Board<BOARD_SIZE>();
    };
    commands_["showboard"] = [&](std::istringstream&) {
        std::cout << "\n" << board_;
    };
    commands_["final_score"] = [&](std::istringstream&) {
        if (board_.is_game_over())
            std::cout << (board_.get_turn() == Player::WHITE ? "B+" : "W+");
        else
            std::cout << "cannot score";
    };
    commands_["play"] = [&](std::istringstream &iss) {
        std::string player;
        iss >> player;
        assert ((player == "white" ? Player::WHITE : Player::BLACK) == board_.get_turn());

        std::string move;
        iss >> move;
        board_.make_move(Move(move, BOARD_SIZE));
    };
    commands_["reg_genmove"] = [&](std::istringstream &iss) {
        std::string player;
        iss >> player;
        assert ((player == "white" ? Player::WHITE : Player::BLACK) == board_.get_turn());

        const auto [move, score] = searcher_.search(board_, limits_);

        std::cout << move.to_string(BOARD_SIZE) << "" << std::endl;

        std::cerr << "Playing with score of " << 100.0 * score << "%";
    };
    commands_["genmove"] = [&](std::istringstream &iss) {
        std::string player;
        iss >> player;

        if (board_.is_game_over()) {
            std::cout << "resign";
            return;
        }

        assert ((player == "white" ? Player::WHITE : Player::BLACK) == board_.get_turn());

        const auto [move, score] = searcher_.search(board_, limits_);

        std::cout << move.to_string(BOARD_SIZE);

        std::cerr << std::endl << "Playing with score of " << 100.0 * score << "%";

        board_.make_move(move);
    };
    commands_["undo"] = [&](std::istringstream&) {
        board_.undo();
    };
    commands_["all_legal_moves"] = [&](std::istringstream&) {
        const auto moves = board_.get_legal_moves();
        for (auto &move : moves)
            std::cout << move.to_string(BOARD_SIZE) << " ";
    };

    commands_["name"] = [&](std::istringstream&) {
        std::cout << "Abeille";
    };
    commands_["version"] = [&](std::istringstream&) {
        std::cout << VERSION;
    };
    commands_["protocol_version"] = [&](std::istringstream&) {
        std::cout << "2";
    };
    commands_["hexgui-analyze_commands"] = [&](std::istringstream&) {
        
    };
    commands_["list_commands"] = [&](std::istringstream&) {
        std::size_t counter = 0;
        for (auto &[command, _] : commands_)
            std::cout << command << (++counter < commands_.size() ? "\n" : "");
    };
    commands_["known_command"] = [&](std::istringstream &iss) {
        std::string command;
        iss >> command;
        std::cout << (commands_.find(command) != commands_.end() ? "true" : "false");
    };
    commands_["quit"] = [&](std::istringstream&) {
        std::cout << std::endl << std::endl;
        exit(0);
    };
}

void UHI::uhi_loop() {
    std::cerr << "This is a noob Hex engine, welcome!" << std::endl;

    std::string input;
    while (getline(std::cin, input)) {
        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (commands_.find(command) == commands_.end()) {
            std::cout << std::format(
                "? unknown command <{}>", command
            ) << std::endl << std::endl;
            continue;
        }

        std::cout << "= ";
        commands_[command](iss);
        std::cout << std::endl << std::endl;
    }
}