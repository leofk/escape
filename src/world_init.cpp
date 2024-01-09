#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include "Utility/Shader.hpp"
#include "common.hpp"
#include "Utility/ResourceManager.hpp"

Entity createLine(vec2 position, vec2 scale, int layer)
{
	Entity entity = Entity();
	// Motion &motion_store = registry.motions.emplace(entity);

	registry.layers[layer]->current_renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EFFECT_COUNT,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE});

	// Create motion
	Motion &motion = registry.layers[layer]->current_motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

int createLayer(int layer_idx)
{

	return registry.layers.size();
}

Entity createBox(vec2 pos, vec2 size, int layer)
{
	Entity entity = Entity();

	// Setting initial motion values
	Motion &motion = registry.layers[layer]->current_motions.emplace(entity);
	motion.position = pos;
	motion.velocity = {0.f, 0.f};
	motion.scale = size;

	Shader sh = ResourceManager::GetShader("Wall");

	registry.walls.insert(entity, {true, sh});
	registry.layers[layer]->current_renderRequests.insert(
		entity, {});

	return entity;
}

Entity createPlayer(vec2 pos, vec2 size, int layer)
{
	Entity entity = Entity();

	WalkTimer &w = registry.walkTimers.emplace(entity);
	w.currentFrame = 0;
	w.startFrame = vec2(0, 0);
	w.animationLength = 6;

	// Create player component

	Player &pl = registry.players.emplace(entity);
	pl.start_pos = pos;
    pl.keys = 0;

	// Testing out placing in an individual layer
	Motion &motion_layer = registry.motions.emplace(entity);
	motion_layer.position = pos;
	motion_layer.angle = 0.f;
	motion_layer.velocity = {0.f, 0.f};
	motion_layer.scale = size;

    Movable& movable = registry.movables.emplace(entity);
    movable.max_velocity = 6.f;
    movable.jumpHeight = 2.5f;
    movable.jumpDist = 1.5f;

	return entity;
}

Entity createPatrolEnemy(vec2 pos, vec2 size, float duration, std::vector<vec2> locations, int layer)

{
	Entity entity = Entity();

	Motion &motion = registry.layers[layer]->current_motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.scale = size;

	WalkTimer &w = registry.walkTimers.emplace(entity);
	w.currentFrame = 0;
	w.startFrame = vec2(0, 8);
	w.animationLength = 4;

	locations.push_back(pos);
	LinearInterp &interpMove = registry.linearInterp.emplace(entity);
	interpMove.locationNdx = 0;
    interpMove.prevLocationNdx = locations.size() - 1;
	interpMove.locations = locations;
    interpMove.isFacingRight = interpMove.locations[interpMove.locationNdx].x < interpMove.locations[interpMove.prevLocationNdx].x;
	interpMove.interp_duration_ms = duration;

	registry.hazards.emplace(entity);

	registry.layers[layer]->current_renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::FISH, // TEXTURE_COUNT indicates that no texture is needed
		 EFFECT_ASSET_ID::EFFECT_COUNT,
		 GEOMETRY_BUFFER_ID::SPRITE});
	return entity;
}

Entity createTile(vec2 pos, vec2 size, int i, int layer)
{
	Entity entity = Entity();

	Shader sh = ResourceManager::GetShader("Wall");

	registry.walls.insert(entity, {false, sh});

	// Testing out placing in an individual layer
	Motion &motion_layer = registry.layers[layer]->current_motions.emplace(entity);
	motion_layer.position = pos;
	motion_layer.angle = 0.f;
	motion_layer.velocity = {0.f, 0.f};
	motion_layer.scale = size;

	registry.layers[layer]->current_renderRequests.insert(
		entity,
		//{val, // TEXTURE_COUNT indicates that no txture is needed
		{TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
		 EFFECT_ASSET_ID::EFFECT_COUNT,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 i});

	return entity;
}

Entity createGoal(vec2 pos, vec2 size, int layer, bool top, bool facingRight)
{
	Entity entity = Entity();

	registry.goal.emplace(entity);

	// Testing out placing in an individual layer
	Motion &motion_layer = registry.layers[layer]->current_motions.emplace(entity);
	motion_layer.position = pos;
	motion_layer.angle = 0.f;
	motion_layer.velocity = {0.f, 0.f};
	motion_layer.scale = size;

	registry.layers[layer]->current_renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
		 EFFECT_ASSET_ID::EFFECT_COUNT,
		 GEOMETRY_BUFFER_ID::SPRITE});

    int sheet_offset = 4;
    sheet_offset += (facingRight) ? 2 : 0;
    sheet_offset += (top) ? 0 : 1;

    AnimTimer& anim = registry.layers[layer]->current_animTimers.emplace(entity);
    anim.startFrame = vec2(0, sheet_offset);
    anim.animFrameLength = 8;
    anim.currentFrame = 0;

	return entity;
}

Entity createDecoration(vec2 pos, vec2 size, int i, int layer)
{
	// Reserve an entity
	Entity entity = Entity();

	Motion &motion = registry.layers[layer]->current_motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = pos;
	motion.scale = size;

	registry.decorations.emplace(entity);
	registry.layers[layer]->current_renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EFFECT_COUNT,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 i});

	return entity;
}

Entity createCollectible(vec2 pos, vec2 size, int layer, int id)
{
	// Reserve an entity
	Entity entity = Entity();

	Motion &motion = registry.layers[layer]->current_motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.f, 0.f};
	motion.position = pos;
	motion.scale = size;

    Collectible & c = registry.collectibles.emplace(entity);
    c.id = id;

    AnimTimer& anim = registry.layers[layer]->current_animTimers.emplace(entity);
    anim.startFrame = vec2(0, 0);
    anim.animFrameLength = 8;
    anim.currentFrame = 0;

	registry.layers[layer]->current_renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EFFECT_COUNT,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createSpike(vec2 pos, vec2 size, float angle, int layer)
{
	Entity entity = Entity();

	// Setting initial motion values
	// Motion &motion_store = registry.motions.emplace(entity);

	auto &motion = registry.layers[layer]->current_motions.emplace(entity);
	motion.angle = angle; // Should be 90 deg
	motion.velocity = {0.f, 0.f};
	motion.position = pos;
	motion.scale = size;

	registry.colors.insert(
		entity,
		{0.85, 0.85, 0.85});

	registry.hazards.emplace(entity);
    registry.spikes.emplace(entity);
	registry.layers[layer]->current_renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::SALMON,
		 GEOMETRY_BUFFER_ID::SPIKE});

	return entity;
}
Entity createMenu(vec2 pos, vec2 size, bool should_be_active, bool mouseover)
{
	// Reserve an entity
	Entity entity = Entity();

	MainMenuElement &menu = registry.menu_btns.emplace(entity);
	menu.position = pos;
	menu.size = size;
	menu.is_active = should_be_active;
	menu.mouseover = mouseover;

	return entity;
}

Entity createMainMenu(vec2 pos, vec2 size, bool should_be_active, bool mouseover)
{
	// Reserve an entity
	Entity entity = Entity();

	MainMenuElement& menu = registry.main_menu_btns.emplace(entity);
	menu.position = pos;
	menu.size = size;
	menu.is_active = should_be_active;
	menu.mouseover = mouseover;

	return entity;
}

Entity createLevelsMenu(vec2 pos, vec2 size, bool should_be_active, bool mouseover)
{
	// Reserve an entity
	Entity entity = Entity();

	MainMenuElement& menu = registry.level_selection_btns.emplace(entity);
	menu.position = pos;
	menu.size = size;
	menu.is_active = should_be_active;
	menu.mouseover = mouseover;

	return entity;
}

Entity createLevelSelection(vec2 pos, vec2 size, int level, bool is_implemented, bool is_unlocked, bool mouseover)
{
	// Reserve an entity
	Entity entity = Entity();

	LevelSelectionElement& menu = registry.level_selections.emplace(entity);
	menu.position = pos;
	menu.size = size;
	menu.level = level;
	menu.is_implemented = is_implemented;
	menu.is_unlocked = is_unlocked;
	menu.mouseover = mouseover;

	return entity;
}

Entity createText(std::string str, vec2 pos, vec2 size, bool should_be_active, bool should_be_highlighted)
{
	// Reserve an entity
	Entity entity = Entity();

	TextElement& text = registry.texts.emplace(entity);
	text.str = str;
	text.is_active = should_be_active;
	text.is_highlighted = should_be_highlighted;
	text.position = pos;
	text.size = size;

	return entity;
}

Entity createPauseText(std::string str, vec2 pos, vec2 size, bool should_be_active, bool should_be_highlighted)
{
	// Reserve an entity
	Entity entity = Entity();

	TextElement& text = registry.pause_text.emplace(entity);
	text.str = str;
	text.is_active = should_be_active;
	text.is_highlighted = should_be_highlighted;
	text.position = pos;
	text.size = size;

	return entity;
}

Entity createHelpText(std::string str, vec2 pos, vec2 size, bool should_be_active, bool should_be_highlighted)
{
	// Reserve an entity
	Entity entity = Entity();

	TextElement& text = registry.help_text.emplace(entity);
	text.str = str;
	text.is_active = should_be_active;
	text.is_highlighted = should_be_highlighted;
	text.position = pos;
	text.size = size;

	return entity;
}

Entity createScore(std::string score, vec2 pos, vec2 size)
{
	// Reserve an entity
	Entity entity = Entity();

	TextElement& text = registry.scores.emplace(entity);
	text.str = score;
	text.is_active = true;
	text.is_highlighted = false;
	text.position = pos;
	text.size = size;

	return entity;
}

Entity createKey(vec2 pos, vec2 size, int layer) {
    // Reserve an entity
    Entity entity = Entity();

    Motion &motion = registry.layers[layer]->current_motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = {0.f, 0.f};
    motion.position = pos;
    motion.scale = size;

    registry.keys.emplace(entity);

    AnimTimer& anim = registry.layers[layer]->current_animTimers.emplace(entity);
    anim.startFrame = vec2(0, 1);
    anim.animFrameLength = 8;
    anim.currentFrame = 0;

    registry.layers[layer]->current_renderRequests.insert(
            entity,
            {TEXTURE_ASSET_ID::TEXTURE_COUNT,
             EFFECT_ASSET_ID::EFFECT_COUNT,
             GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

Entity createLock(vec2 pos, vec2 size, int tile_id, int layer) {
    Entity entity = Entity();
    registry.lock_tiles.emplace(entity);

    // Testing out placing in an individual layer
    Motion &motion_layer = registry.layers[layer]->current_motions.emplace(entity);
    motion_layer.position = pos;
    motion_layer.angle = 0.f;
    motion_layer.velocity = {0.f, 0.f};
    motion_layer.scale = size;

    registry.layers[layer]->current_renderRequests.insert(
            entity,
            //{val, // TEXTURE_COUNT indicates that no txture is needed
            {TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
             EFFECT_ASSET_ID::EFFECT_COUNT,
             GEOMETRY_BUFFER_ID::SPRITE,
             tile_id});

    return entity;
}
Entity createLockRegion(std::set<Entity> locks) {
    Entity entity = Entity();
    LockRegion& lock_region = registry.lock_regions.emplace(entity);
    lock_region.locks = std::move(locks);
    return entity;
}

Entity createPlatform(vec2 pos, vec2 size, int tile_id, int layer)
{
    Entity entity = Entity();


    registry.platforms.emplace(entity);

    // Testing out placing in an individual layer
    Motion &motion_layer = registry.layers[layer]->current_motions.emplace(entity);
    motion_layer.position = pos;
    motion_layer.angle = 0.f;
    motion_layer.velocity = {0.f, 0.f};
    motion_layer.scale = size;

    registry.layers[layer]->current_renderRequests.insert(
            entity,
            //{val, // TEXTURE_COUNT indicates that no txture is needed
            {TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
             EFFECT_ASSET_ID::EFFECT_COUNT,
             GEOMETRY_BUFFER_ID::SPRITE,
             tile_id});

    return entity;
}

Entity createCompanion(vec2 pos, vec2 size) {
    Entity entity = Entity();
    registry.companions.emplace(entity);

    AnimTimer& anim = registry.global_anims.emplace(entity);
    anim.startFrame = vec2(0, 1);
    anim.animFrameLength = 8;
    anim.currentFrame = 0;
    // Testing out placing in an individual layer
    Motion &motion_layer = registry.motions.emplace(entity);
    motion_layer.position = pos;
    motion_layer.angle = 0.f;
    motion_layer.velocity = {0.f, 0.f};
    motion_layer.scale = size;

    return entity;
}
void buildLevel(unsigned int num_layers, int tile_size) {
    registry.curr_level = {
            num_layers,
            tile_size
    };
}
