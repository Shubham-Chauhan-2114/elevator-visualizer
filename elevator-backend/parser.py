def parse_latest_turn(filepath, turn):
    with open(filepath, "r") as f:
        lines = f.readlines()

    block = []
    capture = False
    for line in lines:
        line = line.strip()
        if line == f"TURN_START:{turn}":
            capture = True
            block = []
        if capture:
            block.append(line)
        if line == f"TURN_END:{turn}":
            break

    elevators = []
    riders_waiting = []
    riders_in_elv = []
    stats = {}

    for line in block:
        if line.startswith("ELEVATOR:"):
            parts = line.split(":")[1].split(",")
            elevators.append({
                "id": int(parts[0]),
                "floor": int(parts[1]),
                "direction": parts[2],
                "load": int(parts[3]),
                "waitingQueueLength": int(parts[4])
            })
        elif line.startswith("RIDER_IN_ELV:"):
            parts = line.split(":")[1].split(",")
            riders_in_elv.append({
                "elevatorId": int(parts[0]),
                "requestId": int(parts[1]),
                "from": int(parts[2]),
                "to": int(parts[3])
            })
        elif line.startswith("RIDER_WAITING:"):
            parts = line.split(":")[1].split(",")
            riders_waiting.append({
                "elevatorId": int(parts[0]),
                "requestId": int(parts[1]),
                "from": int(parts[2]),
                "to": int(parts[3])
            })
        elif line.startswith("STATS:"):
            parts = line.split(":")[1].split(",")
            stats = {
                "turn": int(parts[0]),
                "pickedUp": int(parts[1]),
                "dropped": int(parts[2]),
                "fulfilled": int(parts[3])
            }

    return {
        "turn": turn,
        "elevators": elevators,
        "ridersInElevator": riders_in_elv,
        "ridersWaiting": riders_waiting,
        "stats": stats
    }
