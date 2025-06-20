import os
import time
import subprocess
from testcase_writer import write_testcase
from parser import parse_latest_turn

class ElevatorController:
    def __init__(self):
        self.shared_dir = "shared"
        os.makedirs(self.shared_dir, exist_ok=True)

        self.helper_process = None
        self.testcase_id = "1"
        self.request_queue = []
        self.request_id_counter = 0
        self.current_turn = 0

    def get_wsl_path(self, windows_path):
        """Convert Windows path (E:\\folder) → WSL path (/mnt/e/folder)"""
        return windows_path.replace("E:\\", "/mnt/e/").replace("\\", "/")

    def start_simulation(self, elevators, floors, solvers, max_turns,max_requests):
        testcase_file = f"testcase{self.testcase_id}.txt"
        testcase_path = os.path.join(self.shared_dir, testcase_file)

        # Write initial testcase (with 0 requests)

        write_testcase(testcase_path, elevators, floors, solvers, max_turns,max_requests)

        # WSL-compatible path
        wsl_shared_path = self.get_wsl_path(os.path.abspath(self.shared_dir))

        # Command to run helper in WSL
        command = f"cd {wsl_shared_path} && ./helper {self.testcase_id}"
        print(f"[DEBUG] Running in WSL: {command}")

        self.helper_process = subprocess.Popen(
            ["wsl", "bash", "-c", command],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        time.sleep(2)  # give time to generate input.txt/output.txt

    def queue_request(self, start, end):
        self.request_queue.append({
            "requestId": self.request_id_counter,
            "startFloor": start,
            "requestedFloor": end,
            "arrivalTime": self.current_turn + 1
        })
        self.request_id_counter += 1

    def run_next_turn(self):
        self.current_turn += 1

        # Add request if scheduled for this turn
        if self.request_queue:
            req = self.request_queue.pop(0)
            testcase_file = f"testcase{self.testcase_id}.txt"
            testcase_path = os.path.join(self.shared_dir, testcase_file)
            with open(testcase_path, "a") as f:
                f.write(f"{req['startFloor']} {req['requestedFloor']} {req['arrivalTime']}\n")

        time.sleep(1)  # wait for output to update

        output_path = os.path.join(self.shared_dir, "output.txt")
        return parse_latest_turn(output_path, self.current_turn)

    def end_simulation(self):
        if self.helper_process:
            self.helper_process.terminate()
            try:
                self.helper_process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.helper_process.kill()
            self.helper_process = None

        # Delete runtime files
        for file in ["testcase1.txt", "input.txt", "output.txt"]:
            path = os.path.join(self.shared_dir, file)
            if os.path.exists(path):
                os.remove(path)

        # Reset state
        self.request_queue = []
        self.request_id_counter = 0
        self.current_turn = 0
