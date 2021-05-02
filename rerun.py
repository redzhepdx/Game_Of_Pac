import os

PROCESS_CALL_COUNT = 100

def main():
    for run_id in range(PROCESS_CALL_COUNT):
        print(f"[RERUN] Reloading the Game :  {run_id + 1}")
        os.system("./run.sh")


if __name__ == "__main__":
    main()