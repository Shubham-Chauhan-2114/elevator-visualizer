#!/bin/bash

echo "Testing Dynamic Elevator Requests System"
echo "========================================"

# Clean up any existing files
rm -f dynamic_requests.txt input.txt output.txt helper solution

# Compile programs
echo "Compiling programs..."
gcc -o helper helper-program-v2.c -lpthread
gcc -o solution progg_updated.c -lpthread

if [ ! -f helper ] || [ ! -f solution ]; then
    echo "ERROR: Compilation failed"
    exit 1
fi

echo "✅ Compilation successful"

# Create initial dynamic requests file
echo "Creating initial requests..."
cat > dynamic_requests.txt << EOF
0 5 1
2 8 2
1 4 3
EOF

echo "✅ Initial requests created in dynamic_requests.txt"

# Start the simulation in background
echo "Starting simulation..."
./helper 1 &
HELPER_PID=$!

# Wait a moment for it to start
sleep 2

# Add more requests dynamically
echo "Adding dynamic requests during simulation..."
sleep 3
echo "3 7 5" >> dynamic_requests.txt
echo "6 2 8" >> dynamic_requests.txt

sleep 3
echo "4 1 10" >> dynamic_requests.txt
echo "7 3 12" >> dynamic_requests.txt

# Let it run for a bit more
sleep 5

# Stop the simulation
echo "Stopping simulation..."
kill $HELPER_PID 2>/dev/null

echo "✅ Test completed!"
echo ""
echo "Check output.txt for simulation results"
echo "The system should have processed both initial and dynamically added requests"

# Show some results
if [ -f output.txt ]; then
    echo ""
    echo "Sample output (first 10 lines):"
    head -10 output.txt
fi 