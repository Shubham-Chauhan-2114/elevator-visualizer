from fastapi import FastAPI
from pydantic import BaseModel
from controller import ElevatorController
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:3000"],  # or ["*"] for all origins (dev only)
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

controller = ElevatorController()

class InitConfig(BaseModel):
    elevators: int
    floors: int
    solvers: int
    max_turns: int
    max_requests: int

@app.post("/start")
def start_system(config: InitConfig):
    controller.start_simulation(config.elevators, config.floors, config.solvers, config.max_turns, config.max_requests)
    return {"message": "Simulation started"}

class PassengerRequest(BaseModel):
    startFloor: int
    requestedFloor: int

@app.post("/submit-request")
def submit_request(req: PassengerRequest):
    controller.queue_request(req.startFloor, req.requestedFloor)
    return {"message": "Request queued"}

@app.post("/next-turn")
def next_turn():
    result = controller.run_next_turn()
    return result

@app.post("/end")
def end_simulation():
    controller.end_simulation()
    return {"message": "Simulation ended and cleaned up"}

