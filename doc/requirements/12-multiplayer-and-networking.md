# 12. Multiplayer & Networking

This document details the functional requirements for the multiplayer and networking systems, which allow multiple human players to compete against each other in real-time.

## 12.1 Game Lobby & Setup

The pre-game environment where players gather and configure the match.

- **FR-12.1.1 Lobby Creation & Discovery:** Players shall be able to host a game lobby (making it available on a local network or via direct connection) or join an existing lobby.
- **FR-12.1.2 Game Configuration:** The host of the lobby shall have the authority to set all game parameters, including but not limited to:
    - Map script and size
    - Victory conditions (e.g., Conquest, Wonder)
    - Player team assignments
    - Starting age and resources
- **FR-12.1.3 Player Configuration:** All players in the lobby shall be able to choose their civilization, color, and team (if not locked by the host).
- **FR-12.1.4 Readiness System:** Players must signal a "ready" status. The host can only launch the game when all players are marked as ready.
- **FR-12.1.5 Lobby Chat:** A text chat system shall be available for all players in the lobby.

## 12.2 Network Architecture & Synchronization

The core model for ensuring a consistent game state across all clients.

- **FR-12.2.1 Deterministic Lockstep Model:** The game shall be implemented using a deterministic lockstep architecture. All clients will execute the exact same simulation based on a shared stream of commands.
- **FR-12.2.2 Command-Based Communication:** The network traffic shall primarily consist of player commands (e.g., "Player 2 commands unit ID 123 to move to tile (45, 67)"), not the resulting game state changes.
- **FR-12.2.3 Turn-Based Execution:** The simulation will advance in discrete steps or "turns". A command issued by a player is tagged for execution on a future turn.
- **FR-12.2.4 Command Distribution:** The host or a dedicated server is responsible for collecting commands from all players for a given turn and broadcasting them back to all players.
- **FR-12.2.5 Simulation Halt:** The simulation on each client will not advance past turn `T` until it has received the command sets from all active players for turn `T`. This ensures all clients execute the same commands in the same order.

## 12.3 In-Game Communication

- **FR-12.3.1 Text Chat:** Players shall be able to send text messages during the game. This includes options to chat with "All" players, "Team" only, or specific individuals.
- **FR-12.3.2 Taunts:** The game shall support numeric audio taunts (e.g., typing "1" plays a "Yes" sound, "11" plays a laugh).

## 12.4 Session Management

- **FR-12.4.1 Disconnection Handling:** The system must robustly detect when a player loses connection.
- **FR-12.4.2 Game Pause on Drop:** When a player disconnects, the game shall automatically pause for all remaining players, displaying a notification.
- **FR-12.4.3 Save and Exit:** From the paused state, players shall have the option to save the game for later restoration or to drop the disconnected player and continue.
- **FR-12.4.4 Multiplayer Save/Load:** The game must support saving a multiplayer match. Restoring the game requires all original players to be present in a restoration lobby.

## 12.5 Desynchronization (Desync) Detection

- **FR-12.5.1 Checksum Verification:** To ensure simulations have not diverged, clients shall periodically compute and exchange checksums of critical game state data (e.g., total resource counts, unit positions).
- **FR-12.5.2 Desync Notification:** If a checksum mismatch occurs, it indicates a desync. The game must notify all players that the match is out of sync and can no longer continue fairly. The match should be terminated.