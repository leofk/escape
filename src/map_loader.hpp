#pragma once

#include <iostream>

using namespace std;

int is_flag(string name);
void load_map(string name);

enum FLAGS {
    COLLISION = 145,
    PLATFORM = COLLISION+1,
    MOVING_PLATFORM = PLATFORM+1,
    SPAWN = MOVING_PLATFORM+1,
    GOAL = SPAWN+1, // Obsolete; please update levels
    DECOR = GOAL+1,
    DECOR_ANIM = DECOR+1,
    COLLECTIBLE = DECOR_ANIM+1,
    SWITCH = COLLECTIBLE+1,
    TRIGGER = SWITCH+1,
    KEY = TRIGGER+1,
    LOCK = KEY+1,
    LADDER = LOCK+1,
    MOVABLE_OBJ = LADDER+1,
    LEVER = MOVABLE_OBJ+1,
    HAZARD = LEVER+1,
    SPIKE_D = HAZARD+1,
    SPIKE_L = SPIKE_D+1,
    SPIKE_U = SPIKE_L+1,
    SPIKE_R = SPIKE_U+1,
    ENEMY_SPAWN = SPIKE_R+1,
    GOAL_RIGHT = ENEMY_SPAWN+1,
    GOAL_LEFT = GOAL_RIGHT+1,
};