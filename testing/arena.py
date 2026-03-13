import subprocess
import math
import sys

class GTPEngine:
    def __init__(self, binary_path, name):
        self.name = name
        # Spawn the C++ binary. We pipe stdin/stdout to talk to it, 
        # and hide stderr so your std::cerr debug logs don't clutter the console.
        self.process = subprocess.Popen(
            [binary_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL, 
            text=True,
            bufsize=1 # Line buffered
        )

    def send(self, cmd):
        self.process.stdin.write(cmd + "\n")
        self.process.stdin.flush()
        
        response = ""
        while True:
            line = self.process.stdout.readline()
            if line == "\n": # GTP responses always end with a double newline
                break
            response += line
            
        # Strip the "= " or "? " from the start of the GTP response
        return response[2:].strip()

    def close(self):
        try:
            self.send("quit")
            self.process.terminate()
        except:
            pass

def calculate_elo(wins, losses):
    total = wins + losses
    if total == 0: return 0.0
    
    win_rate = wins / total
    
    # Handle infinite Elo for 100% / 0% win rates
    if win_rate == 1.0: return 800.0
    if win_rate == 0.0: return -800.0
    
    # Standard Elo difference formula
    return -400.0 * math.log10(1.0 / win_rate - 1.0)

def play_game(engine_black, engine_white, timelimit):
    engine_black.send(f"setoption time {timelimit}")
    engine_black.send("clear_board")
    engine_white.send(f"setoption time {timelimit}")
    engine_white.send("clear_board")

    print(f"Match: {engine_black.name} (Black) vs {engine_white.name} (White)")

    while True:
        # Black's Turn
        b_move = engine_black.send("genmove black")
        if b_move.lower() == "resign" or b_move == "":
            print(f"--> {engine_white.name} wins! (Black resigned/failed)")
            return 2 # Player 2 (White) wins

        engine_white.send(f"play black {b_move}")

        # White's Turn
        w_move = engine_white.send("genmove white")
        if w_move.lower() == "resign" or w_move == "":
            print(f"--> {engine_black.name} wins! (White resigned/failed)")
            return 1 # Player 1 (Black) wins

        engine_black.send(f"play white {w_move}")

def run_tournament(bin_a, bin_b, games, board_size):
    wins_a = 0
    wins_b = 0

    engine_a = GTPEngine(bin_a, "Engine A")
    engine_b = GTPEngine(bin_b, "Engine B")

    for i in range(games):
        # Swap colors every game to ensure fairness (First-move advantage in Hex is massive)
        if i % 2 == 0:
            winner = play_game(engine_a, engine_b, board_size)
            if winner == 1: wins_a += 1
            else: wins_b += 1
        else:
            winner = play_game(engine_b, engine_a, board_size)
            if winner == 1: wins_b += 1
            else: wins_a += 1

        print(f"Score so far: Engine A [{wins_a} - {wins_b}] Engine B")

    engine_a.close()
    engine_b.close()

    elo_diff = calculate_elo(wins_a, wins_b)
    
    print("\n" + "="*30)
    print("TOURNAMENT COMPLETE")
    print(f"Engine A Wins: {wins_a}")
    print(f"Engine B Wins: {wins_b}")
    print(f"Elo Difference for A: {elo_diff:+.2f} Elo")
    print("="*30)

if __name__ == "__main__":
    # Example usage: python3 arena.py ./hex_old ./hex_new 100 11
    if len(sys.argv) != 5:
        print("Usage: python3 arena.py <binary1> <binary2> <games> <timelimit>")
        sys.exit(1)

    run_tournament(sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4]))