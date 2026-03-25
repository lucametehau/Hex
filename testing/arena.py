import subprocess
import math
import sys
import concurrent.futures
import threading
import random

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
            
        # print(response)
        return response.split('\n')[-2].strip() if len(response.split('\n')) > 2 else response[2:].strip()

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

def generate_opening(num_moves=4):
    """Generates a list of random, unique valid moves for a 13x13 board."""
    cols = [chr(ord('a') + i) for i in range(13)] # 'a' through 'm'
    rows = [str(i) for i in range(1, 14)]         # '1' through '13'
    all_cells = [c + r for c in cols for r in rows]
    
    return random.sample(all_cells, num_moves)

def play_game(engine_black, engine_white, timelimit, nodes, opening_moves):
    engine_black.send(f"setoption time {timelimit}")
    engine_black.send(f"setoption nodes {nodes}")
    engine_black.send("clear_board")
    engine_white.send(f"setoption time {timelimit}")
    engine_white.send(f"setoption nodes {nodes}")
    engine_white.send("clear_board")

    # 1. Play the forced opening moves on both engines
    colors = ["black", "white"]
    print(opening_moves)
    for i, move in enumerate(opening_moves):
        color = colors[i % 2]
        engine_black.send(f"play {color} {move}")
        engine_white.send(f"play {color} {move}")

    # Determine whose turn it is next (if 4 moves, it's Black's turn (0))
    next_turn = len(opening_moves) % 2 

    # 2. Resume normal play
    while True:
        if next_turn == 0: # Black's Turn
            b_move = engine_black.send("genmove black")
            if b_move.lower() == "resign" or b_move == "":
                return 2 # Player 2 (White) wins
            engine_white.send(f"play black {b_move}")
            
        else: # White's Turn
            w_move = engine_white.send("genmove white")
            if w_move.lower() == "resign" or w_move == "":
                return 1 # Player 1 (Black) wins
            engine_black.send(f"play white {w_move}")

        next_turn = 1 - next_turn # Toggle turn

def play_single_match(bin_a, bin_b, timelimit, nodes, is_a_black, opening_moves):
    """Worker function that spins up fresh engines for one single game."""
    engine_a = GTPEngine(bin_a, "Engine A")
    engine_b = GTPEngine(bin_b, "Engine B")
    
    if is_a_black:
        winner = play_game(engine_a, engine_b, timelimit, nodes, opening_moves)
        a_won = (winner == 1)
    else:
        winner = play_game(engine_b, engine_a, timelimit, nodes, opening_moves)
        a_won = (winner == 2)
        
    engine_a.close()
    engine_b.close()
    
    return a_won

def run_tournament(bin_a, bin_b, games, timelimit, nodes, concurrency):
    # Ensure games is even so every opening gets played twice symmetrically
    if games % 2 != 0:
        print("Rounding up to the nearest even number of games for fair pairings.")
        games += 1

    wins_a = 0
    wins_b = 0

    print(f"Starting tournament: {games} games, {concurrency} at a time.")

    # Generate the opening book (half the number of total games)
    num_openings = games // 2
    openings = [generate_opening(4) for _ in range(num_openings)]

    with concurrent.futures.ThreadPoolExecutor(max_workers=concurrency) as executor:
        futures = []
        
        # Submit two games for every opening (A plays Black, then B plays Black)
        for opening in openings:
            futures.append(executor.submit(play_single_match, bin_a, bin_b, timelimit, nodes, True, opening))
            futures.append(executor.submit(play_single_match, bin_a, bin_b, timelimit, nodes, False, opening))
        
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