# Dynamic Elevator Requests System

## Overview
This system allows real-time passenger requests to be added to the elevator simulation through a file-based interface, enabling integration with React UI or other front-end systems.

## How It Works

### 1. Dynamic Requests File Format
The system reads from `dynamic_requests.txt` in the following format:
```
startFloor requestedFloor arrivalTime
```

Example:
```
0 5 1    # Passenger wants to go from floor 0 to floor 5, request arrives at turn 1
2 8 2    # Passenger wants to go from floor 2 to floor 8, request arrives at turn 2
1 4 3    # Passenger wants to go from floor 1 to floor 4, request arrives at turn 3
```

### 2. React UI Integration
Your React UI should append new requests to `dynamic_requests.txt` in the above format:

```javascript
// Example React function to add a request
function addPassengerRequest(startFloor, requestedFloor, arrivalTime) {
    const requestLine = `${startFloor} ${requestedFloor} ${arrivalTime}\n`;
    
    // Send to backend to append to dynamic_requests.txt
    fetch('/api/add-request', {
        method: 'POST',
        body: JSON.stringify({ request: requestLine }),
        headers: { 'Content-Type': 'application/json' }
    });
}
```

### 3. File Pointer Efficiency
The system uses efficient file reading:
- Maintains a file position pointer to only read new content
- Reads incrementally each turn
- No need to reparse the entire file

### 4. Program Execution
```bash
# Compile the helper program
gcc -o helper helper-program-v2.c -lpthread

# Compile your student solution
gcc -o solution progg_updated.c -lpthread

# Run the simulation (no longer needs testcase number)
./helper 1
```

### 5. Real-time Operation
- The simulation runs continuously
- Each turn, it checks for new requests in the file
- Requests are processed based on their `arrivalTime`
- The React UI can add requests at any time by appending to the file

### 6. Communication Flow
```
React UI → dynamic_requests.txt → Helper Program → Shared Memory → Student Solution
```

## Changes Made

### Minimal Changes to Student Program
- `progg_updated.c` requires **no changes**
- It still receives requests via shared memory as before
- All existing IPC communication remains intact

### Helper Program Changes
- Added dynamic file reading capability
- Replaced fixed testcase loading with real-time request processing
- Maintains backward compatibility with existing message/shared memory system

## Benefits
- ✅ Real-time request injection
- ✅ Minimal code changes
- ✅ Efficient file I/O with position tracking
- ✅ Seamless React UI integration
- ✅ Existing communication protocols preserved 