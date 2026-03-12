#pragma once
#include "../search/limits.h"
#include "../search/search.h"
#include <map>
#include <functional>
#include <sstream>

class UHI {
public:
    UHI();

    void uhi_loop();

private:
    Board<BOARD_SIZE> board_;
    Searcher searcher_;
    SearchLimits limits_;

    std::map<std::string, std::function<void(std::istringstream&)>> commands_;
};