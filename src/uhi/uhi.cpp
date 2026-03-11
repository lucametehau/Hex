#include "uhi.h"
#include "../search/search.h"
#include <iostream>

void UHI::uhi_loop() {
    std::cout << "This is a noob Hex engine, welcome!\n";

    Board<BOARD_SIZE> board;

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
        Searcher searcher(board);
        auto [move, score] = searcher.search();

        std::cout << "Playing move " << move.to_string(BOARD_SIZE) << " with score of " << 100.0 * score << "%\n";

        board.make_move(move);

        if (board.is_game_over()) {
            std::cout << "Good game, I won!\n";
            break;
        }

        wait_input();
    }
}