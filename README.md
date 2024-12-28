# Battle Game Server

## Overview

The **Battle Game Server** is a text-based multiplayer game server implemented in C using Unix sockets. It allows players to engage in a turn-based battle modeled after a Pokemon-style combat system. The server facilitates dynamic matchmaking, real-time gameplay, and in-game communication. This project demonstrates the use of Unix sockets, process control, and non-blocking I/O.

---

## Features

### Core Features
- **Dynamic Matchmaking**: Matches players automatically based on availability.
- **Turn-Based Combat**:
  - Players can attack (`a`) or use powermoves (`p`).
  - Regular attacks are guaranteed but weak, while powermoves are stronger but have a 50% chance to miss.
- **Real-Time Messaging**: Players can chat during their turn without using their action.
- **Leaderboard**: Tracks and displays the top three players based on wins.

### Gameplay Mechanics
- Each player starts with:
  - **Hitpoints (HP)**: Randomly assigned between 20-30.
  - **Powermoves**: Randomly assigned between 1-3.
- Damage ranges:
  - Regular attacks: 2-6 HP.
  - Powermoves: 6-18 HP (if they hit).
- Players take turns until one loses all hitpoints.

### Fault Tolerance
- Handles client disconnections gracefully by declaring the opponent the winner.
- Supports multiple simultaneous matches.


## Getting Started
### Prerequisites
- GCC or any compatible C compiler.
- Linux or macOS environment (not tested on Windows).
- Basic understanding of Unix sockets.

### Building the Program
To compile the project, use the provided `Makefile`:
```bash
make PORT=<your-port-number>
```
*Replace <your-port-number> with your designated port (e.g., 53456)*

### Running the Program
Start the server with:
```bash
./battle
```

### Clients can connect using:
```bash
stty -icanon; nc <server-ip> <port>
```
*Replace <server-ip> with the server's IP and <port> with the port number used during compilation.*

## How to Play
*Please note that only users with valid UOFT credentials may be able to login to the server and play the game.*

1. Connect to the Server:
- Run ./battle on the server.
- Use stty -icanon; nc <server-ip> <port> to connect as a client.

2. Gameplay Commands:
- a: Perform a regular attack (2-6 HP damage).
- p: Use a powermove (6-18 HP damage, 50% hit chance).
- s: Send a message to your opponent during your turn.

3. Matchmaking:
- Players are automatically matched when an opponent is available.
- Matches are random and independent.

4. Winning:
- Reduce your opponent's HP to 0 to win.
- The leaderboard tracks the top players based on wins.

## Example Output:
```plaintext
Welcome to the Battle Game Server!
Waiting for an opponent...

You engage Jack!
Your hitpoints: 25
Your powermoves: 2

Options:
(s) Speak
(a) Regular attack
(p) Powermove

> a
You hit Jack for 4 damage!
```

## Key Functions

1. **Dynamic Matchmaking**  
   - Matches players based on availability and ensures players who recently fought do not get rematched immediately.
   - Handles multiple concurrent matches independently.

2. **Real-Time Gameplay**  
   - Processes player inputs during their turn, allowing for regular attacks (`a`), powermoves (`p`), and in-game messaging (`s`).
   - Updates and broadcasts the game state to both players after every action.

3. **Signal Handling**  
   - Manages client disconnections gracefully by notifying the opponent of their win and resetting the server state for new matches.
   - Ensures all resources (sockets and processes) are cleaned up properly.

4. **Leaderboard Management**  
   - Maintains and updates a session-based leaderboard, tracking the number of wins for each player.
   - Displays the top three players dynamically after each match.

## License 
This project is licensed for educational purposes. Please contact me for other use cases.


