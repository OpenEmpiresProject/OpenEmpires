# 7. Combat System

This document details the functional requirements for the combat mechanics, including how damage is calculated, how units target enemies, and the various factors that influence the outcome of a battle.

## 7.1 Damage Calculation

The core formula for determining damage in any engagement.

- **FR-7.1.1 Base Formula:** The final damage dealt to a target shall be calculated as `(Unit's Attack + Bonus Damage) - Target's Relevant Armor`.
- **FR-7.1.2 Minimum Damage:** If the target's armor is greater than or equal to the incoming attack damage, the attack shall deal a minimum of 1 damage.
- **FR-7.1.3 Attack Types:** Every source of damage shall have a specific type (e.g., Melee, Pierce, Siege).
- **FR-7.1.4 Armor Types:** Every unit and building shall have corresponding armor values for each attack type (e.g., Melee Armor, Pierce Armor). The `Relevant Armor` in the damage formula is the armor type that matches the incoming attack type.

## 7.2 Attack Bonuses (Counter System)

A critical component of the game's strategic depth is the system of bonus damage that creates unit counters.

- **FR-7.2.1 Armor Classes:** In addition to armor values, units and buildings shall be assigned one or more "armor classes" (e.g., `Infantry`, `Archer`, `Cavalry`, `Building`, `Ship`, `Eagle Warrior`, `Camel`).
- **FR-7.2.2 Bonus Damage:** Certain units shall have an innate bonus damage attribute against specific armor classes. This bonus damage is added to their base attack before the target's armor is subtracted.
- **FR-7.2.3 Examples:**
    - A Spearman (low base attack) has high bonus damage vs. the `Cavalry` armor class.
    - A Skirmisher has bonus damage vs. the `Archer` armor class.
    - A Mangonel has bonus damage vs. the `Building` armor class.

## 7.3 Projectile Mechanics

For ranged units, the attack is not instantaneous.

- **FR-7.3.1 Projectile Spawning:** When a ranged unit attacks, it shall spawn a projectile object (e.g., an arrow, a bolt, a rock) that travels from the attacker to the target's location.
- **FR-7.3.2 Travel Time:** Projectiles have a defined speed. Damage is applied only when the projectile impacts the target.
- **FR-7.3.3 Accuracy:** Ranged units shall have an accuracy rating (percentage). Each shot has a chance to miss the target, based on this rating.
- **FR-7.3.4 Ballistics Technology:** Researching "Ballistics" shall enable units to fire at the predicted future location of a moving target, rather than its current location, increasing the chance to hit.
- **FR-7.3.5 Chemistry Technology:** Researching "Chemistry" shall change arrow projectiles to "burning arrows" for visual effect and is a prerequisite for certain units/upgrades. It also adds +1 attack to all archer-type units.

## 7.4 Area of Effect (AoE) Damage

Some units deal damage in an area rather than to a single target.

- **FR-7.4.1 Splash Damage:** Units like Mangonels, Scorpions, and Demolition Ships shall deal damage in a radius around the impact point.
- **FR-7.4.2 Damage Falloff:** The damage dealt by an AoE attack may be highest at the center of the impact and decrease towards the edge of the radius.
- **FR--7.4.3 Friendly Fire:** AoE attacks from certain units (e.g., Mangonels) shall damage all units in the radius, including the owner's own units (friendly fire). This should be a configuration option to set during game setup.

## 7.5 Targeting & Aggression (Aggro)

This defines how and when units decide to attack an enemy.

- **FR-7.5.1 Attack Move Logic:** A unit on an "Attack Move" command will engage the first enemy that enters its aggression range. The default priority should be the closest enemy unit.
- **FR-7.5.2 Retaliation:** A unit that is attacked by an enemy will, unless under a "No Attack" or "Stand Ground" stance, retaliate against the attacker, overriding its current command.
- **FR-7.5.3 Target Persistence:** An attacking unit will continue to pursue its current target until the target is destroyed, moves out of range, or the attacker is given a new command.

## 7.6 Environmental & Positional Modifiers

The terrain can provide combat advantages.

- **FR-7.6.1 Elevation Bonus (Hill Advantage):**
    - Units attacking from a higher elevation than their target shall deal a percentage-based damage bonus (e.g., +25% damage).
    - Units attacking a target on higher elevation shall deal a percentage-based damage penalty.
    - This bonus shall apply to both ranged and melee units.
- **FR-7.6.2 Garrison Bonus:**
    - Garrisoning certain units (e.g., foot archers, villagers) inside specific buildings (e.g., Towers, Town Centers, Castles) shall increase the number of projectiles fired by the building.