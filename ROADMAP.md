# Roadmap: OpenEmpires

Welcome to the development roadmap of our openEmpires game. This document outlines our high-level goals, features in progress, and long-term plans. It is intended to keep contributors aligned and help new developers understand where the project is heading.

## Versioning Strategy
We use milestone versions for planning. Early versions are experimental. Feature sets build progressively.

---

## v0.1 – Engine Core & Map Rendering (Tech Foundation)
> Goal: Create a playable sandbox map with movable units.

### Features:
- [x] Window & graphics initialization
- [x] Tile-based isometric map rendering
- [x] Resource loader for images
- [x] Camera movement
- [x] Basic tree rendering (without shadows)
- [x] Unit selection (click and box)
- [x] Command system for units with move and idle commands
- [x] Basic unit movement (click-to-move with path preview)
- [ ] DRS reader
- [x] Building placement system
- [x] Basic building construction (mill)
- [ ] Occlusion culling

---

## v0.2 – Resource Gathering & Economy
> Goal: Create a basic economy with villagers and buildings.

### Features:
- [ ] Trees as gatherable resources
- [ ] Resource collection (wood)
- [ ] Resource UI panel
- [ ] Resource drop-off points
- [ ] Group path finding (with flow fields)
- [ ] Stone and gold mines as gatherable resources
- [ ] Resource collection (stone, gold)
- [ ] LOS (Line of Sight) 
- [ ] Fog of war

---

## v0.3 – Base Building & Population
> Goal: Enable building construction, population limits, and basic game loop.

### Features:
- [ ] Town center, house and barracks mechanics
- [ ] Building queue system
- [ ] Population limit system
- [ ] Multiple player support (not networking)
- [ ] Scripting for entity definition
- [ ] Unit abilities modeling
- [ ] Garrison
- [ ] Walls and gates

---

## v0.4 – Combat System
> Goal: Enable unit combat, formations, and health mechanics.

### Features:
- [ ] Combat units - militia
- [ ] Health and damage system
- [ ] Attack-move
- [ ] Basic formation movement
- [ ] Death animations and corpse handling
- [ ] Projectiles
- [ ] Combat units - archer

---

## v0.5 – Basic AI & Skirmish Mode
> Goal: Add enemy AI and single-player matches.

### Features:

- [ ] AI APIs
- [ ] AI scripting
- [ ] AI economy and military
- [ ] AI difficulty settings (concept only)
- [ ] Victory conditions and triggers
- [ ] Skirmish mode setup (1v1)

---

## v0.6 – Sound & Music
> Goal: Add immersive audio and feedback.

### Features:
- [ ] Resource loader for sounds
- [ ] Animals
- [ ] Ambient sounds (birds, forests)
- [ ] Unit sounds (selection, attack, death)
- [ ] Sound system with volume control

---

## v0.7 – UI Polish & Menus
> Goal: Improve user experience and add game menu system.

### Features:
- [ ] Main menu with game start options
- [ ] Pause and settings menu
- [ ] In-game minimap
- [ ] Build and command UI improvements
- [ ] Tooltip and hover hints
- [ ] Save/load games

---

## v0.8 – Civilization and unit upgrades
> Goal: Enriching with all game entities.

### Focus:
- [ ] Civilizations
- [ ] Full set of buildings and units
- [ ] Unit upgrades and tech tree

---

## v0.9 – Multiplayer (LAN/P2P)
> Goal: Enable two players to play over a network.

### Features:
- [ ] Deterministic simulation model (lockstep or rollback)
- [ ] Lobby and game joining
- [ ] Basic sync protocol (UDP preferred)
- [ ] Chat between players
- [ ] Replay and desync detection

---

## v1.0 – Stable Release
> Goal: First complete, balanced, polished version.

### Focus:
- [ ] Performance and memory optimization
- [ ] Sanitization
- [ ] User-Acceptance-Tests (UAT)

---

## Long-Term Goals

### Modding Support
- Support mods for:
  - Units
  - Buildings
- Campaigns
- Mod loader and sandbox mode

### Expanded Features
- Water units and naval combat
- Terrain elevation
- Multiplayer ladder and matchmaking
- Spectator mode and replay sharing
- Steaming solutions integration