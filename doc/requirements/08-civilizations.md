# 8. Civilizations

This document details the functional requirements for civilizations, which are the distinct factions players can choose. Each civilization has a unique set of bonuses, units, and technologies that shape its playstyle.

## 8.1 Core Civilization Attributes

Each civilization shall be defined by a collection of unique attributes that differentiate it from others.

- **FR-8.1.1 Civilization Bonuses:** A list of passive bonuses that apply throughout the game. These can affect economic, military, or other aspects.
    - *Example: "Town Centers cost -50% Wood in the Castle Age."*
    - *Example: "Infantry attack 15% faster starting in the Feudal Age."*

- **FR-8.1.2 Team Bonus:** A specific bonus that applies to the player and all of their allies in a team game.
    - *Example: "Archery Ranges work 20% faster."*

- **FR-8.1.3 Unique Unit (UU):** Each civilization shall have at least one unique unit, trainable only at the Castle. This unit embodies the civilization's military identity.
    - *Example: The Britons have the Longbowman, a long-ranged archer.*

- **FR-8.1.4 Unique Technologies (UT):** Each civilization shall have two unique technologies, one available in the Castle Age and one in the Imperial Age. These are researched at the Castle.
    - *Example: The Britons have "Yeomen" (increases foot archer range) and "Warwolf" (gives Trebuchets blast damage).*

- **FR-8.1.5 Technology Tree:** Each civilization has a specific technology tree that dictates which generic units, buildings, and technologies are available or unavailable to them.
    - *Example: The Franks do not have access to the Arbalester upgrade for their archers.*
    - *Example: The Goths do not have access to Stone Walls.*

## 8.2 System Implementation

- **FR-8.2.1 Data-Driven Design:** All civilization attributes (bonuses, tech trees, unique units/techs) shall be defined in external data files (e.g., JSON, XML, or the planned Python entity scripts). This allows for easy modification and addition of new civilizations without changing core game code.

- **FR-8.2.2 Bonus Application:** The game engine must correctly apply all bonuses.
    - Static bonuses (e.g., "Villagers have +5 HP") should be applied at the start of the game or upon unit creation.
    - Conditional bonuses (e.g., "buildings build 10% faster starting in Feudal Age") must be activated when the required game state is met.

- **FR-8.2.3 Tech Tree UI:** The in-game technology tree screen must dynamically update to reflect the chosen civilization, graying out or hiding unavailable options.

## 8.3 Example Civilization Breakdown (Britons)

To illustrate the requirements, here is a breakdown of the Britons civilization.

- **Civilization Bonuses:**
    - Town Centers cost -50% wood starting in Castle Age.
    - Foot archers (except Skirmishers) have +1 range in Castle Age and +1 range in Imperial Age (for a total of +2).
    - Shepherds work 25% faster.

- **Unique Unit:**
    - **Longbowman:** An archer unit with exceptionally long range.

- **Unique Technologies:**
    - **Castle Age: Yeomen:** Grants +1 range to foot archers and +2 attack to towers.
    - **Imperial Age: Warwolf:** Gives Trebuchets blast damage and 100% accuracy against stationary targets.

- **Team Bonus:**
    - Archery Ranges work 20% faster.

- **Notable Tech Tree Gaps:**
    - No access to Hand Cannoneer.
    - No access to Siege Onager or Heavy Scorpion.
    - Stable is limited, lacking Bloodlines and Paladin.