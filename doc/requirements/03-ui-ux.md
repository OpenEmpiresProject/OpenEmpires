# 3. UI / UX (User Interface & User Experience)

This document details the functional requirements for all user interface elements, user controls, and feedback mechanisms.

## 3.1 Main Game HUD (Heads-Up Display)

The HUD is the primary interface visible during gameplay. It shall be non-intrusive and provide all necessary information at a glance.

### 3.1.1 Resource Panel
- **FR-3.1.1.1 Resource Display:** A panel at the top of the screen shall display the player's current count for all resource types.
- **FR-3.1.1.2 Population Display:** The panel shall also display the current population and current population capacity (e.g., `50/75`).
- **FR-3.1.1.3 Housed Alert:** When the player is "housed" (population is at maximum), the population display shall be visually distinct (e.g., change color) to alert the player.

### 3.1.2 Minimap
- **FR-3.1.2.1 Minimap Display:** A minimap shall be displayed, typically in the bottom-right corner.
- **FR-3.1.2.2 Minimap Entity Rendering:** The minimap shall render terrain, units, and buildings using simple colored dots/squares corresponding to player colors.
- **FR-3.1.2.3 Minimap Fog of War:** The minimap must reflect the player's current Fog of War state (unexplored, explored, visible).
- **FR-3.1.2.4 Minimap Camera Frustum:** A camera frustum rectangle shall be drawn on the minimap, indicating the portion of the map currently visible on the main screen.
- **FR-3.1.2.5 Minimap Navigation:** Players can click or drag on the minimap to quickly move the main camera to that location.
- **FR-3.1.2.6 Minimap Alerts:** The minimap must show important events (eg: getting attacked) in a proper alerting mechanism

### 3.1.3 Selection & Command Panel
- **FR-3.1.3.1 Selection Panel:** A panel, typically at the bottom-left, shall display information about the currently selected unit(s) or building.
- **FR-3.1.3.2 Single Selection Info:** For a single selection, it shows the entity's icon, name, HP, and other relevant stats.
- **FR-3.1.3.3 Multi-Selection Info:** For a multiple-unit selection, it shows icons for each selected unit type and a count for each.
- **FR-3.1.3.4 Command Grid:** A grid of command buttons shall be displayed, showing all actions the selected entity can perform (e.g., build, attack, research).
- **FR-3.1.3.5 Production Queue Display:** If a production building is selected, this panel will show the unit/technology queue and progress.
- **FR-3.1.3.6 Garrison Display:** If a garrisonable building is selected, this panel will show the units garrisoned in the building.

## 3.2 Menus & Screens

These are the full-screen interfaces outside of direct gameplay.

- **FR-3.2.1 Main Menu:** The initial screen on game launch, providing options for: Single Player, Multiplayer, Options, and Exit Game.
- **FR-3.2.2 Game Setup Screen:** A lobby screen for configuring a match (map, civilization, color, teams, etc.) for both single-player and multiplayer games.
- **FR-3.2.3 Pause Menu:** An in-game menu that pauses the simulation and provides options: Resume, Save Game, Load Game, Options, and Resign/Exit Match.
- **FR-3.2.4 Technology Tree Screen:** A dedicated screen showing the full technology tree for the player's current civilization, indicating researched, available, and unavailable items.
- **FR-3.2.5 Game Summary Screen:** A post-game screen displaying statistics for all players (e.g., units killed, resources gathered, timeline).

## 3.3 Player Interaction & Feedback

This section covers the direct feedback mechanisms for player actions.

### 3.3.1 Mouse Cursors
- **FR-3.3.1.1 Dynamic Cursor:** The mouse cursor shall change dynamically to reflect the current context.
- **FR-3.3.1.2 Cursor States:** Examples of cursor states include:
    - **Default:** Standard pointer.
    - **Move:** When hovering over walkable terrain with a unit selected.
    - **Attack:** When hovering over an enemy unit/building.
    - **Gather:** When hovering over a resource node with a Villager selected.
    - **Build:** When in building placement mode.
    - **Garrison:** When garrison action is initiated for a unit(s).
    - **Cannot Perform Action:** A "forbidden" or red-tinted cursor for invalid actions.

### 3.3.2 Tooltips
- **FR-3.3.2.1 Tooltip Display:** Hovering the mouse over a command button, a unit in the queue, or a technology shall display a tooltip.
- **FR-3.3.2.2 Tooltip Content:** The tooltip shall provide more information, such as the name, resource cost, and a brief description of the item.

### 3.3.3 Visual Feedback
- **FR-3.3.3.1 Building Placement Preview:** When placing a building, a footprint/outline of the building (or the full building) shall be shown under the cursor, colored to indicate valid or invalid (e.g., red) placement.
- **FR-3.3.3.2 Health Bar Display:** Health bars shall be displayed above units and buildings during combat or when damaged and when selected. Friendly, enemy, and neutral units shall have distinct health bar colors.
- **FR-3.3.3.3 Rally Point Indicator:** When a production building is selected, its rally point shall be visually indicated on the map with a flag and a connecting line.
- **FR-3.3.3.4 Minimap Event Notifications:** A visual indicator (e.g., crossed swords icon) shall appear on the minimap at the location of a significant event, such as a building being destroyed or units being attacked outside the current view.

### 3.3.4 Unit Selection
- **FR-3.3.4.1 Single Click Selection:** Selects a single unit or building.
- **FR-3.3.4.2 Box Selection:** Clicking and dragging the mouse shall draw a box to select all units within it.
- **FR-3.3.4.3 Double-Click Selection:** Double-clicking a unit shall select all other units of the same type currently visible on the screen.
- **FR-3.3.4.4 Control Groups:** Players can assign selected units to a control group (e.g., via Ctrl+1) and later re-select that group by pressing the corresponding number key (e.g., 1).