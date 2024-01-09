// Header
#include "map_loader.hpp"
#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
using namespace std;

// json
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

#define horizontal 0x80000000
#define vertical 0x40000000
#define diagonal 0x20000000

void scan_locks(unordered_map<int, Entity>& result, const vector<int>& flags, const vector<int>& tiles, int x, int y, int width, int len, int layer);
bool is_lock_seen(const vector<unordered_map<int, Entity>>& lock_regions, int x, int y);
static inline int cantor(int x, int y) { return (((x + y)*(x + y + 1)) >> 1) + y; }
int is_flag(string name)
{
    return strncmp("Flag", name.c_str(), strlen("Flag")) == 0;
}

void load_map(std::string name)
{
    ifstream f;
    f.open(string(PROJECT_SOURCE_DIR) + "maps/" + name + ".json");
    json data = json::parse(f);
    int height = data["layers"][0]["height"];
    int width = data["layers"][0]["width"];
    unsigned int size = data["layers"].size() / 2; // Loop on visible layers only -- this is also the number of layers!
    int len = window_height_px / height < window_width_px / width ? window_height_px / height : window_width_px / width;
    int collectible_id = 1;
    int enemyNdx = 0;
    std::vector<vec2> enemyLocations;
//    printf("%d\n", len);
    buildLevel(size, len);
    for (int i = 0; i < size; i++)
    {
        int flagLayerNdx = i + size; // Flag layers should appear in sequence after all visible layers
        vector<int> flags = data["layers"][flagLayerNdx]["data"];
        vector<int> tiles = data["layers"][i]["data"];
        vector<unordered_map<int, Entity>> lock_regions; // Cantor pairing function for key
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int tile = tiles[y * width + x];
                int flag = flags[y * width + x];
                if (tile != 0 || flag == SPAWN || (flag >= SPIKE_D && flag <= SPIKE_R) || flag == GOAL_LEFT || flag == GOAL_RIGHT || flag == ENEMY_SPAWN)
                {
                    switch (flag)
                    {
                    case COLLISION:
                        createTile(vec2(x * len + len / 2, y * len + len / 2), vec2(len, len), tile - 1, i);
                        break;
                    case COLLECTIBLE: // CONSTRAINT: All collectibles are coins
                        createCollectible(vec2(x * len + len / 2, y * len + len / 2), vec2(len, len),  i, collectible_id++);
                        break;
                    case KEY:
                        createKey(vec2(x * len + len / 2, y * len + len / 2), vec2(len, len),  i);
                        break;
                    case PLATFORM:
                        createPlatform(vec2(x * len + len / 2, y * len + len / 2), vec2(len, len), tile - 1, i);
                        break;
                    case LOCK:
                        if (!is_lock_seen(lock_regions, x, y)) {
                            lock_regions.emplace_back();
                            scan_locks(lock_regions.back(), flags, tiles, x, y, width, len, i);
                        }
                        break;
                    case SPIKE_D:
                        createSpike(vec2(x * len + len / 2.0, y * len), vec2(len, len / 2), M_PI, i);
                        break;
                    case SPIKE_L:
                        createSpike(vec2(x * len + len, y * len + len / 2.0), vec2(len, len / 2), (3.f/2.f)*M_PI, i);
                        break;
                    case SPIKE_U:
                        createSpike(vec2(x * len + len / 2.0, y * len + len), vec2(len, len / 2), 0.f, i);
                        break;
                    case SPIKE_R:
                        createSpike(vec2(x * len, y * len + len / 2.0), vec2(len, len / 2), (1.f/2.f)*M_PI, i);
                        break;
                    case GOAL:
                    case DECOR:
                        createDecoration(vec2(x * len + len / 2, y * len + len / 2), vec2(len, len), tile - 1, i);
                        break;
                    case SPAWN:
                        createPlayer({x * len + len / 2, y * len + len / 2 + len}, {len, len}, i);
                        break;
                    case GOAL_LEFT:
                        createGoal(vec2(x * len + len / 2, y * len + len / 2), vec2(len, len), i, true, false);
                        createGoal(vec2(x * len + len / 2, (y+1) * len + len / 2), vec2(len, len), i, false, false);
                        break;
                    case GOAL_RIGHT:
                        createGoal(vec2(x * len + len / 2, y * len + len / 2), vec2(len, len), i, true, true);
                        createGoal(vec2(x * len + len / 2, (y+1) * len + len / 2), vec2(len, len), i, false, true);
                        break;
                    case ENEMY_SPAWN:
                        for (std::vector<int> point : data["enemyPoints"][enemyNdx]) {
                            enemyLocations.emplace_back(point[0]*len + len / 2, point[1]*len + len / 2);
                        }
                        createPatrolEnemy(vec2(x*len + len / 2, y * len + len/2), vec2(len, len), 1250.f, enemyLocations, i);
                        enemyNdx++;
                        enemyLocations.clear();
                        break;
                    default:
                        printf("Encountered unspecified flag %d at (%d,%d) in level %s, layer %d! This level may be broken :(\n", flag, x, y, name.c_str(), i);
                        break;
                    }
                }
            }
            for (auto& region : lock_regions) {
                set<Entity> lock_entities;

                for (const auto& pair : region) {
                    lock_entities.insert(pair.second);
                }
                createLockRegion(lock_entities);
            }

        }
    }
}
bool is_lock_seen(const vector<unordered_map<int, Entity>>& lock_regions, int x, int y) {
    for (const auto& region : lock_regions) {
        if (region.count(cantor(x, y)) > 0) {
            return true;
        }
    }
    return false;
}


// Look into making this a closure in load_map maybe?
void scan_locks(unordered_map<int, Entity>& result, const vector<int>& flags, const vector<int>& tiles, int x, int y, int width, int len, int layer) {

    int tile = tiles[y * width + x];
    Entity entity = createLock(vec2(x * len + len / 2, y * len + len / 2), vec2(len, len), tile - 1, layer);
    int key = cantor(x, y); // Cantor pairing function (unique result for each pair of natural numbers)
    result.emplace(key, entity);

    if (flags[y * width + x + 1] == LOCK && !result.count(cantor(x+1, y))) { // Tile to the right is a lock
//        printf("Found unseen lock on the right\n");
        scan_locks(result, flags, tiles, x+1, y, width, len, layer);
    }
    if (flags[(y+1) * width + x] == LOCK && !result.count(cantor(x, y+1))) { // Tile to the bottom is a lock
//        printf("Found lock below\n");
        scan_locks(result, flags, tiles, x, y+1, width, len, layer);
    }
    if (flags[y * width + x - 1] == LOCK && !result.count(cantor(x-1, y))) { // Tile to the left is a lock
//        printf("Found lock on the left\n");
        scan_locks(result, flags, tiles, x-1, y, width, len, layer);
    }
    if (flags[(y-1) * width + x] == LOCK && !result.count(cantor(x, y-1))) { // Tile above is a lock
//        printf("Found lock on the left\n");
        scan_locks(result, flags, tiles, x, y-1, width, len, layer);
    }
}
