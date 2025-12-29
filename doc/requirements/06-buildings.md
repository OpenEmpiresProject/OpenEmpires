# 6. Buildings

This document details the functional requirements for all buildings, covering their attributes, construction, and unique functions within the game.

## 6.1 Core Building Attributes

Every building in the game shall possess a set of core attributes.

- **FR-6.1.1 Hit Points (HP):** The amount of damage a building can sustain before being destroyed.
- **FR-6.1.2 Armor:** Reduces incoming damage. This is split into `Melee Armor` and `Pierce Armor`. Buildings also have a specific "building" armor class.
- **FR-6.1.3 Line of Sight (LoS):** The radius around the building that reveals the map from the Fog of War.
- **FR-6.1.4 Resource Cost:** The amount of Food, Wood, Gold, and/or Stone required to construct the building.
- **FR-6.1.5 Build Time:** The base time in seconds required for a single Villager to construct the building.
- **FR-6.1.6 Population Provided:** For buildings like Houses and Town Centers, the amount of population cap they provide.
- **FR-6.1.7 Footprint:** The grid size the building occupies on the map, which affects placement.

## 6.2 Building Construction

This section describes how buildings are created by Villagers.

- **FR-6.2.1 Placement:** The player selects a building from the UI and places its "foundation" on the map. The placement is invalid if it overlaps with other units/buildings or impassable terrain.
- **FR-6.2.2 Foundation:** Once placed, the building appears as a foundation without any HP. The resource cost is deducted at this time.
- **FR-6.2.3 Construction Process:** A Villager tasked to build will move to the foundation and begin working, playing a construction animation.
- **FR-6.2.4 Build Progress:** As a Villager works, the building's HP increases. The building becomes operational only when it reaches 100% HP.
- **FR-6.2.5 Multiple Builders:** Multiple Villagers can work on the same foundation or repair the same building. Each additional Villager increases the construction/repair speed.
- **FR-6.2.6 Deletion:** The player can delete a building at any stage. If deleted during construction, a percentage of the resource cost is refunded. If deleted after completion, no resources are refunded.

## 6.3 Building States

A building can exist in several states.

- **FR-6.3.1 Foundation:** The initial state after placement, before construction is complete. The building is non-functional.
- **FR-6.3.2 Active/Operational:** The building is complete and can perform its functions (e.g., training units, researching tech).
- **FR-6.3.3 Damaged:** The building's HP is below its maximum. It may show visual damage (e.g., fires, cracks). It remains functional unless destroyed. If the HP goes below a certain limit, building will eject all garrisoned units at once.
- **FR-6.3.4 Researching/Training:** The building is actively working on a queued item.
- **FR-6.3.5 Destroyed:** The building's HP reaches 0. It turns into a non-interactive rubble pile and is removed from the player's control.

## 6.4 Building Categories & Functions

Buildings are grouped by their primary purpose.

- **FR-6.4.1 Economic Buildings:** Used for resource drop-off and economic upgrades (e.g., Town Center, Mill, Lumber Camp, Mining Camp, Market, Dock).
- **FR-6.4.2 Military Buildings:** Used to train military units (e.g., Barracks, Archery Range, Stable, Siege Workshop, Castle).
- **FR-6.4.3 Technology Buildings:** Used primarily to research non-economic upgrades (e.g., Blacksmith, University).
- **FR-6.4.4 Defensive Buildings:** Used for map control and defense (e.g., Walls, Gates, Towers).
- **FR-6.4.5 Population Buildings:** Primarily exist to increase the population limit (e.g., House).

## 6.5 Special Building Abilities

Certain buildings have unique interactive capabilities.

### 6.5.1 Garrisoning
- **FR-6.5.1.1:** Certain buildings (Town Center, Towers, Castle, Docks) can garrison units.
- **FR-6.5.1.2:** Garrisoned units are protected from harm and cannot be targeted.
- **FR-6.5.1.3:** The building's UI shall display the number and type of garrisoned units.
- **FR-6.5.1.4:** Some buildings will slowly heal garrisoned units over time.

### 6.5.2 Firing Projectiles
- **FR-6.5.2.1:** Town Centers, Towers, and Castles can fire arrows at enemy units that enter their range.
- **FR-6.5.2.2:** The number of projectiles fired increases based on the number of certain unit types garrisoned inside (e.g., Archers, Villagers).
- **FR-6.5.2.3:** The building's attack damage and range can be increased via technologies.

### 6.5.3 Setting Rally Points
- **FR-6.5.3.1:** Production buildings (military buildings, Town Centers, Docks) shall have a settable rally point.
- **FR-6.5.3.2:** Newly created units will automatically move to this rally point upon spawning.
- **FR-6.5.3.3:** The rally point can be set on a location, a resource node (for Villagers), or another building (to garrison).

### 6.5.4 Gates
- **FR-6.5.4.1:** Gates are special wall sections that allow friendly units to pass through.
- **FR-6.5.4.2:** Gates automatically open for friendly units and close afterward.
- **FR-6.5.4.3:** The player can manually lock a gate, preventing it from opening for any unit.
- **FR-6.5.4.4:** The player can set the gate's stance to be open for allies as well.

### 6.5.5 Research & Training
- **FR-6.5.5.1:** Buildings can have a queue for training units or researching technologies.
- **FR-6.5.5.2:** Only one item can be actively processed at a time. Other items wait in the queue.
- **FR-6.5.5.3:** The player can cancel items in the queue, and the full resource cost is refunded.