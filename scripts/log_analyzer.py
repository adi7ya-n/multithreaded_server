from datetime import datetime
import subprocess

def getGitRoot() -> str:
    return subprocess.Popen(['git', 'rev-parse', '--show-toplevel'], stdout=subprocess.PIPE).communicate()[0].rstrip().decode('utf-8')

if __name__ == "__main__":
    rootDirectory = getGitRoot()
    print(rootDirectory)
    logPath = rootDirectory + "/build/server.log"
    with open(logPath , mode="r+", encoding='utf-8') as f:
        lines = f.readlines()
        format = "%H:%M:%S.%f"
        sorted_lines = sorted(lines, key=lambda line: datetime.strptime(
            line.split("]")[0][1:], format))
        f.seek(0)
        f.truncate()
        for line in sorted_lines:
            f.write(line)
        f.close()
