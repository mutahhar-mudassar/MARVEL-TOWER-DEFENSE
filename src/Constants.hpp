 #pragma once

namespace Constants {
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;
    const int GAME_AREA_WIDTH = 1000;
    const int UI_PANEL_WIDTH = 280;
    
    // Grid configuration - green tiles for structure placement
    // Grid cells are larger for better visibility and placement
    const int GRID_COLS = 25;  // 33 * 30 = 990 (fits in 1000 width)
    const int GRID_ROWS = 18;  // 24 * 30 = 720 (fits in 720 height)
    const float CELL_SIZE = 40.0f;  // Larger grid cells (30x30 pixels) for better visibility
    
    // Tower position - at extreme right, lower side of screen
    // Positioned ~250 pixels lower than center (360 + 250 = 610, but using 640 for lower side)
    const float TOWER_X = 1000.0f;  // Extreme right, near end of image (1000px width)
    const float TOWER_Y = 450.0f;  // Lower side, ~250 pixels below center (360 + 250 = 610, using 640)
    
    const int EASY_START_COINS = 300;
    const int EASY_TOWER_HP = 200;
    const int EASY_WAVES = 10;
    
    const int EPIC_START_COINS = 250;
    const int EPIC_TOWER_HP = 150;
    const int EPIC_WAVES = 15;
    
    const int GROUND_KILL_REWARD = 5;
    const int FLYING_KILL_REWARD = 8;
    const int BOSS_KILL_REWARD = 100;
    const int WAVE_COMPLETE_BONUS = 20;
    
    // Hero structure costs
    const int BLACKWIDOW_COST = 50;      // Was SHOOTER_COST
    const int CAPTAIN_COST = 65;          // New: Captain America
    const int HAWKEYE_COST = 75;
    const int IRONMAN_COST = 100;
    const int DRSTRANGE_COST = 200;
    const int THOR_COST = 250;
   //NEW STRUCTURE COST
    
    const float SELL_REFUND_PERCENT = 0.5f;
    const int MAX_PATH_LENGTH = 50;
    const int SPAWN_COUNT = 3;
    const int MAX_PROJECTILES = 500;

    // Projectile types for each hero
    enum ProjectileType { 
        PROJ_BULLET,     // Black Widow
        PROJ_SHIELD,     // Captain America
        PROJ_ARROW,      // Hawkeye
        PROJ_MISSILE,    // Iron Man
        PROJ_BEAM,       // Dr. Strange (Time Beam)
        PROJ_LIGHTNING   // Thor
    };

    // Projectile speeds (pixels per second)
    // Projectile speeds (pixels per second) - Balanced for competitive gameplay
    const float BULLET_SPEED = 750.0f;      // Increased from 600 - faster for quick shots
    const float SHIELD_SPEED = 550.0f;      // Increased from 450 - balanced for shield
    const float ARROW_SPEED = 650.0f;
    const float MISSILE_SPEED = 500.0f;
    const float BEAM_SPEED = 900.0f;
    const float LIGHTNING_SPEED = 1100.0f;
    
    // Font path
    const char* const MARVEL_FONT_PATH = "assets/marvel_font.ttf";
    
    // Sound paths (using assets/sound/ folder - singular)
    const char* const SOUND_BACKGROUND = "assets/sound/background.ogg";
    const char* const SOUND_HIT = "assets/sound/hit.ogg";
    const char* const SOUND_BOSS_DEFEAT = "assets/sound/boss_defeat.ogg";
    const char* const SOUND_WAVE_START = "assets/sound/wave_start.ogg";
    const char* const SOUND_VICTORY = "assets/sound/victory.ogg";
    const char* const SOUND_DEFEAT = "assets/sound/defeat.ogg";
    
    // Splash screen images
    const char* const SPLASH_AVENGERS = "assets/splash/avengers_logo.png";
    const char* const SPLASH_CAPTAIN = "assets/splash/captain_america.png";
    const char* const SPLASH_IRONMAN = "assets/splash/iron_man.png";
    const char* const SPLASH_THOR = "assets/splash/thor.png";
    const char* const SPLASH_BLACKWIDOW = "assets/splash/black_widow.png";
    const char* const SPLASH_HAWKEYE = "assets/splash/hawkeye.png";
    const char* const SPLASH_DRSTRANGE = "assets/splash/dr_strange.png";
    const char* const SPLASH_BACKGROUND = "assets/splash/splash_bg.png";
}