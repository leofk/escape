// internal
#include "render_system.hpp"

#include <array>
#include <fstream>

#include "../ext/stb_image/stb_image.h"

// This creates circular header inclusion, that is quite bad.
#include "tiny_ecs_registry.hpp"

#include "Utility/ResourceManager.hpp"
// #include "Utility/SpriteRenderer.hpp"
// #include "Utility/SpriteSheetRenderer.hpp"

// stlib
#include <iostream>
#include <sstream>

// World initialization
bool RenderSystem::init(GLFWwindow *window_arg)
{
	this->window = window_arg;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	const int is_fine = gl3w_init();
	assert(is_fine == 0);

	// Create a frame buffer
	frame_buffer = 0;
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// For some high DPI displays (ex. Retina Display on Macbooks)
	// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
	int frame_buffer_width_px, frame_buffer_height_px;
	glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	if (frame_buffer_width_px != window_width_px)
	{
		printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
		printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
		printf("window width_height = %d,%d\n", window_width_px, window_height_px);
	}

	res_ratio = vec2((float)frame_buffer_width_px / window_width_px, (float)frame_buffer_height_px / window_height_px);

	// glDebugMessageCallback((GLDEBUGPROC)errorCallback, nullptr);

	// We are not really using VAO's but without at least one bound we will crash in
	// some systems.
	//	GLuint vao;
	//	glGenVertexArrays(1, &vao);
	//	glBindVertexArray(1);
	gl_has_errors();

	initScreenTexture();
	// initializeGlTextures();
	initializeGlEffects();
	initializeGlGeometryBuffers();

	initShaders();
	initSpriteRenderer();
	initTextures();

	initRenderData();

	initBasicBoxDetails();

	pg = new ParticleGenerator();

	return true;
}

void RenderSystem::initTextures()
{
	//	std::string num_string;
	//	for (int i = 0; i <= 179; i++)
	//	{
	//		if (i < 10)
	//		{
	//			num_string = "000" + std::to_string(i);
	//		}
	//		else if (i < 100)
	//		{
	//			num_string = "00" + std::to_string(i);
	//		}
	//		else
	//		{
	//			num_string = "0" + std::to_string(i);
	//		}
	//		str = std::string(PROJECT_SOURCE_DIR) + "data/tiles/tile_" + num_string + ".png";
	//		ResourceManager::LoadTexture(str.c_str(), true, "tile_" + num_string);
	//	}

	// "Designed by brgfx / Freepik"
	std::string str = textures_path("cave-1.png");
	ResourceManager::LoadTexture(str.c_str(), true, "cave-1");

	str = textures_path("cave-2.png");
	ResourceManager::LoadTexture(str.c_str(), true, "cave-2");

	str = textures_path("cave-3.png");
	ResourceManager::LoadTexture(str.c_str(), true, "cave-3");

	str = textures_path("cave-4.png");
	ResourceManager::LoadTexture(str.c_str(), true, "cave-4");

	str = textures_path("cave-5.png");
	ResourceManager::LoadTexture(str.c_str(), true, "cave-5");

	str = textures_path("cave-6.png");
	ResourceManager::LoadTexture(str.c_str(), false, "cave-6");

	str = textures_path("forest-1.png");
	ResourceManager::LoadTexture(str.c_str(), false, "forest-1");

	str = textures_path("forest-2.png");
	ResourceManager::LoadTexture(str.c_str(), true, "forest-2");

	str = textures_path("forest-3.png");
	ResourceManager::LoadTexture(str.c_str(), true, "forest-3");

	str = textures_path("forest-4.png");
	ResourceManager::LoadTexture(str.c_str(), true, "forest-4");

	str = textures_path("forest-5.png");
	ResourceManager::LoadTexture(str.c_str(), true, "forest-5");

	str = textures_path("forest-5_n.png");
	ResourceManager::LoadTexture(str.c_str(), true, "forest-5_n");

	str = textures_path("mush_1.png");
	ResourceManager::LoadTexture(str.c_str(), true, "mush_1");

	str = textures_path("mush_1_n.png");
	ResourceManager::LoadTexture(str.c_str(), true, "mush_1_n");

	str = textures_path("mush_2.png");
	ResourceManager::LoadTexture(str.c_str(), true, "mush_2");

	str = textures_path("mush_2_n.png");
	ResourceManager::LoadTexture(str.c_str(), true, "mush_2_n");

	str = textures_path("mush_3.png");
	ResourceManager::LoadTexture(str.c_str(), true, "mush_3");

	str = textures_path("mush_3_n.png");
	ResourceManager::LoadTexture(str.c_str(), true, "mush_3_n");

	str = textures_path("mush_4.png");
	ResourceManager::LoadTexture(str.c_str(), true, "mush_4");

	str = textures_path("mush_4_n.png");
	ResourceManager::LoadTexture(str.c_str(), true, "mush_4_n");

	str = textures_path("lab_1.png");
	ResourceManager::LoadTexture(str.c_str(), false, "lab-1");

	str = textures_path("lab_2.png");
	ResourceManager::LoadTexture(str.c_str(), true, "lab-2");

	str = textures_path("lab_3.png");
	ResourceManager::LoadTexture(str.c_str(), true, "lab-3");

	str = textures_path("lab_4.png");
	ResourceManager::LoadTexture(str.c_str(), true, "lab-4");

	str = textures_path("lab_5.png");
	ResourceManager::LoadTexture(str.c_str(), true, "lab-5");

	str = textures_path("lab_6.png");
	ResourceManager::LoadTexture(str.c_str(), true, "lab-6");

	str = textures_path("lab_obj_1.png");
	ResourceManager::LoadTexture(str.c_str(), true, "lab_obj_1");

	str = textures_path("lab_obj_1_n.png");
	ResourceManager::LoadTexture(str.c_str(), true, "lab_obj_1_n");

	str = textures_path("lab_obj_2.png");
	ResourceManager::LoadTexture(str.c_str(), true, "lab_obj_2");

	str = textures_path("lab_obj_2_n.png");
	ResourceManager::LoadTexture(str.c_str(), true, "lab_obj_2_n");

	str = textures_path("global_anim.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Global_Anim");
	gl_has_errors();

	str = textures_path("Walk.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Walk");

	str = textures_path("lab_full.png");
	ResourceManager::LoadTexture(str.c_str(), true, "TileSheet_LabTheme");
	gl_has_errors();
	tilesheet_params.emplace_back(std::string("TileSheet_LabTheme"), 12, 12);

	str = textures_path("cave_full.png");
	ResourceManager::LoadTexture(str.c_str(), true, "TileSheet_CaveTheme");
	gl_has_errors();
	tilesheet_params.emplace_back(std::string("TileSheet_CaveTheme"), 12, 12);

	str = textures_path("forest_tiles.png");
	ResourceManager::LoadTexture(str.c_str(), true, "TileSheet_ForestTheme");
	gl_has_errors();
	tilesheet_params.emplace_back(std::string("TileSheet_ForestTheme"), 12, 12);

	str = textures_path("help.png");
	ResourceManager::LoadTexture(str.c_str(), false, "Help_Screen");
	gl_has_errors();

	str = textures_path("Close_BTN.png");
	ResourceManager::LoadTexture(str.c_str(), true, "CloseButton");
	gl_has_errors();

	str = textures_path("Menu_BTN.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Menu_BTN");
	gl_has_errors();

	str = textures_path("Play_BTN.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Play_BTN");
	gl_has_errors();

	str = textures_path("Backward_BTN.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Back_BTN");
	gl_has_errors();

	str = textures_path("Generic_Button_No_Highlight.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Generic_Long_Button");
	gl_has_errors();

	str = textures_path("Generic_Button_Highlight.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Generic_Long_Button_Highlight");
	gl_has_errors();

	str = textures_path("main_title.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Title_Screen");
	gl_has_errors();

	str = textures_path("levels_with_tutorials.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Levels_Menu");
	gl_has_errors();

	str = textures_path("level_selection_locked.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Level_Locked");
	gl_has_errors();

	str = textures_path("16-9-rect.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Level_Highlight");
	gl_has_errors();

	str = textures_path("Window_2.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Score_Board");
	gl_has_errors();

	str = textures_path("level_selection_WIP.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Level_Empty");
	gl_has_errors();

	str = textures_path("credits_maybe.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Pause_Menu");
	gl_has_errors();

	// str = std::string(PROJECT_SOURCE_DIR) + "data/textures/cursor_default_active.png";
	// ResourceManager::LoadTexture(str.c_str(), true, "Cursor_Default");
	// gl_has_errors();

    str = textures_path("font.png");
    ResourceManager::LoadTexture(str.c_str(), true, "SpriteSheet_Text");
    gl_has_errors();

	str = textures_path("tutorial_instruction1.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Tutorial1");
	gl_has_errors();

	str = textures_path("tutorial_instruction2.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Tutorial2");
	gl_has_errors();

	str = textures_path("tutorial_instruction3.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Tutorial3");
	gl_has_errors();

	str = textures_path("tutorial_instruction4.png");
	ResourceManager::LoadTexture(str.c_str(), true, "Tutorial4");
	gl_has_errors();

	str = textures_path("tutorial_instruction5.png");
	ResourceManager::LoadTexture(str.c_str(), false, "Tutorial5");
	gl_has_errors();

	str = textures_path("end_1.png");
	ResourceManager::LoadTexture(str.c_str(), true, "End_1");
	gl_has_errors();

	str = textures_path("end_2.png");
	ResourceManager::LoadTexture(str.c_str(), true, "End_2");
	gl_has_errors();

	str = textures_path("end_3.png");
	ResourceManager::LoadTexture(str.c_str(), true, "End_3");
	gl_has_errors();

	str = textures_path("end_4.png");
	ResourceManager::LoadTexture(str.c_str(), true, "End_4");
	gl_has_errors();
}

void RenderSystem::initThumbnailTextures()
{
	std::string str;
	for (int i = 0; i < 11; i++)
	{
		str = std::string(PROJECT_SOURCE_DIR) + "data/thumbnails/" + std::to_string(i) + ".png";
		ResourceManager::LoadTexture(str.c_str(), true, "level_" + std::to_string(i));
		gl_has_errors();
	}
}

void RenderSystem::initTextTextures()
{
	std::string str;
	for (char c = 65; c <= 90; c++)
	{
		str = std::string(PROJECT_SOURCE_DIR) + "data/text/" + c + ".png";
		ResourceManager::LoadTexture(str.c_str(), true, std::string(1, c));
		gl_has_errors();
	}
	for (int i = 0; i < 10; i++)
	{
		str = std::string(PROJECT_SOURCE_DIR) + "data/text/" + std::to_string(i) + ".png";
		ResourceManager::LoadTexture(str.c_str(), true, std::to_string(i));
		gl_has_errors();
	}
	str = std::string(PROJECT_SOURCE_DIR) + "data/text/Colon.png";
	ResourceManager::LoadTexture(str.c_str(), true, "Colon");
	gl_has_errors();
	str = std::string(PROJECT_SOURCE_DIR) + "data/text/QMark.png";
	ResourceManager::LoadTexture(str.c_str(), true, "Question_Mark");
	gl_has_errors();
}

void RenderSystem::initShaders()
{
	std::string vs = std::string(PROJECT_SOURCE_DIR) + "shaders/coloured.vs.glsl";
	std::string fs = std::string(PROJECT_SOURCE_DIR) + "shaders/coloured.fs.glsl";
	ResourceManager::LoadShader(vs, fs, "Wall");
	gl_has_errors();

	ResourceManager::LoadShader(vs, fs, "Enemy");
	gl_has_errors();

	ResourceManager::LoadShader(
		std::string(PROJECT_SOURCE_DIR) + "shaders/second_pass.vs.glsl",
		std::string(PROJECT_SOURCE_DIR) + "shaders/second_pass.fs.glsl",
		"second_pass");

	glBindVertexArray(0);
	gl_has_errors();
}

void RenderSystem::initSpriteRenderer()
{
	ResourceManager::LoadShader(std::string(PROJECT_SOURCE_DIR) + "shaders/sprite.vs.glsl", std::string(PROJECT_SOURCE_DIR) + "shaders/sprite.fs.glsl", "Sprite");
	gl_has_errors();

	ResourceManager::GetShader("Sprite").SetInteger("image", 0, true);
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix(window_width_px, window_height_px);
	ResourceManager::GetShader("Sprite").SetMatrix3("projection", projection_2D);
	gl_has_errors();

	Renderer = new SpriteRenderer(ResourceManager::GetShader("Sprite"));
	gl_has_errors();

	ResourceManager::LoadShader(std::string(PROJECT_SOURCE_DIR) + "shaders/sprite_sheet.vs.glsl", std::string(PROJECT_SOURCE_DIR) + "shaders/sprite_sheet.fs.glsl", "SpriteSheet");
	gl_has_errors();

	ResourceManager::GetShader("SpriteSheet").SetInteger("image", 0, true);
	gl_has_errors();

	ResourceManager::GetShader("SpriteSheet").SetMatrix3("projection", projection_2D);
	gl_has_errors();

	SheetRenderer = new SpriteSheetRenderer(ResourceManager::GetShader("SpriteSheet"));
	gl_has_errors();

	ResourceManager::LoadShader(std::string(PROJECT_SOURCE_DIR) + "shaders/point_lighting.vs.glsl", std::string(PROJECT_SOURCE_DIR) + "shaders/point_lighting.fs.glsl", "PointLighting");
	gl_has_errors();

	PointLightRenderer = new SpriteSheetRenderer(ResourceManager::GetShader("PointLighting"));
	gl_has_errors();
}

// From CPSC 427 tutorial video on basic OpenGL rendering
void RenderSystem::initBasicBoxDetails()
{
	// When we pass attribute data to OpenGL, it just sees it as an array of floats, which we tell it how to unpack.
	// For our own sanity, an array of structs of glm matrices/vectors is stored in that format in memory already,
	// but lets us develop more easily
	struct Vertex
	{
		vec3 position;
		vec2 uv;
	};

	// OpenGL element arrays are again just a big list of uint16s that it reads in threes if we are drawing triangles.
	// Since an array of these structs looks the same in memory, we can work with these structs instead.
	struct Face
	{
		uint16_t v1;
		uint16_t v2;
		uint16_t v3;
	};

	//  Let's make a square!
	//  1       2
	//   *-----*
	//   |   / |
	//   | /   |
	//   *-----*
	//  0       3
	//
	// Note: this square doesn't change every frame, so we just do it once. But if you move this into draw() (and use
	// GL_DYNAMIC_DRAW instead of GL_STATIC_DRAW in glBufferData) and it would still work, but it would let you
	// dynamically generate different vertices each frame.
	std::vector<Vertex> vertices = {
		{{-0.5, -0.5, 0.5}, {0., 0.}}, // 0
		{{-0.5, 0.5, 0.5}, {0., 1.}},  // 1
		{{0.5, 0.5, 0.5}, {1., 1.}},   // 2
		{{0.5, -0.5, 0.5}, {1., 0.}},  // 3
	};
	std::vector<Face> faces = {
		{0, 1, 2}, // top-left triangle
		{2, 3, 0}, // bottom-right triangle
	};

	GLuint vertexBuffer;
	GLuint indexBuffer;

	glGenBuffers(1, &vertexBuffer);		// containts the data representing the data for each vertex
	glGenBuffers(1, &indexBuffer);		// represents sets of vertex indices that form faces
	glGenVertexArrays(1, &general_VAO); // an object that will combine the vertex+index data
	gl_has_errors();

	// Bind our VAO, which will keep track of both vertices and face indices
	glBindVertexArray(general_VAO);

	// Pass the vertex data from the CPU to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	gl_has_errors();
	// ...and then we pass our face vertex indices from the CPU to the GPU as well
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(Face), faces.data(), GL_STATIC_DRAW);
	gl_has_errors();

	// GLint in_position_loc = glGetAttribLocation(sh.ID, "in_position");

	// Now we tell OpenGL how to unpack our vertex array, a big array of floats, into an array of positions and texture
	// coordinates. If you were to add more vertex attributes, you would have to tell OpenGL about it here.
	auto SIZE_OF_EACH_VERTEX = sizeof(Vertex);
	void *POSITION_OFFSET = reinterpret_cast<void *>(offsetof(Vertex, position));
	void *UV_OFFSET = reinterpret_cast<void *>(offsetof(Vertex, uv));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Setup location of vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, SIZE_OF_EACH_VERTEX, POSITION_OFFSET);

	// Setup location of texture coordinates (if one is to be used in the future)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, SIZE_OF_EACH_VERTEX, UV_OFFSET);
	gl_has_errors();

	glBindVertexArray(0); // Unbind the VAO at the end! Otherwise other things you draw might use old data
}

void RenderSystem::initializeGlTextures()
{
	glGenTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());

	for (uint i = 0; i < texture_paths.size(); i++)
	{
		const std::string &path = texture_paths[i];
		ivec2 &dimensions = texture_dimensions[i];

		stbi_uc *data;
		data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

		if (data == NULL)
		{
			const std::string message = "Could not load the file " + path + ".";
			fprintf(stderr, "%s", message.c_str());
			assert(false);
		}
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl_has_errors();
		stbi_image_free(data);
	}
	gl_has_errors();
}

void RenderSystem::initializeGlEffects()
{
	for (uint i = 0; i < effect_paths.size(); i++)
	{
		const std::string vertex_shader_name = effect_paths[i] + ".vs.glsl";
		const std::string fragment_shader_name = effect_paths[i] + ".fs.glsl";

		bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, effects[i]);
		assert(is_valid && (GLuint)effects[i] != 0);
	}
}

// One could merge the following two functions as a template function...
template <class T>
void RenderSystem::bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint)gid]);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	gl_has_errors();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint)gid]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
	gl_has_errors();
}

void RenderSystem::initializeGlMeshes()
{
	for (uint i = 0; i < mesh_paths.size(); i++)
	{
		// Initialize meshes
		GEOMETRY_BUFFER_ID geom_index = mesh_paths[i].first;
		std::string name = mesh_paths[i].second;
		Mesh::loadFromOBJFile(name,
							  meshes[(int)geom_index].vertices,
							  meshes[(int)geom_index].vertex_indices,
							  meshes[(int)geom_index].original_size);

		bindVBOandIBO(geom_index,
					  meshes[(int)geom_index].vertices,
					  meshes[(int)geom_index].vertex_indices);
	}
}

void RenderSystem::initializeGlGeometryBuffers()
{
	// Vertex Buffer creation.
	glGenBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	// Index Buffer creation.
	glGenBuffers((GLsizei)index_buffers.size(), index_buffers.data());

	// Index and Vertex buffer data initialization.
	initializeGlMeshes();

	//////////////////////////
	// Initialize sprite
	// The position corresponds to the center of the texture.
	//	std::vector<TexturedVertex> textured_vertices(4);
	//	textured_vertices[0].position = {-1.f / 2, +1.f / 2, 0.f};
	//	textured_vertices[1].position = {+1.f / 2, +1.f / 2, 0.f};
	//	textured_vertices[2].position = {+1.f / 2, -1.f / 2, 0.f};
	//	textured_vertices[3].position = {-1.f / 2, -1.f / 2, 0.f};
	//	textured_vertices[0].texcoord = {0.f, 1.f};
	//	textured_vertices[1].texcoord = {1.f, 1.f};
	//	textured_vertices[2].texcoord = {1.f, 0.f};
	//	textured_vertices[3].texcoord = {0.f, 0.f};
	//
	//	// Counterclockwise as it's the default opengl front winding direction.
	//	const std::vector<uint16_t> textured_indices = {0, 3, 1, 1, 3, 2};
	//	bindVBOandIBO(GEOMETRY_BUFFER_ID::SPRITE, textured_vertices, textured_indices);
	//
	//	////////////////////////
	//	// Initialize pebble
	//	std::vector<ColoredVertex> pebble_vertices;
	//	std::vector<uint16_t> pebble_indices;
	//	constexpr float z = -0.1f;
	//	constexpr int NUM_TRIANGLES = 62;
	//
	//	for (int i = 0; i < NUM_TRIANGLES; i++)
	//	{
	//		const float t = float(i) * M_PI * 2.f / float(NUM_TRIANGLES - 1);
	//		pebble_vertices.push_back({});
	//		pebble_vertices.back().position = {0.5 * cos(t), 0.5 * sin(t), z};
	//		pebble_vertices.back().color = {0.8, 0.8, 0.8};
	//	}
	//	pebble_vertices.push_back({});
	//	pebble_vertices.back().position = {0, 0, 0};
	//	pebble_vertices.back().color = {0.8, 0.8, 0.8};
	//	for (int i = 0; i < NUM_TRIANGLES; i++)
	//	{
	//		pebble_indices.push_back((uint16_t)i);
	//		pebble_indices.push_back((uint16_t)((i + 1) % NUM_TRIANGLES));
	//		pebble_indices.push_back((uint16_t)NUM_TRIANGLES);
	//	}
	//	int geom_index = (int)GEOMETRY_BUFFER_ID::PEBBLE;
	//	meshes[geom_index].vertices = pebble_vertices;
	//	meshes[geom_index].vertex_indices = pebble_indices;
	//	bindVBOandIBO(GEOMETRY_BUFFER_ID::PEBBLE, meshes[geom_index].vertices, meshes[geom_index].vertex_indices);
	//
	//	//////////////////////////////////
	//	// Initialize debug line
	//	std::vector<ColoredVertex> line_vertices;
	//	std::vector<uint16_t> line_indices;
	//
	//	constexpr float depth = 0.5f;
	//	constexpr vec3 red = {0.8, 0.1, 0.1};
	//
	//	// Corner points
	//	line_vertices = {
	//		{{-0.5, -0.5, depth}, red},
	//		{{-0.5, 0.5, depth}, red},
	//		{{0.5, 0.5, depth}, red},
	//		{{0.5, -0.5, depth}, red},
	//	};
	//
	//	// Two triangles
	//	line_indices = {0, 1, 3, 1, 2, 3};
	//
	//	geom_index = (int)GEOMETRY_BUFFER_ID::DEBUG_LINE;
	//	meshes[geom_index].vertices = line_vertices;
	//	meshes[geom_index].vertex_indices = line_indices;
	//	bindVBOandIBO(GEOMETRY_BUFFER_ID::DEBUG_LINE, line_vertices, line_indices);
	//
	//	///////////////////////////////////////////////////////
	//	// Initialize screen triangle (yes, triangle, not quad; its more efficient).
	//	std::vector<vec3> screen_vertices(3);
	//	screen_vertices[0] = {-1, -6, 0.f};
	//	screen_vertices[1] = {6, -1, 0.f};
	//	screen_vertices[2] = {-1, 6, 0.f};
	//
	//	// Counterclockwise as it's the default opengl front winding direction.
	//	const std::vector<uint16_t> screen_indices = {0, 1, 2};
	//	bindVBOandIBO(GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE, screen_vertices, screen_indices);
}

RenderSystem::~RenderSystem()
{
	// Don't need to free gl resources since they last for as long as the program,
	// but it's polite to clean after yourself.
	glDeleteBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	glDeleteBuffers((GLsizei)index_buffers.size(), index_buffers.data());
	glDeleteTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());
	glDeleteTextures(1, &off_screen_render_buffer_color);
	glDeleteRenderbuffers(1, &off_screen_render_buffer_depth);
	gl_has_errors();

	//	for (uint i = 0; i < effect_count; i++)
	//	{
	//		glDeleteProgram(effects[i]);
	//	}
	// delete allocated resources
	glDeleteFramebuffers(1, &frame_buffer);
	gl_has_errors();

	delete Renderer;
	delete SheetRenderer;
	delete PointLightRenderer;
	delete pg;

	ResourceManager::Clear();

	// remove all entities created by the render system
	while (registry.layerOne.current_renderRequests.entities.size() > 0)
		registry.remove_all_components_of(registry.layerOne.current_renderRequests.entities.back());
	while (registry.layerTwo.current_renderRequests.entities.size() > 0)
		registry.remove_all_components_of(registry.layerTwo.current_renderRequests.entities.back());
}

// Initialize the screen texture from a standard sprite
bool RenderSystem::initScreenTexture()
{

	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(const_cast<GLFWwindow *>(window), &framebuffer_width, &framebuffer_height); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	glGenTextures(1, &off_screen_render_buffer_color);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl_has_errors();

	glGenRenderbuffers(1, &off_screen_render_buffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, off_screen_render_buffer_color, 0);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_render_buffer_depth);
	gl_has_errors();

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	return true;
}

bool gl_compile_shader(GLuint shader)
{
	glCompileShader(shader);
	gl_has_errors();
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
		std::vector<char> log(log_len);
		glGetShaderInfoLog(shader, log_len, &log_len, log.data());
		glDeleteShader(shader);

		gl_has_errors();

		fprintf(stderr, "GLSL: %s", log.data());
		return false;
	}

	return true;
}

bool loadEffectFromFile(
	const std::string &vs_path, const std::string &fs_path, GLuint &out_program)
{
	// Opening files
	std::ifstream vs_is(vs_path);
	std::ifstream fs_is(fs_path);
	if (!vs_is.good() || !fs_is.good())
	{
		fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
		assert(false);
		return false;
	}

	// Reading sources
	std::stringstream vs_ss, fs_ss;
	vs_ss << vs_is.rdbuf();
	fs_ss << fs_is.rdbuf();
	std::string vs_str = vs_ss.str();
	std::string fs_str = fs_ss.str();
	const char *vs_src = vs_str.c_str();
	const char *fs_src = fs_str.c_str();
	GLsizei vs_len = (GLsizei)vs_str.size();
	GLsizei fs_len = (GLsizei)fs_str.size();

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs_src, &vs_len);
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs_src, &fs_len);
	gl_has_errors();

	// Compiling
	if (!gl_compile_shader(vertex))
	{
		fprintf(stderr, "Vertex compilation failed");
		assert(false);
		return false;
	}
	if (!gl_compile_shader(fragment))
	{
		fprintf(stderr, "Fragment compilation failed");
		assert(false);
		return false;
	}

	// Linking
	out_program = glCreateProgram();
	glAttachShader(out_program, vertex);
	glAttachShader(out_program, fragment);
	glLinkProgram(out_program);
	gl_has_errors();

	{
		GLint is_linked = GL_FALSE;
		glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
		if (is_linked == GL_FALSE)
		{
			GLint log_len;
			glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
			std::vector<char> log(log_len);
			glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
			gl_has_errors();

			fprintf(stderr, "Link error: %s", log.data());
			assert(false);
			return false;
		}
	}

	// No need to carry this around. Keeping these objects is only useful if we recycle
	// the same shaders over and over, which we don't, so no need and this is simpler.
	glDetachShader(out_program, vertex);
	glDetachShader(out_program, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	gl_has_errors();

	return true;
}
