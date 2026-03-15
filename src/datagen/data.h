#pragma once
#include "../search/search.h"

struct HexBoard {
    std::array<uint8_t, BOARD_SIZE * BOARD_SIZE> board;
    uint8_t stm;
    uint8_t result;
    uint8_t padding;
};