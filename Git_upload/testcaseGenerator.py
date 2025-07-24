import random
from collections import defaultdict


def generate_test_case(
    N, K, M, T, number_of_requests, max_new_requests_per_turn, testcase_number
):
    filename = f"testcase{testcase_number}.txt"

    with open(filename, "w") as file:
        # Write the initial line in the file
        file.write(f"{N} {K} {M} {T} {number_of_requests}\n")

        # Store how many times each number from 1 to number_of_requests has been used for duplicates
        duplicate_counts = defaultdict(int)

        rows = []

        for _ in range(number_of_requests):
            while True:
                num1, num2 = random.sample(range(K), 2)
                num3 = random.randint(1, number_of_requests)

                # Check if adding this number respects the max_duplicates condition
                if duplicate_counts[num3] < max_new_requests_per_turn:
                    duplicate_counts[num3] += 1
                    rows.append((num1, num2, num3))
                    break

        # Sort rows by the third number in ascending order
        rows.sort(key=lambda x: x[2])

        # Write each row to the file
        for row in rows:
            file.write(f"{row[0]} {row[1]} {row[2]}\n")

    print(f"Test case written to {filename}")


# Adjust values here
N = 5  # number of elevators
K = 20  # number of floors in the buolding
M = 3  # number of solver processes (<= N)
T = 40  # the turn at which the final request is received (turn numbers start at 1)
number_of_requests = 35
max_new_requests_per_turn = 3
testcase_number = 1  # If a testcase file with the given number already exists in the directory, it will be replaced
generate_test_case(
    N, K, M, T, number_of_requests, max_new_requests_per_turn, testcase_number
)
