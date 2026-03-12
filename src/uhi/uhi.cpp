#include "uhi.h"
#include "../search/search.h"
#include <iostream>
#include <sstream>

void UHI::listen_for_options() {
    std::string input;
    while(getline(std::cin, input)) {
        std::istringstream iss(input);
        std::string setoption, name;
        iss >> setoption;
        
        if (setoption != "set")
            return;

        iss >> name;
        if (name != "time")
            return;
        
        std::size_t max_time;
        iss >> max_time;

        limits_.set_max_time(max_time);
    }
}

void UHI::uhi_loop() {
    std::cout << "This is a noob Hex engine, welcome!\n";

    listen_for_options();

    Board<BOARD_SIZE> board;
    Searcher searcher;

    // placeholder until I implement some protocol
    // like FEN in chess
    auto wait_input = [&]() -> bool {
        std::cout << "Please input your move:\n";

        std::string s;
        std::cin >> s;

        if (s == "start")
            return false;

        int row = (s[0] - 'a'), col = std::stoi(s.substr(1)) - 1;

        board.make_move(Move(row, col, BOARD_SIZE));
        return true;
    };

    while (wait_input());

    while (!board.is_game_over()) {
        auto [move, score] = searcher.search(board, limits_);

        std::cout << "Playing move " << move.to_string(BOARD_SIZE) << " with score of " << 100.0 * score << "%\n";

        board.make_move(move);

        if (board.is_game_over()) {
            std::cout << "Good game, I won!\n";
            break;
        }

        wait_input();
    }
}