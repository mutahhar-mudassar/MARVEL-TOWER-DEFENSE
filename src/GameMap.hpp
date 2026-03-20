#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>  // For sqrt() function used in path marking
#include <vector>  // For road segments
#include <algorithm>  // For std::max
#include "Constants.hpp"
#include "TextureManager.hpp"
#include "GameObjects.hpp"

/**
 * GameMap - Manages the game map, paths, grid, and tower
 * 
 * This class handles all map-related functionality including:
 * - Enemy path definitions (3 different spawn points and routes)
 * - Grid system for structure placement (green tiles show valid placement areas)
 * - Tower rendering and positioning
 * - Path marking (ensures green tiles don't appear on enemy paths)
 * - Placement validation (checks if structures can be placed at a location)
 * 
 * Key Features:
 *   - Path System: Defines 3 enemy paths with waypoints leading to the tower
 *   - Grid System: 25x18 grid of 40x40 pixel cells for structure placement
 *   - Green Tiles: Visual indicators showing where structures can be placed
 *   - Occupied Areas: Marks paths, tower area, and X-Mansion as unbuildable
 *   - Placement Preview: Shows green/red preview when hovering to place structure
 *   - Path Marking: Automatically marks all path areas to prevent green tiles on roads
 * 
 * Path Design:
 *   - Top Path: Spawns from top-left, follows route with 4 sharp 90° turns
 *   - Middle Path: Spawns from middle-left, goes straight then turns
 *   - Bottom Path: Spawns from bottom-left, follows different route
 *   - All paths end 60 pixels before the tower (enemies stop before reaching it)
 * 
 * Grid System:
 *   - Grid cells: 40x40 pixels each (25 columns × 18 rows = 1000×720 area)
 *   - Green tiles: Show valid placement areas (not on paths or occupied areas)
 *   - Occupied areas: Tower, X-Mansion (6x6 cells), and all enemy paths
 *   - Placement validation: Checks if location is valid before allowing placement
 * 
 * Members:
 *   - towerSprite: SFML sprite for the tower (defense target)
 *   - paths: Array of 3 paths, each containing waypoints
 *   - pathLengths: Number of waypoints in each path
 *   - occupied: 2D array marking which grid cells are occupied (can't place structures)
 *   - gridCells: 2D array of green tile rectangles for visual grid
 *   - showGrid: Whether to display green placement tiles
 *   - placementPreview: Preview rectangle shown when hovering to place structure
 *   - showPreview: Whether to show placement preview
 *   - previewValid: Whether current preview location is valid for placement
 *   - greenBackground: Solid green background rectangle (no map image used)
 * 
 * Tower:
 *   - Position: (975, 560) - extreme right, lower side of screen
 *   - Size: 470x470 pixels (3x larger for visibility)
 *   - Purpose: Defense target - enemies try to reach it, player must defend it
 */
class GameMap : public sf::Drawable {
private:
    sf::Sprite towerSprite;
    PathPoint paths[3][Constants::MAX_PATH_LENGTH];
    int pathLengths[3];
    bool occupied[Constants::GRID_ROWS][Constants::GRID_COLS];
    sf::RectangleShape gridCells[Constants::GRID_ROWS][Constants::GRID_COLS];
    bool showGrid;
    sf::RectangleShape placementPreview;
    bool showPreview, previewValid;
    sf::RectangleShape greenBackground;  // Green background (no map image used)
    
    // Road system - visual roads along enemy paths
    std::vector<sf::RectangleShape> roadSegments;  // Road rectangles drawn along paths
    std::vector<sf::CircleShape> roadJoints;       // Circles to smooth road corners
    static constexpr float ROAD_WIDTH = 90.0f;     // Slightly wider road
    
    // Park and rooftop details
    std::vector<sf::RectangleShape> parkRects;
    std::vector<sf::CircleShape> parkTrees;
    std::vector<sf::RectangleShape> roofDetails;
    std::vector<sf::RectangleShape> parkBenches;
    std::vector<sf::RectangleShape> parkLights;
    std::vector<sf::CircleShape> roadManholes;    // Small manholes on roads

public:
    // ============================================================================
    // CONSTRUCTOR - Initialize green grid tiles and map elements
    // ============================================================================
    GameMap() : showGrid(true), showPreview(false), previewValid(false) {
        // Initialize all grid cells as green tiles (40x40 pixels each)
        // Green tiles show where structures can be placed
        // Tiles on roads and occupied areas will be marked later and won't show
        // Green tiles will fill a decent amount of the map, avoiding enemy paths
        roofDetails.clear();
        for (int y = 0; y < Constants::GRID_ROWS; y++) {
            for (int x = 0; x < Constants::GRID_COLS; x++) {
                occupied[y][x] = false;  // Start as unoccupied (will be marked later)
                
        // Create rooftop tile for structure placement
        gridCells[y][x].setSize(sf::Vector2f(Constants::CELL_SIZE - 4, Constants::CELL_SIZE - 4));
        gridCells[y][x].setPosition(x * Constants::CELL_SIZE + 2, y * Constants::CELL_SIZE + 2);
        
        // Avengers-style rooftop palette
        gridCells[y][x].setFillColor(sf::Color(82, 92, 112, 150));   // Slate blue with slight alpha
        gridCells[y][x].setOutlineColor(sf::Color(40, 45, 60, 200)); // Dark border
        gridCells[y][x].setOutlineThickness(2);

        // Rooftop details: light, sparse accents (skip near left park columns)
        if (x > 4) {
            if ((x + y) % 9 == 0) {
                sf::RectangleShape chimney;
                chimney.setSize(sf::Vector2f(8.0f, 10.0f));
                chimney.setOrigin(4.0f, 5.0f);
                chimney.setPosition(x * Constants::CELL_SIZE + Constants::CELL_SIZE * 0.7f,
                                    y * Constants::CELL_SIZE + Constants::CELL_SIZE * 0.3f);
                chimney.setFillColor(sf::Color(70, 70, 70, 200));
                roofDetails.push_back(chimney);
        } else if ((x + y) % 13 == 0) {
            // Vent / AC unit
            sf::RectangleShape vent;
            vent.setSize(sf::Vector2f(12.0f, 8.0f));
            vent.setOrigin(6.0f, 4.0f);
            vent.setPosition(x * Constants::CELL_SIZE + Constants::CELL_SIZE * 0.4f,
                             y * Constants::CELL_SIZE + Constants::CELL_SIZE * 0.55f);
            vent.setFillColor(sf::Color(130, 130, 135, 190));
            vent.setOutlineColor(sf::Color(80, 80, 85, 200));
            vent.setOutlineThickness(1.0f);
            roofDetails.push_back(vent);
        } else if ((x * y) % 17 == 0) {
            // Small AC box
            sf::RectangleShape ac;
            ac.setSize(sf::Vector2f(10.0f, 10.0f));
            ac.setOrigin(5.0f, 5.0f);
            ac.setPosition(x * Constants::CELL_SIZE + Constants::CELL_SIZE * 0.6f,
                           y * Constants::CELL_SIZE + Constants::CELL_SIZE * 0.45f);
            ac.setFillColor(sf::Color(145, 145, 150, 190));
            ac.setOutlineColor(sf::Color(90, 90, 95, 200));
            ac.setOutlineThickness(1.0f);
            roofDetails.push_back(ac);
            }
        }
            }
        }
        
        // Placement preview (green/red box when hovering to place structure)
        placementPreview.setSize(sf::Vector2f(Constants::CELL_SIZE - 4, Constants::CELL_SIZE - 4));
        placementPreview.setOutlineThickness(2);
        
        // Initialize path lengths
        for (int i = 0; i < 3; i++) pathLengths[i] = 0;

        // Setup green background rectangle (base map color)
        // Deep background to fit Avengers palette
        greenBackground.setSize(sf::Vector2f(Constants::GAME_AREA_WIDTH, Constants::WINDOW_HEIGHT));
        greenBackground.setFillColor(sf::Color(12, 18, 32));  // Navy midnight
        greenBackground.setPosition(0, 0);

        // Background bands removed to avoid blue strips
    }
    
    void initialize() {
        // Load tower texture (no map background - using green background instead)
        const sf::Texture& towerTex = TextureManager::getInstance().getTexture("tower");
        towerSprite.setTexture(towerTex);
        
        // Tower texture loading and scaling
        // Tower is set to 60x60 pixels display size as requested
        sf::Vector2u towerSize = towerTex.getSize();
        
        if (towerSize.x > 0 && towerSize.y > 0) {
            std::cout << "Tower loaded: " << towerSize.x << "x" << towerSize.y << std::endl;
            // Scale tower to 3x size (210x210 pixels - 3x original 70px)
            float towerDisplaySize = 470.0f;  // 3x larger (210x210 pixels)
            float towerScale = towerDisplaySize / std::max(towerSize.x, towerSize.y);
            towerSprite.setScale(towerScale, towerScale);
            std::cout << "  Scaled to: " << towerScale << " (display size: ~" << towerDisplaySize << "x" << towerDisplaySize << ")" << std::endl;
        } else {
            std::cout << "Tower using placeholder" << std::endl;
        }
        
        // Set tower origin (center) and position
        // Tower position is set in Constants.hpp (TOWER_X, TOWER_Y)
        // Currently: extreme right (975), lower side (640)
        sf::FloatRect bounds = towerSprite.getLocalBounds();
        towerSprite.setOrigin(bounds.width / 2, bounds.height / 2);
        towerSprite.setPosition(Constants::TOWER_X, Constants::TOWER_Y);
        
        for (int y = 0; y < Constants::GRID_ROWS; y++)
            for (int x = 0; x < Constants::GRID_COLS; x++)
                occupied[y][x] = false;
        
        // ============================================================================
        // ENEMY PATHS - Three spawn points with completely different routes
        // All paths only unite at the very end before reaching the tower
        // Tower is at (975, 640) - extreme right, lower side
        // ============================================================================
        
        // PATH 1: TOP-LEFT SPAWN POINT
        // Spawns from top-left corner, follows a completely different route
        // This path has 4 sharp 90-degree turns like walking on roads
        // Edit waypoints below to change the path route
        float topPath[][2] = {
            {-50, 75},       // SPAWN: Enter from top-left (off-screen)
            {50, 75},        // Move right
            {50, 75},       // Continue right
            {150, 75},       // Continue right
            {200, 75},      // 90° turn DOWN (first sharp turn)
            {250, 75},      // Continue down
            {250, 75},      // Continue down
            {400, 75},      // 90° turn RIGHT (second sharp turn)
            {450, 75},      // Continue right
            {550, 75},      // Continue right
            {650, 75},      // Continue right
            {650, 250},      // 90° turn DOWN (third sharp turn)
            {800, 250},      // Continue down
            {930, 250},      // 90° turn RIGHT (fourth sharp turn)
            {930, 520},      // Continue right
            {Constants::TOWER_X - 60, Constants::TOWER_Y}  // End 60 pixels before tower - END
        };
        pathLengths[0] = 16;  // Reduced by 1 (removed duplicate waypoint)
        for (int i = 0; i < 16; i++) paths[0][i] = PathPoint(topPath[i][0], topPath[i][1]);
        
        // PATH 2: MIDDLE SPAWN POINT
        // Spawns from middle-left, goes straight for 200 pixels, then turns
        // This path has 4 sharp 90-degree turns after the initial straight section
        // Edit waypoints below to change the path route
        float midPath[][2] = {
            {-50, 480},      // SPAWN: Enter from left-middle (off-screen)
            {0, 480},        // Move right (straight path)
            {50, 480},       // Continue straight
            {100, 480},      // Continue straight
            {150, 480},      // Continue straight
            {200, 480},      // After 200px straight, now turn (as requested)
            {200, 340},      // 90° turn DOWN (first sharp turn after 200px)
            {250, 340},      // Continue down
            {350, 340},      // 90° turn RIGHT (second sharp turn)
            {450, 340},      // Continue right
            {550, 340},      // Continue right
            {650, 340},      // 90° turn DOWN (third sharp turn)
            {650, 540},      // Continue down
            {700, 540},      // 90° turn RIGHT (fourth sharp turn)
            {850, 540},      // Continue right
            {Constants::TOWER_X - 60, Constants::TOWER_Y}  // End 60 pixels before tower - END
        };
        pathLengths[1] = 16;  // Reduced by 1 (removed duplicate waypoint)
        for (int i = 0; i < 16; i++) paths[1][i] = PathPoint(midPath[i][0], midPath[i][1]);
        
        // PATH 3: BOTTOM SPAWN POINT
        // Spawns from bottom-left, follows a completely different route
        // This path has 4 sharp 90-degree turns like walking on roads
        // Edit waypoints below to change the path route
        float botPath[][2] = {
            {-50, 640},      // SPAWN: Enter from left-bottom (off-screen)
            {100, 640},      // Move right
            {200, 640},      // Continue right
            {300, 640},      // Continue right
            {400, 640},      // Continue right
            {400, 540},      // 90° turn UP (first sharp turn)
            {400, 550},      // Continue up
            {400, 540},      // Continue up
            {550, 540},      // 90° turn RIGHT (second sharp turn)
            {650, 540},      // Continue right
            {750, 540},      // Continue right
            {750, 540},      // 90° turn DOWN (third sharp turn)
            {750, 540},      // Continue down
            {850, 540},      // 90° turn RIGHT (fourth sharp turn)
            {Constants::TOWER_X - 60, Constants::TOWER_Y}  // End 60 pixels before tower - END
        };
        pathLengths[2] = 15;  // Reduced by 1 (removed duplicate waypoint)
        for (int i = 0; i < 15; i++) paths[2][i] = PathPoint(botPath[i][0], botPath[i][1]);


        
        // ============================================================================
        // MARK OCCUPIED AREAS - Areas where structures CANNOT be placed
        // Green tiles will NOT appear in these areas
        // ============================================================================
        
        // Mark X-Mansion and park as occupied (top-left area)
        // Edit the loop bounds below to change the size of this occupied area
        for (int y = 0; y < 6; y++) {  // Rows 0-5 (change 6 to adjust height)
            for (int x = 0; x < 6; x++) {  // Cols 0-5 (change 6 to adjust width)
                occupied[y][x] = true;  // Mark as occupied (no green tiles here)
            }
        }
        
        // Mark all path areas as occupied (roads - no green tiles on roads)
        // This ensures green tiles only appear in clusters near paths, not on them
        markPathsOccupied();
        
        // Mark tower area as occupied (no structures can be placed on tower)
        markTowerOccupied();

        // Add park decor
        createParkDecor();
        
        // Generate visual roads along enemy paths
        generateRoads();
    }
    
    // ============================================================================
    // MARK PATHS AS OCCUPIED - Prevents green tiles from appearing on roads
    // This function marks ALL grid cells along enemy paths as occupied
    // It marks waypoints AND cells between waypoints to ensure continuous coverage
    // Green tiles will NOT show on roads, ensuring they only fill areas between paths
    // ============================================================================
    void markPathsOccupied() {
        // Loop through all 3 enemy paths
        for (int lane = 0; lane < 3; lane++) {
            // First, mark each waypoint and surrounding cells as occupied
            for (int i = 0; i < pathLengths[lane]; i++) {
                // Convert world coordinates to grid coordinates
                int gx = (int)(paths[lane][i].x / Constants::CELL_SIZE);
                int gy = (int)(paths[lane][i].y / Constants::CELL_SIZE);
                
                // Mark the waypoint cell and surrounding cells (3x3 area) as occupied
                // Matches the road width (~2 grid cells) while leaving more build space
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int x = gx + dx, y = gy + dy;
                        if (isValidGrid(x, y)) {
                            occupied[y][x] = true;  // Mark as occupied (road - no green tiles here)
                        }
                    }
                }
            }
            
            // Second, mark cells BETWEEN waypoints to ensure continuous path coverage
            // This fills gaps between waypoints so green tiles don't appear on the path
            for (int i = 0; i < pathLengths[lane] - 1; i++) {
                float x1 = paths[lane][i].x;
                float y1 = paths[lane][i].y;
                float x2 = paths[lane][i + 1].x;
                float y2 = paths[lane][i + 1].y;
                
                // Calculate distance between waypoints
                float dx = x2 - x1;
                float dy = y2 - y1;
                float distance = sqrt(dx * dx + dy * dy);
                
                // Mark cells along the line between waypoints
                // Use small steps to ensure all cells along the path are marked
                int steps = (int)(distance / (Constants::CELL_SIZE / 2)) + 1;
                for (int s = 0; s <= steps; s++) {
                    float t = (float)s / steps;
                    float x = x1 + dx * t;
                    float y = y1 + dy * t;
                    
                    // Convert to grid coordinates
                    int gx = (int)(x / Constants::CELL_SIZE);
                    int gy = (int)(y / Constants::CELL_SIZE);
                    
                    // Mark a 3x3 area around each point on the path (road width)
                    for (int dy2 = -1; dy2 <= 1; dy2++) {
                        for (int dx2 = -1; dx2 <= 1; dx2++) {
                            int x2_grid = gx + dx2, y2_grid = gy + dy2;
                            if (isValidGrid(x2_grid, y2_grid)) {
                                occupied[y2_grid][x2_grid] = true;  // Mark as occupied (road + sidewalk)
                            }
                        }
                    }
                }
            }
        }
        // After marking paths, all remaining unoccupied cells will show green tiles
        // This ensures green tiles fill a decent amount of the map, avoiding only the paths
    }
    
    // ============================================================================
    // MARK TOWER AREA AS OCCUPIED - Prevents structures from being placed on tower
    // ============================================================================
    void markTowerOccupied() {
        // Convert tower world position to grid coordinates
        int tx = (int)(Constants::TOWER_X / Constants::CELL_SIZE);
        int ty = (int)(Constants::TOWER_Y / Constants::CELL_SIZE);
        
        // Mark tower cell and surrounding cells (3x3 area) as occupied
        // Edit the loop bounds to change the size of the occupied tower area
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++) {
                int x = tx + dx, y = ty + dy;
                if (isValidGrid(x, y)) occupied[y][x] = true;  // Mark as occupied (tower area)
            }
    }

    // ============================================================================
    // ============================================================================
    // CREATE PARK DECOR - fills empty left side with a park
    // ============================================================================
    void createParkDecor() {
        parkRects.clear();
        parkTrees.clear();
        parkBenches.clear();
        parkLights.clear();

        // Park background strip on the left side (narrower to reduce intrusion)
        sf::RectangleShape park;
        park.setSize(sf::Vector2f(160.0f, Constants::WINDOW_HEIGHT));
        park.setPosition(0, 0);
        park.setFillColor(sf::Color(32, 80, 60, 190));
        park.setOutlineColor(sf::Color(20, 60, 40, 220));
        park.setOutlineThickness(3);
        parkRects.push_back(park);

        // Add a path inside the park
        sf::RectangleShape path;
        path.setSize(sf::Vector2f(160.0f, 40.0f));
        path.setPosition(20.0f, Constants::WINDOW_HEIGHT * 0.36f);
        path.setFillColor(sf::Color(95, 80, 65, 210));
        path.setOutlineColor(sf::Color(60, 50, 45, 200));
        path.setOutlineThickness(2);
        parkRects.push_back(path);

        // Place trees in a neat grid inside park
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                float x = 35.0f + col * 45.0f;
                float y = 70.0f + row * 110.0f;
                sf::CircleShape tree(15.0f);
                tree.setOrigin(15.0f, 15.0f);
                tree.setPosition(x, y);
                tree.setFillColor(sf::Color(34, 139, 34, 225));
                tree.setOutlineColor(sf::Color(18, 80, 18, 200));
                tree.setOutlineThickness(2);
                parkTrees.push_back(tree);
            }
        }

        // Add benches along the park path
        for (int i = 0; i < 3; i++) {
            sf::RectangleShape bench;
            bench.setSize(sf::Vector2f(30.0f, 6.0f));
            bench.setOrigin(15.0f, 3.0f);
            bench.setPosition(40.0f + i * 45.0f, Constants::WINDOW_HEIGHT * 0.36f + 20.0f);
            bench.setFillColor(sf::Color(120, 90, 70, 220));
            parkBenches.push_back(bench);
        }

        // Add small lights near path ends
        for (int i = 0; i < 2; i++) {
            sf::RectangleShape light;
            light.setSize(sf::Vector2f(4.0f, 18.0f));
            light.setOrigin(2.0f, 18.0f);
            light.setPosition(30.0f + i * 110.0f, Constants::WINDOW_HEIGHT * 0.36f + 5.0f);
            light.setFillColor(sf::Color(200, 210, 230, 200));
            parkLights.push_back(light);
        }

        // Fountain at park center
        sf::CircleShape fountainBase(20.0f);
        fountainBase.setOrigin(20.0f, 20.0f);
        fountainBase.setPosition(80.0f, Constants::WINDOW_HEIGHT * 0.36f - 20.0f);
        fountainBase.setFillColor(sf::Color(60, 120, 160, 180));
        parkTrees.push_back(fountainBase);
    }
    
    // ============================================================================
    // GENERATE ROADS - Simple, clean roads with rounded corners
    // ============================================================================
    void generateRoads() {
        roadSegments.clear();
        roadJoints.clear();
        roadManholes.clear();
        
        sf::Color roadColor(50, 55, 68);          // Cool asphalt with blue tint
        sf::Color centerLineColor(180, 215, 255); // Light blue dashes to match theme
        sf::Color edgeLineColor(140, 140, 140, 180); // neutral gray
        sf::Color slotColor(130, 130, 130, 140);     // neutral gray
        
        for (int lane = 0; lane < 3; lane++) {
            if (pathLengths[lane] < 2) continue;
            
            for (int i = 0; i < pathLengths[lane] - 1; i++) {
                float x1 = paths[lane][i].x;
                float y1 = paths[lane][i].y;
                float x2 = paths[lane][i + 1].x;
                float y2 = paths[lane][i + 1].y;
                
                float dx = x2 - x1;
                float dy = y2 - y1;
                float dist = sqrt(dx * dx + dy * dy);
                if (dist < 0.1f) continue;
                
                float angle = atan2(dy, dx) * 180.0f / 3.14159265f;
                
                // Main road segment
                sf::RectangleShape seg;
                seg.setSize(sf::Vector2f(dist, ROAD_WIDTH));
                seg.setOrigin(0, ROAD_WIDTH / 2.0f);
                seg.setPosition(x1, y1);
                seg.setRotation(angle);
                seg.setFillColor(roadColor);
                roadSegments.push_back(seg);

                float perpX = -dy / dist;
                float perpY = dx / dist;

                // Edge lines
                float edgeOffset = (ROAD_WIDTH / 2.0f) - 4.0f;
                for (int side = -1; side <= 1; side += 2) {
                    sf::RectangleShape edgeLine;
                    edgeLine.setSize(sf::Vector2f(dist, 2.0f));
                    edgeLine.setOrigin(0, 1.0f);
                    edgeLine.setPosition(x1 + perpX * edgeOffset * side, y1 + perpY * edgeOffset * side);
                    edgeLine.setRotation(angle);
                    edgeLine.setFillColor(edgeLineColor);
                    roadSegments.push_back(edgeLine);
                }
                
                // Occasional manholes centered on road
                if ((lane + i) % 4 == 0) {
                    float tHole = 0.55f;
                    float hx = x1 + dx * tHole;
                    float hy = y1 + dy * tHole;
                    sf::CircleShape hole(6.0f);
                    hole.setOrigin(6.0f, 6.0f);
                    hole.setPosition(hx, hy);
                    hole.setFillColor(sf::Color(80, 80, 80, 200));   // gray
                    hole.setOutlineColor(sf::Color(50, 50, 50, 180));
                    hole.setOutlineThickness(1.0f);
                    roadManholes.push_back(hole);
                }

                // Center line dashes (sparser)
                float dashSpacing = 70.0f;
                int dashCount = static_cast<int>(dist / dashSpacing);
                for (int d = 0; d < dashCount; d++) {
                    float t = (d * dashSpacing + dashSpacing / 2.0f) / dist;
                    float cx = x1 + dx * t;
                    float cy = y1 + dy * t;
                    
                    sf::RectangleShape dash;
                    dash.setSize(sf::Vector2f(16.0f, 2.5f));
                    dash.setOrigin(8.0f, 1.25f);
                    dash.setPosition(cx, cy);
                    dash.setRotation(angle);
                    dash.setFillColor(centerLineColor);
                    roadSegments.push_back(dash);
                }
                
                // Corner joint (rounded) at the end of the segment, except last point handled later
                float radius = ROAD_WIDTH / 2.0f;
                sf::CircleShape joint(radius);
                joint.setOrigin(radius, radius);
                joint.setPosition(x2, y2);
                joint.setFillColor(roadColor);
                roadJoints.push_back(joint);
            }
            
            // Cap at start and end to smooth edges
            float radius = ROAD_WIDTH / 2.0f;
            sf::CircleShape startCap(radius);
            startCap.setOrigin(radius, radius);
            startCap.setPosition(paths[lane][0].x, paths[lane][0].y);
            startCap.setFillColor(roadColor);
            roadJoints.push_back(startCap);
            
            int lastIdx = pathLengths[lane] - 1;
            sf::CircleShape endCap(radius);
            endCap.setOrigin(radius, radius);
            endCap.setPosition(paths[lane][lastIdx].x, paths[lane][lastIdx].y);
            endCap.setFillColor(roadColor);
            roadJoints.push_back(endCap);
        }
    }
    
    bool isValidGrid(int x, int y) const {
        return x >= 0 && x < Constants::GRID_COLS && y >= 0 && y < Constants::GRID_ROWS;
    }
    
    bool canPlaceAt(float worldX, float worldY) const {
        int gx = (int)(worldX / Constants::CELL_SIZE);
        int gy = (int)(worldY / Constants::CELL_SIZE);
        if (! isValidGrid(gx, gy)) return false;
        return !occupied[gy][gx];
    }
    
    void snapToGrid(float worldX, float worldY, float& outX, float& outY) const {
        int gx = (int)(worldX / Constants::CELL_SIZE);
        int gy = (int)(worldY / Constants::CELL_SIZE);
        outX = gx * Constants::CELL_SIZE + Constants::CELL_SIZE / 2;
        outY = gy * Constants::CELL_SIZE + Constants::CELL_SIZE / 2;
    }
    
    void setOccupied(float worldX, float worldY, bool val) {
        int gx = (int)(worldX / Constants::CELL_SIZE);
        int gy = (int)(worldY / Constants::CELL_SIZE);
        if (isValidGrid(gx, gy)) occupied[gy][gx] = val;
    }
    
    void updatePreview(float mouseX, float mouseY) {
        float snappedX, snappedY;
        snapToGrid(mouseX, mouseY, snappedX, snappedY);
        placementPreview.setPosition(snappedX - Constants::CELL_SIZE/2 + 2, snappedY - Constants::CELL_SIZE/2 + 2);
        previewValid = canPlaceAt(mouseX, mouseY);
        if (previewValid) {
            placementPreview.setFillColor(sf::Color(0, 255, 0, 100));
            placementPreview.setOutlineColor(sf::Color::Green);
        } else {
            placementPreview.setFillColor(sf::Color(255, 0, 0, 100));
            placementPreview.setOutlineColor(sf::Color::Red);
        }
    }
    
    PathPoint* getPath(int lane, int& length) {
        if (lane < 0) lane = 0;
        if (lane > 2) lane = 2;
        length = pathLengths[lane];
        return paths[lane];
    }
    
    void setShowGrid(bool show) { showGrid = show; }
    void setShowPreview(bool show) { showPreview = show; }
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Background
        target.draw(greenBackground, states);

        // Park decorations on the left side
        for (const auto& r : parkRects) target.draw(r, states);
        for (const auto& t : parkTrees) target.draw(t, states);
        
        // ============================================================================
        // DRAW ROADS - Simple roads with rounded joints
        // ============================================================================
        for (const auto& roadSegment : roadSegments) {
            target.draw(roadSegment, states);
        }
        for (const auto& joint : roadJoints) {
            target.draw(joint, states);
        }
        for (const auto& hole : roadManholes) {
            target.draw(hole, states);
        }
        
        // ============================================================================
        // DRAW GREEN GRID TILES - Fill the map with green tiles (except on paths)
        // Green tiles show where structures can be placed
        // They fill a decent amount of the map, avoiding only enemy paths and occupied areas
        // ============================================================================
        if (showGrid) {
            // Draw green tiles for all unoccupied cells
            // This fills the map with green tiles everywhere except:
            // - Enemy paths (marked as occupied in markPathsOccupied())
            // - Tower area (marked as occupied in markTowerOccupied())
            // - X-Mansion area (marked as occupied in initialize())
            // All other areas will show green tiles, filling a decent amount of the map
            for (int y = 0; y < Constants::GRID_ROWS; y++) {
                for (int x = 0; x < Constants::GRID_COLS; x++) {
                    if (!occupied[y][x]) {
                        target.draw(gridCells[y][x], states);  // Draw green tile if not on path/occupied
                    }
                }
            }
        }
        
        // Rooftop details on buildable tiles (skip occupied/path cells)
        for (const auto& detail : roofDetails) {
            sf::Vector2f pos = detail.getPosition();
            int gx = static_cast<int>(pos.x / Constants::CELL_SIZE);
            int gy = static_cast<int>(pos.y / Constants::CELL_SIZE);
            if (isValidGrid(gx, gy) && !occupied[gy][gx]) {
                target.draw(detail, states);
            }
        }
        
        target.draw(towerSprite, states);
        if (showPreview) target.draw(placementPreview, states);
    }
};