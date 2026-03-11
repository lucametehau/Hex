#include <iostream>
#include "hex/board.h"
#include "search/search.h"

int main() {
    Board<BOARD_SIZE> board;
    Searcher searcher(board);

    searcher.search();

    return 0;
}