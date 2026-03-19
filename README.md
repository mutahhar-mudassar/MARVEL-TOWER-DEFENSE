# Marvel Tower Defense 🦸

A fully-featured tower defense game built in C++ using 
SFML graphics library and ImGui UI framework.

## 🎮 Gameplay

Defend your tower against waves of Marvel villains by 
strategically placing Marvel hero structures. 
Survive all waves to win!

## 🦸 Heroes (6 unique towers)
- Black Widow — Fast bullet attacks
- Captain America — Shield projectiles  
- Hawkeye — Long range arrows
- Iron Man — Explosive missiles
- Dr. Strange — Time beam attacks
- Thor — Lightning strikes

## 👹 Boss Enemies
- Wave 5: Ultron
- Wave 10: Thanos  
- Wave 15: Doctor Doom (Epic mode only)

## ⚙️ Technical Features

### Custom Data Structures (built from scratch)
- Doubly Linked List — enemy/structure/projectile management
- Queue — enemy spawn sequencing
- Circular Buffer — combat log system

### Systems
- Singleton TextureManager — centralized asset loading
- Full game state machine (7 states)
- Pathfinding & homing projectile system
- Wave configuration system
- Sound engine with background music
- ImGui-based game UI

### OOP Architecture
- Inheritance: Enemy, Structure, Projectile all extend sf::Drawable
- Virtual functions for polymorphic rendering
- Encapsulated game systems

## 🕹️ Game Modes
- Easy: 10 waves, 300 coins, 200 HP tower
- Epic: 15 waves, 250 coins, 150 HP tower

## 🛠️ Built With
- C++17
- SFML 2.5+ (Graphics, Audio, Window, System)
- ImGui + imgui-SFML
- CMake build system

## 🎓 Context
2nd Year Project — BS Computer Science  
COMSATS University Islamabad, Lahore Campus  
Developer: Muhammad Mutahhar, Shanzae Mudassar, Mehak Fatima
```

---

## 💼 LinkedIn Project Description — Copy This:
```
Built a fully-featured Marvel Tower Defense game in C++ 
as a 2nd year university project.

Technical highlights:
- 3,000+ lines of C++ across 8 modular files
- Custom data structures built from scratch — 
  Doubly Linked List, Queue, Circular Buffer
- Full OOP architecture with inheritance and 
  virtual functions (Enemy, Structure, Projectile)
- Singleton TextureManager for asset management
- 7-state game state machine
- Homing projectile system with collision detection
- 6 unique Marvel heroes + 3 boss enemies
- Full sound engine + ImGui UI

Built with C++17, SFML, ImGui, and CMake.

GitHub: github.com/mutahhar-mudassar/MarvelTowerDefense
