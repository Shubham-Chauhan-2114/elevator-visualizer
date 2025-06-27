const express = require('express');
const cors = require('cors');
const fs = require('fs').promises;
const path = require('path');
const { spawn } = require('child_process');

const app = express();
const PORT = 8000;

// Middleware
app.use(cors());
app.use(express.json());

// Global state
let currentTurn = 1;
let isSimulationRunning = false;
let helperProcess = null;
let elevatorData = {
  elevators: [],
  floors: 10,
  currentTurn: 0,
  stats: {
    totalRequests: 0,
    fulfilledRequests: 0,
    totalMovement: 0
  }
};

// File paths
const DYNAMIC_REQUESTS_FILE = path.join(__dirname, '..', 'Helper and Testcases', 'dynamic_requests.txt');
const OUTPUT_FILE = path.join(__dirname, '..', 'Helper and Testcases', 'output.txt');

// Initialize dynamic requests file
async function initializeDynamicRequestsFile() {
  try {
    await fs.writeFile(DYNAMIC_REQUESTS_FILE, '');
    console.log('Dynamic requests file initialized');
  } catch (error) {
    console.error('Error initializing dynamic requests file:', error);
  }
}

// Parse output file to get elevator state
async function parseOutputFile() {
  try {
    const content = await fs.readFile(OUTPUT_FILE, 'utf-8');
    const lines = content.split('\n');
    
    let currentTurnData = null;
    const elevators = [];
    
    for (const line of lines) {
      if (line.startsWith('TURN_START:')) {
        currentTurnData = parseInt(line.split(':')[1]);
      } else if (line.startsWith('ELEVATOR:')) {
        const parts = line.split(':')[1].split(',');
        if (parts.length >= 5) {
          elevators.push({
            id: parseInt(parts[0]),
            floor: parseInt(parts[1]),
            direction: parts[2],
            load: parseInt(parts[3]),
            waiting: parseInt(parts[4])
          });
        }
      } else if (line.startsWith('STATS:')) {
        const parts = line.split(':')[1].split(',');
        if (parts.length >= 4) {
          elevatorData.stats = {
            turn: parseInt(parts[0]),
            totalPicked: parseInt(parts[1]),
            totalDropped: parseInt(parts[2]),
            totalFulfilled: parseInt(parts[3])
          };
        }
      }
    }
    
    if (currentTurnData !== null) {
      elevatorData.currentTurn = currentTurnData;
      elevatorData.elevators = elevators;
    }
    
    return elevatorData;
  } catch (error) {
    console.log('Output file not ready yet or error reading:', error.message);
    return elevatorData;
  }
}

// Routes

// Start simulation
app.post('/start', async (req, res) => {
  try {
    const { elevators, floors, solvers, max_turns } = req.body;
    
    console.log('Starting simulation with config:', req.body);
    
    // Initialize state
    currentTurn = 1;
    elevatorData = {
      elevators: Array.from({ length: elevators }, (_, i) => ({
        id: i,
        floor: 0,
        direction: 's',
        load: 0,
        waiting: 0
      })),
      floors: floors,
      currentTurn: 0,
      stats: {
        totalRequests: 0,
        fulfilledRequests: 0,
        totalMovement: 0
      }
    };
    
    // Initialize dynamic requests file
    await initializeDynamicRequestsFile();
    
    // Start the C program in background
    const helperPath = path.join(__dirname, '..', 'Helper and Testcases', 'helper');
    helperProcess = spawn(helperPath, ['1'], {
      cwd: path.join(__dirname, '..', 'Helper and Testcases'),
      detached: false
    });
    
    helperProcess.stdout.on('data', (data) => {
      console.log('Helper output:', data.toString());
    });
    
    helperProcess.stderr.on('data', (data) => {
      console.error('Helper error:', data.toString());
    });
    
    helperProcess.on('close', (code) => {
      console.log('Helper process exited with code:', code);
      isSimulationRunning = false;
    });
    
    isSimulationRunning = true;
    
    res.json({ 
      success: true, 
      message: 'Simulation started',
      turn: currentTurn
    });
  } catch (error) {
    console.error('Error starting simulation:', error);
    res.status(500).json({ error: 'Failed to start simulation' });
  }
});

// Submit a passenger request
app.post('/submit-request', async (req, res) => {
  try {
    const { startFloor, requestedFloor } = req.body;
    
    if (!isSimulationRunning) {
      return res.status(400).json({ error: 'Simulation not running' });
    }
    
    // Write request to dynamic file
    const requestLine = `${startFloor} ${requestedFloor} ${currentTurn}\n`;
    await fs.appendFile(DYNAMIC_REQUESTS_FILE, requestLine);
    
    console.log(`Added request: Floor ${startFloor} → Floor ${requestedFloor} at turn ${currentTurn}`);
    
    res.json({ 
      success: true, 
      message: 'Request submitted',
      request: { startFloor, requestedFloor, arrivalTime: currentTurn }
    });
  } catch (error) {
    console.error('Error submitting request:', error);
    res.status(500).json({ error: 'Failed to submit request' });
  }
});

// Get current simulation state
app.get('/status', async (req, res) => {
  try {
    const data = await parseOutputFile();
    res.json({
      isRunning: isSimulationRunning,
      data: data
    });
  } catch (error) {
    console.error('Error getting status:', error);
    res.status(500).json({ error: 'Failed to get status' });
  }
});

// Advance to next turn (optional - the simulation runs automatically)
app.post('/next-turn', async (req, res) => {
  try {
    currentTurn++;
    const data = await parseOutputFile();
    
    res.json({
      success: true,
      turn: currentTurn,
      data: data
    });
  } catch (error) {
    console.error('Error advancing turn:', error);
    res.status(500).json({ error: 'Failed to advance turn' });
  }
});

// End simulation
app.post('/end', async (req, res) => {
  try {
    if (helperProcess) {
      helperProcess.kill('SIGTERM');
      helperProcess = null;
    }
    
    isSimulationRunning = false;
    
    res.json({ 
      success: true, 
      message: 'Simulation ended' 
    });
  } catch (error) {
    console.error('Error ending simulation:', error);
    res.status(500).json({ error: 'Failed to end simulation' });
  }
});

// Health check
app.get('/health', (req, res) => {
  res.json({ 
    status: 'ok',
    simulation: isSimulationRunning ? 'running' : 'stopped',
    turn: currentTurn
  });
});

// Start server
app.listen(PORT, () => {
  console.log(`🚀 Elevator Backend Server running on http://localhost:${PORT}`);
  console.log(`📁 Dynamic requests file: ${DYNAMIC_REQUESTS_FILE}`);
  console.log(`📊 Output file: ${OUTPUT_FILE}`);
}); 