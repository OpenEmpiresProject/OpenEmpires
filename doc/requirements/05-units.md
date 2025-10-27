# 5. Units

This document details the functional requirements for all units in the game, covering their attributes, behaviors, and interactions.

## 5.1 Core Unit Attributes

Every unit in the game shall possess a set of core attributes that define its capabilities.

- **FR-5.1.1 Hit Points (HP):** The amount of damage a unit can sustain before being destroyed.
- **FR-5.1.2 Attack Damage:** The base amount of damage a unit deals with each attack. This is split into melee and pierce damage types.
- **FR-5.1.3 Armor:** Reduces incoming damage. This is split into `Melee Armor` and `Pierce Armor`.
- **FR-5.1.4 Range:** The maximum distance from which a unit can attack. Melee units have a range of 0.
- **FR-5.1.5 Line of Sight (LoS):** The radius around the unit that reveals the map from the Fog of War.
- **FR-5.1.6 Movement Speed:** The rate at which the unit traverses the map, measured in tiles per second.
- **FR-5.1.7 Attack Rate / Reload Time:** The time delay between a unit's consecutive attacks.
- **FR-5.1.8 Population Cost:** The amount of population space the unit occupies.
- **FR-5.1.9 Resource Cost:** The amount of Food, Wood, Gold, and/or Stone required to train the unit.
- **FR-5.1.10 Creation Time:** The time in seconds required to train the unit in a building.

## 5.2 Unit Commands

These are the actions a player can issue to one or more selected units.

- **FR-5.2.1 Move:** Unit moves to the target location and becomes idle upon arrival.
- **FR-5.2.2 Attack:** Unit moves to and attacks a specific target unit or building. If the target moves, the unit will pursue it.
- **FR-5.2.3 Attack Move:** Unit moves to a target location, automatically engaging any enemy units or buildings that enter its aggression range along the way.
- **FR-5.2.4 Patrol:** Unit moves back and forth along a path defined by its starting point and a target point, engaging any enemies it encounters.
- **FR-5.2.5 Stop:** Unit immediately ceases its current action and becomes idle.
- **FR-5.2.6 Garrison:** Unit moves to a target building (e.g., Town Center, Tower, Castle) and enters it.
- **FR-5.2.7 Ungarrison / Eject:** Ejects garrisoned units from a selected building to a specified rally point.
- **FR-5.2.8 Set Rally Point:** A command for buildings to set a location where newly created units will automatically move. A unit factory can have itself as the rally point to use upon creating units. Such buildings cannot be used for typical manual garrisoning.

## 5.3 Unit States & Behavior

A unit shall exist in one of several states, which dictate its current behavior.

- **FR-5.3.1 Idle:** The unit is not performing any action. It may play an idle animation. It will automatically engage enemies that come within a certain range, depending on its stance.
- **FR-5.3.2 Moving:** The unit is traversing the map to a destination.
- **FR-5.3.3 Attacking:** The unit is actively engaging an enemy. This includes moving into range, performing the attack animation, and waiting for its attack reload time.
- **FR-5.3.4 Gathering:** A Villager is collecting a resource from a node.
- **FR-5.3.5 Building / Repairing:** A Villager is constructing a new building or repairing a damaged friendly building/ship.
- **FR-5.3.6 Garrisoned:** The unit is inside another object (building, ram, transport ship) and is inactive, but protected.
- **FR-5.3.7 Converting:** A Monk is attempting to convert an enemy unit.
- **FR-5.3.8 Healing:** A Monk is healing a friendly unit.
- **FR-5.3.9 Villager attacking:** Villager can attack other units as melee and natural animals (for food) as ranged.

### 5.3.10 Stances
Units shall have selectable combat stances that modify their auto-engagement behavior.
- **FR-5.3.10.1 Aggressive Stance:** Unit will automatically pursue and attack any enemy that enters its LoS.
- **FR-5.3.10.2 Defensive Stance:** Unit will attack enemies that come within a short range and being attacked, but will not pursue them far from its starting position.
- **FR-5.3.10.3 Stand Ground:** Unit will not move from its position, but will attack any enemy that comes into its attack range.
- **FR-5.3.10.4 No Attack Stance:** Unit will not attack enemies under any circumstances, even if attacked.

### 5.3.11 Formations
- **FR-5.3.11.1:** When a group of units is commanded to move, they shall attempt to maintain a formation (e.g., Line, Staggered, Box).
- **FR-5.3.11.2:** The formation should intelligently arrange units, placing melee units at the front and ranged/siege units at the back.

## 5.4 Unit Creation

- **FR-5.4.1 Queuing:** Players can queue multiple units for training in a production building under limits specific to the building.
- **FR-5.4.2 Resource Deduction:** Resources are deducted at the moment a unit is added to the queue, not when it begins training.
- **FR-5.4.3 Cancellation:** A player can cancel a unit in the queue, and the full resource cost is refunded.
- **FR-5.4.4 Spawning:** Upon completion of training, the unit appears at the building's exit and proceeds to the rally point.

## 5.5 Unit Categories & Roles

Units are grouped into broad categories that define their general role on the battlefield.

- **FR-5.5.1 Villagers:** Economic units responsible for gathering resources, constructing, and repairing buildings.
- **FR-5.5.2 Infantry:** Melee ground units, generally strong against buildings and archers, but weak to cavalry.
- **FR-5.5.3 Archers:** Ranged ground units, effective against infantry but vulnerable to cavalry and siege.
- **FR-5.5.4 Cavalry:** Fast-moving mounted units, strong against archers and siege, but often countered by infantry with spears (e.g., Pikemen).
- **FR-5.5.5 Siege Units:** Specialized units for destroying buildings and groups of units. Often slow and vulnerable.
- **FR-5.5.6 Monks:** Support units that can heal friendly units and convert enemy units.
- **FR-5.5.7 Naval Units:** All units that operate on water, including Fishing Ships, Transport Ships, and warships.
- **FR-5.5.8 Unique Units:** A special unit for each civilization, trainable only at the Castle.

## 5.6 Special Unit Abilities

Certain units possess unique abilities beyond standard move and attack commands.

- **FR-5.6.1 Villager - Repair:** Villagers can be tasked to repair friendly buildings, siege units, and ships at the cost of resources (Wood for buildings/ships, Gold for siege).
- **FR-5.6.2 Monk - Conversion:** A Monk can target an enemy unit and, after a successful "chant," convert it to the Monk's player's control. Conversion has a chance to fail, requiring the Monk to rest before trying again.
- **FR-5.6.3 Monk - Healing:** A Monk can target a friendly biological unit to restore its HP over time.
- **FR-5.6.4 Siege - Pack/Unpack:** Some siege units (e.g., Trebuchets) must be unpacked to attack and packed to move.
- **FR-5.6.5 Transport - Load/Unload:** Transport Ships can load friendly ground units to carry them across water.
- **FR-5.6.6 Demolition Ship - Detonate:** A naval unit that can be commanded to self-destruct, dealing massive area-of-effect damage.