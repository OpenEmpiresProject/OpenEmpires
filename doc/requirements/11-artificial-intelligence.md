# 11. Artificial Intelligence (AI)

This document details the functional requirements for the computer-controlled players (AI). The AI must be capable of playing the game competently, managing its economy, military, and technology to challenge a human player.

## 11.1 High-Level AI Behavior

- **FR-11.1.1 Goal-Oriented:** The AI's primary goal is to win the game by meeting one of the victory conditions (e.g., Conquest, Wonder, Relics).
- **FR-11.1.2 Strategic Personas:** The AI shall be capable of adopting different strategic "personas" or "personalities" (e.g., Aggressive Rusher, Defensive Boomer, Wonder Builder). These personas dictate its overall priorities.
- **FR-11.1.3 Adaptability:** The AI should be able to react to the human player's actions. For example, if scouted and found to be building a large army, the AI should prioritize defensive measures or a counter-army.
- **FR-11.1.4 Difficulty Scaling:** The AI's effectiveness shall scale with selectable difficulty levels (e.g., Easiest, Standard, Moderate, Hard, Hardest). This can be achieved through resource cheats, faster reaction times, or more optimal decision-making.

## 11.2 AI Scripting System

- **FR-11.2.1 Script Execution:** The AI shall be controlled by a script that defines its rules and behaviors. The game engine must provide an interpreter for these scripts.
- **FR-11.2.2 Game State Queries:** The AI script must be able to query the current game state. Examples:
    - `(player-resource wood > 500)`
    - `(enemy-unit-count knight > 10)`
    - `(player-in-age castle-age)`
- **FR-11.2.3 Action Commands:** The AI script must be able to issue commands. Examples:
    - `(train villager)`
    - `(build barracks)`
    - `(research feudal-age)`
    - `(attack-now)`

## 11.3 Economic Management

- **FR-11.3.1 Villager Tasking:** The AI must intelligently assign Villagers to gather resources based on its current strategic needs (e.g., needing more wood for archers, more food for an age-up).
- **FR-11.3.2 Building Placement:** The AI must be able to place buildings in logical locations (e.g., houses in a compact area, lumber camps near forests, defensive towers at chokepoints).
- **FR-11.3.3 Technology Research:** The AI must decide when to research economic and military technologies to keep pace with its overall strategy.
- **FR-11.3.4 Age Advancement:** The AI must make the strategic decision to advance to the next age, balancing the cost and time against immediate military or economic needs.

## 11.4 Military Management

- **FR-11.4.1 Unit Composition:** The AI shall build an army with a unit composition that is effective for its strategy and civilization. It should also be able to adapt its composition to counter the enemy's army.
- **FR-11.4.2 Attack & Defense Logic:** The AI must decide when to launch an attack, when to defend its base, and when to retreat from a losing fight.
- **FR-11.4.3 Target Prioritization:** During combat, AI-controlled units must prioritize targets intelligently (e.g., archers targeting infantry, spearmen targeting cavalry, siege targeting buildings).
- **FR-11.4.4 Group Movement:** The AI must be able to move its army in coordinated groups, not just as a stream of individual units.

## 11.5 Scouting

- **FR-11.5.1 Initial Exploration:** The AI shall use its starting scout unit to explore the map, locate its own resources, and find the enemy player(s).
- **FR-11.5.2 Information Gathering:** Throughout the game, the AI should periodically send units to scout the enemy's base to gather information on their army composition, technology level, and economic strength.

## 11.6 AI Cheats (Difficulty Scaling)

- **FR-11.6.1 Resource Cheats:** On higher difficulty levels, the AI shall be granted extra starting resources or a passive resource income to compensate for its lack of human-level efficiency.
- **FR-11.6.2 Vision Cheats:** On lower difficulty levels, the AI may have restricted vision similar to a human player. On higher difficulties, it may have partial or full map awareness to inform its decisions.