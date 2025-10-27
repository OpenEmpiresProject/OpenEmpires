# 2. Player & Camera Controls

This document details the functional requirements for how the player interacts with and views the game world, covering both camera movement and direct input commands.

## 2.1 Camera Controls

The player must be able to navigate the game map efficiently.

- **FR-2.1.1 Edge Scrolling:** Moving the mouse cursor to the edges of the game window shall pan the camera in that direction.
- **FR-2.1.2 Keyboard Scrolling:** Pressing the arrow keys (configurable) shall pan the camera in the corresponding direction.
- **FR-2.1.3 Minimap Navigation:** Clicking on a location on the minimap shall instantly center the camera on that location (as per FR-3.1.2.5).
- **FR-2.1.4 Zoom Control:** Using the mouse scroll wheel shall zoom the camera in and out, within predefined limits.
- **FR-2.1.5 Focus on Selection:** A hotkey (e.g., Spacebar) shall center the camera on the currently selected unit(s) or building. If no entity is selected, it may jump to the location of the last major event notification.
- **FR-2.1.6 Focus on Control Group:** Double-tapping a control group hotkey (e.g., pressing '1' twice quickly) shall center the camera on that group of units.

## 2.2 Player Input & Hotkeys

This section covers the primary input methods for controlling units and interacting with the game.

### 2.2.1 Mouse Controls
- **FR-2.2.1.1 Left-Click:**
    - Selects a single unit or building.
    - Clicks a UI button.
    - Confirms a building placement.
    - With drag, performs a box selection (as per FR-3.3.4.2).
- **FR-2.2.1.2 Right-Click (Context-Sensitive):**
    - On terrain: Issues a "Move" command to selected units.
    - On an enemy unit/building: Issues an "Attack" command.
    - On a resource node with a Villager selected: Issues a "Gather" command.
    - On a friendly building foundation with a Villager selected: Issues a "Build" command.
    - On a damaged friendly unit/building/ship with a Villager selected: Issues a "Repair" command.
    - On a garrisonable building with a unit selected: Issues a "Garrison" command.
    - On a building with a rally point: Sets the rally point.
- **FR-2.2.1.3 Left-Double-Click:**
    - Selects same type of units or buildings within the same window subject to a max limit.

### 2.2.2 Keyboard Hotkeys
- **FR-2.2.2.1 Command Hotkeys:** Specific keys shall be bound to commands in the command panel. For example, when a Villager is selected, pressing 'B' might open the build menu, and then 'H' might select the House for placement.
- **FR-2.2.2.2 Go-To Hotkeys:** The system shall support hotkeys that center the camera on specific important buildings.
    - *Example: A hotkey to cycle through Town Centers.*
    - *Example: A hotkey to cycle through idle Villagers.*
- **FR-2.2.2.3 Control Groups:** The system shall implement control groups as defined in FR-3.3.4.4.
    - **Assignment:** `Ctrl + [Number Key]` (e.g., Ctrl+1) assigns the currently selected units to that number group.
    - **Selection:** `[Number Key]` (e.g., 1) deselects all current units and selects the units in that group.
    - **Add to Group:** `Shift + [Number Key]` adds the currently selected units to the existing group without deselecting other units in that group.
- **FR-2.2.2.4 Global Hotkeys:** The system shall support global hotkeys that are always active.
    - *Example: A key to open the main menu (e.g., F10).*
    - *Example: A key to open the chat box (e.g., Enter).*
- **FR-2.2.2.5 Configurable Hotkeys:** The system should let players modifying any hotkey


### 2.2.3 Input Modifiers
- **FR-2.2.3.1 Shift Key:** Holding the `Shift` key while issuing commands shall queue them. For example, holding `Shift` while right-clicking multiple locations will create a series of move waypoints.
- **FR-2.2.3.2 Control Key:** Holding the `Ctrl` key while clicking a unit in the selection panel shall deselect all units of that type.
- **FR-2.2.3.3 Alt Key:** Holding the `Alt` key can be used for secondary commands or to modify behavior (e.g., `Alt` + right-click to set a formation facing direction).