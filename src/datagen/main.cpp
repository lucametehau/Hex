#include "data.h"
#include <thread>
#include <atomic>
#include <random>
#include <sstream>
#include <format>

auto get_time() {
    return std::chrono::high_resolution_clock::now();
}

void play_book(Board<BOARD_SIZE> &board) {
    std::uniform_int_distribution<> rng;
    const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count() ^ (uintptr_t)&rng;
    std::mt19937 sed(seed);
    constexpr int BOOK_MOVES = 4;

    std::vector<Move> moves;
    moves.reserve(BOARD_SIZE * BOARD_SIZE);

    for (int i = 0; i < 4; i++) {
        board.get_legal_moves(moves);
        board.make_move(moves[rng(sed) % moves.size()]);
    }
}

void datagen_thread(std::atomic<std::size_t> &total_positions, std::size_t positions_goal, std::size_t nodes_limit) {
    // Datagen loop for a thread
    // we will generate 4 random moves and then let the engine play

    std::ostringstream ss;
    ss << "data_thread_" << std::this_thread::get_id() << ".bin";
    
    std::ofstream out_file(ss.str(), std::ios::binary | std::ios::app);
    if (!out_file.is_open()) {
        return; 
    }

    std::size_t positions = 0;
    std::array<HexBoard, BOARD_SIZE * BOARD_SIZE> records;
    Searcher searcher;
    SearchLimits limits;

    limits.set_max_nodes(nodes_limit);

    while (positions < positions_goal) {
        Board<BOARD_SIZE> board;

        // book
        play_book(board);

        // play game
        std::size_t record_count = 0;
        while (!board.is_game_over()) {
            records[record_count].board = board.get_raw_board();
            records[record_count].stm = static_cast<int>(board.get_turn()) + 1;
            record_count++;

            auto [move, score] = searcher.search(board, limits);

            board.make_move(move);
        }

        auto winner = 3 - (static_cast<int>(board.get_turn()) + 1); // opposite of stm
        for (std::size_t i = 0; i < record_count; i++)
            records[i].result = records[i].stm == winner ? 2 : 0;

        // record game
        positions += record_count;
        total_positions += record_count;

        out_file.write(
            reinterpret_cast<const char*>(records.data()),
            record_count * sizeof(HexBoard)
        );
    }
}

int main(int argc, char **argv) {
    // usage ./datagen <positions> <threads> <nodes>
    std::cout << "Datagen mode\n";

    if (argc != 4) {
        std::cout << "Incorrect usage! Use ./datagen <positions> <threads> <nodes>\n";
        return 1;
    }

    std::size_t positions = std::stoull(argv[1]);
    std::size_t num_threads = std::stoull(argv[2]);
    std::size_t nodes_lim = std::stoull(argv[3]);

    assert (num_threads <= std::thread::hardware_concurrency());

    std::vector<std::thread> threads;
    std::atomic<std::size_t> total_positions{};
    auto positions_per_thread = (positions + num_threads - 1) / num_threads;

    for (std::size_t i = 0; i < num_threads; i++) {
        threads.emplace_back(datagen_thread, std::ref(total_positions), positions_per_thread, nodes_lim);
    }

    auto start_time = get_time();

    while (total_positions < positions) {
        auto now_time = get_time();
        auto time_diff = std::chrono::duration_cast<std::chrono::seconds>(now_time - start_time).count();
        auto cur_positions = total_positions.load();
        std::cout << std::format(
            "Generated {} positions in {} seconds; speed: {}\r",
            cur_positions, time_diff, 1.0 * cur_positions / time_diff
        );
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\n";

    for (auto &thread : threads) {
        thread.join();
    }

    return 0;
}