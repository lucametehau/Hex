#pragma once
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>

constexpr int INPUTS = 338;
constexpr int HIDDEN_SIZE = 16;
constexpr int QA = 255;
constexpr int QB = 64;
constexpr int SCALE = 400;

struct Accumulator {
    int16_t vals[HIDDEN_SIZE];
};

struct Network {
    Accumulator feature_weights[INPUTS];
    Accumulator feature_bias;
    int16_t output_weights[HIDDEN_SIZE];
    int16_t output_bias;
};

inline void load_network(Network* nn) {
    std::ifstream file("src/network/quantised.bin", std::ios::binary);
    if (!file) {
        std::cerr << "Failed to load Neural Network!\n";
        return;
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