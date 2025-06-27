#!/bin/bash

echo "🏢 Dynamic Elevator System Startup"
echo "=================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    print_error "Node.js is not installed. Please install Node.js first."
    exit 1
fi

# Check if npm is installed
if ! command -v npm &> /dev/null; then
    print_error "npm is not installed. Please install npm first."
    exit 1
fi

# Check if gcc is installed
if ! command -v gcc &> /dev/null; then
    print_error "gcc is not installed. Please install gcc first."
    exit 1
fi

print_status "All prerequisites found!"

# Step 1: Compile C programs
print_status "Compiling C programs..."
cd "Helper and Testcases"

if gcc -o helper helper-program-v2.c -lpthread; then
    print_success "Helper program compiled successfully"
else
    print_error "Failed to compile helper program"
    exit 1
fi

if gcc -o solution progg_updated.c -lpthread; then
    print_success "Solution program compiled successfully"
else
    print_error "Failed to compile solution program"
    exit 1
fi

cd ..

# Step 2: Setup backend
print_status "Setting up backend server..."
cd elevator-backend

if [ ! -d "node_modules" ]; then
    print_status "Installing backend dependencies..."
    if npm install; then
        print_success "Backend dependencies installed"
    else
        print_error "Failed to install backend dependencies"
        exit 1
    fi
else
    print_success "Backend dependencies already installed"
fi

cd ..

# Step 3: Setup frontend
print_status "Setting up React frontend..."
cd elevator-visualizer

if [ ! -d "node_modules" ]; then
    print_status "Installing frontend dependencies..."
    if npm install; then
        print_success "Frontend dependencies installed"
    else
        print_error "Failed to install frontend dependencies"
        exit 1
    fi
else
    print_success "Frontend dependencies already installed"
fi

cd ..

# Step 4: Initialize dynamic requests file
print_status "Initializing dynamic requests file..."
touch "Helper and Testcases/dynamic_requests.txt"
print_success "Dynamic requests file ready"

# Step 5: Start the system
print_status "Starting the system components..."

# Function to cleanup on exit
cleanup() {
    print_warning "Shutting down system..."
    kill $BACKEND_PID 2>/dev/null
    kill $FRONTEND_PID 2>/dev/null
    print_success "System shutdown complete"
}

# Set trap to cleanup on script exit
trap cleanup EXIT

# Start backend server
print_status "Starting backend server on port 8000..."
cd elevator-backend
node server.js &
BACKEND_PID=$!
cd ..

# Wait for backend to start
sleep 3

# Check if backend is running
if curl -s http://localhost:8000/health > /dev/null; then
    print_success "Backend server is running"
else
    print_error "Backend server failed to start"
    exit 1
fi

# Start frontend
print_status "Starting React frontend on port 3000..."
cd elevator-visualizer
npm start &
FRONTEND_PID=$!
cd ..

print_success "System startup complete!"
echo ""
echo "🌐 Access the system at: http://localhost:3000"
echo "🔧 Backend API available at: http://localhost:8000"
echo "📁 Dynamic requests file: $(pwd)/Helper\ and\ Testcases/dynamic_requests.txt"
echo ""
print_warning "Press Ctrl+C to stop all services"

# Wait for user to stop
wait 