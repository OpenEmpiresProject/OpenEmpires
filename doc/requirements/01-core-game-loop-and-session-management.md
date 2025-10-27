# 1. Core Game Loop & Session Management

This document details the functional requirements for the overall game flow, from starting and ending a match to saving and loading game states.

## 1.1 Game Initialization

- **FR-1.1.1 Game Start:** A match shall be initiated from the Game Setup Screen (as per FR-3.2.2) after all configurations are set.
- **FR-1.1.2 Asset Validating:** Upon starting, the game must validate all necessary assets, including scripts, definitions, configurations, and graphics/sound resources.
- **FR-1.1.3 World Generation:** The game world shall be generated according to the selected map and settings (as per FR-10.1), including placing initial player units (Town Center, Villagers, Scout) and resources.

## 1.2 Main Game Loop

- **FR-1.2.1 Tick-Based Simulation:** The game state simulation shall advance in discrete, fixed-time steps (ticks). This is crucial for ensuring deterministic behavior required for multiplayer (FR-12.2.1).
- **FR-1.2.2 Input Processing:** Player inputs (mouse and keyboard) are collected and processed on each tick.
- **FR-1.2.3 State Update:** All game logic (movement, combat, resource gathering, etc.) is updated based on the state on each tick.
- **FR-1.2.4 Rendering:** The visual representation of the game state is rendered to the screen. This process should be in-sync with the simulation.

## 1.3 Victory Conditions

The game shall support multiple, configurable victory conditions. The first player or team to meet a condition wins the match.

- **FR-1.3.1 Conquest:** A player/team wins if all enemy players have resigned or have been defeated (all units and production buildings destroyed).
- **FR-1.3.2 Wonder:** A player who builds a Wonder and successfully defends it for a specified duration (e.g., 200 game years) wins. A countdown timer must be visible to all players.
- **FR-1.3.3 Relics:** A player who collects all Relics on the map and holds them for a specified duration wins. A countdown timer must be visible to all players.
- **FR-1.3.4 Time Limit:** If a time limit is set, the player/team with the highest score when the timer expires wins.

## 1.4 Defeat & Resignation

- **FR-1.4.1 Resignation:** A player can choose to resign at any time via the in-game menu. This immediately removes them from the game as a defeated player.
- **FR-1.4.2 Conquest Defeat:** A player is defeated under the Conquest condition if they have no units and no buildings capable of producing units or constructing other buildings (e.g., Town Centers, Barracks, Castles, Villagers).
- **FR-1.4.3 Game End Trigger:** When only one player or team remains who has not been defeated, the game ends.

## 1.5 Session Management (Save & Load)

This applies primarily to single-player games and pre-arranged multiplayer restorations.

- **FR-1.5.1 Save Game:** The player shall be able to save the entire current game state to a file at any time via the pause menu. This must include:
    - All unit and building states (HP, position, commands, etc).
    - Player resource counts, technologies researched, and current age.
    - The state of the map, including Fog of War for each player.
- **FR-1.5.2 Load Game:** The player shall be able to load a previously saved game file from the main menu or pause menu, restoring the game to the exact state it was in when saved.
- **FR-1.5.3 Multiplayer Save/Load:** As per FR-12.4.4, multiplayer games can be saved. Loading requires all original players to be present in a special lobby to resume the session.

## 1.6 Game Conclusion

- **FR-1.6.1 Game Over Screen:** Upon the conclusion of a match (through victory, defeat, or resignation), the player shall be taken to a Game Summary screen.
- **FR-1.6.2 Statistics Display:** The summary screen (as per FR-3.2.5) shall display detailed statistics for all players, such as military, economy, technology scores, and a timeline of key events.