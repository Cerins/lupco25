import subprocess
import random
import statistics

def generate_board(n=10, m=10, chance=0.4):
    mines = [[random.random() < chance for _ in range(m)] for _ in range(n)]
    board = []
    mine_count = 0
    for i in range(n):
        row = []
        for j in range(m):
            if mines[i][j]:
                row.append(".")
                mine_count += 1
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
    return board, mine_count

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
    stdout, stderr = proc.communicate(board_str)
    return stdout.strip(), stderr.strip()

def parse_lahc_score(output):
    # print("Output:\n", output)
    for line in output.splitlines():
        if "LAHC" in line and "score" in line.lower():
            parts = line.strip().split()
            for token in parts:
                if token.isdigit():
                    return int(token)
    return None

def main():
    attempts = 20
    for size in range(10, 61, 10):
        board, mine_count = generate_board(size, size, chance=0.1)
        board_str = board_to_str(board)

        scores = []
        for _ in range(attempts):
            stdout, stderr = run_sweeper(board_str)
            if stderr:
                print(f"Board {size}x{size} error: {stderr}")
            score = parse_lahc_score(stdout)
            if score is not None:
                scores.append(score)
        print("Scores:", scores)
        if scores:
            avg_score = statistics.mean(scores)
            score_per_mine = avg_score / mine_count if mine_count > 0 else 0
            print(f"Board {size}x{size}: "
                  f"Avg LAHC score = {avg_score:.2f}, "
                  f"Avg score per mine = {score_per_mine:.4f}")
        else:
            print(f"Board {size}x{size}: No scores found")

if __name__ == "__main__":
    main()