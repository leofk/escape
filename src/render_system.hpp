#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

#include "Utility/SpriteRenderer.hpp"
#include "Utility/SpriteSheetRenderer.hpp"
#include "Utility/ParticleGenerator.hpp"

struct TilesheetParams
{
	std::string texture_name;
	int width;
	int height;

	TilesheetParams(std::string n, int w, int h)
		: texture_name(std::move(n)), width(w), height(h) {}
};

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
		{
			std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::SPIKE, mesh_path("spikesimple.obj"))
			// specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("fish.png"),
		textures_path("turtle.png")};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("spikes")};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;
	SpriteRenderer *Renderer;

	SpriteSheetRenderer *SheetRenderer;
	SpriteSheetRenderer *PointLightRenderer;

	GLuint general_VAO;

public:
	// Initialize the window
	bool init(GLFWwindow *window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh &getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix(int width, int height);
	mat3 createProjectionMatrix(int width, int height, float scale);

	void draw_box(Entity entity,
				  const mat3 &projection,
				  vec3 colour);

	void initSpriteRenderer();
	void initShaders();
	void initTextures();
	void initThumbnailTextures();
	void initBasicBoxDetails();
	void initTextTextures();
	void draw_cave_background();
	void draw_lab_background();
	void draw_forest_background();
	void point_light(float parallax, Texture2D diffuse, Texture2D normal, vec4 ambient);

	void Draw_Controller(GameState gs);

	void DrawMenu(GameState gs);

	void drawMenuButtons(GameState gs);
	void drawMainMenuButtons();
	void drawLevelSelectionButtons();

	void drawText(GameState gs);

	vec2 res_ratio;

	void draw_player();

	// Tilesheet selection
	void use_tilesheet(int id);

	float draw_save_message_time = 0.0f;
	float draw_load_message_time = 0.0f;
	float draw_unable_load_time = 0.0;
	float cooldown_between_save_and_load = 0.0f;


	const float SAVE_LOAD_DRAW_TIME = 750.0f;
	const float COOLDOWN_BETWEEN_SAVE_AND_LOAD_TIME = 2500.0f;

    int tutorial_instruction_number = 1;

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3 &projection);
	void drawToScreen();
	void draw_animated();
	Transform applyTransformations(const Motion &motion);
	void render_text(vec4 color, float scale, float xpos, float ypos, const char *message);

	GLuint quadVAO;
	void initRenderData();

	vec2 player_pos;
	float spot_max;
	float xDiff;

	// Window handle
	GLFWwindow *window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	// id of current tilesheet in use
	int tilesheet_id;
	// Name of current tilesheet in use
	std::string tilesheet_texture_name;
	// Width and height of current tilesheet, in number of tiles
	int tilesheet_width;
	int tilesheet_height;
	// Available tilesheets
	std::vector<TilesheetParams> tilesheet_params;

	void draw_globalanims();

	ParticleGenerator *pg;
};

bool loadEffectFromFile(
	const std::string &vs_path, const std::string &fs_path, GLuint &out_program);
