#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>
#include <string>
#include "Constants.hpp"

/**
 * TextureManager - Singleton class for managing all game textures
 * 
 * This class provides centralized texture loading and management for the entire game.
 * It implements the Singleton pattern to ensure only one instance exists, providing
 * efficient texture storage and easy access throughout the codebase.
 * 
 * Key Features:
 *   - Singleton Pattern: Single global instance accessible via getInstance()
 *   - Multi-path Loading: Tries multiple directory paths to find texture files
 *   - Case-insensitive Extensions: Handles both .png and .PNG file extensions
 *   - Placeholder System: Creates colored placeholders for missing textures
 *   - Debug Output: Logs which textures loaded successfully or failed
 *   - Asset Loading: Bulk loads all game assets (enemies, structures, projectiles, etc.)
 * 
 * Texture Categories:
 *   - Game Assets: Tower, enemies, bosses, structures, projectiles
 *   - Splash Screen: Background, logo, hero portraits
 *   - Placeholders: Auto-generated colored rectangles for missing textures
 * 
 * Loading Strategy:
 *   1. Tries original path (e.g., "assets/image.png")
 *   2. Tries one level up (e.g., "../assets/image.png")
 *   3. Tries two levels up (e.g., "../../assets/image.png")
 *   4. Tries uppercase extensions (.PNG) for each path
 *   5. Creates placeholder if all paths fail
 * 
 * Members:
 *   - MAX_TEXTURES: Maximum number of textures that can be stored (80)
 *   - textures: Array of SFML Texture objects
 *   - textureNames: Array of texture name strings (for lookup)
 *   - textureCount: Current number of loaded textures
 * 
 * Usage:
 *   TextureManager::getInstance().getTexture("enemy_ground");
 *   TextureManager::getInstance().loadAllAssets();
 * 
 * Thread Safety:
 *   Not thread-safe. Should only be accessed from main thread during initialization.
 */
class TextureManager {
private:
    static const int MAX_TEXTURES = 80;  // Increased for splash screen images
    sf::Texture textures[MAX_TEXTURES];
    char textureNames[MAX_TEXTURES][48];  // Increased for longer names
    int textureCount;
    
    TextureManager() : textureCount(0) {
        for (int i = 0; i < MAX_TEXTURES; i++) textureNames[i][0] = '\0';
    }

public:
    static TextureManager& getInstance() {
        static TextureManager instance;
        return instance;
    }
    
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    
    int findTextureIndex(const char* name) const {
        for (int i = 0; i < textureCount; i++) {
            if (strcmp(textureNames[i], name) == 0) return i;
        }
        return -1;
    }
    
    void addTexture(const char* name, const sf::Texture& tex) {
        if (textureCount >= MAX_TEXTURES) return;
        int existing = findTextureIndex(name);
        if (existing >= 0) {
            textures[existing] = tex;
            return;
        }
        textures[textureCount] = tex;
        strcpy(textureNames[textureCount], name);
        textureCount++;
    }
    
    sf::Texture& getTexture(const char* name) {
        int idx = findTextureIndex(name);
        if (idx >= 0) return textures[idx];
        return textures[0];
    }
    
    void loadTexture(const char* filename, const char* name) {
        // Try multiple path locations (executable may run from build/ directory)
        // Also try both lowercase and uppercase extensions for Windows compatibility
        std::string basePath = filename;
        std::string paths[9];  // 3 base paths × 3 extension variants
        
        // Find the last dot to replace extension
        size_t lastDot = basePath.find_last_of('.');
        std::string baseWithoutExt = (lastDot != std::string::npos) ? basePath.substr(0, lastDot) : basePath;
        
        // Build path array: try .png, .PNG, and original extension for each path location
        int idx = 0;
        for (int i = 0; i < 3; i++) {
            std::string prefix = (i == 0) ? "" : (i == 1) ? "../" : "../../";
            paths[idx++] = prefix + baseWithoutExt + ".png";      // Try lowercase
            paths[idx++] = prefix + baseWithoutExt + ".PNG";      // Try uppercase
            paths[idx++] = prefix + basePath;                     // Try original (in case it's different)
        }
        
        bool loaded = false;
        std::string loadedPath;
        
        for (const std::string& path : paths) {
            sf::Texture tex;
            if (tex.loadFromFile(path)) {
                sf::Vector2u size = tex.getSize();
                if (size.x > 0 && size.y > 0) {
                    addTexture(name, tex);
                    std::cout << "✓ Loaded: " << path << " as " << name << " (" << size.x << "x" << size.y << ")" << std::endl;
                    loaded = true;
                    loadedPath = path;
                    break;
                } else {
                    std::cout << "✗ Texture " << path << " has invalid size (0x0)" << std::endl;
                }
            }
        }
        
        if (!loaded) {
            std::cout << "✗ Failed to load: " << filename << std::endl;
            std::cout << "  Tried paths:" << std::endl;
            for (const std::string& path : paths) {
                std::cout << "    - " << path << std::endl;
            }
            std::cout << "  Using placeholder for " << name << std::endl;
            createPlaceholder(name);
        }
    }
    
    void createPlaceholder(const char* name) {
        sf::Color color;
        unsigned int size = 40;
        
        if (strcmp(name, "map_background") == 0) {
            sf::Image img;
            img.create(Constants::GAME_AREA_WIDTH, Constants::WINDOW_HEIGHT, sf::Color(34, 49, 34));
            sf::Texture tex;
            tex.loadFromImage(img);
            addTexture(name, tex);
            return;
        } else if (strcmp(name, "tower") == 0) {
            color = sf::Color(255, 215, 0);
            size = 80;
        } else if (strncmp(name, "enemy_", 6) == 0) {
            if (strcmp(name, "enemy_ground") == 0) color = sf::Color(139, 69, 19);
            else if (strcmp(name, "enemy_flying") == 0) color = sf::Color(100, 149, 237);
            else if (strcmp(name, "enemy_boss") == 0) { color = sf::Color(178, 34, 34); size = 64; }
        } else if (strncmp(name, "boss_", 5) == 0) {
            // Different colors for each boss type
            size = 64;
            if (strcmp(name, "boss_ultron") == 0) {
                color = sf::Color(192, 192, 192);  // Silver/metallic for Ultron
            } else if (strcmp(name, "boss_thanos") == 0) {
                color = sf::Color(128, 0, 128);    // Purple for Thanos
            } else if (strcmp(name, "boss_doom") == 0) {
                color = sf::Color(0, 100, 0);      // Dark green for Doctor Doom
            }
        } else if (strncmp(name, "struct_", 7) == 0) {
            color = sf::Color(34, 139, 34);
            if (strstr(name, "blackwidow")) color = sf::Color(30, 30, 30);        // Black for Black Widow
            else if (strstr(name, "captain")) color = sf::Color(0, 71, 171);      // Blue for Captain America
            else if (strstr(name, "hawkeye")) color = sf::Color(148, 0, 211);     // Purple for Hawkeye
            else if (strstr(name, "ironman")) color = sf::Color(220, 20, 60);     // Red for Iron Man
            else if (strstr(name, "drstrange")) color = sf::Color(255, 140, 0);   // Orange for Dr Strange
            else if (strstr(name, "thor")) color = sf::Color(30, 144, 255);       // Blue for Thor
            else if (strstr(name, "Marvel")) color = sf::Color(30, 184, 255);
        } else if (strncmp(name, "proj_", 5) == 0) {
            color = sf::Color(255, 255, 0);
            size = 16;
            if (strstr(name, "bullet")) color = sf::Color(255, 200, 0);           // Yellow bullet
            else if (strstr(name, "shield")) { color = sf::Color(200, 0, 0); size = 20; }  // Red/white shield
            else if (strstr(name, "arrow")) color = sf::Color(160, 82, 45);       // Brown arrow
            else if (strstr(name, "missile")) color = sf::Color(255, 0, 0);       // Red missile
            else if (strstr(name, "beam")) color = sf::Color(0, 255, 255);        // Cyan time beam
            else if (strstr(name, "lightning")) color = sf::Color(255, 255, 0);   // Yellow lightning
        } else if (strncmp(name, "splash_", 7) == 0) {
            // Splash screen hero placeholders (larger images)
            if (strcmp(name, "splash_background") == 0) {
                sf::Image img;
                img.create(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT, sf::Color(15, 15, 25));
                // Add gradient effect
                for (unsigned int y = 0; y < Constants::WINDOW_HEIGHT; y++) {
                    for (unsigned int x = 0; x < Constants::WINDOW_WIDTH; x++) {
                        int r = 15 + (y * 20 / Constants::WINDOW_HEIGHT);
                        int g = 15;
                        int b = 25 + (y * 15 / Constants::WINDOW_HEIGHT);
                        img.setPixel(x, y, sf::Color(r, g, b));
                    }
                }
                sf::Texture tex;
                tex.loadFromImage(img);
                addTexture(name, tex);
                return;
            }
            // Hero portraits (120x150)
            unsigned int w = 120, h = 150;
            sf::Color heroColor(100, 100, 100);
            if (strstr(name, "captain")) heroColor = sf::Color(0, 71, 171);
            else if (strstr(name, "iron")) heroColor = sf::Color(180, 30, 30);
            else if (strstr(name, "thor")) heroColor = sf::Color(30, 100, 180);
            else if (strstr(name, "widow")) heroColor = sf::Color(40, 40, 40);
            else if (strstr(name, "hawkeye")) heroColor = sf::Color(100, 50, 150);
            else if (strstr(name, "strange")) heroColor = sf::Color(180, 100, 30);
            else if (strstr(name, "avengers")) { w = 200; h = 100; heroColor = sf::Color(180, 50, 50); }
            
            sf::Image img;
            img.create(w, h, heroColor);
            // Add border
            for (unsigned int i = 0; i < w; i++) {
                img.setPixel(i, 0, sf::Color(200, 180, 50));
                img.setPixel(i, h-1, sf::Color(200, 180, 50));
            }
            for (unsigned int i = 0; i < h; i++) {
                img.setPixel(0, i, sf::Color(200, 180, 50));
                img.setPixel(w-1, i, sf::Color(200, 180, 50));
            }
            sf::Texture tex;
            tex.loadFromImage(img);
            addTexture(name, tex);
            return;
        }
        
        sf::Image img;
        img.create(size, size, color);
        for (unsigned int i = 0; i < size; i++) {
            img.setPixel(i, 0, sf::Color::Black);
            img.setPixel(i, size-1, sf::Color::Black);
            img.setPixel(0, i, sf::Color::Black);
            img.setPixel(size-1, i, sf::Color::Black);
        }
        sf::Texture tex;
        tex.loadFromImage(img);
        addTexture(name, tex);
    }
    
    void loadAllAssets() {
        // Load splash screen assets first
        loadTexture(Constants::SPLASH_BACKGROUND, "splash_background");
        loadTexture(Constants::SPLASH_AVENGERS, "splash_avengers");
        loadTexture(Constants::SPLASH_CAPTAIN, "splash_captain");
        loadTexture(Constants::SPLASH_IRONMAN, "splash_ironman");
        loadTexture(Constants::SPLASH_THOR, "splash_thor");
        loadTexture(Constants::SPLASH_BLACKWIDOW, "splash_blackwidow");
        loadTexture(Constants::SPLASH_HAWKEYE, "splash_hawkeye");
        loadTexture(Constants::SPLASH_DRSTRANGE, "splash_drstrange");
        
        // Try to load real textures, fall back to placeholders
        loadTexture("assets/map_background.png", "map_background");
        loadTexture("assets/tower.png", "tower");
        
        const char* enemyTextures[] = {"enemy_ground", "enemy_flying", "enemy_boss"};
        for (auto& tex : enemyTextures) {
            char filename[64];
            sprintf(filename, "assets/%s.png", tex);
            loadTexture(filename, tex);
        }
        
        // Load boss-specific textures
        const char* bossTextures[] = {"boss_ultron", "boss_thanos", "boss_doom"};
        for (auto& tex : bossTextures) {
            char filename[64];
            sprintf(filename, "assets/%s.png", tex);
            loadTexture(filename, tex);
        }
        
        // Load hero structure textures (Black Widow, Captain America, Hawkeye, Iron Man, Dr Strange, Thor)
        const char* structTypes[] = {"blackwidow", "captain", "hawkeye", "ironman", "drstrange", "thor","hulk"};
        for (auto& stype : structTypes) {
            for (int lv = 1; lv <= 3; lv++) {
                char filename[64], name[64];
                sprintf(filename, "assets/struct_%s_lv%d.png", stype, lv);
                sprintf(name, "struct_%s_lv%d", stype, lv);
                loadTexture(filename, name);
            }
        }
        
        // Load projectile textures (bullet, shield, arrow, missile, beam, lightning)
        const char* projTextures[] = {"proj_bullet", "proj_shield", "proj_arrow", "proj_missile", "proj_beam", "proj_lightning"};
        for (auto& tex : projTextures) {
            char filename[64];
            sprintf(filename, "assets/%s.png", tex);
            loadTexture(filename, tex);
        }
    }
};