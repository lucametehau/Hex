# Placeholder name

This is a recreative Hex, the grid game, engine, as means to learn more about MCTS. Don't know if there is an official Hex protocol for testing yet...

# Customization

- The engine supports any  board size, however, you must change `BOARD_SIZE` in `search.h`.
- You can also set how much time the engine can spend on a move via `set time <time in milliseconds>`.

# TODO

- Add PUCT (Predictor UCT)
- Try RAVE (Rapid Action Value Estimation)
- Look how to introduce Neural Networks?
- Improve self-play of selected node. It's purely random now