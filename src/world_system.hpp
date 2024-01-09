#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{

public:
	WorldSystem();

	// Creates a window
	GLFWwindow *create_window();

	// starts the game
	void init(RenderSystem *renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	bool step_UI(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over() const;

	void setup_menus();
	void setup_main_menus();
	void setup_scoreboard();
	void setup_times();
	void setup_scores();
	void setup_titles();
	void setup_level_selection();

    static inline Entity createPoofAnim(vec2 pos, vec2 size) {
        return createOneTimeAnim(pos, size, vec2(0,3), 5, 45.f);
    };
    static inline Entity createPickupAnim(vec2 pos, vec2 size) {
        return createOneTimeAnim(pos, size, vec2(0, 2), 6, 45.f);
    };
    static Entity createOneTimeAnim(vec2 pos, vec2 size, vec2 start_frame, int animFrameLength, float animTimeLength);

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_action(int key, int action, int mod);

	// Change to selected level (level must exist)
	void change_level();
	void reset_level();
	// Reset Player to initial start point
	void reset_player();

    // Save and load game
    void saveGame();
    void loadGame();
    bool canLoadGame();
    void deleteSaveData();

	void delete_current_level();

    void save_unlocked_level(int level);
    std::set<int> load_unlocked_levels();
	void save_highscores(int level);
	std::vector<int> load_highscores();
	void save_time(int score);
	std::vector<int> load_time();
	void save_score(int score, uint64_t time, uint64_t deaths);
	std::vector<int> load_score();
//	void build_level();

	void load_levels();

	void level_test1();
	void level_demo();
    void level_test_no_shift();
//	void level_oliver();
//	void level_xinru();
//	void level_james();
//    void level_ricky();
//    void level_leo();
//    void level_kenji();

	void load_level(const std::string& level);
	void gen_levels();
    void build_tutorial_level();
	void build_level_with_string();
	void build_level_with_number();

    bool safe_to_layer_swap(int next_layer);
    void checkBounds();

	void change_gamestate_to_help();
	void change_gamestate_to_play();

    void reset_movement_key();
	void back_to_mainmenu();

    void update_points_message();

    // OpenGL window handle
	GLFWwindow *window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points = 0;
	uint64_t time = 0;
	uint64_t deaths = 0;

	// Game state
	RenderSystem *renderer;
	Entity player;

	// music references
	Mix_Music *background_music;
	Mix_Chunk *death_sounds[3];
	Mix_Chunk *success_sound;
    Mix_Chunk *consume_effect;
    Mix_Chunk *swap_denied;
    Mix_Chunk *swap_success;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
	std::uniform_int_distribution<int> uniform_int_dist{ 1, 4 };

	const float JUMP_TIMER = 2.5f;

    // used for save load
    std::vector<int> collected_ids;

};
