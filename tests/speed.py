import subprocess
import random
import time
import statistics

def generate_board(n=10, m=10, chance=0.2):
    mines = [[random.random() < chance for _ in range(m)] for _ in range(n)]
    board = []
    for i in range(n):
        row = []
        for j in range(m):
            if mines[i][j]:
                row.append(".")
            else:
                count = 0
                for di in (-1, 0, 1):
                    for dj in (-1, 0, 1):
                        if di == 0 and dj == 0:
                            continue
                        ni, nj = i + di, j + dj
                        if 0 <= ni < n and 0 <= nj < m and mines[ni][nj]:
                            count += 1
                row.append(str(count) if count > 0 else ".")
        board.append(row)
    return board

def board_to_str(board):
    return "\n".join("".join(row) for row in board)

def run_sweeper(board_str):
    proc = subprocess.Popen(
        ["./bin/sweeper"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    start = time.time()
    stdout, stderr = proc.communicate(board_str)
    elapsed = time.time() - start
    return elapsed, stdout, stderr

def main():
    trials = 3
    for size in range(10, 101, 10):
        board = generate_board(size, size, chance=0.1)
        board_str = board_to_str(board)

        times = []
        for _ in range(trials):
            elapsed, stdout, stderr = run_sweeper(board_str)
            times.append(elapsed)
            if stderr:
                print(f"Board {size}x{size} trial error: {stderr.strip()}")

        avg_time = statistics.mean(times)
        print(f"Board {size}x{size}: {avg_time:.4f} seconds (avg of {trials})")

if __name__ == "__main__":
    main()
