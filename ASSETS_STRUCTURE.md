# Assets File Structure Guide

This document shows the complete file structure for all assets needed in Marvel Tower Defense.

## Complete Directory Structure

```
MarvelTowerDefense/
├── assets/
│   ├── marvel_font.ttf                    # Marvel-style font (optional)
│   │
│   ├── tower.png                          # Tower sprite (60x60 recommended, scales automatically)
│   │
│   ├── enemy_ground.png                   # Ground enemy sprite (scales to 96x96 in game)
│   ├── enemy_flying.png                   # Flying enemy sprite (scales to 96x96 in game)
│   ├── enemy_boss.png                     # Generic boss sprite (fallback, scales to 150x150)
│   │
│   ├── boss_ultron.png                    # Ultron boss (scales to 150x150 in game)
│   ├── boss_thanos.png                    # Thanos boss (scales to 150x150 in game)
│   ├── boss_doom.png                      # Doctor Doom boss (scales to 150x150 in game)
│   │
│   ├── struct_blackwidow_lv1.png          # Black Widow level 1 (scales to 32x32)
│   ├── struct_blackwidow_lv2.png          # Black Widow level 2 (scales to 32x32)
│   ├── struct_blackwidow_lv3.png          # Black Widow level 3 (scales to 32x32)
│   │
│   ├── struct_captain_lv1.png             # Captain America level 1 (scales to 32x32)
│   ├── struct_captain_lv2.png             # Captain America level 2 (scales to 32x32)
│   ├── struct_captain_lv3.png             # Captain America level 3 (scales to 32x32)
│   │
│   ├── struct_hawkeye_lv1.png             # Hawkeye level 1 (scales to 32x32)
│   ├── struct_hawkeye_lv2.png             # Hawkeye level 2 (scales to 32x32)
│   ├── struct_hawkeye_lv3.png             # Hawkeye level 3 (scales to 32x32)
│   │
│   ├── struct_ironman_lv1.png             # Iron Man level 1 (scales to 32x32)
│   ├── struct_ironman_lv2.png             # Iron Man level 2 (scales to 32x32)
│   ├── struct_ironman_lv3.png             # Iron Man level 3 (scales to 32x32)
│   │
│   ├── struct_drstrange_lv1.png           # Dr. Strange level 1 (scales to 32x32)
│   ├── struct_drstrange_lv2.png           # Dr. Strange level 2 (scales to 32x32)
│   ├── struct_drstrange_lv3.png           # Dr. Strange level 3 (scales to 32x32)
│   │
│   ├── struct_thor_lv1.png                # Thor level 1 (scales to 32x32)
│   ├── struct_thor_lv2.png                # Thor level 2 (scales to 32x32)
│   ├── struct_thor_lv3.png                # Thor level 3 (scales to 32x32)
│   │
│   ├── proj_bullet.png                    # Black Widow's bullet (scales to 20x20)
│   ├── proj_shield.png                    # Captain America's shield (scales to 25x25)
│   ├── proj_arrow.png                     # Hawkeye's arrow (scales to 20x20)
│   ├── proj_missile.png                   # Iron Man's missile (scales to 20x20)
│   ├── proj_beam.png                      # Dr. Strange's time beam (scales to 20x20)
│   ├── proj_lightning.png                 # Thor's lightning (scales to 20x20)
│   │
│   ├── sounds/
│   │   ├── background.ogg                 # Background music (looping, OGG or WAV)
│   │   ├── hit.ogg                        # Sound when projectile hits enemy
│   │   ├── boss_defeat.ogg                # Sound when boss is defeated
│   │   ├── wave_start.ogg                 # Sound when wave starts
│   │   ├── victory.ogg                    # Victory music
│   │   └── defeat.ogg                     # Defeat sound
│   │
│   └── splash/
│       ├── splash_bg.png                  # Splash screen background (1280x720)
│       ├── avengers_logo.png              # Avengers logo (200x100 recommended)
│       ├── captain_america.png            # Captain America portrait (120x150)
│       ├── iron_man.png                   # Iron Man portrait (120x150)
│       ├── thor.png                       # Thor portrait (120x150)
│       ├── black_widow.png                # Black Widow portrait (120x150)
│       ├── hawkeye.png                    # Hawkeye portrait (120x150)
│       └── dr_strange.png                 # Dr. Strange portrait (120x150)
```

## File Details

### Fonts
- **marvel_font.ttf** - Optional Marvel-style font. If not found, default system font is used.
- **Location**: `assets/marvel_font.ttf`

### Game Assets (Root of assets/)

#### Tower
- **tower.png** - The tower to defend
  - **Display Size**: 60x60 pixels (scales automatically)
  - **Location**: `assets/tower.png`

#### Enemies
- **enemy_ground.png** - Ground enemy sprite
  - **Display Size**: 96x96 pixels (3x larger)
  - **Location**: `assets/enemy_ground.png`
  
- **enemy_flying.png** - Flying enemy sprite
  - **Display Size**: 96x96 pixels (3x larger)
  - **Location**: `assets/enemy_flying.png`

- **enemy_boss.png** - Generic boss sprite (fallback)
  - **Display Size**: 150x150 pixels (3x larger)
  - **Location**: `assets/enemy_boss.png`

#### Bosses
- **boss_ultron.png** - Ultron boss (Wave 5 in Easy mode, Wave 5 in Epic mode)
  - **Display Size**: 150x150 pixels (3x larger)
  - **Location**: `assets/boss_ultron.png`
  
- **boss_thanos.png** - Thanos boss (Wave 10 in Easy mode, Wave 10 in Epic mode)
  - **Display Size**: 150x150 pixels (3x larger)
  - **Location**: `assets/boss_thanos.png`
  
- **boss_doom.png** - Doctor Doom boss (Wave 15 in Epic mode only)
  - **Display Size**: 150x150 pixels (3x larger)
  - **Location**: `assets/boss_doom.png`

#### Hero Structures (6 heroes × 3 levels = 18 files)
All hero structures scale to 32x32 pixels in game.

**Black Widow:**
- `assets/struct_blackwidow_lv1.png`
- `assets/struct_blackwidow_lv2.png`
- `assets/struct_blackwidow_lv3.png`

**Captain America:**
- `assets/struct_captain_lv1.png`
- `assets/struct_captain_lv2.png`
- `assets/struct_captain_lv3.png`

**Hawkeye:**
- `assets/struct_hawkeye_lv1.png`
- `assets/struct_hawkeye_lv2.png`
- `assets/struct_hawkeye_lv3.png`

**Iron Man:**
- `assets/struct_ironman_lv1.png`
- `assets/struct_ironman_lv2.png`
- `assets/struct_ironman_lv3.png`

**Dr. Strange:**
- `assets/struct_drstrange_lv1.png`
- `assets/struct_drstrange_lv2.png`
- `assets/struct_drstrange_lv3.png`

**Thor:**
- `assets/struct_thor_lv1.png`
- `assets/struct_thor_lv2.png`
- `assets/struct_thor_lv3.png`

#### Projectiles (6 files)
- **proj_bullet.png** - Black Widow's bullets (20x20 pixels)
- **proj_shield.png** - Captain America's shield (25x25 pixels)
- **proj_arrow.png** - Hawkeye's arrows (20x20 pixels)
- **proj_missile.png** - Iron Man's missiles (20x20 pixels)
- **proj_beam.png** - Dr. Strange's time beam (20x20 pixels)
- **proj_lightning.png** - Thor's lightning (20x20 pixels)

**Location**: All in `assets/` folder

### Sounds (assets/sounds/)
All sound files should be in **OGG format** (recommended) or **WAV format**:

- **background.ogg** - Looping background music
- **hit.ogg** - Short sound effect for projectile hits
- **boss_defeat.ogg** - Boss defeat sound
- **wave_start.ogg** - Wave start sound
- **victory.ogg** - Victory music
- **defeat.ogg** - Defeat sound

**Location**: `assets/sounds/` folder

### Splash Screen (assets/splash/)
- **splash_bg.png** - Background for splash screen (1280x720 recommended)
- **avengers_logo.png** - Main Avengers logo (200x100 recommended)
- **captain_america.png** - Hero portrait (120x150 recommended)
- **iron_man.png** - Hero portrait (120x150 recommended)
- **thor.png** - Hero portrait (120x150 recommended)
- **black_widow.png** - Hero portrait (120x150 recommended)
- **hawkeye.png** - Hero portrait (120x150 recommended)
- **dr_strange.png** - Hero portrait (120x150 recommended)

**Location**: `assets/splash/` folder

## Recommended Image Sizes

| Asset Type | Original Size | Display Size in Game | Notes |
|------------|---------------|---------------------|-------|
| Tower | Any | 60x60 | Scales automatically |
| Enemy (Ground/Flying) | Any | 96x96 | 3x larger, scales automatically |
| Boss | Any | 150x150 | 3x larger, scales automatically |
| Hero Structures | Any | 32x32 | Scales automatically, 3 levels each |
| Projectiles | Any | 20x20 (shield: 25x25) | Scales automatically |
| Splash Background | 1280x720 | 1280x720 | Full window size |
| Avengers Logo | 200x100 | Variable | Scales as needed |
| Hero Portraits | 120x150 | Variable | Scales as needed |

## File Naming Rules

### Important:
1. **Use lowercase with underscores** (e.g., `boss_ultron.png`, not `BossUltron.png`)
2. **File extensions**: Use `.png` for images (or `.PNG` - both work)
3. **Sound extensions**: Use `.ogg` or `.wav` (OGG recommended)
4. **Exact names required**: The code looks for these exact filenames

### Boss Files:
- Must be named exactly: `boss_ultron.png`, `boss_thanos.png`, `boss_doom.png`
- Case-sensitive on some systems, so use lowercase

### Structure Files:
- Format: `struct_[heroname]_lv[1-3].png`
- Examples: `struct_blackwidow_lv1.png`, `struct_captain_lv2.png`

### Projectile Files:
- Format: `proj_[type].png`
- Examples: `proj_bullet.png`, `proj_shield.png`

## Notes

1. **Placeholder System**: If any image is missing, the game will create a colored placeholder automatically. The game will still work!

2. **File Formats**:
   - **Images**: PNG (recommended) or any format SFML supports (JPG, BMP, etc.)
   - **Sounds**: OGG (recommended) or WAV
   - **Fonts**: TTF (TrueType Font)

3. **Transparency**: PNG files with transparency (alpha channel) are fully supported.

4. **Case Sensitivity**: 
   - Windows: Case-insensitive (both `.png` and `.PNG` work)
   - Linux/Mac: Case-sensitive (use lowercase `.png`)
   - The code tries both cases automatically

5. **Missing Files**: The game will work even if files are missing - placeholders will be used instead. Check console output to see which files loaded successfully.

6. **Scaling**: All images are automatically scaled to the correct display size. You can use any size image, but recommended sizes work best.

## Quick Setup Checklist

### Required Folders:
- [ ] Create `assets/sounds/` folder
- [ ] Create `assets/splash/` folder

### Required Files:
- [ ] Font: `assets/marvel_font.ttf` (optional)
- [ ] Tower: `assets/tower.png`
- [ ] Enemies: `assets/enemy_ground.png`, `assets/enemy_flying.png`
- [ ] Bosses: `assets/boss_ultron.png`, `assets/boss_thanos.png`, `assets/boss_doom.png`
- [ ] Hero structures: 18 files (6 heroes × 3 levels)
- [ ] Projectiles: 6 files (`proj_bullet.png`, `proj_shield.png`, etc.)
- [ ] Splash screen: 8 files in `assets/splash/`
- [ ] Sounds: 6 files in `assets/sounds/`

## Example: Creating the Folder Structure

### On Windows (PowerShell):
```powershell
cd C:\Users\dell\Documents\MarvelTowerDefense\assets
mkdir sounds
mkdir splash
```

### On Linux/Mac:
```bash
cd assets
mkdir -p sounds splash
```

## Asset Loading Order

The game loads assets in this order:
1. Splash screen images
2. Map and tower (map no longer used, but code still tries to load)
3. Enemy textures (ground, flying, generic boss)
4. Boss-specific textures (Ultron, Thanos, Doctor Doom)
5. Hero structure textures (all 6 heroes, 3 levels each)
6. Projectile textures (all 6 types)

Sounds are loaded separately when the game starts.

## Troubleshooting

### Images not showing?
1. Check console output - it shows which files loaded successfully
2. Verify file names match exactly (case-sensitive on some systems)
3. Check file is in the correct folder (`assets/` or `assets/splash/`)
4. Verify file extension is `.png` (or `.PNG`)

### Sounds not playing?
1. Check console output for sound loading errors
2. Verify files are in `assets/sounds/` folder
3. Check file format (OGG or WAV)
4. Verify file names match exactly

### Placeholders showing?
- This means the image file wasn't found
- Check the console output to see which paths were tried
- Verify the file exists and is named correctly
- The game will still work with placeholders!
