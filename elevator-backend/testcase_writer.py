def write_testcase(filename, elevators, floors, solvers, max_turns,max_requests):
    with open(filename, "w") as f:
        f.write(f"{elevators} {floors} {solvers} {max_turns} {max_requests}\n")# 0 = initial #requests
