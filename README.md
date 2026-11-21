# Orthodoxy

**Orthodoxy** is a 2D action-platformer developed in C++ using SFML. The player controls a knight exploring a monastery-themed level, navigating platforms, fighting enemies, and avoiding hazards.

## Features

- Knight movement, jumping, double jump, crouching, and dashing
- Melee strikes and combat against enemies
- Enemy types including Golem and Phoenix
- Projectiles and spike hazards
- Health system with a visual HUD
- Main menu and pause screen
- Camera system and platform collision
- Sprite animations and a monastery background

## Project Structure

```
Orthodoxy/
├── Assets/
│   └── Sprites/          # Player, enemy, health, and background images
├── Classes/               # Game classes and gameplay systems
├── Source Files/           # C++ implementation files
├── main.cpp                # Program entry point
└── Orthodoxy.vcxproj       # Visual Studio project
```

## Main Components

- **Engine** – Manages the game window, main loop, game states, menus, updates, and rendering.
- **Player** – Handles knight movement, animation, health, and combat abilities.
- **Level** – Loads and draws the background and platforms.
- **Enemy / Golem / Phoenix** – Enemy behavior and different enemy types.
- **Projectile** – Handles projectile movement and interactions.
- **Spike** – Represents damaging level hazards.
- **Camera** – Controls the view as the player moves.
- **Collision** – Contains collision-related logic.

## Requirements

- C++ compiler compatible with the project
- [SFML 3](https://www.sfml-dev.org/) (Graphics, Window, and System modules)
- Visual Studio / MSVC project environment (project file included)

## Controls

J to attack
WASD to move
L Shift to dash

## Project Status

This is a work in progress; gameplay systems and content may change as development continues.
