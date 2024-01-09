#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface *> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<LinearInterp> linearInterp;
	ComponentContainer<Hazard> hazards;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<LightUp> lightUp;
	ComponentContainer<Wall> walls;
	ComponentContainer<Goal> goal;
	ComponentContainer<Decoration> decorations;
	ComponentContainer<Collectible> collectibles;
	ComponentContainer<MainMenuElement> menu_btns;
	ComponentContainer<MainMenuElement> main_menu_btns;
	ComponentContainer<MainMenuElement> level_selection_btns;
	ComponentContainer<LevelSelectionElement> level_selections;
	ComponentContainer<WalkTimer> walkTimers;
	ComponentContainer<OneTimeAnim> oneTimeAnims;
	ComponentContainer<Spike> spikes;
	ComponentContainer<Key> keys;
	ComponentContainer<Companion> companions;
	ComponentContainer<Lock> lock_tiles;
	ComponentContainer<LockRegion> lock_regions;
	ComponentContainer<AnimTimer> global_anims;
	ComponentContainer<Platform> platforms;
	ComponentContainer<TextElement> texts;
	ComponentContainer<TextElement> pause_text;
	ComponentContainer<TextElement> help_text;
	ComponentContainer<TextElement> scores;
	ComponentContainer<Movable> movables;
	ComponentContainer<DeathTimer> deathTimers;

	/**
	 * This is the layer registry for our multiple layer maps
	 * It contains the motion and render components separated by layer
	 *
	 * Though the ComponentContainer has an entity vector, it will be unused since layers will be indexed in to
	 * according to the 0 indexed current layer visible
	 *
	 * This is intended to allow iteration across active motion components and active render requests, so we do not need to do
	 * additional parsing/checking while iterating through every motion component
	 *
	 * This is for iteration only, not for checking, the components that determine functionality and interactions remain in their
	 * traditional component containers
	 */

	Layer layerOne;
	Layer layerTwo;

	std::vector<Layer *> layers;
    Level curr_level;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{

		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&lightUp);
		registry_list.push_back(&walls);
		registry_list.push_back(&goal);
		registry_list.push_back(&linearInterp);
		registry_list.push_back(&decorations);
		registry_list.push_back(&collectibles);
		registry_list.push_back(&hazards);
		registry_list.push_back(&menu_btns);
		registry_list.push_back(&main_menu_btns);
		registry_list.push_back(&level_selection_btns);
		registry_list.push_back(&level_selections);
		registry_list.push_back(&texts);
		registry_list.push_back(&pause_text);
		registry_list.push_back(&help_text);
		registry_list.push_back(&scores);
		registry_list.push_back(&walkTimers);
		registry_list.push_back(&oneTimeAnims);
		registry_list.push_back(&keys);
		registry_list.push_back(&companions);
		registry_list.push_back(&lock_tiles);
		registry_list.push_back(&lock_regions);
		registry_list.push_back(&platforms);
		registry_list.push_back(&global_anims);

		registry_list.push_back(&movables);
		registry_list.push_back(&spikes);

		registry_list.push_back(&deathTimers);

		layers.push_back(&layerOne);
		layers.push_back(&layerTwo);
	}

	void clear_all_components()
	{
		for (ContainerInterface *reg : registry_list)
			reg->clear();

		for (Layer *l : layers)
		{
			l->current_motions.clear();
			l->current_renderRequests.clear();
			l->current_movables.clear();
			l->current_animTimers.clear();
		}
	}

	void list_all_components()
	{
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface *reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e)
	{
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface *reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e)
	{
		for (ContainerInterface *reg : registry_list)
			reg->remove(e);

		for (Layer *l : layers)
		{
			l->current_motions.remove(e);
			l->current_renderRequests.remove(e);
			l->current_animTimers.remove(e);
			l->current_movables.remove(e);
		}
	}

	void print_motion(Entity e)
	{
		if (!motions.has(e))
		{
			printf("this entity does not have a motion component\n");
			return;
		}
		Motion m = motions.get(e);
		printf("position of entity is x=%f, y=%f\n", m.position.x, m.position.y);
		printf("velocity of entity is dx=%f, dy=%f\n", m.velocity.x, m.velocity.y);
	}

    Motion* getMotion(Entity e) {
        if (layers[0]->current_motions.has(e)) {
            return &(layers[0]->current_motions.get(e));
        } else if (layers[1]->current_motions.has(e)) {
            return &(layers[1]->current_motions.get(e));
        } else {
            printf("This entity does not have a layer-specific motion component\n");
            assert(false);
            return nullptr;
        }
    }
};

extern ECSRegistry registry;