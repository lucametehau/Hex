import subprocess
import math
import sys
import concurrent.futures
import threading

# We need a lock to prevent parallel threads from printing over each other
print_lock = threading.Lock()

class GTPEngine:
    def __init__(self, binary_path, name):
        self.name = name
        self.process = subprocess.Popen(
            [binary_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL, 
            text=True,
            bufsize=1 
        )

    def send(self, cmd):
        self.process.stdin.write(cmd + "\n")
        self.process.stdin.flush()
        
        response = ""
        while True:
            line = self.process.stdout.readline()
            if line == "\n": 
                break
            response += line
            
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
    
    if win_rate == 1.0: return 800.0
    if win_rate == 0.0: return -800.0
    
    return -400.0 * math.log10(1.0 / win_rate - 1.0)

def play_game(engine_black, engine_white, timelimit, nodes):
    engine_black.send(f"setoption time {timelimit}")
    engine_black.send(f"setoption nodes {nodes}")
    engine_black.send("clear_board")
    engine_white.send(f"setoption time {timelimit}")
    engine_black.send(f"setoption nodes {nodes}")
    engine_white.send("clear_board")

    while True:
        # Black's Turn
        b_move = engine_black.send("genmove black")
        if b_move.lower() == "resign" or b_move == "":
            return 2 # Player 2 (White) wins

        engine_white.send(f"play black {b_move}")

        # White's Turn
        w_move = engine_white.send("genmove white")
        if w_move.lower() == "resign" or w_move == "":
            return 1 # Player 1 (Black) wins

        engine_black.send(f"play white {w_move}")

def play_single_match(bin_a, bin_b, timelimit, nodes, game_index):
    """Worker function that spins up fresh engines for one single game."""
    engine_a = GTPEngine(bin_a, "Engine A")
    engine_b = GTPEngine(bin_b, "Engine B")
    
    # Swap colors based on the game index
    is_a_black = (game_index % 2 == 0)
    
    if is_a_black:
        winner = play_game(engine_a, engine_b, timelimit, nodes)
        a_won = (winner == 1)
    else:
        winner = play_game(engine_b, engine_a, timelimit, nodes)
        a_won = (winner == 2)
        
    # Clean up the subprocesses so we don't leak memory
    engine_a.close()
    engine_b.close()
    
    return a_won

def run_tournament(bin_a, bin_b, games, timelimit, nodes, concurrency):
    wins_a = 0
    wins_b = 0

    print(f"Starting tournament: {games} games, {concurrency} at a time.")

    # Create a pool of worker threads
    with concurrent.futures.ThreadPoolExecutor(max_workers=concurrency) as executor:
        # Submit all the games to the pool
        futures = [
            executor.submit(play_single_match, bin_a, bin_b, timelimit, nodes, i) 
            for i in range(games)
        ]
        
        # As each game finishes, tally the score and print safely
        for future in concurrent.futures.as_completed(futures):
            a_won = future.result()
            
            if a_won:
                wins_a += 1
            else:
                wins_b += 1
                
            with print_lock:
                print(f"Score so far: Engine A [{wins_a} - {wins_b}] Engine B")

    elo_diff = calculate_elo(wins_a, wins_b)
    
    print("\n" + "="*30)
    print("TOURNAMENT COMPLETE")
    print(f"Engine A Wins: {wins_a}")
    print(f"Engine B Wins: {wins_b}")
    print(f"Elo Difference for A: {elo_diff:+.2f} Elo")
    print("="*30)

if __name__ == "__main__":
    # Example usage: python3 arena.py ./hex_old ./hex_new 100 1000 8
    if len(sys.argv) != 7:
        print("Usage: python3 arena.py <binary1> <binary2> <games> <timelimit> <nodes> <threads>")
        sys.exit(1)

    bin_a = sys.argv[1]
    bin_b = sys.argv[2]
    games = int(sys.argv[3])
    timelimit = int(sys.argv[4])
    nodes = int(sys.argv[5])
    concurrency = int(sys.argv[6])

    run_tournament(bin_a, bin_b, games, timelimit, nodes, concurrency)