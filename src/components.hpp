#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include "Utility/Shader.hpp"
#include "Utility/ResourceManager.hpp"

#include <json.hpp>
using json = nlohmann::json;

// Player component
struct Player
{
	vec2 start_pos;
	unsigned int keys = 0;
};

// Enemies and spikes are hazards
struct Hazard
{
	std::vector<vec2> locations;
	int locationNdx;
	float interp_duration_ms = 5000.f;
	float timer_ms = 0.f;
	vec3 color = {1.0f, 1.0f, 1.0f};
};

struct LinearInterp
{
	std::vector<vec2> locations;
	int locationNdx;
    int prevLocationNdx;
	float interp_duration_ms = 5000.f;
	float timer_ms = 0.f;
    bool isFacingRight;
};

// Flag component
struct Spike
{
};

// Flag component; primarily for collision resolution and rendering
struct Key
{
	bool isPresent = true;
};

// For things that float around the player
struct Companion
{
};

// Flag component for "locked" tiles.
struct Lock
{
	bool isPresent = true;
};

// Flag component for platforms. Used to change collision logic.
struct Platform
{
};

// A region of "locked tiles"
struct LockRegion
{
	std::set<Entity> locks;
};

// All data relevant to the shape and motion of entities
struct Motion
{
	vec2 position = {0.f, 0.f};
	vec2 prev_position = {0.f, 0.f};
	float angle = 0.f;
	vec2 velocity = {0.f, 0.f};
	vec2 scale = {10.f, 10.f};
	bool isOnGround = false;
};

// Data relevant to movement characteristics/state of entity
struct Movable
{
    // Units are in # of tiles
	float max_velocity;
	float jumpHeight;
	float jumpDist;
	bool left_blocked = false;
	bool right_blocked = false;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	Collision(Entity &other_entity) { this->other_entity = other_entity; };
};

// Data structure for toggling debug mode
struct Debug
{
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & spikes.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
	vec2 original_size = {1, 1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// Light up component, isPresent when salmon should be lit up
struct LightUp
{
	float timer_ms = 300.f;
};

// A wall flag
struct Wall
{
	bool use_shader;
	Shader shader;
};

struct Decoration
{
	// flag for entities with no collision
};

struct Collectible
{
	// flag for collectables
	bool isPresent = true;
    int id = 0;
};

struct OneTimeAnim
{
	// flag for onetime animation entities
};

struct SpriteSheet
{
	Texture2D &t;
	vec2 base_dimensions;
};

struct Level
{
    unsigned int num_layers;
    int tile_size; // Assuming square tiles
//	std::string theme;
};

struct MainMenuElement
{
	bool is_active;
	bool mouseover;
	vec2 position;
	vec2 size;
};

struct LevelSelectionElement
{
	bool is_implemented;
	bool is_unlocked;
	bool mouseover;
	int level;
	vec2 position;
	vec2 size;
};

struct TextElement
{
	std::string str;
	bool is_active;
	bool is_highlighted;
	vec2 position;
	vec2 size;
};

struct WalkTimer
{
	float walkTime = 0.0;
	int currentFrame = 0;
	vec2 startFrame;
	int animationLength;
};

struct AnimTimer
{
	int currentFrame = 0;
	vec2 startFrame;
	int animFrameLength;
	float animTimeLength = 100.f;
	float timer_ms = animTimeLength;
};

struct Goal
{
};

// Death timer for player death animation
struct DeathTimer
{
	float countdown = 5000.0f;
	vec2 pos;
	bool particles_arrived = false;
	vec2 goal;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum TILESHEET_INDEX_ID
{
	TILESHEET_LABTHEME = 0,
	TILESHEET_CAVETHEME = TILESHEET_LABTHEME + 1,
    TILESHEET_FORESTTHEME = TILESHEET_CAVETHEME + 1,
	TILESHEET_COUNT = TILESHEET_FORESTTHEME + 1,
};

enum MENU_BUTTON_ID
{
	MENU_MAIN_MENU = 0,
	MENU_BUTTON_HELP = MENU_MAIN_MENU + 1,
	MENU_BUTTON_RESUME = MENU_BUTTON_HELP + 1,
	MENU_BUTTON_BACK = MENU_BUTTON_RESUME + 1,
	MENU_BUTTON_COUNT = MENU_BUTTON_BACK + 1
};

enum MAIN_MENU_BUTTON_ID
{
	//    MAIN_BUTTON_EXIT = 0,
	//    MAIN_BUTTON_RANDOM_START = MAIN_BUTTON_EXIT + 1,
	//    MAIN_BUTTON_LEVEL_SELECTION = MAIN_BUTTON_RANDOM_START + 1,
	//    MAIN_BUTTON_COUNT = MAIN_BUTTON_LEVEL_SELECTION + 1
	MAIN_BUTTON_EXIT = 0,
	MAIN_BUTTON_LOAD_GAME = 1,
	MAIN_BUTTON_LEVEL_SELECTION = 2,
	MAIN_BUTTON_RANDOM_START = 3,
    MAIN_BUTTON_TUTORIAL = 4,
	MAIN_BUTTON_COUNT = 5
};

enum LEVEL_SELECTION_BUTTON_ID
{
	LEVEL_SELECTION_BACK = 0,
	LEVEL_SELECTION_COUNT = LEVEL_SELECTION_BACK + 1
};

enum TEXT_ID
{
	TEXT_TIME = 0,
	TEXT_POINTS = TEXT_TIME + 1,
	TEXT_COUNT = TEXT_POINTS + 1
};

enum class TEXTURE_ASSET_ID
{
	FISH = 0,
	TURTLE = FISH + 1,
	TILE_0 = TURTLE + 1,
	TILE_1 = TILE_0 + 1,
	TILE_2 = TILE_1 + 1,
	TILE_3 = TILE_2 + 1,
	TILE_Player = TILE_3 + 1,
	TEXTURE_COUNT = TILE_Player + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID
{
	SALMON = 0,
	EFFECT_COUNT = SALMON + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID
{
	SPIKE = 0,
	SPRITE = SPIKE + 1,
	PEBBLE = SPRITE + 1,
	DEBUG_LINE = PEBBLE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest
{
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	int tile_id;
};

// Layer of a given map, contains all the render requests and Motion details for iteration
struct Layer
{
	ComponentContainer<Motion> current_motions;
	ComponentContainer<RenderRequest> current_renderRequests;
	ComponentContainer<AnimTimer> current_animTimers;
	ComponentContainer<Movable> current_movables;
	int level_layer;
};
