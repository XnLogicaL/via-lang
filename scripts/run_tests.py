import subprocess
import os
from loguru import logger

TESTS_DIR="./test"
BINARY="./build/via"

def run_test(test_file):
    # Construct the command to run your interpreter with the test file
    result = subprocess.run([BINARY, "run", test_file], capture_output=True, text=True)

    # Get the output of the program
    actual_output = result.stdout.strip()

    # Construct the expected output file path
    expected_output_file = f"{test_file}.out"

    # Read the expected output
    with open(expected_output_file, "r") as f:
        expected_output = f.read().strip()

    # Compare actual output with expected output
    if actual_output == expected_output:
        logger.info(f"Test passed: {test_file}")
    else:
        logger.error(f"Test failed: {test_file}")
        logger.info(f"Expected:\n{expected_output}")
        logger.info(f"Got:\n{actual_output}")
        return False
    return True

def run_all_tests():
    all_tests_passed = True

    # Run all the test files in the tests directory
    for test_file in os.listdir(TESTS_DIR):
        if test_file.endswith(".via"):
            test_file_path = os.path.join(TESTS_DIR, test_file)
            if not run_test(test_file_path):
                all_tests_passed = False

    return all_tests_passed

if __name__ == "__main__":
    all_tests_passed = run_all_tests()
    if all_tests_passed:
        logger.info("All tests passed!")
        exit(0)
    else:
        logger.error("Some tests failed.")
        exit(1)