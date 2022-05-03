from datetime import datetime

if __name__ == "__main__":
    with open("/home/aditya/Documents/Projects/non_blocking_game_server/server/server.log", mode="r+", encoding = 'utf-8') as f:
        lines = f.readlines()
        format = "%H:%M:%S.%f"
        sorted_lines = sorted(lines, key=lambda line: datetime.strptime(line.split("]")[0][1:], format))
        f.seek(0)
        f.truncate()
        for line in sorted_lines:
            f.write(line)
        f.close()
        


