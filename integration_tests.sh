#!/bin/bash

lunch_servers_and_clients() {
    host="127.0.0.1"
    port="4242"
    sercret_1="123"
    sercret_2="456"

    # Compile binaries
    make re --silent
    if (( $? != 0 )); then
        echo "Compilation failed"
        exit 1
    fi
    make clean --silent
    # Run the server
    echo "Starting server..."
    ./server_side --test $port &
    PID_SERVER=$!

    # Wait for the server to start
    sleep 1

    echo "Starting clients..."
    echo
    # Run 2 clients
    ./client_side --test $host $port $sercret_1 &
    sleep 0.01
    ./client_side --test $host $port $sercret_1 &
    sleep 5

    # silently kill the process if it exists
    kill -s 0 $PID_SERVER 2> /dev/null
    echo "Server killed"
}

# Run the tests and save the output in a file
rm -f log.txt
touch log.txt
lunch_servers_and_clients > log.txt

# Check if the output is correct

# Define the array of strings
strings_to_find=(
    "New client connected." 
    "Connected to 127.0.0.1:4242"
    "Both clients are connected."
    "Other client with the same secret is now connected."
    "Server Received: TESTING SERVER"
    "Server Received: [CMD]ECHOREPLY echoreply_test"
    "Client Received: [CMD]ECHOREPLY echoreply_test"
    "Client Received: echoreply_test"
    "Server Received: echoreply_test"
)

NB_TESTS=${#strings_to_find[@]}
NB_TEST_PASSED=0

# Loop through the array and check if the string is in the log file
for string in "${strings_to_find[@]}"; do
  if grep -q "$string" log.txt; then
    echo "$string found in log.txt"
    echo -e "\033[32mTEST PASSED\033[0m"
    NB_TEST_PASSED=$((NB_TEST_PASSED + 1))
    # w
  else
    echo "$string not found in log.txt"
    echo -e "\033[31mTEST NOT PASSED\033[0m"
  fi
done

echo
echo -e "\033[34mNB TEST PASSED: $NB_TEST_PASSED/$NB_TESTS\033[0m"
