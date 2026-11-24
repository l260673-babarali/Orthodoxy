# Orthodoxy

**Orthodoxy** is a gothic 2D action-platformer built in **C++** with **SFML 3**. Play as a knight navigating ruined monasteries, cemeteries, and dangerous platforms while fighting increasingly difficult enemies and collecting scrolls to improve your abilities.

The project is developed in **Visual Studio for Windows** and is still in active development.

## Gameplay Features

- **Movement and platforming:** walking, double jumping, crouching, crouch-walking, dashing, wall climbing, and dropping through thin platforms.
- **Sword combat:** light attacks and a heavy attack that activates after holding **K for 0.50 seconds**.
- **Enemies:** Skeleton (undead), Burst Spirit, Phoenix, and Golem, with region-based difficulty and controlled enemy respawning.
- **Boss fight:** Draconic Lord encounter in a separate arena.
- **Progression:** eight collectible scrolls that can be spent on Vitality, Strength, Endurance, and Agility upgrades.
- **Game statistics:** kill counter, combo tracking, stage timer, objectives, and stage-clear results.
- **Save and load:** F5/F9 shortcuts, a Continue option, and automatic saves during region transitions.
- **Presentation:** animated sprites, gothic HUD and menus, dedicated Controls screen, background music, combat sounds, ambient effects, portals, and camera tracking.
- **Hazards:** projectiles, spikes, and enemy attacks. Dashing briefly avoids incoming damage.

## Regions

The current progression order is:

1. **Ashen Barrow** — ruined outdoor platforms and undead enemies.
2. **Ashen Cemetery** — a continuation of the opening area, with more platforming and enemies.
3. **Red Moon Ruins** — a more challenging area featuring Skeletons, Burst Spirits, and Phoenixes.
4. **Cathedral** — interior gothic ruins featuring stronger enemies, including Golems.
5. **Boss Room** — the final Draconic Lord encounter.

Players advance through portals, with a stage-clear screen between regions.

## Controls

You can also view these controls from the **title screen** or **pause menu**.

| Key | Action |
| --- | --- |
| **A / D** | Move left / right |
| **Space** | Jump / double jump |
| **S** | Crouch |
| **S + Space** | Drop through a thin platform |
| **Left Shift** | Dash |
| **J** | Light sword attack |
| **Hold K** | Charged heavy attack (0.50 seconds) |
| **Space + J** near a tall wall | Wall climb |
| **E** near a portal | Move toward the next region |
| **U** | Open or close upgrades |
| **F5** | Save progress |
| **F9** | Load progress |
| **Esc** | Pause / resume / go back |
| **W / S or arrow keys** | Navigate menus |
| **Enter / Space** | Confirm menu selection |

The wall climb uses a short input buffer, so Space and J can be pressed almost together.

## Building and Running

### Requirements

- **Windows** with **Visual Studio** and C++ desktop development support.
- The **MSVC toolset** and Windows SDK required by the included Visual Studio project.
- **SFML 3.1.0 (MSVC)** — the project expects its existing `SFML-3.1.0 MSVC` folder to remain inside `Orthodoxy/`.

### Steps

1. Clone or download the repository.
2. Open **`Orthodoxy/Orthodoxy.vcxproj`** in Visual Studio (or open the repository's solution if available).
3. Choose **x64** and **Debug** to start. The project uses **C++17** for x64 configurations.
4. Make sure the project can find `Orthodoxy/SFML-3.1.0 MSVC/include`, `lib`, and `bin`. The project file already references these locations and copies the SFML DLLs after building.
5. Set the **debugging working directory** to the folder containing `Assets` (normally `$(ProjectDir)`).
6. Build and run the project.

**Important:** Keep the `SFML-3.1.0 MSVC` folder and the `Assets` folder in their expected locations. The game loads textures, music, and effects using relative file paths. The UI also looks for fonts in the standard Windows Fonts directory.

## Project Structure

```text
README.md
Orthodoxy/
├── Assets/
│   ├── Sounds/
│   └── Sprites/
│       ├── Backgrounds/
│       ├── Effects/
│       ├── Enemies/
│       ├── Health/
│       ├── Items/
│       └── Player/
├── Classes/
│   ├── Camera.h
│   ├── Enemy.h
│   ├── Engine.h
│   ├── GameAudio.h
│   ├── GameplayExtras.h
│   ├── Level.h
│   ├── Player.h
│   ├── Projectile.h
│   └── Spike.h
├── Source Files/
│   ├── Camera.cpp
│   ├── Enemy.cpp
│   ├── Engine.cpp
│   ├── Level.cpp
│   ├── Player.cpp
│   ├── Projectile.cpp
│   └── Spike.cpp
├── SFML-3.1.0 MSVC/
├── main.cpp
└── Orthodoxy.vcxproj
```

### Main Code Responsibilities

| Component | Responsibility |
| --- | --- |
| `Engine` | Window, game loop, menus, enemy management, game progress, HUD, save/load |
| `Player` | Movement, platform collisions, animations, sword combat, RPG stats |
| `Level` | Region backgrounds, platforms, ramps, portals, spawn positions |
| `Enemy` | Shared enemy behavior and Skeleton, Burst Spirit, Golem, Phoenix, and boss implementations |
| `Camera` | Following the knight while keeping the view within level boundaries |
| `Projectile` / `Spike` | Ranged attacks and dangerous hazards |
| `GameAudio` / `GameplayExtras` | Music, sword effects, portals, flames, and ambient audio |

## Saving Progress

- Press **F5** while playing to save, and **F9** to load.
- **Continue** appears on the title screen when a recognized save file is present.
- The game writes `orthodoxy_save.txt` in its **current working directory**.
- Saved values include the region, player position, health, RPG attributes, scrolls and collection flags, kill statistics, best combo, and play time.
- Region transitions also trigger an automatic save.

The save file now uses the readable `ORTHODOXY_SAVE_V2` format with field names, region names, and `Yes`/`No` scroll flags. Older `ORTHODOXY_SAVE_V1` files can still be loaded; the next save writes V2. Saves from the Ashen Cemetery region (region 4 in V1) are supported.

For example:

```text
ORTHODOXY_SAVE_V2

Region: AshenBarrow
PlayerX: 120
PlayerY: 500
Health: 5

Vitality: 1
Strength: 1
Endurance: 1
Agility: 1

ScrollsAvailable: 1
ScrollsCollected: 1
TotalKills: 5
StageKills: 2
StageRelics: 1
BestCombo: 3
StageSeconds: 60
TotalSeconds: 120

Scroll1Collected: Yes
Scroll2Collected: No
...
```

The actual save contains a line for all eight scrolls. A saved value is associated with the label immediately before it. Individual defeated-enemy states are not saved, and you should keep the field names intact if editing the text file manually.

## Development Status

Orthodoxy is a work in progress. Gameplay balance, assets, boss encounters, and level polish may continue to change. This README documents features present in the repository's current source code.
