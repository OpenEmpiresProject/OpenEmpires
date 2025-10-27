# 9. Ages & Technology Tree

This document details the functional requirements for the game's progression system, which includes advancing through ages and researching technologies from the technology tree.

## 9.1 Game Ages

The game is structured into four distinct ages, each unlocking more powerful units, buildings, and technologies.

- **FR-9.1.1 Dark Age:** The starting age. Limited units and buildings. Focus is on initial scouting and economy building.
- **FR-9.1.2 Feudal Age:** Unlocks basic military units (Archers, Spearmen), defensive walls, and initial economic upgrades.
- **FR-9.1.3 Castle Age:** Unlocks powerful units (Knights), siege weapons, Castles, and significant economic and military upgrades.
- **FR-9.1.4 Imperial Age:** The final age. Unlocks the most powerful units (e.g., Paladin, Trebuchet), all remaining technologies, and the strongest upgrades.

## 9.2 Advancing Ages

The process of moving from one age to the next is a major strategic investment.

- **FR-9.2.1 Initiation:** Advancing to the next age is an action initiated from the Town Center.
- **FR-9.2.2 Resource Cost:** Each age advancement has a specific Food and Gold cost.
- **FR-9.2.3 Building Prerequisites:** To advance to a new age, the player must have constructed specific buildings from their current age.
    - **To Feudal Age:** Requires two buildings from the Dark Age set: Barracks, Mill, Lumber Camp, Mining Camp, Dock.
    - **To Castle Age:** Requires two buildings from the Feudal Age set: Blacksmith, Market, Stable, Archery Range.
    - **To Imperial Age:** Requires a Castle and one building from the Castle Age set (e.g., Monastery, University). A civilization-specific exception may exist.
- **FR-9.2.4 Research Time:** The advancement process takes a significant amount of time, during which the Town Center is occupied and cannot train Villagers.
- **FR-9.2.5 Completion:** Upon completion, a global audio and visual notification is triggered for all players. The player who advanced gains access to all units, buildings, and technologies of the new age as permitted by their civilization's tech tree.

## 9.3 Technology Tree Structure

The technology tree is the complete map of all possible upgrades and units in the game.

- **FR-9.3.1 Dependency Graph:** The technology tree shall be structured as a dependency graph. Researching a technology or training a unit may require one or more prerequisite technologies to be completed first.
    - *Example: Researching the "Man-at-Arms" upgrade requires the player to be in the Feudal Age and have a Barracks.*
- **FR-9.3.2 Civilization-Specific Trees:** As defined in `08-civilizations.md`, each civilization has a unique technology tree where certain nodes (technologies, units, or buildings) are disabled.
- **FR-9.3.3 UI Representation:** The game must provide a UI screen that visually displays the full technology tree, clearly indicating available, unavailable, and completed items for the player's civilization.

## 9.4 Researching Technologies

This describes the process of acquiring individual upgrades.

- **FR-9.4.1 Research Location:** Each technology is researched at a specific building (e.g., "Forging" at the Blacksmith, "Loom" at the Town Center).
- **FR-9.4.2 Cost and Time:** Each technology has a resource cost and a time cost. Resources are deducted when research begins.
- **FR-9.4.3 Building Occupation:** The building where the research is taking place is considered busy and cannot be used for other research or training until the current research is complete.
- **FR-9.4.4 Permanent Effect:** Once a technology is researched, its effects are permanent for that player for the duration of the match.

## 9.5 Technology Types

Technologies can be broadly categorized by their effects.

- **FR-9.5.1 Economic Technologies:** These improve a player's economy.
    - *Example: "Double-Bit Axe" (Lumber Camp) increases the rate at which Villagers gather wood.*
    - *Example: "Wheelbarrow" (Town Center) increases Villager speed and carry capacity.*

- **FR-9.5.2 Military Stat Technologies:** These improve the combat statistics of classes of units.
    - *Example: "Forging" (Blacksmith) increases the attack of all infantry and cavalry units.*
    - *Example: "Bodkin Arrow" (Blacksmith) increases the attack and range of archer units.*

- **FR-9.5.3 Unit Line Upgrades:** These technologies upgrade an entire class of units to their next tier.
    - *Example: "Man-at-Arms" (Barracks) upgrades all existing Militia and allows the training of Man-at-Arms.*
    - *Example: "Crossbowman" (Archery Range) upgrades all existing Archers.*

- **FR-9.5.4 Building Technologies:** These improve buildings.
    - *Example: "Guard Tower" (University) upgrades all existing Watch Towers and allows the construction of Guard Towers.*
    - *Example: "Masonry" (University) increases the HP and armor of all buildings.*