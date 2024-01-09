#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

Entity createBox(vec2 pos, vec2 size, int layer);

Entity createPlayer(vec2 pos, vec2 size, int layer);

Entity createPatrolEnemy(vec2 pos, vec2 size, float duration, std::vector<vec2> locations, int layer);

Entity createTile(vec2 pos, vec2 size, int i, int layer);

Entity createGoal(vec2 pos, vec2 size, int layer, bool top, bool facingRight);

Entity createDecoration(vec2 pos, vec2 size, int i, int layer);

Entity createCollectible(vec2 pos, vec2 size, int layer, int id);

int createLayer(int layer_idx);
Entity createSpike(vec2 pos, vec2 size, float angle, int layer);

Entity createMenu(vec2 pos, vec2 size, bool should_be_active = false, bool mouseover = false);
Entity createMainMenu(vec2 pos, vec2 size, bool should_be_active = false, bool mouseover = false);
Entity createLevelsMenu(vec2 pos, vec2 size, bool should_be_active, bool mouseover);
Entity createLevelSelection(vec2 pos, vec2 size, int level, bool is_implemented, bool is_unlocked, bool mouseover);

Entity createText(std::string str, vec2 pos, vec2 size, bool should_be_active, bool should_be_highlighted);
Entity createPauseText(std::string str, vec2 pos, vec2 size, bool should_be_active, bool should_be_highlighted);
Entity createHelpText(std::string str, vec2 pos, vec2 size, bool should_be_active, bool should_be_highlighted);
Entity createScore(std::string score, vec2 pos, vec2 size);

Entity createKey(vec2 pos, vec2 size, int layer);

Entity createLock(vec2 pos, vec2 size, int tile_id, int layer);

Entity createLockRegion(std::set<Entity> locks);

Entity createCompanion(vec2 pos, vec2 size);

Entity createPlatform(vec2 pos, vec2 size, int i, int layer);

// Modifies the curr_level member in ECSRegistry
void buildLevel(unsigned int num_layers, int tile_size);
