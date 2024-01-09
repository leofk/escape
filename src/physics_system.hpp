#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_system.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms);

	PhysicsSystem()
	{
	}


private:
    inline float convertWorldScale(float num_tiles) const { return registry.curr_level.tile_size * num_tiles; }

    void updatePlayerVelocity(float elapsed_ms);
    void stepEntity(float step_seconds, Motion& motion, Movable& movable, Entity e) const;

    void stepCompanions();

};
