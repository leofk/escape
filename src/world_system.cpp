// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
using namespace std;
#include "physics_system.hpp"
#include "map_loader.hpp"
#include "common.hpp"

// json
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

#include "Utility/ResourceManager.hpp"

/** An array that represents the current pressed vs not pressed state of the arrow keys
 *
 * Up : arrow_pressed[0];
 * Down : arrow_pressed[1];
 * Left : arrow_pressed[2];
 * Right : arrow_pressed[3];
 *
 * Example: (For up arrow, which is the 0 element of the arrays)
 * If (movement_key[0] == true && arrow_key_pressed_prev_frame[0] == false) then it means that
 * this is the first press of the up arrow
 *
 * If (movement_key[0] == true && arrow_key_pressed_prev_frame[0] == true) then it means that
 * the up arrow is being held down
 **/
bool movement_key[4];
bool facingRight = true;
bool dropPlatform = false;
bool demoMesh;
bool isInJump;

int frame = 0;

// only used for tutorials
int tutorial_level_number = 0;
int total_tutorial_levels = 4;
bool is_tutorial = false;


int level_number = 1;		   // start at level 1
int max_level = 16;			   // total number of levels
int first_two_layer_level = 4; // first level that incorporates two layers
int max_selection = 20;
int max_tutorials = 5;
std::vector<int> level_order;
bool is_practice = false;

float jumpTimer;
extern float phase_success_trans_timer;
extern float phase_fail_trans_timer;
float currentTime = (float)glfwGetTime();

vec2 mouse_position;

GameState gameState = Start_Screen;
bool debug = false;

// 0-3: option, resume, help, and exit
Entity menu_buttons[MENU_BUTTON_COUNT];
Entity main_menu_buttons[MAIN_BUTTON_COUNT];
// Entity level_selection_buttons[LEVEL_SELECTION_COUNT];

int current_layer = 0;

void handle_movement_input(int key, int action);
void handle_state_input(int key, int action);

void changeGameState_toHelp();

// Again, only to be used with Collectible, Key, Lock
template <class T>
static inline void makePresent(ComponentContainer<T> &container)
{
	static_assert(std::is_same<T, Key>::value || std::is_same<T, Collectible>::value || std::is_same<T, Lock>::value, "invalid container to make present");
	for (auto entity : container.entities)
	{
		T &component = container.get(entity);
		component.isPresent = true; // This *will* break if you pass in something that does not have isPresent
	}
}
// Create the fish world
WorldSystem::WorldSystem()
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);

	if (death_sounds[0] != nullptr)
		Mix_FreeChunk(death_sounds[0]);
	if (death_sounds[1] != nullptr)
		Mix_FreeChunk(death_sounds[1]);
	if (death_sounds[2] != nullptr)
		Mix_FreeChunk(death_sounds[2]);

	if (success_sound != nullptr)
		Mix_FreeChunk(success_sound);
	if (consume_effect != nullptr)
		Mix_FreeChunk(consume_effect);
	if (swap_denied != nullptr)
		Mix_FreeChunk(swap_denied);
	if (swap_success != nullptr)
		Mix_FreeChunk(swap_success);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace
{
	void glfw_err_cb(int error, const char *desc)
	{
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window()
{
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}
	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#if __APPLE__

#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);
	int height, width;
	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	width = mode->width;
	height = mode->height;

#ifdef DEBUG_OPENGL
	// Create the main window (for rendering, keyboard, and mouse input)
	 window = glfwCreateWindow(960, 540, "Escape Phase", nullptr, nullptr); // superior for debugging!!
#else
	 window = glfwCreateWindow(width, height, "Escape Phase", glfwGetPrimaryMonitor(), nullptr);
#endif
	if (window == nullptr)
	{
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1}); };
	auto cursor_input_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_action(_0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, cursor_input_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("background_music.wav").c_str());
	death_sounds[0] = Mix_LoadWAV(audio_path("death1.wav").c_str());
	death_sounds[1] = Mix_LoadWAV(audio_path("death2.wav").c_str());
	death_sounds[2] = Mix_LoadWAV(audio_path("death3.wav").c_str());

	success_sound = Mix_LoadWAV(audio_path("success.wav").c_str());
	consume_effect = Mix_LoadWAV(audio_path("consume.wav").c_str());
	swap_denied = Mix_LoadWAV(audio_path("swap_denied.wav").c_str());
	swap_success = Mix_LoadWAV(audio_path("swap_success.wav").c_str());

	if (background_music == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is isPresent",
				audio_path("background_music.wav").c_str());
		return nullptr;
	}

	if (success_sound == nullptr || consume_effect == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n make sure the data directory is isPresent",
				audio_path("success.wav").c_str(),
				audio_path("consume.wav").c_str());
		return nullptr;
	}

	if (death_sounds[0] == nullptr || death_sounds[1] == nullptr || death_sounds[1] == nullptr)
	{
		fprintf(stderr, "Failed to load death_sounds\nMake sure the data directory is isPresent");
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
	// build_level();
	createText("TIME: 0:0:0", vec2(20.f, 20.f), vec2(10.f, 10.f), false, false);
	createText("POINTS: " + to_string(points), vec2(20.f, 35.f), vec2(10.f, 10.f), false, false);
	setup_menus();
	setup_main_menus();
	setup_level_selection();
}

void WorldSystem::checkBounds()
{
	Entity &player = registry.players.entities[0];
	Motion &motion = registry.motions.get(player);
	if (motion.position.y > window_height_px ||
		motion.position.y < 0 ||
		motion.position.x > window_width_px ||
		motion.position.x < 0)
	{
		reset_player();
	}
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	time = time + elapsed_ms_since_last_update;
	registry.texts.components[TEXT_TIME].str = "TIME: " + to_string(time / 60000) + "m" + to_string((time / 1000) % 60) + "s";
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Escape Phase! Current Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	// while (registry.debugComponents.entities.size() > 0)
	// 	registry.remove_all_components_of(registry.debugComponents.entities.back());

	assert(!registry.players.entities.empty());

	checkBounds();

	if (movement_key[0])
	{
		jumpTimer -= 0.5f;
		if (jumpTimer < 0)
		{
			jumpTimer = 0.f;
		}
	}

	// Step death timer
	for (int i = 0; i < registry.deathTimers.size(); ++i)
	{
		DeathTimer &dt = registry.deathTimers.components[i];

		// progress timer

		dt.countdown += elapsed_ms_since_last_update;
	}

	for (Entity e : registry.layers[current_layer]->current_animTimers.entities)
	{
		AnimTimer &anim = registry.layers[current_layer]->current_animTimers.get(e);
		anim.timer_ms -= elapsed_ms_since_last_update;
		if (anim.timer_ms < 0.f)
		{

			anim.timer_ms = anim.animTimeLength;
			anim.currentFrame++;
			anim.currentFrame %= anim.animFrameLength;
		}
	}

	// Step global animations
    for (Entity e : registry.global_anims.entities)
    {
        AnimTimer &anim = registry.global_anims.get(e);
		anim.timer_ms -= elapsed_ms_since_last_update;
		if (anim.timer_ms < 0.f)
		{
            // Delete pickup animation if it's played through its entire animation frame
            if (registry.oneTimeAnims.has(e) && anim.currentFrame == anim.animFrameLength - 1)
            {
                registry.remove_all_components_of(e);
                continue;
            }
			anim.timer_ms = anim.animTimeLength;
			anim.currentFrame++;
			anim.currentFrame %= anim.animFrameLength;
		}
	}

	if (phase_success_trans_timer > 0)
	{
        phase_success_trans_timer -= elapsed_ms_since_last_update;
	}
	else
	{
        phase_success_trans_timer = 0;
	}

    if (phase_fail_trans_timer > 0)
    {
        phase_fail_trans_timer -= elapsed_ms_since_last_update;
    }
    else
    {
        phase_fail_trans_timer = 0;
    }

	for (int i = 0; i < registry.walkTimers.components.size(); ++i)
	{
		WalkTimer &wt = registry.walkTimers.components[i];
		// Player done seperately to allow checking if moving or not
		if (registry.players.has(registry.walkTimers.entities[i]))
		{

			Motion &player_motion = registry.motions.get(registry.walkTimers.entities[i]);
			// If player is not moving sideways do not animate run
			if (player_motion.velocity.x == 0)
			{
				wt.currentFrame = 0;
				wt.walkTime = 0.0;
				continue;
			}
			wt.walkTime += elapsed_ms_since_last_update;
			if (wt.walkTime > 100.f)
			{
				wt.walkTime = 0;
				wt.currentFrame++;
				wt.currentFrame %= wt.animationLength;
			}
			continue;
		}

		// progress timer

		wt.walkTime += elapsed_ms_since_last_update;
		if (wt.walkTime > 200.f)
		{
			wt.walkTime = 0;
			wt.currentFrame++;
			wt.currentFrame %= wt.animationLength;
		}
	}

	// Interpolation
	for (Entity e : registry.linearInterp.entities)
	{
//		Motion &motion = registry.layers[current_layer]->current_motions.get(e);
        Motion *motion = registry.getMotion(e);
		LinearInterp &hazard = registry.linearInterp.get(e);
		vec2 targetLocation = hazard.locations[hazard.locationNdx];
        vec2 prevLocation = hazard.locations[hazard.prevLocationNdx];

		hazard.timer_ms += elapsed_ms_since_last_update;

		if (hazard.timer_ms < hazard.interp_duration_ms)
		{
			motion->position = prevLocation + (targetLocation - prevLocation) * (hazard.timer_ms / hazard.interp_duration_ms);
		}
		else
		{
			motion->position = targetLocation;
            hazard.prevLocationNdx = hazard.locationNdx;
			hazard.locationNdx = (hazard.locationNdx + 1) % (hazard.locations.size());
			hazard.timer_ms = 0.f;
            // Maintain direction if x location does not change
            if(hazard.locations[hazard.locationNdx].x < hazard.locations[hazard.prevLocationNdx].x)
            {
                hazard.isFacingRight = true;
            }
            else if (hazard.locations[hazard.locationNdx].x > hazard.locations[hazard.prevLocationNdx].x)
            {
                hazard.isFacingRight = false;
            }
		}
	}

	step_UI(elapsed_ms_since_last_update);

	return true;
}

bool WorldSystem::step_UI(float elapsed_ms_since_last_update)
{
	if (renderer->draw_save_message_time > 0.0f)
		renderer->draw_save_message_time -= elapsed_ms_since_last_update;
	if (renderer->draw_load_message_time > 0.0f)
		renderer->draw_load_message_time -= elapsed_ms_since_last_update;
	if (renderer->draw_unable_load_time > 0.0f)
		renderer->draw_unable_load_time -= elapsed_ms_since_last_update;

	if (renderer->cooldown_between_save_and_load > 0.0f)
		renderer->cooldown_between_save_and_load -= elapsed_ms_since_last_update;
	return true;
}


// Compute collisions between entities
void WorldSystem::handle_collisions()
{
	// Loop over all collisions detected by the physics system
	auto &collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++)
	{
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;

		// For now, we are only interested in collisions that involve the player
		if (registry.players.has(entity))
		{

			/**
			 * @brief When meet walls
			 */
			// Checking Player - Walls collisions
			if (registry.goal.has(entity_other))
			{
                reset_movement_key();
                if (is_practice)
				{
					level_number = 1;
					registry.collisions.clear();
					delete_current_level();
					gameState = Level_Selection;
					registry.texts.components[TEXT_TIME].is_active = false;
					registry.texts.components[TEXT_POINTS].is_active = false;
					registry.level_selection_btns.components[LEVEL_SELECTION_BACK].is_active = true;
					registry.texts.components[TEXT_COUNT + MAIN_BUTTON_COUNT].is_active = true;
					break;
				}
                else if (is_tutorial) {
                    Motion &player_motion = registry.motions.get(entity);
                    player_motion.velocity = {0, 0};
                    player_motion.prev_position = registry.players.components[0].start_pos;
                    player_motion.position = registry.players.components[0].start_pos;
                    tutorial_level_number++;
                    renderer->tutorial_instruction_number++;
                    registry.collisions.clear();
                    delete_current_level();
                    gameState = Tutorial_Instruction;
                    registry.texts.components[TEXT_POINTS].is_active = false;
					registry.texts.components[TEXT_TIME].is_active = false;
                    break;
                }
				else
				{
					save_unlocked_level(level_order[level_number - 1]);
					LevelSelectionElement &m = registry.level_selections.components[level_number - 1];
					m.is_unlocked = true;
					Mix_PlayChannel(-1, success_sound, 0);
					if (level_number < max_level)
					{
						level_number++;
					}
					else
					{
						level_number = 1;
						delete_current_level();
						if (points < 5) gameState = End_4;
						else if (points < 10) gameState = End_1;
						else if (points < 15) gameState = End_2;
						else gameState = End_3;
						for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
						{
							registry.main_menu_btns.components[j].is_active = true;
							registry.texts.components[j + TEXT_COUNT].is_active = true;
						}
						registry.texts.components[TEXT_TIME].is_active = false;
						registry.texts.components[TEXT_POINTS].is_active = false;
						save_highscores(points);
						save_time(time);
						save_score(points, time, deaths);
						setup_scoreboard();
						setup_times();
						setup_scores();
						setup_titles();
						break;
					}
					change_level();
				}
			}

			// Kill first
			if (registry.hazards.has(entity_other))
			{
				deaths += 1;
				int death_sound = rand() % 3;
				Mix_PlayChannel(-1, death_sounds[death_sound], 0);

				//points = points_before_curr_level;
				//registry.texts.components[TEXT_POINTS].str = "POINTS: " + to_string(points);

				if (!registry.deathTimers.has(entity))
				{
					DeathTimer &dt = registry.deathTimers.emplace(entity);
					dt.countdown = 0.f;
					Motion &player_motion = registry.motions.get(entity);
					dt.pos = player_motion.position;
					dt.goal = registry.players.components[0].start_pos;
				}

				reset_player();
			}

			if (registry.lock_tiles.has(entity_other))
			{ // Player has collided with a lock tile with >= 1 keys
				if (!isPresent(entity_other, registry.lock_tiles))
				{
					continue;
				}
				Motion &test = registry.layers[current_layer]->current_motions.get(entity_other);
//				printf("Collided with lock_tile at %f %f\n", test.position.x, test.position.y);
				Player &player_comp = registry.players.components[0];
				player_comp.keys--;

				// Delete key; maybe use Observer on key in player component in future instead?
				// Only really needs to change if we add "companions" that are not keys
				Entity del_key = registry.companions.entities[0];
				Motion &companion_motion = registry.motions.get(del_key);
				createPoofAnim(companion_motion.position, companion_motion.scale);
				registry.remove_all_components_of(del_key);

				for (auto region_entity : registry.lock_regions.entities)
				{
					LockRegion &region = registry.lock_regions.get(region_entity);
					if (region.locks.count(entity_other))
					{ // The lock region has this entity
						for (Entity e : region.locks)
						{
							Motion &lock_motion = registry.layers[current_layer]->current_motions.get(e);
							createPoofAnim(lock_motion.position, lock_motion.scale);
							Lock &lock = registry.lock_tiles.get(e);
							lock.isPresent = false;
						}
						break;
					}
				}
			}
			if (registry.collectibles.has(entity_other))
			{
				// chew, count points, and set the LightUp timer
				Motion &motion = registry.layers[current_layer]->current_motions.get(entity_other);
				createPickupAnim(motion.position, motion.scale);
				Collectible &collectible = registry.collectibles.get(entity_other);
				collectible.isPresent = false;
                collected_ids.push_back(collectible.id);
				Mix_PlayChannel(-1, consume_effect, 0);
				points++;
				update_points_message();
			}

			if (registry.keys.has(entity_other))
			{
				Motion &old_motion = registry.layers[current_layer]->current_motions.get(entity_other);
				createPickupAnim(old_motion.position, old_motion.scale);
				Mix_PlayChannel(-1, consume_effect, 0);

				Entity player = registry.players.entities[0];
				Player &player_component = registry.players.components[0];
				const Motion &player_motion = registry.motions.get(player);
				vec2 companion_pos = vec2(player_motion.prev_position.x, player_motion.prev_position.y - (player_motion.scale.y / 2.f) - 5.f);
				vec2 companion_scale = old_motion.scale / 2.f;
				createCompanion(companion_pos, companion_scale);

				player_component.keys++;

				Key &key = registry.keys.get(entity_other);
				key.isPresent = false;
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

Entity WorldSystem::createOneTimeAnim(vec2 pos, vec2 size, vec2 start_frame, int animFrameLength, float animTimeLength)
{
	Entity entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;

	AnimTimer &anim = registry.global_anims.emplace(entity);
	anim.currentFrame = 0;
	anim.animFrameLength = animFrameLength;
	anim.startFrame = start_frame;
	anim.animTimeLength = animTimeLength;

	registry.oneTimeAnims.emplace(entity);

	return entity;
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{
	handle_state_input(key, action);
	if (gameState == Level_Active)
	{
		if (registry.players.size() > 0 && registry.deathTimers.has(registry.players.entities[0]))
		{
            reset_movement_key();
            facingRight = true;
			return;
		}

		handle_movement_input(key, action);
		/*if (key == GLFW_KEY_P && action == GLFW_PRESS && gameState == Level_Active && ((is_practice ? level_number : level_order[level_number - 1] + 1) >= first_two_layer_level) && current_layer != 1)
		{
			if (safe_to_layer_swap(1))
			{
				phase_success_trans_timer = 150.f;
				current_layer = 1;
			}
		}

		if (key == GLFW_KEY_O && action == GLFW_PRESS && gameState == Level_Active && ((is_practice ? level_number : level_order[level_number - 1] + 1) >= first_two_layer_level) && current_layer != 0)
		{
			if (safe_to_layer_swap(0))
			{
				phase_success_trans_timer = 150.f;
				current_layer = 0;
			}
		}*/

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && gameState == Level_Active && registry.curr_level.num_layers > 1)
		{
			if (current_layer == 0 && safe_to_layer_swap(1))
			{
                phase_success_trans_timer = 150.f;
				current_layer = 1;
			}
			else if (current_layer == 1 && safe_to_layer_swap(0))
			{
                phase_success_trans_timer = 150.f;
				current_layer = 0;
			} else {
                phase_fail_trans_timer = 150.f;
            }
		}

		if (key == GLFW_KEY_R && action == GLFW_PRESS && gameState == Level_Active && (is_practice || debug) && !is_tutorial)
		{
			update_points_message();
			reset_player();
			current_layer = 0;
		}

		if (key == GLFW_KEY_N && gameState == Level_Active && !is_practice && level_number < max_level && action == GLFW_PRESS && debug)
		{
			level_number++;
			reset_level();
		}

		if (key == GLFW_KEY_B && gameState == Level_Active && !is_practice && level_number > 1 && action == GLFW_PRESS && debug)
		{
			level_number--;
			reset_level();
		}

		if (key == GLFW_KEY_T && gameState == Level_Active && !is_practice && action == GLFW_PRESS && debug)
		{
			points += 5;
			registry.texts.components[TEXT_POINTS].str = "POINTS: " + to_string(points);
		}

		if (key == GLFW_KEY_Y && gameState == Level_Active && !is_practice && action == GLFW_PRESS && debug)
		{
			level_number = 1;
			delete_current_level();
			if (points < 5) gameState = End_4;
			else if (points < 10) gameState = End_1;
			else if (points < 15) gameState = End_2;
			else gameState = End_3;
			for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
			{
				registry.main_menu_btns.components[j].is_active = true;
				registry.texts.components[j + TEXT_COUNT].is_active = true;
			}
			registry.texts.components[TEXT_TIME].is_active = false;
			registry.texts.components[TEXT_POINTS].is_active = false;
			save_highscores(points);
			save_time(time);
			save_score(points, time, deaths);
			setup_scoreboard();
			setup_times();
			setup_scores();
			setup_titles();
		}

		if (key == GLFW_KEY_9 && action == GLFW_PRESS && mod == (GLFW_MOD_SHIFT | GLFW_MOD_ALT) && !is_tutorial)
		{
			debug = !debug;
		}
	}

    if (gameState == Tutorial_Instruction && key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        if (tutorial_level_number == total_tutorial_levels) {
            is_tutorial = false;
            tutorial_level_number = 0;
            renderer->tutorial_instruction_number = 1;

            delete_current_level();
            gameState = Start_Screen;
            for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
            {
                registry.main_menu_btns.components[j].is_active = true;
                registry.texts.components[j + 2].is_active = true;
            }
            registry.texts.components[TEXT_POINTS].is_active = false;
        } else {
            build_tutorial_level();
            change_gamestate_to_play();
            return;
        }
    }

	// Resetting game

	// Debugging
	//	if (key == GLFW_KEY_D)
	//	{
	//		if (action == GLFW_RELEASE)
	//			debugging.in_debug_mode = false;
	//		else
	//			debugging.in_debug_mode = true;
	//	}

	// Saving current game state
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		if (gameState == Level_Active && !is_practice && !is_tutorial)
		{
			saveGame();
		}
	}

	// Loading game
	if (key == GLFW_KEY_7 && action == GLFW_PRESS && !is_practice && !is_tutorial)
	{
		loadGame();
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_pos)
{
	mouse_position = vec2(mouse_pos.x, mouse_pos.y);

	(vec2) mouse_pos; // dummy to avoid compiler warning

	// getting window size
	int window_width, window_height;
	glfwGetFramebufferSize(window, &window_width, &window_height);

#if __APPLE__
	window_width /= 2;
	window_height /= 2;
#endif

	float targetRatio = (float)window_width_px / (float)window_height_px;

	int width = window_width;
	int height = (int)(width / targetRatio);

	if (height > window_height)
	{
		height = window_height;
		width = (int)(height * targetRatio);
	}

	int vp_x = (window_width / 2) - (width / 2);
	int vp_y = (window_height / 2) - (height / 2);

	window_width -= vp_x * 2;
	window_height -= vp_y * 2;
	mouse_position.x -= vp_x;
	mouse_position.y -= vp_y;

	bool mouseover_any = false;
	if (gameState == Pause_Screen || gameState == Tutorial_Instruction_Pause_Screen)
	{
		for (int i = MENU_BUTTON_BACK - 1; i >= 0; i--)
		{
			MainMenuElement &m = registry.menu_btns.get(registry.menu_btns.entities[i]);
			if (!mouseover_any && m.is_active && point_AABB_intersect(vec2(mouse_position.x / window_width * window_width_px, mouse_position.y / window_height * window_height_px), m.position, m.size))
			{
				m.mouseover = true;
				mouseover_any = true;
			}
			else
			{
				m.mouseover = false;
			}
		}
	}
	else if (gameState == Help_Screen)
	{
		MainMenuElement& m = registry.menu_btns.get(registry.menu_btns.entities[MENU_BUTTON_BACK]);
		if (!mouseover_any && m.is_active && point_AABB_intersect(vec2(mouse_position.x / window_width * window_width_px, mouse_position.y / window_height * window_height_px), m.position, m.size))
		{
			m.mouseover = true;
			mouseover_any = true;
		}
		else
		{
			m.mouseover = false;
		}
	}
	else if (gameState == Start_Screen)
	{
		for (int i = MAIN_BUTTON_COUNT - 1; i >= 0; i--)
		{
			MainMenuElement &m = registry.main_menu_btns.get(registry.main_menu_btns.entities[i]);
			if (!mouseover_any && m.is_active && point_AABB_intersect(vec2(mouse_position.x / window_width * window_width_px, mouse_position.y / window_height * window_height_px), m.position, m.size))
			{
				m.mouseover = true;
				registry.texts.components[i + TEXT_COUNT].is_highlighted = true;
				mouseover_any = true;
			}
			else
			{
				m.mouseover = false;
				registry.texts.components[i + TEXT_COUNT].is_highlighted = false;
			}
		}
	}
	else if (gameState == Level_Selection)
	{
		for (int i = LEVEL_SELECTION_COUNT - 1; i >= 0; i--)
		{
			MainMenuElement &m = registry.level_selection_btns.get(registry.level_selection_btns.entities[i]);
			if (!mouseover_any && m.is_active && point_AABB_intersect(vec2(mouse_position.x / window_width * window_width_px, mouse_position.y / window_height * window_height_px), m.position, m.size))
			{
				m.mouseover = true;
				registry.texts.components[i + TEXT_COUNT + MAIN_BUTTON_COUNT].is_highlighted = true;
			}
			else
			{
				m.mouseover = false;
				registry.texts.components[i + TEXT_COUNT + MAIN_BUTTON_COUNT].is_highlighted = false;
			}
		}
		for (int i = max_selection - 1; i >= 0; i--)
		{
			LevelSelectionElement &m = registry.level_selections.components[i];
			if (point_AABB_intersect(vec2(mouse_position.x / window_width * window_width_px, mouse_position.y / window_height * window_height_px), m.position, m.size))
			{
				m.mouseover = true;
			}
			else
			{
				m.mouseover = false;
			}
		}
	}
}

void WorldSystem::on_mouse_action(int button, int action, int mod)
{
	if (gameState >= End_1 && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		gameState = Start_Screen;
	}

	// getting cursor position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		// getting window size
		int window_width, window_height;
		glfwGetFramebufferSize(window, &window_width, &window_height);

#if __APPLE__
		window_width /= 2;
		window_height /= 2;
#endif

		float targetRatio = (float)window_width_px / (float)window_height_px;

		int width = window_width;
		int height = (int)(width / targetRatio);

		if (height > window_height)
		{
			height = window_height;
			width = (int)(height * targetRatio);
		}

		int vp_x = (window_width / 2) - (width / 2);
		int vp_y = (window_height / 2) - (height / 2);

		window_width -= vp_x * 2;
		window_height -= vp_y * 2;
		xpos -= vp_x;
		ypos -= vp_y;

		if (gameState == Pause_Screen || gameState == Tutorial_Instruction_Pause_Screen)
		{
			for (int i = MENU_BUTTON_COUNT - 2; i >= 0; i--)
			{
				MainMenuElement m = registry.menu_btns.components[i];
				if (m.is_active && point_AABB_intersect(vec2(xpos / window_width * window_width_px, ypos / window_height * window_height_px), m.position, m.size))
				{
					if (i == MENU_MAIN_MENU)
					{
						delete_current_level();
						back_to_mainmenu();
					}
					if (i == MENU_BUTTON_HELP)
					{
						gameState = Help_Screen;
					}
					if (i == MENU_BUTTON_RESUME)
					{
						change_gamestate_to_play();
					}
					break;
				}
			}
		}
		else if (gameState == Help_Screen)
		{
			MainMenuElement m = registry.menu_btns.components[MENU_BUTTON_BACK];
			if (m.is_active && point_AABB_intersect(vec2(xpos / window_width * window_width_px, ypos / window_height * window_height_px), m.position, m.size))
			{
				gameState = Pause_Screen;
			}
		}
		else if (gameState == Start_Screen)
		{
			for (int i = MAIN_BUTTON_COUNT - 1; i >= 0; i--)
			{
				MainMenuElement m = registry.main_menu_btns.components[i];
				if (m.is_active && point_AABB_intersect(vec2(xpos / window_width * window_width_px, ypos / window_height * window_height_px), m.position, m.size))
				{
					if (i == MAIN_BUTTON_EXIT)
					{
						// handle quit
						while (registry.menu_btns.size() > 0)
						{
							registry.menu_btns.remove(registry.menu_btns.entities.back());
						}
						while (registry.main_menu_btns.size() > 0)
						{
							registry.main_menu_btns.remove(registry.main_menu_btns.entities.back());
						}
						while (registry.level_selection_btns.size() > 0)
						{
							registry.level_selection_btns.remove(registry.level_selection_btns.entities.back());
						}
						while (registry.level_selections.size() > 0)
						{
							registry.level_selections.remove(registry.level_selections.entities.back());
						}
						while (registry.texts.size() > 0)
						{
							registry.texts.remove(registry.texts.entities.back());
						}
						while (registry.pause_text.size() > 0)
						{
							registry.pause_text.remove(registry.pause_text.entities.back());
						}
						while (registry.help_text.size() > 0)
						{
							registry.help_text.remove(registry.help_text.entities.back());
						}
						while (registry.scores.size() > 0)
						{
							registry.scores.remove(registry.scores.entities.back());
						}
						registry.list_all_components();
						gameState = Game_Exit;
						glfwSetWindowShouldClose(window, true);
						break;
					}
					if (i == MAIN_BUTTON_RANDOM_START)
					{
						// handle start
						gen_levels();
						is_practice = false;
						is_tutorial = false;
						level_number = 1;
						points = 0;
						update_points_message();
						time = 0;
						deaths = 0;
						//points_before_curr_level = 0;
						//registry.texts.components[TEXT_POINTS].str = "POINTS: " + to_string(points);
						build_level_with_string();
						for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
						{
							registry.texts.components[j + 2].is_active = false;
						}
						change_gamestate_to_play();
						deleteSaveData();
						break;
					}
					if (i == MAIN_BUTTON_LEVEL_SELECTION)
					{
						std::set<int> unlocked_levels = load_unlocked_levels();
						int j = max_level - 1;
						for (int i = max_selection - max_level; i < max_selection; i++)
						{
							bool is_unlocked = unlocked_levels.find(j) != unlocked_levels.end();
							registry.level_selections.components[i].is_unlocked = is_unlocked;
							j--;
						}
						gameState = Level_Selection;
						for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
						{
							registry.main_menu_btns.components[j].is_active = false;
							registry.texts.components[j + 2].is_active = false;
						}
						registry.level_selection_btns.components[LEVEL_SELECTION_BACK].is_active = true;
						registry.texts.components[2 + MAIN_BUTTON_COUNT].is_active = true;
						break;
					}

					if (i == MAIN_BUTTON_LOAD_GAME)
					{
						if (canLoadGame())
						{
							gen_levels();
							loadGame();
							for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
							{
								registry.texts.components[j + 2].is_active = false;
							}
							change_gamestate_to_play();
						}
						else
						{
							renderer->draw_unable_load_time = renderer->SAVE_LOAD_DRAW_TIME;
						}
						break;
					}

                    if (i == MAIN_BUTTON_TUTORIAL) {
                        renderer->tutorial_instruction_number = 1;
                        gameState = Tutorial_Instruction;
						is_practice = false;
                        is_tutorial = true;
                        tutorial_level_number = 0;
                        for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
                        {
                            registry.texts.components[j + 2].is_active = false;
                        }
                        break;
                    }

					break;
				}
			}
		}
		else if (gameState == Level_Selection)
		{
			for (int i = LEVEL_SELECTION_COUNT - 1; i >= 0; i--)
			{
				MainMenuElement m = registry.level_selection_btns.components[i];
				if (m.is_active && point_AABB_intersect(vec2(xpos / window_width * window_width_px, ypos / window_height * window_height_px), m.position, m.size))
				{
					if (i == LEVEL_SELECTION_BACK)
					{
						gameState = Start_Screen;
						for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
						{
							registry.main_menu_btns.components[j].is_active = true;
							registry.texts.components[j + 2].is_active = true;
						}
						registry.level_selection_btns.components[LEVEL_SELECTION_BACK].is_active = false;
						registry.texts.components[2 + MAIN_BUTTON_COUNT].is_active = false;
						break;
					}
					break;
				}
			}
			for (int i = max_selection - 1; i >= 0; i--)
			{
				LevelSelectionElement m = registry.level_selections.components[i];
				if (point_AABB_intersect(vec2(xpos / window_width * window_width_px, ypos / window_height * window_height_px), m.position, m.size))
				{
					if (m.is_implemented && m.is_unlocked)
					{
						registry.level_selection_btns.components[LEVEL_SELECTION_BACK].is_active = false;
						registry.texts.components[2 + MAIN_BUTTON_COUNT].is_active = false;
						is_practice = true;
						points = 0;
						update_points_message();
						time = 0;
						deaths = 0;
						//points_before_curr_level = 0;
						//registry.texts.components[TEXT_POINTS].str = "POINTS: " + to_string(points);
						if (m.level >= 100)
						{
							is_tutorial = true;
							tutorial_level_number = m.level - 100;
							renderer->tutorial_instruction_number = tutorial_level_number + 1;
							build_tutorial_level();
						}
						else
						{
							is_tutorial = false;
							level_number = m.level + 1;
							build_level_with_number();
						}
						change_gamestate_to_play();
						deleteSaveData();
						break;
					}
					else
					{
						Mix_PlayChannel(-1, swap_denied, 0);
					}
				}
			}
		}
	}
}

void handle_state_input(int key, int action)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && (gameState == Level_Active || gameState == Pause_Screen || gameState == Help_Screen || gameState == Tutorial_Instruction_Pause_Screen))
	{
		if (gameState == Level_Active)
		{
			registry.texts.components[TEXT_TIME].is_active = false;
			registry.texts.components[TEXT_POINTS].is_active = false;
			gameState = is_tutorial ? Tutorial_Instruction_Pause_Screen: Pause_Screen;
		}
		else
		{
			movement_key[0] = false;
			movement_key[1] = false;
			movement_key[2] = false;
			movement_key[3] = false;
			registry.texts.components[TEXT_TIME].is_active = true;
			registry.texts.components[TEXT_POINTS].is_active = true;
			gameState = Level_Active;
		}
	}
}

// Handle the arrow key inputs to set and unset movement_key and arrow_key_pressed_prev_frame
// as necessary
void handle_movement_input(int key, int action)
{
	if (key == GLFW_KEY_W && (gameState == Level_Active || gameState == Pause_Screen))
	{
		if (action == GLFW_RELEASE)
		{
			movement_key[0] = false;
			jumpTimer = 2.5f; // Removed in other branches anyways
		}
		else
		{
			jumpTimer -= 0.5f;
			movement_key[0] = true;
		}
	}

	if (key == GLFW_KEY_S && (gameState == Level_Active || gameState == Pause_Screen))
	{
		if (action == GLFW_RELEASE)
		{
			movement_key[1] = false;
		}
		else
		{
			movement_key[1] = true;
		}
	}

	if (key == GLFW_KEY_A && (gameState == Level_Active || gameState == Pause_Screen))
	{
		if (action == GLFW_RELEASE)
		{
			movement_key[2] = false;
		}
		else
		{
			movement_key[2] = true;
			facingRight = false;
		}
	}

	if (key == GLFW_KEY_D && (gameState == Level_Active || gameState == Pause_Screen))
	{
		if (action == GLFW_RELEASE)
		{
			movement_key[3] = false;
		}
		else
		{
			movement_key[3] = true;
			facingRight = true;
		}
	}
}

void WorldSystem::level_test1()
{
	renderer->use_tilesheet(TILESHEET_LABTHEME);
	load_map("test_platforms");
}

void WorldSystem::level_test_no_shift()
{
	renderer->use_tilesheet(TILESHEET_CAVETHEME);
	load_map("test_keylock");
}

void WorldSystem::load_level(const std::string& level)
{
    if (is_tutorial) {
        renderer->use_tilesheet(TILESHEET_LABTHEME);
    } else {
		int level_num = std::stoi(level);
		switch (level_num % 3) {
			case 0: renderer->use_tilesheet(TILESHEET_LABTHEME); break;
			case 1: renderer->use_tilesheet(TILESHEET_CAVETHEME); break;
			case 2: renderer->use_tilesheet(TILESHEET_FORESTTHEME); break;
		}
	}

    load_map(level);
//	renderer->use_tilesheet(TILESHEET_BASETHEME);
//	if (level == to_string(1))
//	{
//		renderer->use_tilesheet(TILESHEET_FOODTHEME);
//	}
//	load_map(level);
//	if (level == to_string(2))
//	{
//		std::vector<vec2> loc1 = {vec2(150.f, 380.f), vec2(150.f, 200.f)};
//		createPatrolEnemy({350.f, 380.f}, {32, 32}, 1250.f, loc1, glm::vec3(0.988f, 0.f, 0.353f), 0);
//		std::vector<vec2> loc2 = {vec2(600.f, 275.f), vec2(400.f, 275.f), vec2(400.f, 50.f)};
//		createPatrolEnemy({600.f, 50.f}, {24, 24}, 1000.f, loc2, glm::vec3(0.94f, 0.0f, 0.0f), 0);
//	}
}

void WorldSystem::gen_levels()
{
	// uniform_int_dist(rng);
	level_order.clear();
	for (int i = 0; i < max_level; i++)
	{
		level_order.push_back(i);
	}
	std::shuffle(level_order.begin(), level_order.end(), rng);
}

void WorldSystem::build_tutorial_level()
{
    jumpTimer = JUMP_TIMER;
    load_level("tutorial" + to_string(tutorial_level_number));
    reset_player();
}

void WorldSystem::build_level_with_string()
{
	jumpTimer = JUMP_TIMER;
	load_level(to_string(level_order[level_number - 1]));
	reset_player();
}

void WorldSystem::build_level_with_number()
{
	jumpTimer = JUMP_TIMER;
	load_level(to_string(level_number - 1));
	reset_player();
}

void WorldSystem::change_level()
{
	delete_current_level();
	// build_level();
	build_level_with_string();
	reset_player();
}

void WorldSystem::reset_level()
{
	delete_current_level();
	// build_level();
	if (is_practice)
	{
		build_level_with_number();
	}
    else if (is_tutorial) {
        build_tutorial_level();
    }
	else
	{
		build_level_with_string();
	}
	reset_player();
	update_points_message();
}

void WorldSystem::delete_current_level()
{
	current_layer = 0;
	registry.list_all_components();
	printf("Restarting\n");

	// Remove all entities that we created
	while (registry.motions.entities.size() > 0)
	{
		registry.remove_all_components_of(registry.motions.entities.back());
	}

	while (registry.lock_regions.size() > 0)
	{
		registry.lock_regions.remove(registry.lock_regions.entities.back());
	}

	// registry.list_all_components_of(registry.layers[0].current_motions.entities[0]);
	std::cout << "REGISTRY LAYER 0 COUNT " << registry.layers[0]->current_motions.entities.size() << "\n";
	while (registry.layers[0]->current_motions.entities.size() > 0)
	{
		registry.remove_all_components_of(registry.layers[0]->current_motions.entities.back());
	}
	std::cout << "REGISTRY LAYER 0 COUNT AFTER " << registry.layers[0]->current_motions.entities.size() << "\n";

	std::cout << "REGISTRY LAYER 1 COUNT " << registry.layers[1]->current_motions.entities.size() << "\n";
	while (registry.layers[1]->current_motions.entities.size() > 0)
	{
		registry.remove_all_components_of(registry.layers[1]->current_motions.entities.back());
	}
	std::cout << "REGISTRY LAYER 1 COUNT AFTER " << registry.layers[1]->current_motions.entities.size() << "\n";

	// Debugging for memory/component leaks
	registry.list_all_components();

    collected_ids.clear();
}

void WorldSystem::reset_player()
{
	// Reset companions
	while (registry.companions.size() > 0)
	{
		registry.remove_all_components_of(registry.companions.entities.back());
	}

	Player &pl = registry.players.components[0];
	Entity &pl_entity = registry.players.entities[0];
	Motion &player_motion = registry.motions.get(pl_entity);
	player_motion.velocity = vec2(0, 0);
	if (!registry.deathTimers.has(pl_entity))
	{
		player_motion.position = pl.start_pos;
		player_motion.prev_position = pl.start_pos;
	}

	pl.keys = 0;

	makePresent(registry.lock_tiles);
//	makePresent(registry.collectibles);
	makePresent(registry.keys);
}

void WorldSystem::load_levels()
{
	Level toAdd;
}

void WorldSystem::change_gamestate_to_help()
{
	registry.texts.components[TEXT_TIME].is_active = false;
	registry.texts.components[TEXT_POINTS].is_active = false;
	gameState = is_tutorial ? Tutorial_Instruction_Pause_Screen: Pause_Screen;
}

void WorldSystem::change_gamestate_to_play()
{
	movement_key[0] = false;
	movement_key[1] = false;
	movement_key[2] = false;
	movement_key[3] = false;
	registry.texts.components[TEXT_TIME].is_active = true;
	registry.texts.components[TEXT_POINTS].is_active = true;
	gameState = Level_Active;
}

void WorldSystem::back_to_mainmenu()
{
    is_tutorial = false;
	level_number = 1;
	gameState = Start_Screen;
	for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
	{
		registry.main_menu_btns.components[j].is_active = true;
		registry.texts.components[j + 2].is_active = true;
	}
	registry.texts.components[TEXT_TIME].is_active = false;
	registry.texts.components[TEXT_POINTS].is_active = false;
}

// For mouse clicks
void respond_to_back_button()
{
	level_number = 1;
	gameState = Start_Screen;
	for (int j = 0; j < MAIN_BUTTON_COUNT; j++)
	{
		registry.main_menu_btns.components[j].is_active = true;
		registry.texts.components[j + 2].is_active = true;
	}
	registry.texts.components[TEXT_TIME].is_active = false;
	registry.texts.components[TEXT_POINTS].is_active = false;
}

void respond_to_resume_button()
{
	registry.texts.components[TEXT_TIME].is_active = true;
	registry.texts.components[TEXT_POINTS].is_active = true;
	gameState = Level_Active;
}

void WorldSystem::setup_menus()
{
	vec2 menu_pos = vec2(window_width_px / 5, window_height_px * 3 / 4);
	vec2 menu_size = vec2(window_width_px / 13.333f, window_width_px / 13.333f / 72.f * 20.f);
	vec2 text_size = vec2(8, 8);

	createMenu(menu_pos + vec2(0.f, menu_size.y * 4), menu_size, true, false);
	createPauseText("HOME", menu_pos + vec2(-3.5f, menu_size.y * 4) - vec2(menu_size.x / 3, 1), text_size, true, false);
	createMenu(menu_pos + vec2(0.f, menu_size.y * 2), menu_size, true, false);
	createPauseText("HELP", menu_pos + vec2(-3.5f, menu_size.y * 2) - vec2(menu_size.x / 3, 1), text_size, true, false);
	createMenu(menu_pos, menu_size, true, false);
	createPauseText("RESUME", menu_pos - vec2(menu_size.x / 3 + 3.5f, 1), text_size, true, false);
	createMenu(menu_pos + vec2(0.f, menu_size.y * 4), menu_size, true, false);
	createHelpText("BACK", menu_pos + vec2(-3.5f, menu_size.y * 4) - vec2(menu_size.x / 3, 1), text_size, true, false);
}

void WorldSystem::setup_main_menus()
{
	vec2 menu_pos = vec2(window_width_px / 5, window_height_px * 3 / 4);
	vec2 menu_size = vec2(window_width_px / 13.333f, window_width_px / 13.333f / 72.f * 20.f);
	vec2 text_size = vec2(7.5, 7.5);

	main_menu_buttons[MAIN_BUTTON_EXIT] = createMainMenu(menu_pos + vec2(0.f, menu_size.y * 4), menu_size, true, false);
	createText("EXIT", menu_pos + vec2(0.f, menu_size.y * 4) - vec2(menu_size.x / 3 + 2.2f, 1), text_size, true, false);

	main_menu_buttons[MAIN_BUTTON_LOAD_GAME] = createMainMenu(menu_pos, menu_size, true, false);
	createText("LOAD", menu_pos - vec2(menu_size.x / 3 + 2.2f, 1), text_size, true, false);

	main_menu_buttons[MAIN_BUTTON_LEVEL_SELECTION] = createMainMenu(menu_pos + vec2(0.f, menu_size.y * 2), menu_size, true, false);
	createText("LEVELS", menu_pos + vec2(0.f, menu_size.y * 2) - vec2(menu_size.x / 3 + 2.2f, 1), text_size, true, false);

    main_menu_buttons[MAIN_BUTTON_RANDOM_START] = createMainMenu(menu_pos - vec2(0.f, menu_size.y * 2), menu_size, true, false);
    createText("START", menu_pos - vec2(0.f, menu_size.y * 2) - vec2(menu_size.x / 3 + 2.2f, 1), text_size, true, false);

    main_menu_buttons[MAIN_BUTTON_TUTORIAL] = createMainMenu(menu_pos - vec2(0.f, menu_size.y * 4), menu_size, true, false);
    createText("TUTORIAL", menu_pos - vec2(0.f, menu_size.y * 4) - vec2(menu_size.x / 3 + 2.2f, 1), text_size, true, false);

	menu_pos = menu_pos + vec2(window_width_px / 5.7, menu_size.y / 3.f * 2.f);
	menu_size = vec2(menu_size.y * 8.f / 140.f * 94.f, menu_size.y * 8.f);

	createMainMenu(menu_pos, menu_size, true, false);
	createMainMenu(menu_pos + vec2(window_width_px / 5, 0.f), menu_size, true, false);
	createMainMenu(menu_pos + vec2(2.f * window_width_px / 5, 0.f), menu_size, true, false);
	setup_scoreboard();
	setup_times();
	setup_scores();
	setup_titles();
}

void WorldSystem::setup_scoreboard()
{
	while (registry.scores.size() > 0)
	{
		registry.scores.remove(registry.scores.entities.back());
	}
	vec2 menu_pos = vec2(window_width_px / 5.7, window_height_px * 3 / 4);
	vec2 menu_size = vec2(window_width_px / 13.333f, window_width_px / 13.333f / 72.f * 20.f);
	vec2 text_size = vec2(10, 10);
	vec2 score_pos = menu_pos + vec2(window_width_px / 5 - menu_size.x / 2.f, menu_size.y * 3.5f);

	std::vector<int> hs = load_highscores();
	for (int i = 0; i < 5; i++)
	{
		createScore("-", score_pos - vec2(0.f, (float)i * menu_size.y * 1.31f), text_size);
	}
	for (int i = 0; i < hs.size(); i++)
	{
		registry.scores.components[4 - i].str = to_string(hs[i]);
	}
}

void WorldSystem::setup_times()
{
	vec2 menu_pos = vec2(window_width_px / 5.7, window_height_px * 3 / 4) + vec2(window_width_px / 5, 0.f);
	vec2 menu_size = vec2(window_width_px / 13.333f, window_width_px / 13.333f / 72.f * 20.f);
	vec2 text_size = vec2(10, 10);
	vec2 score_pos = menu_pos + vec2(window_width_px / 5 - menu_size.x / 2.f, menu_size.y * 3.5f);

	std::vector<int> hs = load_time();
	for (int i = 0; i < 5; i++)
	{
		createScore("-", score_pos - vec2(0.f, (float)i * menu_size.y * 1.31f), text_size);
	}
	for (int i = 0; i < hs.size(); i++)
	{
		registry.scores.components[9 - i].str = to_string(hs[i] / 60000) + "m" + to_string((hs[i] / 1000) % 60) + "s";
	}
}

void WorldSystem::setup_scores()
{
	vec2 menu_pos = vec2(window_width_px / 5.7, window_height_px * 3 / 4) + vec2(window_width_px / 5, 0.f) * 2.f;
	vec2 menu_size = vec2(window_width_px / 13.333f, window_width_px / 13.333f / 72.f * 20.f);
	vec2 text_size = vec2(10, 10);
	vec2 score_pos = menu_pos + vec2(window_width_px / 5 - menu_size.x / 2.f, menu_size.y * 3.5f);

	std::vector<int> hs = load_score();
	for (int i = 0; i < 5; i++)
	{
		createScore("-", score_pos - vec2(0.f, (float)i * menu_size.y * 1.31f), text_size);
	}
	for (int i = 0; i < hs.size(); i++)
	{
		registry.scores.components[14 - i].str = to_string(hs[i]);
	}
}

void WorldSystem::setup_titles()
{
	vec2 text_size = vec2(7.5, 7.5);
	vec2 menu_pos = vec2(window_width_px / 5.7, window_height_px * 3 / 4);
	vec2 menu_size = vec2(window_width_px / 13.333f, window_width_px / 13.333f / 72.f * 20.f);
	vec2 score_pos = menu_pos + vec2(window_width_px / 5 - menu_size.x / 3.5f, menu_size.y * 3.5f) - vec2(0.f, 4.9f * menu_size.y * 1.31f);
	createScore("COINS", score_pos, text_size);
	createScore("TIME", score_pos + vec2(window_width_px / 5, 0.f) + vec2(4.f, 0.f), text_size);
	createScore("SCORE", score_pos + vec2(window_width_px / 5, 0.f) * 2.f, text_size);
}

void WorldSystem::setup_level_selection()
{
	vec2 menu_pos = vec2(window_width_px / 10, window_height_px / 10);
	vec2 menu_size = vec2(72, 20);
	vec2 text_size = vec2(10, 10);
	vec2 thumbnail_size = vec2(window_width_px / 9, window_width_px / 16);
	vec2 bottom_right = vec2(window_width_px - window_width_px / 18 * 3, window_height_px - window_width_px / 16);
	float horizontal_gap = window_width_px / 18;
	float vertical_gap = window_width_px / 32;

	createLevelsMenu(menu_pos, menu_size, false, false);
	createText("BACK", menu_pos - vec2(menu_size.x / 3, 1), text_size, false, false);

	std::set<int> unlocked_levels = load_unlocked_levels();

	// WIP levels
	/*for (int i = 0; i < max_selection - max_level; i++)
	{
		bool is_unlocked = unlocked_levels.find(i) != unlocked_levels.end();
		createLevelSelection(bottom_right - vec2((i % 5) * (horizontal_gap + thumbnail_size.x), (i / 5) * (vertical_gap + thumbnail_size.y)), thumbnail_size, -1, false, is_unlocked, false);
	}*/
	// tutorial levels
	for (int i = 0; i < max_selection - max_level; i++)
	{
		createLevelSelection(bottom_right - vec2((i % 5) * (horizontal_gap + thumbnail_size.x), (i / 5) * (vertical_gap + thumbnail_size.y)), thumbnail_size, 100+(3 - i), true, true, false);
	}
	// regular levels
	int j = max_level - 1;
	for (int i = max_selection - max_level; i < max_selection; i++)
	{
		bool is_unlocked = unlocked_levels.find(j) != unlocked_levels.end();
		createLevelSelection(bottom_right - vec2((i % 5) * (horizontal_gap + thumbnail_size.x), (i / 5) * (vertical_gap + thumbnail_size.y)), thumbnail_size, j, true, is_unlocked, false);
		j--;
	}
}

bool WorldSystem::safe_to_layer_swap(int next_layer)
{
	auto check_collision = [](Motion &one, Motion &two)
	{
		vec2 A_one = vec2(one.position.x - (one.scale.x / 2.0), one.position.y - (one.scale.y / 2.0));
		vec2 A_two = vec2(two.position.x - (two.scale.x / 2.0), two.position.y - (two.scale.y / 2.0));

		// Bottom right corner
		vec2 B_one = vec2(one.position.x + (one.scale.x / 2.0), one.position.y + (one.scale.y / 2.0));
		vec2 B_two = vec2(two.position.x + (two.scale.x / 2.0), two.position.y + (two.scale.y / 2.0));

		// collision x-axis?
		bool collisionX = A_one.x < B_two.x &&
						  A_two.x < B_one.x;
		// collision y-axis?
		bool collisionY = A_one.y < B_two.y &&
						  A_two.y < B_one.y;

		// collision only if on both axes
		return collisionX && collisionY;
	};

	Entity &pl_entity = registry.players.entities[0];
	Motion &player_motion = registry.motions.get(pl_entity);

	for (uint i = 0; i < registry.layers[next_layer]->current_motions.components.size(); i++)
	{
		Motion &motion_i = registry.layers[next_layer]->current_motions.components[i];
		Entity entity_i = registry.layers[next_layer]->current_motions.entities[i];
		if (pl_entity == entity_i || registry.decorations.has(entity_i) || registry.collectibles.has(entity_i) ||
            registry.keys.has(entity_i) || !isPresent(entity_i, registry.lock_tiles))
		{
			continue;
		}

		if (check_collision(player_motion, motion_i))
		{
			Mix_PlayChannel(-1, swap_denied, 0);
			return false;
		}
	}
	Mix_PlayChannel(-1, swap_success, 0);
	return true;
}

void WorldSystem::saveGame()
{
	if (renderer->cooldown_between_save_and_load > 0.0f)
		return;
	renderer->cooldown_between_save_and_load = renderer->COOLDOWN_BETWEEN_SAVE_AND_LOAD_TIME;
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/latest_save.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	jsonfile["has_saved"] = true;
	jsonfile["level"] = level_number;
	//    jsonfile["layer"] = current_layer;
	jsonfile["level_order"] = level_order;

	//    Entity &pl_entity = registry.players.entities[0];
	//    Motion& player_motion = registry.motions.get(pl_entity);
	//    jsonfile["x_pos"] = player_motion.position.x;
	//    jsonfile["y_pos"] = player_motion.position.y;
    jsonfile["points"] = points;
	jsonfile["time"] = time;
	jsonfile["death"] = deaths;
    jsonfile["collected_ids"] = collected_ids;


    std::ofstream file(file_string);
	file << jsonfile;

	renderer->draw_save_message_time = renderer->SAVE_LOAD_DRAW_TIME;
	renderer->draw_load_message_time = 0.0f;
}

void WorldSystem::loadGame()
{
	if (renderer->cooldown_between_save_and_load > 0.0f)
		return;
	renderer->cooldown_between_save_and_load = renderer->COOLDOWN_BETWEEN_SAVE_AND_LOAD_TIME;
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/latest_save.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	// Nothing to load if we have never saved
	if (!jsonfile.contains("has_saved") || jsonfile["has_saved"] == false)
	{
		renderer->draw_unable_load_time = renderer->SAVE_LOAD_DRAW_TIME;
		renderer->draw_load_message_time = 0.0f;
		renderer->draw_save_message_time = 0.0f;
		return;
	}

	points = jsonfile["points"];
	time = jsonfile["time"];
	deaths = jsonfile["death"];
	level_number = jsonfile["level"];
	level_order = jsonfile["level_order"].get<std::vector<int>>();

	delete_current_level();
	build_level_with_string();

	reset_player(); // I may remove this after M3

    collected_ids = jsonfile["collected_ids"].get<std::vector<int>>();
    std::set<int> s(collected_ids.begin(), collected_ids.end());
    for(auto & c : registry.collectibles.components) {
        if (s.find(c.id) != s.end()) {
            c.isPresent = false;
        }
    }
    update_points_message();

	renderer->draw_load_message_time = renderer->SAVE_LOAD_DRAW_TIME;
	renderer->draw_save_message_time = 0.0f;
}

bool WorldSystem::canLoadGame()
{
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/latest_save.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	// Nothing to load if we have never saved
	if (!jsonfile.contains("has_saved") || jsonfile["has_saved"] == false)
		return false;
	else
		return true;
}

void WorldSystem::deleteSaveData()
{
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/latest_save.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	jsonfile["has_saved"] = false;
	std::ofstream file(file_string);
	file << jsonfile;
}

void WorldSystem::save_unlocked_level(int level)
{
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/unlocked_levels.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

    std::vector<int> unlocked_levels;
    if (jsonfile.contains("levels")) unlocked_levels = jsonfile["levels"].get<std::vector<int>>();
    if (std::find(unlocked_levels.begin(), unlocked_levels.end(), level) == unlocked_levels.end()) unlocked_levels.push_back(level);
    jsonfile["levels"] = unlocked_levels;
    std::ofstream file(file_string);
    file << jsonfile;
}

std::set<int> WorldSystem::load_unlocked_levels()
{
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/unlocked_levels.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	std::set<int> unlocked_levels = jsonfile["levels"].get<std::set<int>>();
	return unlocked_levels;
}

void WorldSystem::update_points_message() {
    registry.texts.components[TEXT_POINTS].str = "POINTS: " + to_string(points);
}

void WorldSystem::reset_movement_key() {
    movement_key[0] = false;
    movement_key[1] = false;
    movement_key[2] = false;
    movement_key[3] = false;
}

void WorldSystem::save_highscores(int score) {
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/highscores.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	std::vector<int> highscores = jsonfile["highscores"];
	if (highscores.size() < 5)
	{
		highscores.push_back(score);
	}
	else
	{
		if (score > highscores[4])
		{
			highscores[4] = score;
		}
	}
	sort(highscores.begin(), highscores.end(), greater<int>());
	jsonfile["highscores"] = highscores;
	std::ofstream file(file_string);
	file << jsonfile;
}

std::vector<int> WorldSystem::load_highscores() {
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/highscores.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	std::vector<int> highscores = jsonfile["highscores"];
	return highscores;
}

void WorldSystem::save_time(int score) {
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/time.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	std::vector<int> highscores = jsonfile["time"];
	if (highscores.size() < 5)
	{
		highscores.push_back(score);
	}
	else
	{
		if (score < highscores[0])
		{
			highscores[0] = score;
		}
	}
	sort(highscores.begin(), highscores.end());
	jsonfile["time"] = highscores;
	std::ofstream file(file_string);
	file << jsonfile;
}

std::vector<int> WorldSystem::load_time() {
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/time.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	std::vector<int> highscores = jsonfile["time"];
	return highscores;
}

void WorldSystem::save_score(int score, uint64_t time, uint64_t deaths) {
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/scores.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	std::vector<int> highscores = jsonfile["highscores"];
	int new_score = std::max((int)(1000000000 / (double)time + score * 100 - deaths * 50), 0);
	if (highscores.size() < 5)
	{
		highscores.push_back(new_score);
	}
	else
	{
		if (new_score > highscores[4])
		{
			highscores[4] = new_score;
		}
	}
	sort(highscores.begin(), highscores.end(), greater<int>());
	jsonfile["highscores"] = highscores;
	std::ofstream file(file_string);
	file << jsonfile;
}

std::vector<int> WorldSystem::load_score() {
	ifstream f;
	std::string file_string = string(PROJECT_SOURCE_DIR) + "data/scores.json";
	f.open(file_string);
	json jsonfile = json::parse(f);

	std::vector<int> highscores = jsonfile["highscores"];
	return highscores;
}
