#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstring>
#include "Constants.hpp"
#include "TextureManager.hpp"

enum GameState { 
    STATE_SPLASH,       // Initial splash screen with hero images
    STATE_HOWTOPLAY,    // How to play instructions
    STATE_MENU,         // Game mode selection menu
    STATE_WAVE_PREP, 
    STATE_WAVE_ACTIVE, 
    STATE_PAUSED, 
    STATE_VICTORY, 
    STATE_DEFEAT 
};
enum GameMode { MODE_EASY, MODE_EPIC };
enum EnemyType { ENEMY_GROUND, ENEMY_FLYING, ENEMY_BOSS };
enum BossType { BOSS_NONE, BOSS_ULTRON, BOSS_THANOS, BOSS_DOOM };
enum StructureType { STRUCT_NONE, STRUCT_BLACKWIDOW, STRUCT_CAPTAIN, STRUCT_HAWKEYE, STRUCT_IRONMAN, STRUCT_DRSTRANGE, STRUCT_THOR};

inline const char* getBossName(BossType boss) {
    switch(boss) {
        case BOSS_ULTRON: return "Ultron";
        case BOSS_THANOS: return "Thanos";
        case BOSS_DOOM: return "Doctor Doom";
        default: return "Boss";
    }
}

/**
 * PathPoint - Represents a single waypoint in an enemy path
 * 
 * This structure stores a 2D coordinate point that enemies follow along their
 * path from spawn point to the tower. Multiple PathPoints form a complete path.
 * 
 * Members:
 *   - x: X coordinate in world space (pixels)
 *   - y: Y coordinate in world space (pixels)
 * 
 * Usage:
 *   Used by GameMap to define enemy paths. Each path is an array of PathPoints.
 *   Enemies move from one PathPoint to the next, following the path until they
 *   reach the tower or are destroyed.
 * 
 * Example:
 *   PathPoint spawn(-50, 100);  // Enemy spawns off-screen left
 *   PathPoint tower(975, 560);  // Tower position
 */
struct PathPoint {
    float x, y;
    PathPoint() : x(0), y(0) {}
    PathPoint(float px, float py) : x(px), y(py) {}
};

inline float calcDistance(float x1, float y1, float x2, float y2) {
    return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

inline const char* getStructureName(StructureType type) {
    switch(type) {
        case STRUCT_BLACKWIDOW: return "Black Widow";
        case STRUCT_CAPTAIN: return "Captain America";
        case STRUCT_HAWKEYE: return "Hawkeye";
        case STRUCT_IRONMAN: return "Iron Man";
        case STRUCT_DRSTRANGE: return "Dr Strange";
        case STRUCT_THOR: return "Thor";


        default: return "None";
    }
}

inline int getStructureCost(StructureType type) {
    switch(type) {
        case STRUCT_BLACKWIDOW: return Constants::BLACKWIDOW_COST;
        case STRUCT_CAPTAIN: return Constants::CAPTAIN_COST;
        case STRUCT_HAWKEYE: return Constants::HAWKEYE_COST;
        case STRUCT_IRONMAN: return Constants::IRONMAN_COST;
        case STRUCT_DRSTRANGE: return Constants::DRSTRANGE_COST;
        case STRUCT_THOR: return Constants::THOR_COST;

        default: return 0;
    }
}

/**
 * Enemy - Represents an enemy unit that moves along paths toward the tower
 * 
 * This class handles all enemy behavior including movement, health, damage,
 * and rendering. Enemies spawn at the start of paths and move toward the tower.
 * If they reach the tower, they damage it. If destroyed by structures, they
 * reward the player with coins.
 * 
 * Enemy Types:
 *   - ENEMY_GROUND: Ground-based enemy, can be targeted by all structures
 *   - ENEMY_FLYING: Flying enemy, can only be targeted by certain structures
 *   - ENEMY_BOSS: Special boss enemy with unique stats and textures
 * 
 * Boss Types:
 *   - BOSS_ULTRON: Appears in wave 5 (Easy) and wave 5 (Epic)
 *   - BOSS_THANOS: Appears in wave 10 (Easy) and wave 10 (Epic)
 *   - BOSS_DOOM: Appears in wave 15 (Epic mode only)
 * 
 * Key Features:
 *   - Path following: Moves along predefined waypoints
 *   - Health system: Takes damage from projectiles, dies when health reaches 0
 *   - Scaling: Stats increase with wave number (15% per wave)
 *   - Visual feedback: Health bar shows remaining health
 *   - Rewards: Drops coins when destroyed
 * 
 * Members:
 *   - id: Unique identifier for this enemy
 *   - type: Enemy type (ground, flying, boss)
 *   - bossType: Specific boss type if this is a boss enemy
 *   - sprite: SFML sprite for rendering the enemy
 *   - healthBarBg/Fill: Visual health bar components
 *   - maxHealth/currentHealth: Health tracking
 *   - coinReward: Coins player receives when enemy is destroyed
 *   - towerDamage: Damage dealt to tower if enemy reaches it
 *   - moveSpeed: Movement speed in pixels per second
 *   - alive: Whether enemy is still alive
 *   - path: Array of waypoints this enemy follows
 *   - pathLength: Number of waypoints in the path
 *   - pathIndex: Current waypoint index enemy is moving toward
 * 
 * Size Scaling:
 *   - Regular enemies: 96x96 pixels (3x larger for visibility)
 *   - Boss enemies: 150x150 pixels (3x larger for visibility)
 */
class Enemy : public sf::Drawable {
private:
    int id;
    EnemyType type;
    BossType bossType;
    sf::Sprite sprite;
    sf::RectangleShape healthBarBg;
    sf::RectangleShape healthBarFill;
    int maxHealth, currentHealth, coinReward, towerDamage;
    float moveSpeed;
    bool alive;
    float currentAngle;        // Smoothed facing angle
    float turnSpeed;           // Degrees per second for turning
    bool rotationInitialized;  // Tracks first angle set
    PathPoint path[Constants::MAX_PATH_LENGTH];
    int pathLength, pathIndex;

public:
    Enemy() : id(0), type(ENEMY_GROUND), bossType(BOSS_NONE), maxHealth(50), currentHealth(50),
              coinReward(10), towerDamage(10), moveSpeed(45.0f),  // Even slower movement
              alive(true), currentAngle(0.0f), turnSpeed(540.0f), rotationInitialized(false),
              pathLength(0), pathIndex(0) {}
    
    // Regular enemy initialization
    void initialize(int enemyId, EnemyType enemyType, int wave) {
        initializeWithBoss(enemyId, enemyType, wave, BOSS_NONE);
    }
    
    // Boss-specific initialization
    void initializeWithBoss(int enemyId, EnemyType enemyType, int wave, BossType boss) {
        id = enemyId;
        type = enemyType;
        bossType = boss;
        alive = true;
        pathIndex = 0;
        rotationInitialized = false;
        

        float waveMultiplier = 1.0f + (wave - 1) * 0.20f;  // Increased scaling for competitive difficulty
        const char* textureName = "enemy_ground";

        switch(type) {
        case ENEMY_GROUND:
            textureName = "enemy_ground";
            maxHealth = (int)(70 * waveMultiplier);  // Increased from 50 for more challenge
            coinReward = Constants::GROUND_KILL_REWARD;
            towerDamage = 12;
            moveSpeed = 45.0f;  // Slightly faster for competitive gameplay
            break;
        case ENEMY_FLYING:
            textureName = "enemy_flying";
            maxHealth = (int)(50 * waveMultiplier);  // Increased from 30 for balance
            coinReward = Constants::FLYING_KILL_REWARD;
            towerDamage = 18;
            moveSpeed = 62.0f;  // Faster for competitive challenge
            break;
            case ENEMY_BOSS:
                // Different stats and textures for each boss
                switch(bossType) {
                    case BOSS_ULTRON:
                        textureName = "boss_ultron";
                        maxHealth = (int)(600 * waveMultiplier);
                        coinReward = Constants::BOSS_KILL_REWARD;
                        towerDamage = 45;
                        moveSpeed = 55.0f;  // Faster than other bosses
                        break;
                    case BOSS_THANOS:
                        textureName = "boss_thanos";
                        maxHealth = (int)(900 * waveMultiplier);
                        coinReward = (int)(Constants::BOSS_KILL_REWARD * 1.5f);
                        towerDamage = 80;
                        moveSpeed = 40.0f;  // Slower but more powerful
                        break;
                    case BOSS_DOOM:
                        textureName = "boss_doom";
                        maxHealth = (int)(1200 * waveMultiplier);
                        coinReward = Constants::BOSS_KILL_REWARD * 2;
                        towerDamage = 120;
                        moveSpeed = 35.0f;  // Slowest but most dangerous
                        break;
                    default:
                        textureName = "boss_ultron";
                        maxHealth = (int)(500 * waveMultiplier);
                        coinReward = Constants::BOSS_KILL_REWARD;
                        towerDamage = 50;
                        moveSpeed = 40.0f;
                        break;
                }
                break;
        }
        
        currentHealth = maxHealth;
        const sf::Texture& tex = TextureManager::getInstance().getTexture(textureName);
        sprite.setTexture(tex);
        
        // Scale sprite to appropriate size based on type
        sf::Vector2u texSize = tex.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            // Enemy sprite sizes - increased to 3x for better visibility
            float targetSize;
            if (type == ENEMY_BOSS) {
                targetSize = 150.0f;  // Bosses are larger (150x150 pixels - 3x original 50px)
            } else {
                targetSize = 96.0f;  // Regular enemies (96x96 pixels - 3x original 32px)
            }
            float scale = targetSize / std::max(texSize.x, texSize.y);
            sprite.setScale(scale, scale);
        }
        
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2, bounds.height / 2);
        
        // Health bar widths - half size for better visibility
        float barWidth = (type == ENEMY_BOSS) ? 82.5f : 57.0f;
        healthBarBg.setSize(sf::Vector2f(barWidth, 6));
        healthBarBg.setFillColor(sf::Color(50, 50, 50));
        healthBarFill.setSize(sf::Vector2f(barWidth - 2, 4));
        healthBarFill.setFillColor(sf::Color::Green);
    }
    
    void setPath(PathPoint* newPath, int length) {
        pathLength = (length > Constants::MAX_PATH_LENGTH) ?  Constants::MAX_PATH_LENGTH : length;
        for (int i = 0; i < pathLength; i++) path[i] = newPath[i];
        if (pathLength > 0) sprite.setPosition(path[0]. x, path[0].y);
    }
    
    void update(float deltaTime) {
        if (!alive) return;
        if (pathIndex < pathLength) {
            float targetX = path[pathIndex].x;
            float targetY = path[pathIndex]. y;
            sf::Vector2f current = sprite.getPosition();
            float dx = targetX - current.x;
            float dy = targetY - current.y;
            float distance = sqrt(dx*dx + dy*dy);
            
            if (distance < 5.0f) {
                pathIndex++;
            } else {
                dx /= distance;
                dy /= distance;
                // Smooth turning toward movement direction
                float targetAngle = atan2(dy, dx) * 180.0f / 3.14159265f;
                if (!rotationInitialized) {
                    currentAngle = targetAngle;
                    rotationInitialized = true;
                }
                // Normalize shortest angular difference
                float diff = targetAngle - currentAngle;
                while (diff > 180.0f) diff -= 360.0f;
                while (diff < -180.0f) diff += 360.0f;
                float maxStep = turnSpeed * deltaTime;
                if (std::fabs(diff) <= maxStep) {
                    currentAngle = targetAngle;
                } else {
                    currentAngle += (diff > 0 ? 1.0f : -1.0f) * maxStep;
                }
                sprite.setRotation(currentAngle);

                sprite. move(dx * moveSpeed * deltaTime, dy * moveSpeed * deltaTime);
            }
        }
        
        sf::Vector2f pos = sprite.getPosition();
        float barWidth = healthBarBg.getSize().x;
        healthBarBg.setPosition(pos.x - barWidth/2, pos.y - 35);
        healthBarFill.setPosition(pos.x - barWidth/2 + 1, pos.y - 34);
        
        float healthPercent = (float)currentHealth / maxHealth;
        healthBarFill.setSize(sf::Vector2f((barWidth - 2) * healthPercent, 4));
        
        if (healthPercent > 0.6f) healthBarFill.setFillColor(sf::Color::Green);
        else if (healthPercent > 0.3f) healthBarFill.setFillColor(sf::Color::Yellow);
        else healthBarFill.setFillColor(sf::Color::Red);
    }
    
    void takeDamage(int damage) {
        currentHealth -= damage;
        if (currentHealth <= 0) { currentHealth = 0; alive = false; }
    }
    
    bool hasReachedEnd() const { return pathIndex >= pathLength; }
    bool isAlive() const { return alive; }
    int getCoinReward() const { return coinReward; }
    int getTowerDamage() const { return towerDamage; }

    EnemyType getType() const { return type; }
    BossType getBossType() const { return bossType; }
    sf::Vector2f getPosition() const { return sprite. getPosition(); }
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (alive) {
            target.draw(sprite, states);
            target.draw(healthBarBg, states);
            target. draw(healthBarFill, states);
        }
    }
};

/**
 * Projectile - Represents a projectile fired by structures at enemies
 * 
 * This class handles projectile behavior including movement, collision detection,
 * and rendering. Each hero structure fires a different type of projectile with
 * unique properties (speed, damage, appearance).
 * 
 * Projectile Types:
 *   - PROJ_BULLET: Black Widow's rapid-fire bullets (fast, moderate damage)
 *   - PROJ_SHIELD: Captain America's shield (medium speed, bounces)
 *   - PROJ_ARROW: Hawkeye's arrows (medium speed, high damage)
 *   - PROJ_MISSILE: Iron Man's missiles (slow, high damage, area effect)
 *   - PROJ_BEAM: Dr. Strange's time beam (very fast, high damage)
 *   - PROJ_LIGHTNING: Thor's lightning (fastest, very high damage)
 * 
 * Key Features:
 *   - Homing: Can track moving enemies (if target is provided)
 *   - Lifetime: Projectiles expire after a maximum lifetime to prevent memory leaks
 *   - Collision: Detects hits with enemies within a hit radius
 *   - Visual scaling: Each projectile type has appropriate size
 * 
 * Members:
 *   - type: Projectile type (determines texture and behavior)
 *   - sprite: SFML sprite for rendering the projectile
 *   - position: Current world position (x, y)
 *   - velocity: Movement vector (direction and speed)
 *   - damage: Damage dealt to enemies on hit
 *   - lifetime: Current age of projectile in seconds
 *   - maxLifetime: Maximum age before projectile expires
 *   - speed: Movement speed in pixels per second
 *   - alive: Whether projectile is still active
 *   - target: Optional pointer to target enemy (for homing projectiles)
 * 
 * Size Scaling:
 *   - Regular projectiles: 48x48 pixels (3x larger)
 *   - Shield projectile: 60x60 pixels (3x larger, Captain America's shield)
 */
class Projectile : public sf::Drawable {
private:
    Constants::ProjectileType type;
    sf::Sprite sprite;
    sf::Vector2f position, velocity;
    int damage;
    float lifetime, maxLifetime, speed;
    bool alive;
    Enemy* target;  // Add target reference for homing

public:
    Projectile() : alive(false), lifetime(0), maxLifetime(0), target(nullptr) {}

    void initialize(Constants::ProjectileType ptype, sf::Vector2f startPos, sf::Vector2f targetPos, int dmg, float spd, Enemy* targ = nullptr) {
        type = ptype;
        position = startPos;
        speed = spd;
        damage = dmg;
        target = targ;  // Set target for homing
        maxLifetime = 5.0f;  // Fallback lifetime
        lifetime = 0;
        alive = true;
        
        // Initial velocity toward target position
        sf::Vector2f dir = targetPos - startPos;
        float dist = sqrt(dir.x * dir.x + dir.y * dir.y);
        if (dist > 0.0001f) {
            velocity = (dir / dist) * speed;
        } else {
            velocity = sf::Vector2f(speed, 0.0f); // default rightward if same point
        }
        
        const sf::Texture& tex = TextureManager::getInstance().getTexture(getTextureName());
        sprite.setTexture(tex);
        
        // Scale projectile sprite to appropriate size - 3x larger (48x48 pixels, shield is 60x60)
        sf::Vector2u texSize = tex.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            float targetSize = (ptype == Constants::PROJ_SHIELD) ? 100.0f : 78.0f;  // 3x larger
            float scale = targetSize / std::max(texSize.x, texSize.y);
            sprite.setScale(scale, scale);
        }
        
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2, bounds.height / 2);
        sprite.setPosition(position);
        // Face initial direction
        float initialAngle = atan2(velocity.y, velocity.x) * 180.0f / 3.14159265f;
        sprite.setRotation(initialAngle);
    }

    const char* getTextureName() {
        switch(type) {
            case Constants::PROJ_BULLET: return "proj_bullet";
            case Constants::PROJ_SHIELD: return "proj_shield";
            case Constants::PROJ_ARROW: return "proj_arrow";
            case Constants::PROJ_MISSILE: return "proj_missile";
            case Constants::PROJ_BEAM: return "proj_beam";
            case Constants::PROJ_LIGHTNING: return "proj_lightning";
        }
        return "proj_bullet";
    }

    void update(float dt) {
        if (!alive) return;
        lifetime += dt;

        // Homing logic: update direction toward target if it exists and is alive
        if (target && target->isAlive()) {
            sf::Vector2f targetPos = target->getPosition();
            sf::Vector2f dir = targetPos - position;
            float dist = sqrt(dir.x * dir.x + dir.y * dir.y);
            if (dist > 0) {
                velocity = (dir / dist) * speed;  // Recalculate velocity toward current target position
            }
        }

        position += velocity * dt;
        sprite.setPosition(position);
        // Rotate to face movement
        if (velocity.x != 0 || velocity.y != 0) {
            float ang = atan2(velocity.y, velocity.x) * 180.0f / 3.14159265f;
            sprite.setRotation(ang);
        }

        if (lifetime > maxLifetime) alive = false;
    }

    bool isAlive() const { return alive; }
    sf::Vector2f getPosition() const { return position; }
    int getDamage() const { return damage; }
    void setAlive(bool a) { alive = a; }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (alive) target.draw(sprite, states);
    }
};

/**
 * Structure - Represents a hero tower/defense structure placed by the player
 * 
 * This class handles all structure behavior including targeting, attacking,
 * upgrading, and rendering. Structures are the player's main defense against
 * enemies. Each structure represents a different Marvel hero with unique abilities.
 * 
 * Structure Types (Heroes):
 *   - STRUCT_BLACKWIDOW: Rapid-fire bullets, low cost, good for early game
 *   - STRUCT_CAPTAIN: Shield throw, medium cost, balanced stats
 *   - STRUCT_HAWKEYE: Arrows, medium cost, high damage
 *   - STRUCT_IRONMAN: Missiles, high cost, area damage
 *   - STRUCT_DRSTRANGE: Time beam, very high cost, very high damage
 *   - STRUCT_THOR: Lightning, highest cost, highest damage
 * 
 * Upgrade System:
 *   - Level 1: Base stats, initial cost
 *   - Level 2: +50% damage, +25% range, upgrade cost = original cost
 *   - Level 3: +100% damage (from base), +50% range, upgrade cost = original cost
 * 
 * Key Features:
 *   - Targeting: Automatically finds closest enemy in range
 *   - Attack cooldown: Fires projectiles based on fire rate
 *   - Range system: Only attacks enemies within attack range
 *   - Upgrade system: Can be upgraded twice for better stats
 *   - Visual feedback: Shows attack direction and cooldown
 *   - Type-specific: Each hero has unique projectile type and stats
 * 
 * Members:
 *   - id: Unique identifier for this structure
 *   - type: Structure type (which hero)
 *   - level: Upgrade level (1, 2, or 3)
 *   - sprite: SFML sprite for rendering the structure
 *   - position: World position where structure is placed
 *   - damage: Damage dealt per projectile
 *   - range: Attack range in pixels
 *   - fireRate: Time between attacks in seconds
 *   - fireTimer: Time since last attack
 *   - cost: Cost to build/upgrade this structure
 *   - projType: Type of projectile this structure fires
 *   - canTargetGround: Whether structure can target ground enemies
 *   - canTargetFlying: Whether structure can target flying enemies
 *   - attackVisual: Visual indicator showing last attack direction
 * 
 * Size Scaling:
 *   - All structures: 96x96 pixels (3x larger for visibility)
 */
class Structure : public sf::Drawable {
private:
    int id;
    StructureType type;
    int level, damage, totalKills, totalDamageDealt;
    float range, fireRate, cooldown;
    bool canTargetAir, canTargetGround, showRange;
    sf::Sprite sprite;
    sf::CircleShape rangeIndicator;
    sf::VertexArray attackLine;
    float attackVisualTimer;
    Constants::ProjectileType projType;

public:
    Structure() : id(0), type(STRUCT_NONE), level(1), damage(0), totalKills(0), totalDamageDealt(0),
                  range(0), fireRate(1.0f), cooldown(0), canTargetAir(false), canTargetGround(true),
                  showRange(false), attackVisualTimer(0), attackLine(sf::Lines, 2) {}
    
    void initialize(int structId, StructureType structType, float posX, float posY) {
        id = structId;
        type = structType;
        level = 1;
        cooldown = 0;
        totalKills = 0;
        totalDamageDealt = 0;
        
        switch(type) {
            case STRUCT_BLACKWIDOW:
                projType = Constants::PROJ_BULLET;
                damage = 15; range = 110.0f; fireRate = 2.0f;  // Fast firing bullets
                canTargetAir = false; canTargetGround = true;
                break;
            case STRUCT_CAPTAIN:
                projType = Constants::PROJ_SHIELD;
                damage = 18; range = 130.0f; fireRate = 1.2f;  // Shield throw
                canTargetAir = false; canTargetGround = true;
                break;
            case STRUCT_HAWKEYE:
                projType = Constants::PROJ_ARROW;
                damage = 16; range = 160.0f; fireRate = 0.8f;  // Precise arrows
                canTargetAir = false; canTargetGround = true;
                break;
            case STRUCT_IRONMAN:
                projType = Constants::PROJ_MISSILE;
                damage = 35; range = 180.0f; fireRate = 1.0f;  // Homing missiles
                canTargetAir = true; canTargetGround = true;
                break;
            case STRUCT_DRSTRANGE:
                projType = Constants::PROJ_BEAM;
                damage = 40; range = 170.0f; fireRate = 1.1f;  // Time beam
                canTargetAir = true; canTargetGround = true;
                break;
            case STRUCT_THOR:
                projType = Constants::PROJ_LIGHTNING;
                damage = 150; range = 190.0f; fireRate = 1.8f;  // Lightning
                canTargetAir = true; canTargetGround = true;
                break;


            default: break;
        }
        
        // Get the texture for this structure from the TextureManager singleton
        // getTextureName() returns the texture name based on structure type and level (e.g., "struct_thor_lv3")
        const sf::Texture& tex = TextureManager::getInstance().getTexture(getTextureName());
        
        // Set the sprite's texture to the loaded texture
        // This associates the visual image with the sprite object
        sprite.setTexture(tex);
        
        // Scale structure sprite to appropriate size
        // Structures are scaled to 110x110 pixels as requested
        // Use uniform scaling based on the larger dimension to maintain aspect ratio
        
        // Get the original texture dimensions (width and height in pixels)
        // texSize.x = texture width, texSize.y = texture height
        sf::Vector2u texSize = tex.getSize();
        
        // Check if texture has valid dimensions (both width and height > 0)
        // This prevents division by zero errors and handles invalid textures
        if (texSize.x > 0 && texSize.y > 0) {
            // Target display size: 110x110 pixels (as requested by user)
            // This is the maximum size the structure should appear on screen
            float targetSize = 110.0f;
            
            // Calculate the maximum dimension (either width or height, whichever is larger)
            // Cast to float to ensure floating-point division for accurate scaling
            // Example: If texture is 2125x1615, maxDimension = 2125
            float maxDimension = std::max((float)texSize.x, (float)texSize.y);
            
            // Calculate the scale factor needed to fit the texture within target size
            // Formula: scale = targetSize / maxDimension
            // Example: scale = 110 / 2125 = 0.0518 (texture will be scaled down to 5.18% of original)
            float scale = targetSize / maxDimension;
            
            // Apply the scale factor to both X and Y axes uniformly
            // Uniform scaling (same scale for X and Y) maintains aspect ratio (prevents distortion)
            // If texture is 2125x1615 and scale is 0.0518, result is approximately 110x83.6 pixels
            sprite.setScale(scale, scale);
            
            // Debug output for Thor level 3 to help diagnose fitting issues
            // Thor level 3 has different dimensions (1000x760) compared to other structures (2125x1615)
            // This debug output helps identify scaling issues
            if (type == STRUCT_THOR && level == 3) {
                // Get the actual scaled bounds of the sprite after scaling is applied
                // bounds.width = actual display width, bounds.height = actual display height
                sf::FloatRect bounds = sprite.getLocalBounds();
                
                // Print debug information: original texture size, scale factor, and final display size
                // This helps verify that scaling is working correctly
                std::cout << "Thor Lv3 - Texture: " << texSize.x << "x" << texSize.y 
                          << ", Scale: " << scale 
                          << ", Scaled size: " << bounds.width << "x" << bounds.height << std::endl;
            }
        }
        
        // Set origin to center of sprite for proper rotation and positioning
        // Origin is the point around which the sprite rotates and is positioned
        // Getting local bounds after scaling to get the actual scaled dimensions
        sf::FloatRect bounds = sprite.getLocalBounds();
        
        // Set origin to center: width/2 horizontally, height/2 vertically
        // This ensures the sprite is centered at its position (posX, posY)
        // Example: If sprite is 110x83.6, origin is at (55, 41.8) - the center point
        sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        
        // Set the sprite's position in world coordinates
        // posX and posY are the grid-snapped coordinates where the structure should be placed
        // The sprite will be centered at this position due to the origin being set to center
        sprite.setPosition(posX, posY);
        
        rangeIndicator.setRadius(range);
        rangeIndicator.setOrigin(range, range);
        rangeIndicator.setPosition(posX, posY);
        rangeIndicator.setFillColor(sf::Color(100, 200, 100, 40));
        rangeIndicator.setOutlineColor(sf::Color(100, 200, 100, 150));
        rangeIndicator.setOutlineThickness(2);
    }
    
    void updateCooldown(float deltaTime) {
        if (cooldown > 0) cooldown -= deltaTime;
        if (attackVisualTimer > 0) attackVisualTimer -= deltaTime;
    }
    
    bool canFire() const { return cooldown <= 0; }
    void fire() { cooldown = 1.0f / fireRate; }
    
    bool isInRange(sf::Vector2f targetPos) const {
        sf::Vector2f pos = sprite.getPosition();
        return calcDistance(pos.x, pos.y, targetPos.x, targetPos.y) <= range;
    }
    
    bool canTarget(EnemyType enemyType) const {
        if (enemyType == ENEMY_FLYING) return canTargetAir;
        return canTargetGround;
    }
    
    void setAttackVisual(sf::Vector2f targetPos) {
        // Visual line disabled for cleaner look
        attackVisualTimer = 0.0f;
    }
    
    bool upgrade() {
        if (level >= 3) return false;
        level++;
        damage = (int)(damage * 1.3f);
        range *= 1.15f;
        rangeIndicator.setRadius(range);
        rangeIndicator.setOrigin(range, range);
        
        // Update texture and maintain scale
        const sf::Texture& tex = TextureManager::getInstance().getTexture(getTextureName());
        sf::Vector2f currentScale = sprite.getScale();
        sprite.setTexture(tex);
        sprite.setScale(currentScale);  // Maintain the same scale
        
        return true;
    }
    
    int getUpgradeCost() const {
        if (level >= 3) return 0;
        return (int)(getStructureCost(type) * 0.5f * level);
    }
    
    int getSellValue() const {
        return (int)(getStructureCost(type) * Constants::SELL_REFUND_PERCENT * level);
    }
    
    void addKill() { totalKills++; }
    void addDamage(int dmg) { totalDamageDealt += dmg; }
    void setShowRange(bool show) { showRange = show; }
    
    int getDamage() const { return damage; }
    float getRange() const { return range; }
    int getLevel() const { return level; }
    int getTotalKills() const { return totalKills; }
    int getTotalDamageDealt() const { return totalDamageDealt; }
    StructureType getType() const { return type; }
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
    Constants::ProjectileType getProjectileType() const { return projType; }
    
    const char* getTextureName() const {
        static char buf[64];
        const char* base = "";
        switch(type) {
            case STRUCT_BLACKWIDOW: base = "struct_blackwidow"; break;
            case STRUCT_CAPTAIN: base = "struct_captain"; break;
            case STRUCT_HAWKEYE: base = "struct_hawkeye"; break;
            case STRUCT_IRONMAN: base = "struct_ironman"; break;
            case STRUCT_DRSTRANGE: base = "struct_drstrange"; break;
            case STRUCT_THOR: base = "struct_thor"; break;


            default: base = "struct_blackwidow"; break;

        }
        sprintf(buf, "%s_lv%d", base, level);
        return buf;
    }//STRUCT_HULK_LV1
    
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (showRange) target.draw(rangeIndicator, states);
        target.draw(sprite, states);
    }
};