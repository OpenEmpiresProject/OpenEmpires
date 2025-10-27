# OpenEmpires Feature Matrix

This document tracks the implementation status of all defined functional requirements for the `openEmpires` project.

| Index | Category | FR code | Feature | Status | Remarks |
|:---:|---|---|---|:---:|---|
| 1 | **Core Game Loop** | FR-1.1.1 | Game Start | | |
| 2 | | FR-1.1.2 | Asset Validating | | |
| 3 | | FR-1.1.3 | World Generation | | |
| 4 | | FR-1.2.1 | Tick-Based Simulation | ✅ | |
| 5 | | FR-1.2.2 | Input Processing | ✅ | |
| 6 | | FR-1.2.3 | State Update | ✅ | |
| 7 | | FR-1.2.4 | Rendering | ✅ | |
| 8 | | FR-1.3.1 | Conquest | | |
| 9 | | FR-1.3.2 | Wonder | | |
| 10 | | FR-1.3.3 | Relics | | |
| 11 | | FR-1.3.4 | Time Limit | | |
| 12 | | FR-1.4.1 | Resignation | | |
| 13 | | FR-1.4.2 | Conquest Defeat | | |
| 14 | | FR-1.4.3 | Game End Trigger | | |
| 15 | | FR-1.5.1 | Save Game | | |
| 16 | | FR-1.5.2 | Load Game | | |
| 17 | | FR-1.5.3 | Multiplayer Save/Load | | |
| 18 | | FR-1.6.1 | Game Over Screen | | |
| 19 | | FR-1.6.2 | Statistics Display | | |
| 20 | **Player Controls** | FR-2.1.1 | Edge Scrolling | ✅ | |
| 21 | | FR-2.1.2 | Keyboard Scrolling | ✅ | |
| 22 | | FR-2.1.3 | Minimap Navigation | | |
| 23 | | FR-2.1.4 | Zoom Control | | |
| 24 | | FR-2.1.5 | Focus on Selection | | |
| 25 | | FR-2.1.6 | Focus on Control Group | | |
| 26 | | FR-2.2.1.1 | Left-Click | | |
| 27 | | FR-2.2.1.2 | Right-Click (Context-Sensitive) | | |
| 27 | | FR-2.2.1.3 | Left-Double-Click | | |
| 28 | | FR-2.2.2.1 | Command Hotkeys | | |
| 29 | | FR-2.2.2.2 | Go-To Hotkeys | | |
| 30 | | FR-2.2.2.3 | Control Groups | | |
| 31 | | FR-2.2.2.4 | Global Hotkeys | | |
| 31 | | FR-2.2.2.5 | Configurable Hotkeys | | |
| 32 | | FR-2.2.3.1 | Shift Key | | |
| 33 | | FR-2.2.3.2 | Control Key | | |
| 34 | | FR-2.2.3.3 | Alt Key | | |
| 35 | **UI / UX** | FR-3.1.1.1 | Resource Display | | |
| 36 | | FR-3.1.1.2 | Population Display | ✅ | |
| 37 | | FR-3.1.1.3 | Housed Alert | | |
| 38 | | FR-3.1.2.1 | Minimap Display | | |
| 39 | | FR-3.1.2.2 | Minimap Entity Rendering | | |
| 40 | | FR-3.1.2.3 | Minimap Fog of War | | |
| 41 | | FR-3.1.2.4 | Minimap Camera Frustum | | |
| 42 | | FR-3.1.2.5 | Minimap Navigation | | |
| 42 | | FR-3.1.2.6 | Minimap Alerts | | |
| 43 | | FR-3.1.3.1 | Selection Panel | | |
| 44 | | FR-3.1.3.2 | Single Selection Info | | |
| 45 | | FR-3.1.3.3 | Multi-Selection Info | | |
| 46 | | FR-3.1.3.4 | Command Grid | | |
| 47 | | FR-3.1.3.5 | Production Queue Display | | |
| 47 | | FR-3.1.3.6 | Garrison Display | | |
| 48 | | FR-3.2.1 | Main Menu | | |
| 49 | | FR-3.2.2 | Game Setup Screen | | |
| 50 | | FR-3.2.3 | Pause Menu | | |
| 51 | | FR-3.2.4 | Technology Tree Screen | | |
| 52 | | FR-3.2.5 | Game Summary Screen | | |
| 53 | | FR-3.3.1.1 | Dynamic Cursor | | |
| 54 | | FR-3.3.1.2 | Cursor States | | |
| 55 | | FR-3.3.2.1 | Tooltip Display | | |
| 56 | | FR-3.3.2.2 | Tooltip Content | | |
| 57 | | FR-3.3.3.1 | Building Placement Preview | | |
| 58 | | FR-3.3.3.2 | Health Bar Display | | |
| 59 | | FR-3.3.3.3 | Rally Point Indicator | | |
| 60 | | FR-3.3.3.4 | Minimap Event Notifications | | |
| 61 | | FR-3.3.4.1 | Single Click Selection | ✅ | |
| 62 | | FR-3.3.4.2 | Box Selection | ✅ | |
| 63 | | FR-3.3.4.3 | Double-Click Selection | | |
| 64 | | FR-3.3.4.4 | Control Groups | | |
| 65 | **Resources & Economy** | FR-4.1.1 | Food Resource | | |
| 66 | | FR-4.1.2 | Wood Resource | ✅ | |
| 67 | | FR-4.1.3 | Gold Resource | ✅ | |
| 68 | | FR-4.1.4 | Stone Resource | ✅ | |
| 69 | | FR-4.2.1.1 | Villager Tasking | ✅ | |
| 70 | | FR-4.2.1.2 | Gathering Animation | ✅ | |
| 71 | | FR-4.2.1.3 | Inventory Accrual | ✅ | |
| 72 | | FR-4.2.1.4 | Gather Rate | ✅ | |
| 73 | | FR-4.2.1.5 | Node Depletion | ✅ | |
| 73 | | FR-4.2.1.6 | Auto Lookup for Similar | ✅ | |
| 74 | | FR-4.2.2.1 | Base Carry Capacity | ✅ | |
| 75 | | FR-4.2.2.2 | Carry Capacity Upgrades | | |
| 76 | | FR-4.2.3.1 | Auto Drop-off | ✅ | |
| 77 | | FR-4.2.3.2 | Valid Drop-off Buildings | | |
| 78 | | FR-4.2.3.3 | Resource Deposit | ✅ | |
| 79 | | FR-4.2.3.4 | Auto Return to Work | ✅ | |
| 80 | | FR-4.2.3.5 | Manual Drop-off | ✅ | |
| 81 | | FR-4.3.1.1 | Resource UI Display | | |
| 82 | | FR-4.3.2.1 | Population Limit | | |
| 83 | | FR-4.3.2.2 | Unit Population Cost | ✅ | |
| 84 | | FR-4.3.2.3 | Population Capacity Source | ✅ | |
| 85 | | FR-4.3.2.4 | Population UI Display | ✅ | |
| 86 | | FR-4.3.2.5 | Population Limit Enforcement | ✅ | |
| 87 | | FR-4.3.3.1 | Action Resource Cost | | |
| 88 | | FR-4.3.3.2 | Resource Check | | |
| 89 | | FR-4.3.3.3 | Resource Deduction | | |
| 90 | | FR-4.3.3.4 | Insufficient Resource Feedback | | |
| 91 | | FR-4.4.1.1 | Farm Construction | | |
| 92 | | FR-4.4.1.2 | Farm Gathering | | |
| 93 | | FR-4.4.1.3 | Farm Depletion | | |
| 94 | | FR-4.4.1.4 | Farm Auto-Reseed | | |
| 95 | | FR-4.4.2.1 | Fishing Ship Gathering | | |
| 96 | | FR-4.4.2.2 | Fishing Ship Drop-off | | |
| 97 | | FR-4.4.2.3 | Fish Trap Mechanics | | |
| 98 | | FR-4.4.3.1 | Trade Unit Function | | |
| 99 | | FR-4.4.3.2 | Trade Route Assignment | | |
| 100 | | FR-4.4.3.3 | Trade Route Travel | | |
| 101 | | FR-4.4.3.4 | Gold Generation | | |
| 102 | | FR-4.4.3.5 | Trade Loop | | |
| 103 | | FR-4.4.3.3 | Trade Route Travel | | |
| 104 | | FR-4.4.3.4 | Gold Generation | | |
| 105 | | FR-4.4.3.5 | Trade Loop | | |
| 106 | **Units** | FR-5.1.1 | Hit Points (HP) | | |
| 107 | | FR-5.1.2 | Attack Damage | | |
| 108 | | FR-5.1.3 | Armor | | |
| 109 | | FR-5.1.4 | Range | | |
| 110 | | FR-5.1.5 | Line of Sight (LoS) | | |
| 111 | | FR-5.1.6 | Movement Speed | | |
| 112 | | FR-5.1.7 | Attack Rate | | |
| 113 | | FR-5.1.8 | Population Cost | | |
| 114 | | FR-5.1.9 | Resource Cost | | |
| 115 | | FR-5.1.10 | Creation Time | | |
| 116 | | FR-5.2.1 | Move Command | | |
| 117 | | FR-5.2.2 | Attack Command | | |
| 118 | | FR-5.2.3 | Attack Move Command | | |
| 119 | | FR-5.2.4 | Patrol Command | | |
| 120 | | FR-5.2.5 | Stop Command | | |
| 121 | | FR-5.2.6 | Garrison Command | | |
| 122 | | FR-5.2.7 | Ungarrison Command | | |
| 123 | | FR-5.2.8 | Set Rally Point | | |
| 124 | | FR-5.3.1 | Idle State | | |
| 125 | | FR-5.3.2 | Moving State | | |
| 126 | | FR-5.3.3 | Attacking State | | |
| 127 | | FR-5.3.4 | Gathering State | | |
| 128 | | FR-5.3.5 | Building/Repairing State | | |
| 129 | | FR-5.3.6 | Garrisoned State | | |
| 130 | | FR-5.3.7 | Converting State | | |
| 131 | | FR-5.3.8 | Healing State | | |
| 131 | | FR-5.3.9 | Villager attacking | | |
| 132 | | FR-5.3.10.1 | Aggressive Stance | | |
| 133 | | FR-5.3.10.2 | Defensive Stance | | |
| 134 | | FR-5.3.10.3 | Stand Ground Stance | | |
| 135 | | FR-5.3.10.4 | No Attack Stance | | |
| 136 | | FR-5.3.11.1 | Group Formation | | |
| 137 | | FR-5.3.11.2 | Intelligent Formation | | |
| 138 | | FR-5.4.1 | Unit Queuing | | |
| 139 | | FR-5.4.2 | Unit Resource Deduction | | |
| 140 | | FR-5.4.3 | Unit Queue Cancellation | | |
| 141 | | FR-5.4.4 | Unit Spawning | | |
| 142 | | FR-5.5.1 | Villager Role | | |
| 143 | | FR-5.5.2 | Infantry Role | | |
| 144 | | FR-5.5.3 | Archer Role | | |
| 145 | | FR-5.5.4 | Cavalry Role | | |
| 146 | | FR-5.5.5 | Siege Role | | |
| 147 | | FR-5.5.6 | Monk Role | | |
| 148 | | FR-5.5.7 | Naval Role | | |
| 149 | | FR-5.5.8 | Unique Unit Role | | |
| 150 | | FR-5.6.1 | Villager Repair | | |
| 151 | | FR-5.6.2 | Monk Conversion | | |
| 152 | | FR-5.6.3 | Monk Healing | | |
| 153 | | FR-5.6.4 | Siege Pack/Unpack | | |
| 154 | | FR-5.6.5 | Transport Load/Unload | | |
| 155 | | FR-5.6.6 | Demolition Ship Detonate | | |
| 156 | **Buildings** | FR-6.1.1 | Building HP | | |
| 157 | | FR-6.1.2 | Building Armor | | |
| 158 | | FR-6.1.3 | Building LoS | | |
| 159 | | FR-6.1.4 | Building Resource Cost | | |
| 160 | | FR-6.1.5 | Building Build Time | | |
| 161 | | FR-6.1.6 | Building Population Provided | | |
| 162 | | FR-6.1.7 | Building Footprint | | |
| 163 | | FR-6.2.1 | Building Placement | | |
| 164 | | FR-6.2.2 | Building Foundation | | |
| 165 | | FR-6.2.3 | Building Construction Process | | |
| 166 | | FR-6.2.4 | Building Progress | | |
| 167 | | FR-6.2.5 | Multiple Builders | | |
| 168 | | FR-6.2.6 | Building Deletion | | |
| 169 | | FR-6.5.1.1 | Building Garrisoning | | |
| 170 | | FR-6.5.1.2 | Garrisoned Unit Protection | | |
| 171 | | FR-6.5.1.3 | Garrison UI | | |
| 172 | | FR-6.5.1.4 | Garrison Healing | | |
| 173 | | FR-6.5.2.1 | Building Firing | | |
| 174 | | FR-6.5.2.2 | Building Garrison Attack Bonus | | |
| 175 | | FR-6.5.2.3 | Building Attack Upgrades | | |
| 176 | | FR-6.5.3.1 | Building Rally Points | | |
| 177 | | FR-6.5.4.1 | Gate Function | | |
| 178 | | FR-6.5.5.1 | Building Queues | | |
| 179 | **Combat System** | FR-7.1.1 | Base Damage Formula | | |
| 180 | | FR-7.1.2 | Minimum Damage | | |
| 181 | | FR-7.1.3 | Attack Types | | |
| 182 | | FR-7.1.4 | Armor Types | | |
| 183 | | FR-7.2.1 | Armor Classes | | |
| 184 | | FR-7.2.2 | Bonus Damage | | |
| 185 | | FR-7.3.1 | Projectile Spawning | | |
| 186 | | FR-7.3.2 | Projectile Travel Time | | |
| 187 | | FR-7.3.3 | Projectile Accuracy | | |
| 188 | | FR-7.3.4 | Ballistics Technology | | |
| 189 | | FR-7.3.5 | Chemistry Technology | | |
| 190 | | FR-7.4.1 | Splash Damage | | |
| 191 | | FR-7.4.2 | Damage Falloff | | |
| 192 | | FR-7.4.3 | Friendly Fire | | |
| 193 | | FR-7.5.1 | Attack Move Logic | | |
| 194 | | FR-7.5.2 | Retaliation Logic | | |
| 195 | | FR-7.5.3 | Target Persistence | | |
| 196 | | FR-7.6.1 | Elevation Bonus | | |
| 197 | | FR-7.6.2 | Garrison Attack Bonus | | |
| 198 | | FR-7.5.3 | Target Persistence | | |
| 199 | | FR-7.6.1 | Elevation Bonus | | |
| 200 | | FR-7.6.2 | Garrison Attack Bonus | | |
| 201 | **Civilizations** | FR-8.1.1 | Civilization Bonuses | | |
| 202 | | FR-8.1.2 | Team Bonus | | |
| 203 | | FR-8.1.3 | Unique Unit (UU) | | |
| 204 | | FR-8.1.4 | Unique Technologies (UT) | | |
| 205 | | FR-8.1.5 | Civilization Tech Tree | | |
| 206 | | FR-8.2.1 | Data-Driven Civ Design | | |
| 207 | | FR-8.2.2 | Bonus Application | | |
| 208 | | FR-8.2.3 | Tech Tree UI | | |
| 209 | **Ages & Tech Tree** | FR-9.1.1 | Dark Age | | |
| 210 | | FR-9.1.2 | Feudal Age | | |
| 211 | | FR-9.1.3 | Castle Age | | |
| 212 | | FR-9.1.4 | Imperial Age | | |
| 213 | | FR-9.2.1 | Age Up Initiation | | |
| 214 | | FR-9.2.2 | Age Up Resource Cost | | |
| 215 | | FR-9.2.3 | Age Up Building Prerequisites | | |
| 216 | | FR-9.2.4 | Age Up Research Time | | |
| 217 | | FR-9.2.5 | Age Up Completion | | |
| 218 | | FR-9.3.1 | Tech Tree Dependency Graph | | |
| 219 | | FR-9.3.2 | Civilization-Specific Tech Trees | | |
| 220 | | FR-9.3.3 | Tech Tree UI Representation | | |
| 221 | | FR-9.4.1 | Tech Research Location | | |
| 222 | | FR-9.4.2 | Tech Cost and Time | | |
| 223 | | FR-9.4.3 | Tech Building Occupation | | |
| 224 | | FR-9.4.4 | Tech Permanent Effect | | |
| 225 | | FR-9.5.1 | Economic Technologies | | |
| 226 | | FR-9.5.2 | Military Stat Technologies | | |
| 227 | | FR-9.5.3 | Unit Line Upgrades | | |
| 228 | | FR-9.5.4 | Building Technologies | | |
| 229 | **Map & World** | FR-10.1.1 | Procedural Map Generation | | |
| 230 | | FR-10.1.2 | Map Scripts | | |
| 231 | | FR-10.1.3 | Map Sizes | | |
| 232 | | FR-10.1.4 | Player Placement | | |
| 233 | | FR-10.1.5 | Initial Resources | | |
| 234 | | FR-10.2.1 | Terrain Types | | |
| 235 | | FR-10.2.2 | Tile Passability | | |
| 236 | | FR-10.2.3 | Tile Buildability | | |
| 237 | | FR-10.2.4 | Tile Elevation | | |
| 238 | | FR-10.3.1 | Resource Nodes | | |
| 239 | | FR-10.3.2 | Fauna (Animals) | | |
| 240 | | FR-10.3.3 | Relics | | |
| 241 | | FR-10.4.1 | Three States of Vision | | |
| 242 | | FR-10.4.2 | Line of Sight (LoS) | | |
| 243 | | FR-10.4.3 | LoS Blockers | | |
| 244 | | FR-10.4.4 | Shared Vision | | |
| 245 | **AI** | FR-11.1.1 | AI Goal-Oriented | | |
| 246 | | FR-11.1.2 | AI Strategic Personas | | |
| 247 | | FR-11.1.3 | AI Adaptability | | |
| 248 | | FR-11.1.4 | AI Difficulty Scaling | | |
| 249 | | FR-11.2.1 | AI Script Execution | | |
| 250 | | FR-11.2.2 | AI Script State Queries | | |
| 251 | | FR-11.2.3 | AI Script Action Commands | | |
| 253 | | FR-11.3.1 | AI Villager Tasking | | |
| 254 | | FR-11.3.2 | AI Building Placement | | |
| 255 | | FR-11.3.3 | AI Tech Research | | |
| 256 | | FR-11.3.4 | AI Age Advancement | | |
| 257 | | FR-11.4.1 | AI Unit Composition | | |
| 258 | | FR-11.4.2 | AI Attack/Defense Logic | | |
| 259 | | FR-11.4.3 | AI Target Prioritization | | |
| 260 | | FR-11.4.4 | AI Group Movement | | |
| 261 | | FR-11.5.1 | AI Initial Scouting | | |
| 262 | | FR-11.5.2 | AI Info Gathering | | |
| 263 | | FR-11.6.1 | AI Resource Cheats | | |
| 264 | | FR-11.6.2 | AI Vision Cheats | | |
| 265 | **Multiplayer** | FR-12.1.1 | Lobby Creation/Discovery | | |
| 266 | | FR-12.1.2 | Lobby Game Configuration | | |
| 267 | | FR-12.1.3 | Lobby Player Configuration | | |
| 268 | | FR-12.1.4 | Lobby Readiness System | | |
| 269 | | FR-12.1.5 | Lobby Chat | | |
| 270 | | FR-12.2.1 | Net: Deterministic Lockstep | | |
| 271 | | FR-12.2.2 | Net: Command Communication | | |
| 272 | | FR-12.2.3 | Net: Turn-Based Execution | | |
| 273 | | FR-12.2.4 | Net: Command Distribution | | |
| 274 | | FR-12.2.5 | Net: Simulation Halt | | |
| 275 | | FR-12.3.1 | In-Game Chat | | |
| 276 | | FR-12.3.2 | In-Game Taunts | | |
| 277 | | FR-12.4.1 | Session: Disconnect Handling | | |
| 278 | | FR-12.4.2 | Session: Pause on Drop | | |
| 279 | | FR-12.4.3 | Session: Save and Exit | | |
| 280 | | FR-12.4.4 | Session: MP Save/Load | | |
| 281 | | FR-12.5.1 | Desync: Checksum | | |
| 282 | | FR-12.5.2 | Desync: Notification | | |
| 283 | **Audio & Visuals** | FR-13.1.1.1 | SFX: Unit Feedback | | |
| 284 | | FR-13.1.1.2 | SFX: Economic Activity | | |
| 285 | | FR-13.1.1.3 | SFX: Combat | | |
| 286 | | FR-13.1.1.4 | SFX: UI | | |
| 287 | | FR-13.1.1.5 | SFX: Notifications | | |
| 288 | | FR-13.1.2.1 | Dynamic Music | | |
| 289 | | FR-13.1.2.2 | Civilization Themes | | |
| 290 | | FR-13.1.3.1 | Ambient Sounds | | |
| 291 | | FR-13.2.1.1 | State-Based Animations | | |
| 292 | | FR-13.2.1.2 | Directional Sprites | | |
| 293 | | FR-13.2.2.1 | Construction Animation | | |
| 294 | | FR-13.2.2.2 | Damage States | | |
| 295 | | FR-13.2.2.3 | Destruction Animation | | |
| 296 | | FR-13.2.2.4 | Building Working Animation | | |
| 297 | | FR-13.2.3.1 | VFX: Projectiles | | |
| 298 | | FR-13.2.3.2 | VFX: Impacts | | |
| 299 | | FR-13.2.3.3 | VFX: Area of Effect | | |
| 300 | | FR-13.2.3.4 | VFX: Selection Indicators | | |
| 301 | | FR-13.2.4.1 | Player Colors | | |
| 302 | | FR-13.2.4.2 | Fog of War Rendering | | |
