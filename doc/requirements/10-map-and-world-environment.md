# 10. Map & World Environment

This document details the functional requirements for the game world, including map generation, terrain features, environmental objects, and the vision system.

## 10.1 Map Generation

The system for creating the play area for each match.

- **FR-10.1.1 Procedural Generation:** Maps shall be generated procedurally based on a selected map type  (e.g., Arabia, Black Forest, Islands).
- **FR-10.1.2 Map Type:** Each type defines rules for terrain placement, elevation, and distribution of resources and objects.
- **FR-10.1.3 Map Sizes:** The game shall support various map sizes (e.g., Tiny, Small, Medium, Large) which determine the overall dimensions of the grid.
- **FR-10.1.4 Player Placement:** The generation script must place starting Town Centers for each player according to fairness principles (e.g., equitable distance, non-disadvantaged terrain).
- **FR-10.1.5 Initial Resources:** Each player's starting location must be generated with a standard set of initial resources nearby (e.g., 1 Town Center, 3 Villagers, 1 Scout, 2-3 Sheep, 2 Boar, 6-7 Gold Mines, 4-5 Stone Mines, nearby forests and forage bushes).

## 10.2 Terrain & Grid

The map shall be based on a tile grid, with each tile having specific properties.

- **FR-10.2.1 Terrain Types:** The map shall consist of various terrain types, including but not limited to:
    - Grass, Dirt, Sand (standard buildable and walkable terrain)
    - Water, Deep Water (impassable to land units, navigable by ships)
    - Shallows (walkable by land units)
    - Forests (impassable but contain choppable trees)
    - Roads (may provide a movement speed bonus)
- **FR-10.2.2 Passability:** Each tile shall have a passability property, determining whether land units, sea units, or both can traverse it.
- **FR-10.2.3 Buildability:** Each tile shall have a buildability property, determining whether buildings can be placed on it.
- **FR-10.2.4 Elevation:** The map shall support multiple elevation levels (hills). Elevation affects combat (see FR-7.6.1) and Line of Sight.

## 10.3 Gaia (World) Objects

These are neutral objects and units that belong to the world ("Gaia").

- **FR-10.3.1 Resource Nodes:** Finite objects that can be gathered from.
    - **Trees:** Source of Wood.
    - **Forage Bushes:** Source of Food.
    - **Gold Mines:** Source of Gold.
    - **Stone Mines:** Source of Stone.
    - **Fish:** Nodes in water that are a source of Food for Fishing Ships.

- **FR-10.3.2 Fauna (Animals):**
    - **Herdables (e.g., Sheep, Turkey):** Passive animals that can be converted to a player's control by moving a unit near them. A source of Food.
    - **Huntables (e.g., Deer):** Passive animals that flee when approached. Must be killed to be harvested for Food.
    - **Aggressive Huntables (e.g., Boar):** Will attack a player's unit that attacks it. A rich source of Food.
    - **Predators (e.g., Wolves):** Aggressive units that will attack any player units that come into their line of sight.

- **FR-10.3.3 Relics:** Special objects that can be picked up by a Monk. When garrisoned in a Monastery, a Relic generates a slow, steady stream of Gold.

## 10.4 Exploration & Fog of War

The system that governs player vision and map discovery.

- **FR-10.4.1 Three States of Vision:** Each tile on the map shall exist in one of three states for each player:
    - **Unexplored (Black):** The tile has never been within a player's line of sight. It is completely black, and no information is known.
    - **Explored (Shrouded):** The tile has been seen before but is not currently in a player's line of sight. The terrain is visible, but units and changes to buildings (e.g., destruction) are not shown.
    - **Visible:** The tile is currently within the line of sight of a friendly unit or building. All terrain, units, and buildings are fully visible and updated in real-time.

- **FR-10.4.2 Line of Sight (LoS):**
    - Every unit and building shall have a LoS attribute, defining the radius of its vision.
    - The player's total visible area is the union of all their units' and buildings' LoS.

- **FR-10.4.3 LoS Blockers:**
    - Terrain elevation shall block LoS. A unit at the bottom of a hill cannot see what is on top or on the other side.
    - Trees and other foliage shall block LoS. A unit cannot see through a dense forest.

- **FR-10.4.4 Shared Vision:** In a team game, players shall share vision with their allies. A tile visible to one ally is visible to all allies.