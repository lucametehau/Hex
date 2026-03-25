#pragma once
#include "../hex/board.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>

constexpr int INPUTS = BOARD_SIZE * BOARD_SIZE * 2;
constexpr int HIDDEN_SIZE = 16;
constexpr int QA = 255;
constexpr int QB = 64;

struct Accumulator {
    std::array<int16_t, HIDDEN_SIZE> vals;
};

class Network {
public:
    Network() = default;

    float evaluate(const Board<BOARD_SIZE> &board) const {
        const auto cells = board.get_raw_board();
        const auto turn = static_cast<int>(board.get_turn());

        std::array<int16_t, HIDDEN_SIZE> acc;

        for (int i = 0; i < HIDDEN_SIZE; i++) {
            acc[i] = feature_bias.vals[i];
        }
        
        for (int sq = 0; sq < BOARD_SIZE * BOARD_SIZE; sq++) {
            const auto piece = cells[sq];

            if (piece == static_cast<int>(Player::NONE)) 
                continue;

            int x = sq % BOARD_SIZE;
            int y = sq / BOARD_SIZE;

            if (turn == static_cast<int>(Player::BLACK))
                std::swap(x, y);
            
            int mapped_sq = y * BOARD_SIZE + x;

            int feature_idx = piece == turn ? mapped_sq : BOARD_SIZE * BOARD_SIZE + mapped_sq;

            for (int i = 0; i < HIDDEN_SIZE; i++) {
                acc[i] += feature_weights[feature_idx].vals[i];
            }
        }

        int32_t output = 0;
        for (int i = 0; i < HIDDEN_SIZE; i++) {
            int32_t act = std::clamp(static_cast<int>(acc[i]), 0, QA);
            act *= act;
            output += act * output_weights[i];
        }

        output /= QA;
        output += output_bias;

        return static_cast<float>(output) / (QA * QB);
    }

private:
    std::array<Accumulator, INPUTS> feature_weights;
    Accumulator feature_bias;
    std::array<int16_t, HIDDEN_SIZE> output_weights;
    int16_t output_bias;
};

inline void load_network(Network* nn) {
    std::ifstream file("src/network/quantised_copy.bin", std::ios::binary);
    if (!file) {
        std::cerr << "Failed to load Neural Network!\n";
        throw std::abort;
    }
    
    // Read the bytes directly into the struct
    file.read(reinterpret_cast<char*>(nn), sizeof(Network));
    file.close();

    // for (int i = 0; i < INPUTS; i++) {
    //     for (int j = 0; j < HIDDEN_SIZE; j++) {
    //         if (nn->feature_weights[i].vals[j])
    //             std::cout << i << " " << j << " " << nn->feature_weights[i].vals[j] << "\n";
    //     }
    // }
}