#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Compile the program
echo "Compiling naive.c..."
gcc -o naive naive.c

if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed${NC}"
    exit 1
fi
echo -e "${GREEN}Compilation successful${NC}"

# Function to run a single test
run_test() {
    test_file=$1
    test_name=$(basename $test_file)
    expected_file="${test_file}.expected"
    output_file="${test_file}.output"
    
    echo -n "Running test ${test_name}... "
    
    # Run the program with the test input
    ./naive "${test_file}" > "${output_file}"
    
    # Compare output with expected output
    if diff -w "${output_file}" "${expected_file}" > /dev/null; then
        echo -e "${GREEN}PASS${NC}"
        return 0
    else
        echo -e "${RED}FAIL${NC}"
        echo "Expected:"
        cat "${expected_file}"
        echo "Got:"
        cat "${output_file}"
        return 1
    fi
}

# Find and run all tests
echo "Running tests..."
failed=0
total=0

for test_file in tests/test*; do
    # Skip expected output files and already generated output files
    if [[ "${test_file}" != *.expected && "${test_file}" != *.output ]]; then
        run_test "${test_file}"
        if [ $? -ne 0 ]; then
            failed=$((failed + 1))
        fi
        total=$((total + 1))
    fi
done

# Display summary
echo "----------------------"
echo "Test Summary: $((total - failed))/$total tests passed"

if [ $failed -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}$failed test(s) failed${NC}"
    exit 1
fi