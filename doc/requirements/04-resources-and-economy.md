# 4. Resources & Economy

This document details the functional requirements for the resource and economic systems in the game.

## 4.1 Resource Types

- **FR-4.1.1 Food Resource:** The game shall include "Food" as a primary resource, used for training most units, researching technologies, and advancing ages. It is gathered from sources like forage bushes, animals, fish, and farms.
- **FR-4.1.2 Wood Resource:** The game shall include "Wood" as a primary resource, used for constructing buildings, farms, ships, and training archer/siege units. It is gathered from trees.
- **FR-4.1.3 Gold Resource:** The game shall include "Gold" as a primary resource, used for training advanced units and researching advanced technologies. It is gathered from gold mines, relics, and trade.
- **FR-4.1.4 Stone Resource:** The game shall include "Stone" as a primary resource, used for constructing defensive structures like walls, towers, and castles. It is gathered from stone mines.

## 4.2 Resource Gathering

This section describes the process by which units (primarily Villagers) collect resources.

### 4.2.1 Gathering Process
- **FR-4.2.1.1 Villager Tasking:** A Villager assigned to a resource node (e.g., a tree, a gold mine) will move to it.
- **FR-4.2.1.2 Gathering Animation:** Upon reaching the node, the Villager will play a gathering animation (e.g., chopping wood, mining gold).
- **FR-4.2.1.3 Inventory Accrual:** Resources are added to the Villager's carried inventory at a fixed rate per second (the "gather rate").
- **FR-4.2.1.4 Gather Rate:** Each resource type has a base gather rate. This rate can be modified by civilization bonuses and technologies.
- **FR-4.2.1.5 Node Depletion:** Resource nodes are finite and have a specific amount of resources. When depleted, the node is removed or changes its visual state (e.g., a tree stump).
- **FR-4.2.1.6 Auto Lookup for Similar:** Once the resource node the unit was assigned to is depleted, the unit should look for the nearest same resource type and gather.

### 4.2.2 Carry Capacity
- **FR-4.2.2.1 Base Carry Capacity:** Villagers have a maximum amount of a resource they can carry at one time (e.g., 10 units of Food, Wood, Gold, or Stone).
- **FR-4.2.2.2 Carry Capacity Upgrades:** This carry capacity can be increased by specific technologies (e.g., Wheelbarrow, Hand Cart).

### 4.2.3 Resource Drop-off
- **FR-4.2.3.1 Auto Drop-off:** When a Villager's carry capacity is full, they will automatically stop gathering and seek the nearest valid drop-off building.
- **FR-4.2.3.2 Valid Drop-off Buildings:** Valid drop-off buildings are:
    - **All Resources:** Town Center.
    - **Food:** Mill.
    - **Wood:** Lumber Camp.
    - **Gold/Stone:** Mining Camp.
    - **Food (from Fish):** Dock.
- **FR-4.2.3.3 Resource Deposit:** Upon reaching the drop-off building, the Villager deposits their carried resources into the player's global resource pool.
- **FR-4.2.3.4 Auto Return to Work:** After dropping off resources, the Villager will automatically return to the last-worked resource node to continue gathering, unless a new command was issued.
- **FR-4.2.3.5 Manual Drop-off:** The player can manually command a Villager to drop off resources at any time, even if not at full capacity.

## 4.3 Player Resource Management

This section covers the player's interaction with the global resource pool.

### 4.3.1 Resource Storage
- **FR-4.3.1.1 Resource UI Display:** The current count for each resource shall be displayed on the game's main UI (HUD).

### 4.3.2 Population
- **FR-4.3.2.1 Population Limit:** Population determines the maximum number of units a player can control. This should be configurable at the game setup.
- **FR-4.3.2.2 Unit Population Cost:** Each unit shall costs a certain amount of population (typically 1, some unique units may vary).
- **FR-4.3.2.3 Population Capacity Source:** The maximum population is determined by the number of Houses and the Town Center(s) the player has built.
- **FR-4.3.2.4 Population UI Display:** The UI shall display current population and current population capacity (e.g., `45/50`).
- **FR-4.3.2.5 Population Limit Enforcement:** Players cannot create new units if it would cause their current population to exceed their maximum population.

### 4.3.3 Spending Resources
- **FR-4.3.3.1 Action Resource Cost:** Actions like creating a unit, constructing a building, or researching a technology have a resource cost.
- **FR-4.3.3.2 Resource Check:** Before an action can be initiated, the system must verify the player has sufficient resources (including population).
- **FR-4.3.3.3 Resource Deduction:** If the player has sufficient resources, the cost is immediately deducted from their global resource pool, and the action begins.
- **FR-4.3.3.4 Insufficient Resource Feedback:** If the player does not have sufficient resources, the action cannot be started, and UI feedback (e.g., a sound, a disabled button) should be provided.

## 4.4 Special Economic Units & Buildings

### 4.4.1 Farms
- **FR-4.4.1.1 Farm Construction:** A Farm is a building constructed by a Villager that provides a source of Food. Construction requires Wood.
- **FR-4.4.1.2 Farm Gathering:** Only one Villager can be assigned to a Farm to gather Food from it.
- **FR-4.4.1.3 Farm Depletion:** Farms have a finite amount of Food. Once depleted, they must be "re-seeded" (re-built) at a cost of Wood.
- **FR-4.4.1.4 Farm Auto-Reseed:** A "Farm Re-seeding Queue" shall be available after researching the "Mill" technology, allowing farms to be automatically re-seeded if the player has enough wood.

### 4.4.2 Fishing Ships & Fish Traps
- **FR-4.4.2.1 Fishing Ship Gathering:** Fishing Ships are naval units that gather Food from fish nodes in the water.
- **FR-4.4.2.2 Fishing Ship Drop-off:** They drop off their gathered Food at a Dock.
- **FR-4.4.2.3 Fish Trap Mechanics:** Fish Traps are naval structures that provide an infinite, but slow, source of Food. A fishing ship must be assigned to work the trap.

### 4.4.3 Trade Carts & Trade Cogs
- **FR-4.4.3.1 Trade Unit Function:** Trade Carts (land) and Trade Cogs (sea) are units used to generate Gold.
- **FR-4.4.3.2 Trade Route Assignment:** A player can set a target Market or Dock belonging to another player (ally).
- **FR-4.4.3.3 Trade Route Travel:** The trade unit will travel from the player's Market/Dock to the target Market/Dock.
- **FR-4.4.3.4 Gold Generation:** Upon reaching the target, the unit generates an amount of Gold based on the distance traveled.
- **FR-4.4.3.5 Trade Loop:** The unit will then automatically return to its home Market/Dock to deposit the Gold, and repeat the trip.