#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>

#include "Constants.hpp"
#include "DataStructures.hpp"
#include "TextureManager.hpp"
#include "GameObjects.hpp"
#include "GameMap.hpp"


//Actual Game Verion 3

/**
 * WaveConfig - Configuration structure for enemy waves
 * 
 * This structure defines the composition of a single wave in the game.
 * Each wave specifies how many ground enemies, flying enemies, and whether
 * a boss appears in that wave.
 * 
 * Members:
 *   - groundEnemies: Number of ground-based enemies to spawn in this wave
 *   - flyingEnemies: Number of flying enemies to spawn in this wave
 *   - hasBoss: Boolean flag indicating if a boss enemy appears in this wave
 * 
 * Usage:
 *   Used in Game class to define wave configurations for Easy and Epic game modes.
 *   Each game mode has an array of WaveConfig structures (10 for Easy, 15 for Epic).
 */
struct WaveConfig {
    int groundEnemies;
    int flyingEnemies;
    bool hasBoss;
};

/**
 * Game - Main game class that manages the entire game loop and state
 * 
 * This is the core class that handles all game logic, rendering, input processing,
 * and state management. It coordinates between all game systems including:
 * - Window management and rendering
 * - Game state machine (splash, menu, gameplay, victory, defeat)
 * - Enemy spawning and management
 * - Structure (tower) placement and upgrades
 * - Projectile physics and collision detection
 * - Sound system (background music and sound effects)
 * - UI rendering using ImGui
 * - Wave progression and difficulty scaling
 * 
 * Key Responsibilities:
 *   - Initializes SFML window, ImGui, and all game systems
 *   - Manages game state transitions (splash -> menu -> gameplay -> victory/defeat)
 *   - Handles user input (mouse clicks, keyboard, window events)
 *   - Updates game entities (enemies, structures, projectiles) each frame
 *   - Renders all game elements and UI
 *   - Manages resources (textures, sounds, fonts)
 *   - Tracks game statistics (kills, coins, tower health)
 * 
 * Game States:
 *   - STATE_SPLASH: Initial splash screen with hero portraits
 *   - STATE_HOWTOPLAY: Instructions screen
 *   - STATE_MENU: Game mode selection (Easy/Epic)
 *   - STATE_WAVE_PREP: Between waves, player can place structures
 *   - STATE_WAVE_ACTIVE: Active gameplay, enemies spawning and moving
 *   - STATE_PAUSED: Game paused, shows pause menu
 *   - STATE_VICTORY: Player won, all waves completed
 *   - STATE_DEFEAT: Player lost, tower destroyed
 * 
 * Game Modes:
 *   - MODE_EASY: 10 waves, 300 starting coins, 200 tower HP
 *   - MODE_EPIC: 15 waves, 250 starting coins, 150 tower HP (harder)
 */
class Game {
private:
    sf::RenderWindow window;
    sf::View view;
    sf::Clock deltaClock; //frame rate per miliseconds
    
    // Sound system
    sf::Music backgroundMusic;
    sf::SoundBuffer hitBuffer, bossDefeatBuffer, waveStartBuffer, victoryBuffer, defeatBuffer;//in ram
    sf::Sound hitSound, bossDefeatSound, waveStartSound, victorySound, defeatSound;
    bool soundEnabled;  //playsound();
    
    // Font
    sf::Font marvelFont;
    bool fontLoaded;
    
    // Animation timers
    float menuAnimTimer;//sine pulse
    float splashAnimTimer;
    
    // Splash screen sprites
    sf::Sprite splashBgSprite;
    sf::Sprite avengersLogoSprite;
    sf::Sprite heroSprites[6];
    
    GameState state; //Enum ins startmenu start game gameObject
 //   STATE_SPLASH, STATE_MENU, STATE_WAVE_ACTIVE,
    GameMode mode; //easy mode // epic mode
    GameMap gameMap; //game map
    
    LinkedList<Enemy*> enemies; //alive eneies
    LinkedList<Structure*> structures; //tower build
    LinkedList<Projectile*> projectiles;//list of bullets in air
    Queue<Enemy*> spawnQueue; //list of enemies in quques of current wave
    CombatLog combatLog;
    //The circular buffer for text messages.
    //current states
    int towerHP, maxTowerHP;//percentage calculation
    int coins;
    int totalKills, groundKills, flyingKills, bossKills;
    
    int currentWave, totalWaves;
    float spawnTimer, spawnInterval;//accumulate itme and 0.8 enemy
    //spawnTimer >= spawnInterval, the game spawns one enemy from the spawnQueue and resets spawnTimer to 0. This ensures enemies march out in a line rather than all appearing instantly on top of each other.
    int nextEnemyId, nextStructureId;
    
    WaveConfig easyWaves[10];
    WaveConfig epicWaves[15];
    
    StructureType selectedBuildType; //selected structure
    Structure* selectedStructure;//specific tower point
    bool showGrid;//Toggle G
    
public:
Game() : window(sf::VideoMode::getDesktopMode(), "Marvel: Avengers Tower Defense", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize),
         state(STATE_SPLASH), mode(MODE_EASY),
         towerHP(200), maxTowerHP(200), coins(300),
         totalKills(0), groundKills(0), flyingKills(0), bossKills(0),
         currentWave(0), totalWaves(10),
         spawnTimer(0), spawnInterval(0.8f),
         nextEnemyId(1), nextStructureId(1),
         selectedBuildType(STRUCT_NONE), selectedStructure(nullptr),
         showGrid(true), soundEnabled(true), fontLoaded(false), 
         menuAnimTimer(0.0f), splashAnimTimer(0.0f) {
    
    srand((unsigned)time(nullptr));
    window.setFramerateLimit(60);
    
    // Initialize view to the Logical Window Size (1280x720)
    view.setSize(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT);
    view.setCenter(Constants::WINDOW_WIDTH / 2.0f, Constants::WINDOW_HEIGHT / 2.0f);
    window.setView(view);
    
    if (!ImGui::SFML::Init(window)) {
        throw std::runtime_error("ImGui-SFML init failed!");
    }
    
    // Load Marvel font (try multiple paths)
    std::string fontPaths[] = {
        Constants::MARVEL_FONT_PATH,
        "../" + std::string(Constants::MARVEL_FONT_PATH),
        "../../" + std::string(Constants::MARVEL_FONT_PATH)
    };
    fontLoaded = false;
    for (const std::string& path : fontPaths) {
        if (marvelFont.loadFromFile(path)) {
            fontLoaded = true;
            std::cout << "Loaded font: " << path << std::endl;
            break;
        }
    }
    if (!fontLoaded) {
        std::cout << "Marvel font not found (tried: " << fontPaths[0] << ", " << fontPaths[1] << ", " << fontPaths[2] << "), using default font" << std::endl;
    }
    
    // Load sounds
    loadSounds();
    
    initWaveConfigs();
    TextureManager::getInstance().loadAllAssets();
    gameMap.initialize();
    applyStyle(); //ui color
    initSplashScreen();
    
    // Start background music (try multiple paths)
    std::string bgMusicPaths[] = {
        Constants::SOUND_BACKGROUND,
        "../" + std::string(Constants::SOUND_BACKGROUND),
        "../../" + std::string(Constants::SOUND_BACKGROUND)
    };
    bool musicLoaded = false;
    for (const std::string& path : bgMusicPaths) {
        if (backgroundMusic.openFromFile(path)) {
            backgroundMusic.setLoop(true);
            backgroundMusic.setVolume(60.0f);
            backgroundMusic.play();
            std::cout << "✓ Background music loaded: " << path << std::endl;
            musicLoaded = true;
            break;
        }
    }
    if (!musicLoaded) {
        std::cout << "✗ Failed to load background music. Tried:" << std::endl;

    }
    
    std::cout << "Game ready!" << std::endl;
}
    //Decontrucutor
    ~Game() {
        ListNode<Enemy*>* eNode = enemies.getHead();
        while (eNode) {
            delete eNode->data;
            eNode = eNode->next;
        }
        
        ListNode<Structure*>* sNode = structures. getHead();
        while (sNode) {
            delete sNode->data;
            sNode = sNode->next;
        }
        
        ListNode<Projectile*>* pNode = projectiles.getHead();
        while (pNode) {
            delete pNode->data;
            pNode = pNode->next;
        }
        
        Enemy* e;
        while (spawnQueue.dequeue(e)) {
            delete e;
        }
        
        ImGui::SFML::Shutdown();
    }
    
    void initWaveConfigs() {
        easyWaves[0] = {10, 5, false};
        easyWaves[1] = {15, 8, false};
        easyWaves[2] = {20, 10, false};
        easyWaves[3] = {25, 12, false};
        easyWaves[4] = {20, 10, true};
        easyWaves[5] = {30, 15, false};
        easyWaves[6] = {35, 18, false};
        easyWaves[7] = {40, 20, false};
        easyWaves[8] = {45, 25, false};
        easyWaves[9] = {35, 20, true};
        
        epicWaves[0] = {15, 8, false};
        epicWaves[1] = {20, 12, false};
        epicWaves[2] = {28, 15, false};
        epicWaves[3] = {35, 20, false};
        epicWaves[4] = {30, 15, true};
        epicWaves[5] = {40, 25, false};
        epicWaves[6] = {50, 30, false};
        epicWaves[7] = {60, 35, false};
        epicWaves[8] = {70, 40, false};
        epicWaves[9] = {50, 30, true};
        epicWaves[10] = {80, 50, false};
        epicWaves[11] = {90, 60, false};
        epicWaves[12] = {100, 70, false};
        epicWaves[13] = {110, 80, false};
        epicWaves[14] = {80, 50, true};
    }
    
    void loadSounds() {
        // Load sound effects (try multiple paths, with debug output)
        std::string hitPaths[] = {
            Constants::SOUND_HIT,
            "../" + std::string(Constants::SOUND_HIT),
            "../../" + std::string(Constants::SOUND_HIT)
        };
        bool hitLoaded = false;
        for (const std::string& path : hitPaths) {
            if (hitBuffer.loadFromFile(path)) {
                hitSound.setBuffer(hitBuffer);
                hitSound.setVolume(50.0f);
                std::cout << "✓ Hit sound loaded: " << path << " (samples: " << hitBuffer.getSampleCount() << ", duration: " << hitBuffer.getDuration().asSeconds() << "s)" << std::endl;
                hitLoaded = true;
                break;
            } else {
                std::cout << "✗ Failed to load hit sound from: " << path << " (format may not be supported)" << std::endl;
            }
        }
        if (!hitLoaded) {
            std::cout << "✗ Failed to load hit sound from all paths. Tried:" << std::endl;
            for (const std::string& path : hitPaths) {
                std::cout << "  - " << path << std::endl;
            }
            std::cout << "  Note: OGG files may need to be re-encoded. Try converting to WAV format." << std::endl;
        }
        if (bossDefeatBuffer.loadFromFile(Constants::SOUND_BOSS_DEFEAT)) {
            bossDefeatSound.setBuffer(bossDefeatBuffer);
            bossDefeatSound.setVolume(60.0f);
        }
        if (waveStartBuffer.loadFromFile(Constants::SOUND_WAVE_START)) {
            waveStartSound.setBuffer(waveStartBuffer);
            waveStartSound.setVolume(70.0f);
        }
        if (victoryBuffer.loadFromFile(Constants::SOUND_VICTORY)) {
            victorySound.setBuffer(victoryBuffer);
            victorySound.setVolume(80.0f);
        }
        if (defeatBuffer.loadFromFile(Constants::SOUND_DEFEAT)) {
            defeatSound.setBuffer(defeatBuffer);
            defeatSound.setVolume(80.0f);
        }
    }
    
    void playHitSound() {
        if (!soundEnabled) {
            return;  // Sound is disabled
        }
        
        // Check if buffer is loaded or file is empty
        if (hitBuffer.getSampleCount() == 0) {
            // Buffer not loaded, try to reload
            std::string hitPaths[] = {
                Constants::SOUND_HIT,
                "../" + std::string(Constants::SOUND_HIT),
                "../../" + std::string(Constants::SOUND_HIT)
            };
            for (const std::string& path : hitPaths) {
                if (hitBuffer.loadFromFile(path)) {
                    hitSound.setBuffer(hitBuffer);
                    hitSound.setVolume(40.0f);
                    break;
                }
            }
        }
        
        // Play sound if buffer is loaded
        if (hitBuffer.getSampleCount() > 0) {
            // Stop the sound if it's already playing to allow immediate replay
            if (hitSound.getStatus() == sf::Sound::Playing) {
                hitSound.stop();
            }
            hitSound.play();
        }
    }
    
    void playBossDefeatSound() {
        if (soundEnabled && bossDefeatBuffer.getSampleCount() > 0) {
            bossDefeatSound.play();
        }
    }
    
    void playWaveStartSound() {
        if (soundEnabled && waveStartBuffer.getSampleCount() > 0) {
            waveStartSound.play();
        }
    }
    
    void playVictorySound() {
        if (soundEnabled && victoryBuffer.getSampleCount() > 0) {
            victorySound.play();
        }
    }
    
    void playDefeatSound() {
        if (soundEnabled && defeatBuffer.getSampleCount() > 0) {
            defeatSound.play();
        }
    }
    //call above
    void applyStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
        
        colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 0.95f);
        colors[ImGuiCol_Header] = ImVec4(0.7f, 0.15f, 0.15f, 0.8f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.8f, 0.2f, 0.2f, 0.9f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.9f, 0.25f, 0.25f, 1.0f);
        colors[ImGuiCol_Button] = ImVec4(0.6f, 0.1f, 0.1f, 0.8f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.75f, 0.15f, 0.15f, 0.9f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.22f, 0.9f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.5f, 0.1f, 0.1f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.7f, 0.15f, 0.15f, 1.0f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
        
        style.WindowRounding = 5.0f;//roynding the cornwes
        style.FrameRounding = 3.0f;
        style.WindowPadding = ImVec2(10, 10);
    }
    
    void initSplashScreen() {
        // Initialize splash background
        splashBgSprite.setTexture(TextureManager::getInstance().getTexture("splash_background"));
        
        // Initialize Avengers logo at top center
        avengersLogoSprite.setTexture(TextureManager::getInstance().getTexture("splash_avengers"));
        sf::FloatRect logoBounds = avengersLogoSprite.getLocalBounds();
        avengersLogoSprite.setOrigin(logoBounds.width / 2, logoBounds.height / 2);
        avengersLogoSprite.setPosition(Constants::WINDOW_WIDTH / 2.0f, 80);
        
        // Initialize hero sprites in a row
        const char* heroTextures[] = {
            "splash_captain", "splash_ironman", "splash_thor",
            "splash_blackwidow", "splash_hawkeye", "splash_drstrange"
        };
        
        float startX = 90;
        float heroY = 320;
        float spacing = 200;
        //loop for positioning
        for (int i = 0; i < 6; i++) {
            heroSprites[i].setTexture(TextureManager::getInstance().getTexture(heroTextures[i]));
            sf::FloatRect bounds = heroSprites[i].getLocalBounds();
            heroSprites[i].setOrigin(bounds.width / 2, bounds.height / 2);
            heroSprites[i].setPosition(startX + i * spacing, heroY);
        }
    }
    
    //game is running
    void run() {
        while (window.isOpen()) {
            processEvents();
            sf::Time dt = deltaClock.restart();
            float deltaTime = dt.asSeconds();
            menuAnimTimer += deltaTime;
            splashAnimTimer += deltaTime;
            
            ImGui::SFML::Update(window, dt);
            update(deltaTime);
            render();
        }
    }

private:
    // Helper function to get UI scale factor based on current window size
    float getUIScale() const {
        sf::Vector2i topLeft = window.mapCoordsToPixel(sf::Vector2f(0, 0), view);
        sf::Vector2i bottomRight = window.mapCoordsToPixel(
            sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT), view);
        float currentWidth = (float)(bottomRight.x - topLeft.x);
        return currentWidth / Constants::WINDOW_WIDTH;
    }

 void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) 
                window.close();

            // FIX: Handle Window Resizing to prevent stretching
            if (event.type == sf::Event::Resized) {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                
                // Calculate ratios to maintain aspect ratio (Letterboxing)
                float windowRatio = event.size.width / (float)event.size.height;
                float viewRatio = (float)Constants::WINDOW_WIDTH / (float)Constants::WINDOW_HEIGHT;
                float sizeX = 1;
                float sizeY = 1;
                float posX = 0;
                float posY = 0;

                if (windowRatio > viewRatio) {
                    // Window is wider than game -> bars on left/right
                    sizeX = viewRatio / windowRatio;
                    posX = (1 - sizeX) / 2.f;
                } else {
                    // Window is taller than game -> bars on top/bottom
                    sizeY = windowRatio / viewRatio;
                    posY = (1 - sizeY) / 2.f;
                }

                // Reset view to logical size but set viewport to fit window
                view.setSize(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT);
                view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
                window.setView(view);
            }

            if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard)
                continue;

            if (event.type == sf::Event::MouseButtonPressed) {
                // FIX: Convert screen pixels to world coordinates
                // This ensures clicks land on the correct spot after resizing
                sf::Vector2i pixelPos(event.mouseButton.x, event.mouseButton.y);
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, view);
                
                handleClick(worldPos.x, worldPos.y, event.mouseButton.button);
            }

            if (event.type == sf::Event::KeyPressed) {
                handleKey(event.key.code);
            }
        }
    }
    
    void handleClick(float x, float y, sf::Mouse::Button btn) {
        if (state != STATE_WAVE_PREP && state != STATE_WAVE_ACTIVE) return;
        if (x >= Constants::GAME_AREA_WIDTH) return;
        
        if (btn == sf::Mouse::Left) {
            if (selectedBuildType != STRUCT_NONE) {
                if (gameMap.canPlaceAt(x, y)) {
                    int cost = getStructureCost(selectedBuildType);
                    if (coins >= cost) {
                        placeStructure(selectedBuildType, x, y);
                        coins -= cost;
                        //combo log
                        char msg[64];
                        sprintf(msg, "Built %s", getStructureName(selectedBuildType));
                        combatLog.add(msg);
                    } else {
                        combatLog.add("Not enough coins!");
                    }
                }
                selectedBuildType = STRUCT_NONE;
                gameMap.setShowPreview(false);
                return;
            }
            
            ListNode<Structure*>* node = structures.getHead();
            while (node) {
                if (node->data->getBounds().contains(x, y)) {
                    if (selectedStructure) selectedStructure->setShowRange(false);
                    selectedStructure = node->data;
                    selectedStructure->setShowRange(true);
                    return;
                }
                node = node->next;
            }
            
            if (selectedStructure) {
                selectedStructure->setShowRange(false);
                selectedStructure = nullptr;
            }
        }
        
        if (btn == sf::Mouse::Right) {
            selectedBuildType = STRUCT_NONE;
            gameMap. setShowPreview(false);
            if (selectedStructure) {
                selectedStructure->setShowRange(false);
                selectedStructure = nullptr;
            }
        }
    }
    
    void handleKey(sf::Keyboard::Key key) {
        if (key == sf::Keyboard::Escape) {
            if (state == STATE_WAVE_PREP || state == STATE_WAVE_ACTIVE)
                state = STATE_PAUSED;
            else if (state == STATE_PAUSED)
                state = STATE_WAVE_PREP;
        }
        if (key == sf::Keyboard::G) {
            showGrid = !showGrid;
            gameMap.setShowGrid(showGrid);
        }
        if (key == sf::Keyboard::Space && state == STATE_WAVE_PREP) {
            startWave();
        }
    }
    
    void placeStructure(StructureType type, float x, float y) {
        float snappedX, snappedY;
        gameMap.snapToGrid(x, y, snappedX, snappedY);
        
        Structure* s = new Structure();
        s->initialize(nextStructureId++, type, snappedX, snappedY);
        structures. pushBack(s);
        gameMap.setOccupied(snappedX, snappedY, true);
    }
    
    void startGame(GameMode m) {
        mode = m;
        
        ListNode<Enemy*>* eNode = enemies.getHead();
        while (eNode) {
            ListNode<Enemy*>* next = eNode->next;
            delete eNode->data;
            enemies.remove(eNode);
            eNode = next;
        }
        
        ListNode<Structure*>* sNode = structures. getHead();
        while (sNode) {
            ListNode<Structure*>* next = sNode->next;
            delete sNode->data;
            structures.remove(sNode);
            sNode = next;
        }
        
        ListNode<Projectile*>* pNode = projectiles.getHead();
        while (pNode) {
            ListNode<Projectile*>* next = pNode->next;
            delete pNode->data;
            projectiles.remove(pNode);
            pNode = next;
        }
        
        Enemy* e;
        while (spawnQueue.dequeue(e)) {
            delete e;
        }
        
        combatLog.clear();
        gameMap.initialize();
        
        if (mode == MODE_EASY) {
            coins = Constants::EASY_START_COINS;
            towerHP = maxTowerHP = Constants::EASY_TOWER_HP;
            totalWaves = Constants::EASY_WAVES;
        } else {
            coins = Constants::EPIC_START_COINS;
            towerHP = maxTowerHP = Constants::EPIC_TOWER_HP;
            totalWaves = Constants::EPIC_WAVES;
        }
        
        currentWave = 0;
        totalKills = groundKills = flyingKills = bossKills = 0;
        nextEnemyId = nextStructureId = 1;
        selectedBuildType = STRUCT_NONE;
        selectedStructure = nullptr;
        
        state = STATE_WAVE_PREP;
        combatLog.add("Game started!");
    }
    
    void startWave() {
        currentWave++;
        state = STATE_WAVE_ACTIVE;
        spawnTimer = 0;
        playWaveStartSound();
        
        WaveConfig config;
        if (mode == MODE_EASY) {
            config = easyWaves[currentWave - 1];
        } else {
            config = epicWaves[currentWave - 1];
        }
        
        for (int i = 0; i < config.groundEnemies; i++) {
            Enemy* enemy = new Enemy();
            enemy->initialize(nextEnemyId++, ENEMY_GROUND, currentWave);
            int lane = rand() % 4;
            int pathLen;
            PathPoint* path = gameMap.getPath(lane, pathLen);
            enemy->setPath(path, pathLen);
            spawnQueue.enqueue(enemy);
        }
        
        for (int i = 0; i < config.flyingEnemies; i++) {
            Enemy* enemy = new Enemy();
            enemy->initialize(nextEnemyId++, ENEMY_FLYING, currentWave);
            int lane = rand() % 4;
            int pathLen;
            PathPoint* path = gameMap. getPath(lane, pathLen);
            enemy->setPath(path, pathLen);
            spawnQueue.enqueue(enemy);
        }
        
        if (config.hasBoss) {
            Enemy* boss = new Enemy();
            
            // Determine which boss to spawn based on wave number
            // Easy mode: Wave 5 = Ultron, Wave 10 = Thanos
            // Epic mode: Wave 5 = Ultron, Wave 10 = Thanos, Wave 15 = Doctor Doom
            BossType bossToSpawn = BOSS_ULTRON;
            if (currentWave == 5) {
                bossToSpawn = BOSS_ULTRON;
            } else if (currentWave == 10) {
                bossToSpawn = BOSS_THANOS;
            } else if (currentWave == 15 && mode == MODE_EPIC) {
                bossToSpawn = BOSS_DOOM;
            }
            
            boss->initializeWithBoss(nextEnemyId++, ENEMY_BOSS, currentWave, bossToSpawn);
            int pathLen;
            PathPoint* path = gameMap.getPath(1, pathLen);
            boss->setPath(path, pathLen);
            spawnQueue.enqueue(boss);
            
            // Log which boss is appearing
            char bossMsg[64];
            sprintf(bossMsg, "%s has arrived!", getBossName(bossToSpawn));
            combatLog.add(bossMsg);
        }
        
        char msg[64];
        sprintf(msg, "Wave %d started!", currentWave);
        combatLog.add(msg);
    }
    //buiding tower
    void update(float dt) {
        if (selectedBuildType != STRUCT_NONE) {
            sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
            // FIX: Convert pixel coords to world coords for correct preview position after resize
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePixel, view);
            gameMap.updatePreview(worldPos.x, worldPos.y);
            gameMap.setShowPreview(true);
        }
        
        if (state == STATE_WAVE_ACTIVE) {
            spawnTimer += dt;
            if (spawnTimer >= spawnInterval && !spawnQueue. isEmpty()) {
                Enemy* enemy;
                if (spawnQueue. dequeue(enemy)) {
                    enemies.pushBack(enemy);
                }
                spawnTimer = 0;
            }
            
            ListNode<Enemy*>* eNode = enemies.getHead();
            while (eNode) {
                Enemy* e = eNode->data;
                e->update(dt);//movement
                
                if (e->hasReachedEnd() && e->isAlive()) {
                    towerHP -= e->getTowerDamage();
                    char msg[64];
                    sprintf(msg, "Tower hit!  -%d HP", e->getTowerDamage());
                    combatLog.add(msg);
                    e->takeDamage(9999);
                }
                eNode = eNode->next;
            }
            
            ListNode<Structure*>* sNode = structures.getHead();
            while (sNode) {
                Structure* s = sNode->data;
                s->updateCooldown(dt);
                
                if (s->canFire()) {
                    Enemy* target = findTarget(s);
                    if (target) {
                        processAttack(s, target);
                        s->fire();
                    }
                }
                sNode = sNode->next;
            }
            
            // Update projectiles
            ListNode<Projectile*>* pNode = projectiles.getHead();
            while (pNode) {
                pNode->data->update(dt);
                pNode = pNode->next;
            }
            
            // Check projectile collisions
            checkProjectileCollisions();
            
            cleanupDeadEnemies();
            cleanupDeadProjectiles();
            
            if (enemies.isEmpty() && spawnQueue.isEmpty()) {
                int bonus = Constants::WAVE_COMPLETE_BONUS;
                if (currentWave % 5 == 0) bonus *= 2;
                coins += bonus;
                
                char msg[64];
                sprintf(msg, "Wave complete! +%d coins", bonus);
                combatLog.add(msg);
                
                if (currentWave >= totalWaves) {
                    state = STATE_VICTORY;
                    playVictorySound();
                } else {
                    state = STATE_WAVE_PREP;
                }
            }
            
            if (towerHP <= 0) {
                state = STATE_DEFEAT;
                playDefeatSound();
            }
        }
    }
    
    Enemy* findTarget(Structure* s) {
        Enemy* closest = nullptr;
        float closestDist = 9999.0f;
        //search enemy loop
        ListNode<Enemy*>* node = enemies.getHead();
        while (node) {
            Enemy* e = node->data;
            if (e->isAlive() && s->canTarget(e->getType()) && s->isInRange(e->getPosition())) {
                sf::Vector2f sPos = s->getPosition();
                sf::Vector2f ePos = e->getPosition();
                float dist = calcDistance(sPos. x, sPos. y, ePos. x, ePos. y);
                if (dist < closestDist) {//find closet enemy
                    closestDist = dist;
                    closest = e;
                }
            }
            node = node->next;
        }
        return closest;
    }
    // create  the bullet projectiel
void processAttack(Structure* s, Enemy* target) {
    Projectile* p = new Projectile();
    p->initialize(s->getProjectileType(), s->getPosition(), target->getPosition(), s->getDamage(), getProjectileSpeed(s->getProjectileType()), target);  // Pass target for homing
    projectiles.pushBack(p);
    s->setAttackVisual(target->getPosition());
}
    
    float getProjectileSpeed(Constants::ProjectileType type) {
        switch(type) {
            case Constants::PROJ_BULLET: return Constants::BULLET_SPEED;
            case Constants::PROJ_SHIELD: return Constants::SHIELD_SPEED;
            case Constants::PROJ_ARROW: return Constants::ARROW_SPEED;
            case Constants::PROJ_MISSILE: return Constants::MISSILE_SPEED;
            case Constants::PROJ_BEAM: return Constants::BEAM_SPEED;
            case Constants::PROJ_LIGHTNING: return Constants::LIGHTNING_SPEED;
        }
        return Constants::BULLET_SPEED;
    }
    
    void checkProjectileCollisions() {
        ListNode<Projectile*>* pNode = projectiles.getHead();
        while (pNode) {
            ListNode<Projectile*>* nextP = pNode->next;
            Projectile* proj = pNode->data;
            if (!proj->isAlive()) {
                pNode = nextP;
                continue;
            }
            
            ListNode<Enemy*>* eNode = enemies.getHead();
            bool hit = false;
            while (eNode) {
                Enemy* e = eNode->data;
                if (e->isAlive()) {
                    sf::Vector2f dist = proj->getPosition() - e->getPosition();
                    if (sqrt(dist.x*dist.x + dist.y*dist.y) < 20.0f) { // Hit radius
                        e->takeDamage(proj->getDamage());
                        if (!e->isAlive()) {
                            coins += e->getCoinReward();
                            // Find the structure that shot this projectile (not implemented, but could add)
                            totalKills++;
                            switch(e->getType()) {
                                case ENEMY_GROUND: groundKills++; break;
                                case ENEMY_FLYING: flyingKills++; break;
                                case ENEMY_BOSS: 
                                    bossKills++; 
                                    // Log which boss was defeated
                                    char bossDefeatMsg[64];
                                    sprintf(bossDefeatMsg, "%s DEFEATED! +$%d", 
                                            getBossName(e->getBossType()), e->getCoinReward());
                                    combatLog.add(bossDefeatMsg);
                                    playBossDefeatSound();
                                    break;
                            }
                        }
                        playHitSound();
                        hit = true;
                        break;
                    }
                }
                eNode = eNode->next;
            }
            
            if (hit) {
                proj->setAlive(false);
            }
            pNode = nextP;
        }
    }
    
    void cleanupDeadEnemies() {
        ListNode<Enemy*>* node = enemies.getHead();
        while (node) {
            ListNode<Enemy*>* next = node->next;
            if (! node->data->isAlive()) {
                delete node->data;
                enemies.remove(node);
            }
            node = next;
        }
    }
    
    void cleanupDeadProjectiles() {
        ListNode<Projectile*>* node = projectiles.getHead();
        while (node) {
            ListNode<Projectile*>* next = node->next;
            if (! node->data->isAlive()) {
                delete node->data;
                projectiles.remove(node);
            }
            node = next;
        }
    }
    
    void render() {
        window.clear(sf::Color(20, 25, 30));
        
        // Only draw game elements during gameplay states
        if (state == STATE_WAVE_PREP || state == STATE_WAVE_ACTIVE || 
            state == STATE_PAUSED || state == STATE_VICTORY || state == STATE_DEFEAT) {
            window.draw(gameMap);
            
            ListNode<Structure*>* sNode = structures.getHead();
            while (sNode) {
                window.draw(*(sNode->data));
                sNode = sNode->next;
            }
            
            // Draw projectiles
            ListNode<Projectile*>* pNode = projectiles.getHead();
            while (pNode) {
                window.draw(*(pNode->data));
                pNode = pNode->next;
            }
            
            ListNode<Enemy*>* eNode = enemies.getHead();
            while (eNode) {
                window.draw(*(eNode->data));
                eNode = eNode->next;
            }
        }
        
        switch (state) {
            case STATE_SPLASH: renderSplashScreen(); break;
            case STATE_HOWTOPLAY: renderHowToPlay(); break;
            case STATE_MENU: renderMenu(); break;
            case STATE_WAVE_PREP:
            case STATE_WAVE_ACTIVE: renderGameUI(); break;
            case STATE_PAUSED: renderGameUI(); renderPauseMenu(); break;
            case STATE_VICTORY: renderVictory(); break;
            case STATE_DEFEAT: renderDefeat(); break;
        }
        
        ImGui::SFML::Render(window);
        window.display();
    }
    
    void renderSplashScreen() {
        // Draw animated background
        window.draw(splashBgSprite);
        
        // Draw animated diagonal lines
        float lineOffset = fmod(splashAnimTimer * 30.0f, 80.0f);
        for (int i = -2; i < 20; i++) {
            sf::RectangleShape line(sf::Vector2f(Constants::WINDOW_WIDTH * 2, 2));
            line.setPosition(-200, i * 80 + lineOffset);
            line.setFillColor(sf::Color(120, 30, 30, 40));
            line.setRotation(20);
            window.draw(line);
        }
        
        // Draw Avengers logo with pulsing effect (smaller size)
        float baseLogoScale = 0.7f;  // Reduced logo size to 70%
        float logoPulse = baseLogoScale + 0.03f * sin(splashAnimTimer * 2.0f);
        avengersLogoSprite.setScale(logoPulse, logoPulse);
        window.draw(avengersLogoSprite);
        
        // Draw hero sprites with floating animation (reduced size)
        for (int i = 0; i < 6; i++) {
            float floatOffset = 10.0f * sin(splashAnimTimer * 1.5f + i * 0.5f);
            sf::Vector2f originalPos = heroSprites[i].getPosition();
            heroSprites[i].setPosition(originalPos.x, 320 + floatOffset);
            
            // Add glow effect (pulsing scale) - increased by 10% from 0.75x to 0.825x
            float baseScale = 0.975f;  // Increased by 10% (0.75 * 1.1 = 0.825)
            float heroScale = baseScale + 0.02f * sin(splashAnimTimer * 2.0f + i * 0.3f);
            heroSprites[i].setScale(heroScale, heroScale);
            
            window.draw(heroSprites[i]);
            heroSprites[i].setPosition(originalPos);
        }
        
        // Draw ImGui buttons
        sf::Vector2i center = window.mapCoordsToPixel(
            sf::Vector2f(Constants::WINDOW_WIDTH / 2.0f, Constants::WINDOW_HEIGHT / 2.0f), view);
        float scale = getUIScale();
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        
        float buttonWidth = 250 * scale;
        float buttonHeight = 50 * scale;
        float buttonY = 480 * scale;
        float buttonSpacing = 70 * scale;
        
        ImGui::SetNextWindowPos(ImVec2(center.x - buttonWidth * 1.8f, buttonY));
        ImGui::SetNextWindowSize(ImVec2(buttonWidth * 3.6f, buttonHeight * 3));
        
        ImGui::Begin("##SplashButtons", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
        
        ImGui::SetWindowFontScale(scale * 1.2f);
        
        // Title text
        ImGui::SetWindowFontScale(scale * 1.3f);
        ImGui::SetCursorPosX((buttonWidth * 3.6f - 400 * scale * 1.3f) / 2);
        float titleGlow = 0.8f + 0.2f * sin(splashAnimTimer * 3.0f);
        ImGui::TextColored(ImVec4(titleGlow, 0.2f, 0.2f, 1.0f), "AVENGERS TOWER DEFENSE");
        ImGui::SetWindowFontScale(scale);
        
        ImGui::Dummy(ImVec2(0, 20 * scale));
        
        // START button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.15f, 0.15f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::SetCursorPosX(20 * scale);
        if (ImGui::Button("START", ImVec2(buttonWidth, buttonHeight))) {
            state = STATE_MENU;
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        
        // HOW TO PLAY button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
        if (ImGui::Button("HOW TO PLAY", ImVec2(buttonWidth, buttonHeight))) {
            state = STATE_HOWTOPLAY;
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        
        // SOUND button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.35f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.6f, 1.0f));
        const char* soundLabel = soundEnabled ? "SOUND: ON" : "SOUND: OFF";
        if (ImGui::Button(soundLabel, ImVec2(buttonWidth, buttonHeight))) {
            soundEnabled = !soundEnabled;
            if (soundEnabled) {
                backgroundMusic.play();
            } else {
                backgroundMusic.pause();
            }
        }
        ImGui::PopStyleColor(3);
        
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
    
    void renderHowToPlay() {
        // Draw background
        drawMenuBackground();
        
        sf::Vector2i center = window.mapCoordsToPixel(
            sf::Vector2f(Constants::WINDOW_WIDTH / 2.0f, Constants::WINDOW_HEIGHT / 2.0f), view);
        float scale = getUIScale();
        
        float windowWidth = 700 * scale;
        float windowHeight = 550 * scale;
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.12f, 0.98f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f * scale);
        
        ImGui::SetNextWindowPos(ImVec2(center.x - windowWidth/2, center.y - windowHeight/2));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
        
        ImGui::Begin("##HowToPlay", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        ImGui::SetWindowFontScale(scale);
        
        // Title
        ImGui::SetCursorPosX((windowWidth - 250 * scale) / 2);
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "HOW TO PLAY");
        ImGui::Dummy(ImVec2(0, 10 * scale));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 15 * scale));
        
        // Objective
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "OBJECTIVE:");
        ImGui::TextWrapped("Defend the Avengers Tower from waves of enemies! Place heroes strategically to stop the invasion before your tower falls.");
        
        ImGui::Dummy(ImVec2(0, 15 * scale));
        
        // Controls
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "CONTROLS:");
        ImGui::BulletText("LEFT CLICK - Select a hero to place, then click on the map");
        ImGui::BulletText("RIGHT CLICK - Cancel selection / Deselect");
        ImGui::BulletText("SPACE - Start the next wave");
        ImGui::BulletText("G - Toggle grid overlay");
        ImGui::BulletText("ESC - Pause game");
        
        ImGui::Dummy(ImVec2(0, 15 * scale));
        
        // Heroes
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "YOUR HEROES:");
        ImGui::Columns(2, nullptr, false);
        
        ImGui::TextColored(ImVec4(0.2f, 0.2f, 0.2f, 1.0f), "[*]");
        ImGui::SameLine(); ImGui::Text("Black Widow - Rapid bullets");
        
        ImGui::TextColored(ImVec4(0.0f, 0.3f, 0.7f, 1.0f), "[*]");
        ImGui::SameLine(); ImGui::Text("Captain America - Shield throw");
        
        ImGui::TextColored(ImVec4(0.6f, 0.0f, 0.8f, 1.0f), "[*]");
        ImGui::SameLine(); ImGui::Text("Hawkeye - Precise arrows");
        
        ImGui::NextColumn();
        
        ImGui::TextColored(ImVec4(0.9f, 0.1f, 0.1f, 1.0f), "[*]");
        ImGui::SameLine(); ImGui::Text("Iron Man - Homing missiles (Air)");
        
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[*]");
        ImGui::SameLine(); ImGui::Text("Dr Strange - Time beam (Air)");
        
        ImGui::TextColored(ImVec4(0.1f, 0.6f, 1.0f, 1.0f), "[*]");
        ImGui::SameLine(); ImGui::Text("Thor - Lightning strike (Air)");
        
        ImGui::Columns(1);
        
        ImGui::Dummy(ImVec2(0, 15 * scale));
        
        // Enemies
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "ENEMIES:");
        ImGui::BulletText("Ground Units - Can only be hit by all heroes");
        ImGui::BulletText("Flying Units - Require Iron Man, Dr Strange, or Thor");
        ImGui::BulletText("Bosses - Ultron, Thanos, Doctor Doom (very powerful!)");
        
        ImGui::Dummy(ImVec2(0, 15 * scale));
        
        // Tips
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "TIPS:");
        ImGui::BulletText("Upgrade heroes to increase damage and range");
        ImGui::BulletText("Mix ground and air-targeting heroes");
        ImGui::BulletText("Save coins for boss waves!");
        
        ImGui::Dummy(ImVec2(0, 20 * scale));
        
        // Back button
        ImGui::SetCursorPosX((windowWidth - 200 * scale) / 2);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.15f, 0.15f, 1.0f));
        if (ImGui::Button("BACK TO MENU", ImVec2(200 * scale, 40 * scale))) {
            state = STATE_SPLASH;
        }
        ImGui::PopStyleColor(2);
        
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }
    
    void renderMenu() {
        // Draw animated Marvel-themed background
        drawMenuBackground();
        
        sf::Vector2i center = window.mapCoordsToPixel(
            sf::Vector2f(Constants::WINDOW_WIDTH / 2.0f, Constants::WINDOW_HEIGHT / 2.0f), view);
        float scale = getUIScale();
        
        float windowWidth = 500 * scale;
        float windowHeight = 520 * scale;
        
        // Animated glow effect for the menu
        float glowIntensity = 0.7f + 0.3f * sin(menuAnimTimer * 2.0f);
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.08f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f * glowIntensity, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f * scale);
        
        ImGui::SetNextWindowPos(ImVec2(center.x - windowWidth/2, center.y - windowHeight/2));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
        
        ImGui::Begin("##MarvelMenu", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        
        ImGui::SetWindowFontScale(scale);
        
        // Title with pulsing effect
        ImGui::Dummy(ImVec2(0, 10 * scale));
        float titleGlow = 0.8f + 0.2f * sin(menuAnimTimer * 3.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(titleGlow, 0.15f, 0.15f, 1.0f));
        ImGui::SetCursorPosX((windowWidth - 380 * scale) / 2);
        ImGui::Text("M A R V E L");
        ImGui::PopStyleColor();
        
        // Center and make bold with new color (red/marvel theme)
        ImGui::SetWindowFontScale(scale * 1.3f);  // Make it 30% larger (bold effect)
        ImVec2 textSize = ImGui::CalcTextSize("AVENGERS TOWER DEFENSE");
        ImGui::SetCursorPosX((windowWidth - textSize.x) / 2);  // Perfect centering based on actual text size
        ImGui::TextColored(ImVec4(0.95f, 0.2f, 0.2f, 1.0f), "AVENGERS TOWER DEFENSE");  // Red/Marvel color
        ImGui::SetWindowFontScale(scale);  // Reset font scale back to normal
        
        ImGui::Dummy(ImVec2(0, 5 * scale));
        ImGui::SetCursorPosX((windowWidth - 200 * scale) / 2);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), "Defend Earth's Mightiest Tower");
        
        ImGui::Dummy(ImVec2(0, 20 * scale));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 15 * scale));
        
        // Game mode buttons with Marvel styling
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.4f, 0.15f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.8f, 0.1f, 1.0f));
        ImGui::SetCursorPosX((windowWidth - 380 * scale) / 2);
        if (ImGui::Button("Avengers:Infinity War\n10 Waves | $300 | 200 HP", ImVec2(380 * scale, 55 * scale))) {
            startGame(MODE_EASY);
        }
        ImGui::PopStyleColor(3);
        
        ImGui::Dummy(ImVec2(0, 8 * scale));
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
        ImGui::SetCursorPosX((windowWidth - 380 * scale) / 2);
        if (ImGui::Button("Avengers: Doomsday\n15 Waves | $250 | 150 HP", ImVec2(380 * scale, 55 * scale))) {
            startGame(MODE_EPIC);
        }
        ImGui::PopStyleColor(3);
        
        ImGui::Dummy(ImVec2(0, 15 * scale));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10 * scale));
        
        // Heroes section
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "ASSEMBLE YOUR HEROES:");
        ImGui::Dummy(ImVec2(0, 5 * scale));
        
        // Hero list with icons/colors
        ImGui::TextColored(ImVec4(0.2f, 0.2f, 0.2f, 1.0f), "[*]"); ImGui::SameLine();
        ImGui::Text("Black Widow ($%d) - Rapid Fire", Constants::BLACKWIDOW_COST);
        
        ImGui::TextColored(ImVec4(0.0f, 0.3f, 0.7f, 1.0f), "[*]"); ImGui::SameLine();
        ImGui::Text("Captain America ($%d) - Shield Throw", Constants::CAPTAIN_COST);
        
        ImGui::TextColored(ImVec4(0.6f, 0.0f, 0.8f, 1.0f), "[*]"); ImGui::SameLine();
        ImGui::Text("Hawkeye ($%d) - Precise Arrows", Constants::HAWKEYE_COST);
        
        ImGui::TextColored(ImVec4(0.9f, 0.1f, 0.1f, 1.0f), "[*]"); ImGui::SameLine();
        ImGui::Text("Iron Man ($%d) - Homing Missiles", Constants::IRONMAN_COST);
        
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[*]"); ImGui::SameLine();
        ImGui::Text("Dr Strange ($%d) - Time Beam", Constants::DRSTRANGE_COST);
        
        ImGui::TextColored(ImVec4(0.1f, 0.6f, 1.0f, 1.0f), "[*]"); ImGui::SameLine();
        ImGui::Text("Thor ($%d) - Lightning Strike", Constants::THOR_COST);


        
        ImGui::Dummy(ImVec2(0, 10 * scale));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 8 * scale));
        
        // Controls
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "CONTROLS: LMB Place | RMB Cancel | G Grid | SPACE Wave | ESC Pause");
        
        ImGui::Dummy(ImVec2(0, 10 * scale));
        
        // Sound toggle and Exit
        ImGui::SetCursorPosX((windowWidth - 300 * scale) / 2);
        if (ImGui::Button(soundEnabled ? "SOUND: ON" : "SOUND: OFF", ImVec2(140 * scale, 30 * scale))) {
            soundEnabled = !soundEnabled;
            if (soundEnabled) {
                backgroundMusic.play();
            } else {
                backgroundMusic.pause();
            }
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.1f, 0.1f, 0.9f));
        if (ImGui::Button("EXIT GAME", ImVec2(140 * scale, 30 * scale))) {
            window.close();
        }
        ImGui::PopStyleColor();
        
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }
    
    void drawMenuBackground() {
        // Draw a dark gradient background with animated elements
        sf::RectangleShape bg(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
        bg.setFillColor(sf::Color(15, 15, 25));
        window.draw(bg);
        
        // Draw animated Marvel-style lines/grid
        float lineOffset = fmod(menuAnimTimer * 20.0f, 50.0f);
        for (int i = -1; i < 30; i++) {
            sf::RectangleShape line(sf::Vector2f(Constants::WINDOW_WIDTH * 1.5f, 1));
            line.setPosition(-100, i * 50 + lineOffset);
            line.setFillColor(sf::Color(80, 20, 20, 30));
            line.setRotation(15);
            window.draw(line);
        }
        
        // Draw pulsing circles in corners
        float pulse = 0.5f + 0.5f * sin(menuAnimTimer * 1.5f);
        sf::CircleShape corner1(100 * pulse);
        corner1.setPosition(-50, -50);
        corner1.setFillColor(sf::Color(150, 30, 30, (sf::Uint8)(30 * pulse)));
        window.draw(corner1);
        
        sf::CircleShape corner2(80 * pulse);
        corner2.setPosition(Constants::WINDOW_WIDTH - 100, Constants::WINDOW_HEIGHT - 100);
        corner2.setFillColor(sf::Color(150, 30, 30, (sf::Uint8)(25 * pulse)));
        window.draw(corner2);
    }
    
    void renderGameUI() {
        renderTopBar();
        renderSidePanel();
    }
    
  void renderTopBar() {
        // Calculate screen positions dynamically
        sf::Vector2i pos = window.mapCoordsToPixel(sf::Vector2f(0, 0), view);
        sf::Vector2i size = window.mapCoordsToPixel(sf::Vector2f(Constants::GAME_AREA_WIDTH, 60), view);
        
        // Calculate dimensions
        float width = (float)(size.x - pos.x);
        float height = (float)(size.y - pos.y);
        
        ImGui::SetNextWindowPos(ImVec2((float)pos.x, (float)pos.y));
        ImGui::SetNextWindowSize(ImVec2(width, height));
        
        // Optional: Scale font slightly based on window size
        float scale = (width > 0 && Constants::GAME_AREA_WIDTH > 0) ? width / Constants::GAME_AREA_WIDTH : 1.0f;
        if (scale <= 0.0f) scale = 1.0f;  // Ensure scale is always positive to prevent assertion error
        
        ImGui::Begin("##TopBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
        
        ImGui::SetWindowFontScale(scale); // Apply scaling to text

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "WAVE: %d/%d", currentWave, totalWaves);
        
        if (currentWave > 0 && currentWave % 5 == 0) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "[BOSS]");
        }
        
        // Adjust spacing for scaling
        ImGui::SameLine(180 * scale);
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "COINS: $%d", coins);
        
        ImGui::SameLine(350 * scale);
        ImGui::Text("KILLS: %d", totalKills);
        
        ImGui::SameLine(500 * scale);
        ImGui::Text("TOWER:");
        ImGui::SameLine();
        
        float hpPercent = (float)towerHP / maxTowerHP;
        ImVec4 hpColor;
        if (hpPercent > 0.6f) hpColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
        else if (hpPercent > 0.3f) hpColor = ImVec4(0.9f, 0.9f, 0.2f, 1.0f);
        else hpColor = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, hpColor);
        ImGui::ProgressBar(hpPercent, ImVec2(180 * scale, 20 * scale), "");
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        ImGui::Text("%d/%d", towerHP, maxTowerHP);
        
        if (state == STATE_WAVE_PREP) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), 
                              ">> PREPARATION PHASE - Press SPACE to start wave <<");
        } else if (state == STATE_WAVE_ACTIVE) {
            int remaining = enemies.getSize() + spawnQueue.getSize();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), 
                              ">> WAVE IN PROGRESS - Enemies remaining: %d <<", remaining);
        }
        
        ImGui::End();
    }
    
 void renderSidePanel() {
        // Calculate the screen position for the start of the side panel (1000, 0)
        sf::Vector2i panelStart = window.mapCoordsToPixel(sf::Vector2f(Constants::GAME_AREA_WIDTH, 0), view);
        // Calculate the screen position for the bottom-right of the screen (1280, 720)
        sf::Vector2i panelEnd = window.mapCoordsToPixel(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT), view);
        
        float width = (float)(panelEnd.x - panelStart.x);
        float height = (float)(panelEnd.y - panelStart.y);
        
        ImGui::SetNextWindowPos(ImVec2((float)panelStart.x, (float)panelStart.y));
        ImGui::SetNextWindowSize(ImVec2(width, height));
        
        // Calculate scale factor
        float scale = width / Constants::UI_PANEL_WIDTH;

        ImGui::Begin("##SidePanel", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::SetWindowFontScale(scale); // Scale text

        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "ASSEMBLE HEROES");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5 * scale));
        
        // Hero structure buttons with descriptions
        renderStructureButton("Black Widow", Constants::BLACKWIDOW_COST, STRUCT_BLACKWIDOW, "Ground, Rapid Bullets", scale);
        renderStructureButton("Captain America", Constants::CAPTAIN_COST, STRUCT_CAPTAIN, "Ground, Shield Throw", scale);
        renderStructureButton("Hawkeye", Constants::HAWKEYE_COST, STRUCT_HAWKEYE, "Ground, Precise Arrows", scale);
        renderStructureButton("Iron Man", Constants::IRONMAN_COST, STRUCT_IRONMAN, "Air+Ground, Missiles", scale);
        renderStructureButton("Dr Strange", Constants::DRSTRANGE_COST, STRUCT_DRSTRANGE, "Air+Ground, Time Beam", scale);
        renderStructureButton("Thor", Constants::THOR_COST, STRUCT_THOR, "Air+Ground, Lightning", scale);

        
        ImGui::Dummy(ImVec2(0, 10 * scale));
        ImGui::Separator();
        
        if (selectedStructure) {
            ImGui::Dummy(ImVec2(0, 5 * scale));
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "SELECTED STRUCTURE:");
            
            ImGui::Text("%s (Lv. %d)", getStructureName(selectedStructure->getType()),
                       selectedStructure->getLevel());
            ImGui::Text("Damage: %d", selectedStructure->getDamage());
            ImGui::Text("Range: %.0f", selectedStructure->getRange());
            ImGui::Text("Kills: %d", selectedStructure->getTotalKills());
            ImGui::Text("Total Damage: %d", selectedStructure->getTotalDamageDealt());
            
            ImGui::Dummy(ImVec2(0, 5 * scale));
            
            int upgradeCost = selectedStructure->getUpgradeCost();
            if (upgradeCost > 0) {
                char upgradeLabel[64];
                sprintf(upgradeLabel, "UPGRADE ($%d)", upgradeCost);
                
                bool canAfford = coins >= upgradeCost;
                if (!canAfford) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                
                if (ImGui::Button(upgradeLabel, ImVec2(125 * scale, 35 * scale))) {
                    if (canAfford) {
                        coins -= upgradeCost;
                        selectedStructure->upgrade();
                        combatLog.add("Structure upgraded!");
                    }
                }
                
                if (!canAfford) ImGui::PopStyleVar();
                ImGui::SameLine();
            }
            
            char sellLabel[64];
            sprintf(sellLabel, "SELL ($%d)", selectedStructure->getSellValue());
            
            if (ImGui::Button(sellLabel, ImVec2(125 * scale, 35 * scale))) {
                coins += selectedStructure->getSellValue();
                
                sf::Vector2f pos = selectedStructure->getPosition();
                gameMap.setOccupied(pos.x, pos.y, false);
                
                ListNode<Structure*>* node = structures.getHead();
                while (node) {
                    if (node->data == selectedStructure) {
                        delete node->data;
                        structures.remove(node);
                        break;
                    }
                    node = node->next;
                }
                
                selectedStructure = nullptr;
                combatLog.add("Structure sold!");
            }
            
            ImGui::Separator();
        }
        
        ImGui::Dummy(ImVec2(0, 10 * scale));
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "COMBAT LOG");
        
        ImGui::BeginChild("##CombatLog", ImVec2(255 * scale, 140 * scale), true);
        ImGui::SetWindowFontScale(scale); // Ensure child window text is scaled
        for (int i = 0; i < combatLog.getCount(); i++) {
            ImGui::TextWrapped("%s", combatLog.getMessage(i));
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        
        ImGui::Dummy(ImVec2(0, 10 * scale));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5 * scale));
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "STATISTICS");
        
        ImGui::Text("Ground Kills: %d", groundKills);
        ImGui::Text("Flying Kills: %d", flyingKills);
        ImGui::Text("Boss Kills: %d", bossKills);
        ImGui::Text("Structures: %d", structures.getSize());
        
        ImGui::End();
    }


    
   // FIX: Added 'scale' parameter to resize buttons correctly
    void renderStructureButton(const char* name, int cost, StructureType type, const char* desc, float scale) {
        char label[128];
        sprintf(label, "%s ($%d)\n%s", name, cost, desc);
        
        bool canAfford = coins >= cost;
        bool isSelected = (selectedBuildType == type);
        
        if (!canAfford) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        }
        
        // Use the passed 'scale' instead of calling ImGui::GetWindowFontScale()
        float width = ImGui::GetContentRegionAvail().x;
        float height = 45.0f * scale; 

        if (ImGui::Button(label, ImVec2(width, height))) {
            if (canAfford) {
                selectedBuildType = type;
                if (selectedStructure) {
                    selectedStructure->setShowRange(false);
                    selectedStructure = nullptr;
                }
            }
        }
        
        if (isSelected) ImGui::PopStyleColor();
        if (!canAfford) ImGui::PopStyleVar();
    }
    
    void renderPauseMenu() {
        // FIX: Convert world center to screen pixel coords for proper centering
        sf::Vector2i center = window.mapCoordsToPixel(
            sf::Vector2f(Constants::WINDOW_WIDTH / 2.0f, Constants::WINDOW_HEIGHT / 2.0f), view);
        float scale = getUIScale();
        
        float windowWidth = 300 * scale;
        float windowHeight = 200 * scale;
        
        ImGui::SetNextWindowPos(ImVec2(center.x - windowWidth/2, center.y - windowHeight/2));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
        
        ImGui::Begin("PAUSED", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::SetWindowFontScale(scale);
        ImGui::Dummy(ImVec2(0, 20 * scale));
        
        ImGui::SetCursorPosX(50 * scale);
        if (ImGui::Button("RESUME GAME", ImVec2(200 * scale, 40 * scale))) {
            state = STATE_WAVE_PREP;
        }
        
        ImGui::Dummy(ImVec2(0, 10 * scale));
        ImGui::SetCursorPosX(50 * scale);
        if (ImGui::Button("MAIN MENU", ImVec2(200 * scale, 40 * scale))) {
            state = STATE_MENU;
        }
        
        ImGui::Dummy(ImVec2(0, 10 * scale));
        ImGui::SetCursorPosX(50 * scale);
        if (ImGui::Button("EXIT GAME", ImVec2(200 * scale, 40 * scale))) {
            window.close();
        }
        
        ImGui::End();
    }
    
    void renderVictory() {
        // FIX: Convert world center to screen pixel coords for proper centering
        sf::Vector2i center = window.mapCoordsToPixel(
            sf::Vector2f(Constants::WINDOW_WIDTH / 2.0f, Constants::WINDOW_HEIGHT / 2.0f), view);
        float scale = getUIScale();
        
        float windowWidth = 440 * scale;
        float windowHeight = 360 * scale;
        
        ImGui::SetNextWindowPos(ImVec2(center.x - windowWidth/2, center.y - windowHeight/2));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
        
        ImGui::Begin("VICTORY!", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::SetWindowFontScale(scale);
        ImGui::Dummy(ImVec2(0, 10 * scale));
        
        ImGui::SetCursorPosX(150 * scale);
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "V I C T O R Y !");
        
        ImGui::Dummy(ImVec2(0, 10 * scale));
        ImGui::SetCursorPosX(70 * scale);
        ImGui::Text("Congratulations! You defended the tower!");
        ImGui::SetCursorPosX(90 * scale);
        ImGui::Text("All %d waves have been defeated!", totalWaves);
        
        ImGui::Dummy(ImVec2(0, 15 * scale));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10 * scale));
        
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "FINAL STATISTICS:");
        ImGui::Text("Total Kills: %d", totalKills);
        ImGui::Text("  Ground: %d", groundKills);
        ImGui::Text("  Flying: %d", flyingKills);
        ImGui::Text("  Bosses: %d", bossKills);
        ImGui::Text("Final Coins: $%d", coins);
        ImGui::Text("Tower HP: %d/%d", towerHP, maxTowerHP);
        ImGui::Text("Structures Built: %d", structures.getSize());
        
        Structure* mvp = nullptr;
        int maxKills = 0;
        ListNode<Structure*>* node = structures. getHead();
        while (node) {
            if (node->data->getTotalKills() > maxKills) {
                maxKills = node->data->getTotalKills();
                mvp = node->data;
            }
            node = node->next;
        }
        
        if (mvp && maxKills > 0) {
            ImGui::Dummy(ImVec2(0, 5 * scale));
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 
                              "MVP: %s (Lv. %d) - %d kills! " ,
                              getStructureName(mvp->getType()), 
                              mvp->getLevel(), 
                              maxKills);
        }
        
        ImGui::Dummy(ImVec2(0, 20 * scale));
        ImGui::SetCursorPosX(120 * scale);
        if (ImGui::Button("MAIN MENU", ImVec2(200 * scale, 40 * scale))) {
            state = STATE_MENU;
        }
        
        ImGui::End();
    }
    
    void renderDefeat() {
        // FIX: Convert world center to screen pixel coords for proper centering
        sf::Vector2i center = window.mapCoordsToPixel(
            sf::Vector2f(Constants::WINDOW_WIDTH / 2.0f, Constants::WINDOW_HEIGHT / 2.0f), view);
        float scale = getUIScale();
        
        float windowWidth = 400 * scale;
        float windowHeight = 300 * scale;
        
        ImGui::SetNextWindowPos(ImVec2(center.x - windowWidth/2, center.y - windowHeight/2));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
        
        ImGui::Begin("GAME OVER", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::SetWindowFontScale(scale);
        ImGui::Dummy(ImVec2(0, 10 * scale));
        
        ImGui::SetCursorPosX(140 * scale);
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "G A M E   O V E R");
        
        ImGui::Dummy(ImVec2(0, 10 * scale));
        ImGui::SetCursorPosX(90 * scale);
        ImGui::Text("Your tower was destroyed on Wave %d", currentWave);
        
        ImGui::Dummy(ImVec2(0, 15 * scale));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10 * scale));
        
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "STATISTICS:");
        ImGui::Text("Total Kills: %d", totalKills);
        ImGui::Text("  Ground: %d", groundKills);
        ImGui::Text("  Flying: %d", flyingKills);
        ImGui::Text("  Bosses: %d", bossKills);
        ImGui::Text("Waves Survived: %d/%d", currentWave - 1, totalWaves);
        
        ImGui::Dummy(ImVec2(0, 25 * scale));
        
        ImGui::SetCursorPosX(55 * scale);
        if (ImGui::Button("TRY AGAIN", ImVec2(130 * scale, 40 * scale))) {
            startGame(mode);
        }
        ImGui::SameLine();
        if (ImGui::Button("MAIN MENU", ImVec2(130 * scale, 40 * scale))) {
            state = STATE_MENU;
        }
        
        ImGui::End();
    }
};

int main() {

    // Play intro video in fullscreen before game starts (8 seconds)
#ifdef _WIN32
    // Play intro video in fullscreen with VLC (8 seconds, auto-closes)
    std::string videoPath = "C:\\Users\\dell\\Documents\\MarvelTowerDefense\\assets\\video.mp4";

    // VLC paths to try
    const char* vlcPaths[] = {
        "C:\\Program Files (x86)\\VideoLAN\\VLC\\vlc.exe",
        "C:\\Program Files\\VideoLAN\\VLC\\vlc.exe",
        "C:\\Users\\dell\\AppData\\Local\\Programs\\VLC\\vlc.exe"
    };

    bool played = false;
    for (int i = 0; i < 3; i++) {
        // Build full command line: "vlc.exe" --fullscreen ... "video.mp4"
        std::string cmdLine = "\"" + std::string(vlcPaths[i]) + "\" --fullscreen --intf dummy --play-and-exit --no-video-title-show \"" + videoPath + "\"";

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_MAXIMIZE;

        // Create process - handles paths with spaces correctly
        if (CreateProcessA(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            // Wait for process to finish (video completes and VLC closes)
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            played = true;
            break;
        }
    }

    // Fallback: try default Windows player
    if (!played) {
        ShellExecuteA(NULL, "open", videoPath.c_str(), NULL, NULL, SW_SHOWMAXIMIZED);
        Sleep(8500); // Wait 8.5 seconds for video
    }
#else
    system("xdg-open --fullscreen \"C:/Users/dell/Documents/MarvelTowerDefense/assets/video.mp4\" &");
    sleep(7);
#endif


    try {
        Game game;
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n";
    std::cout << "+==========================================+\n";
    std::cout << "|  Thanks for playing Marvel Tower Defense!  |\n";
    std::cout << "|  See you next time, Hero!                 |\n";
    std::cout << "+==========================================+\n";
    std::cout << "\n";
    
    return 0;
}