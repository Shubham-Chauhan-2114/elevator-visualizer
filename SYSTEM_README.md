# 🏢 Dynamic Elevator System

A comprehensive real-time elevator simulation system with React UI that integrates with C-based elevator scheduling algorithms.

## 🎯 System Overview

This system provides a complete elevator simulation environment where:
- **Real-time Requests**: Add passenger requests dynamically through a React UI
- **Efficient Scheduling**: C-based elevator algorithm processes requests optimally
- **Live Visualization**: See elevators moving in real-time
- **File-based Communication**: Uses efficient file I/O for minimal code changes

## 🚀 Quick Start

### Prerequisites
- Node.js (v14 or higher)
- npm
- gcc compiler
- curl (for health checks)

### Start the Complete System
```bash
# Make script executable (first time only)
chmod +x start_system.sh

# Start everything at once
./start_system.sh
```

This will:
1. ✅ Compile C programs
2. ✅ Install dependencies
3. ✅ Start backend server (port 8000)
4. ✅ Start React UI (port 3000)
5. ✅ Initialize dynamic requests system

### Access the System
- 🌐 **Web UI**: http://localhost:3000
- 🔧 **API**: http://localhost:8000
- 📁 **Dynamic Requests**: `Helper and Testcases/dynamic_requests.txt`

## 🏗️ System Architecture

```
React UI (3000) ←→ Node.js Backend (8000) ←→ C Helper Program ←→ Student Algorithm
```

## 🎮 How to Use

1. **Start**: `./start_system.sh`
2. **Configure**: Set elevators, floors, solvers
3. **Add Requests**: Use the web form
4. **Monitor**: Watch real-time visualization

## 🔧 Key Features

- ✅ **Zero Changes** to your algorithm code
- ✅ **Real-time** request processing  
- ✅ **Live visualization** of elevator movement
- ✅ **Performance metrics** and statistics
- ✅ **Responsive UI** with modern design

Perfect for demonstrating elevator scheduling algorithms!

🚀 **Start exploring**: `./start_system.sh` 