// internal
#include "render_system.hpp"
#include <SDL.h>
#include <gl3w.h>

// glfw (OpenGL)
#include <GLFW/glfw3.h>

// gltext
#define GLT_IMPLEMENTATION
#include <gltext.hpp>

// stlib
#include <chrono>
#include <iostream>
#include <vector>
using Clock = std::chrono::high_resolution_clock;
extern bool facingRight;
extern int frame;
extern int current_layer;

// Matrix/vector classes in a format that we can pass to a shader
#include <glm/ext/vector_int2.hpp> // ivec2
#include <glm/mat3x3.hpp>		   // mat3
#include <glm/vec2.hpp>			   // vec2
#include <glm/vec3.hpp>			   // vec3
#include <glm/gtc/type_ptr.hpp>

#include "tiny_ecs_registry.hpp"

#include "Utility/ResourceManager.hpp"

Motion background;
float phase_success_trans_timer = 0.f;
float phase_fail_trans_timer = 0.f;

vec2 move_to_center;

float scaling_factor = 1.2f;

Transform RenderSystem::applyTransformations(const Motion &motion)
{
	Transform transform;
	//    transform.scale(res_ratio);
	transform.translate(motion.position);
	transform.translate(move_to_center);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);
	return transform;
}

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{
	Motion &motion = registry.layers[current_layer]->current_motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;

	transform.translate(motion.position);
	transform.translate(move_to_center);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	// !!! TODO A1: add rotation to the chain of transformations, mind the order
	// of transformations

	// Needs a (unused) VAO to be bound, crashes otherwise
	//    GLuint vao;
	//    glGenVertexArrays(1, &vao);
	glBindVertexArray(general_VAO);

	assert(registry.layers[current_layer]->current_renderRequests.has(entity));
	const RenderRequest &render_request = registry.layers[current_layer]->current_renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();
	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::SALMON)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		GLint check;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &check);
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);

	glBindVertexArray(0);
	gl_has_errors();
}

// second pass draw call
void RenderSystem::drawToScreen()
{
	Shader &second_pass = ResourceManager::GetShader("second_pass").Use();
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Adapted from David Amador's tutorial at https://www.david-amador.com/2013/04/opengl-2d-independent-resolution-rendering/
	float targetRatio = (float)window_width_px / (float)window_height_px;

	int width = w;
	int height = (int)(width / targetRatio);

	if (height > h)
	{
		height = h;
		width = (int)(height * targetRatio);
	}

	int vp_x = (w / 2) - (width / 2);
	int vp_y = (h / 2) - (height / 2);

	glViewport(vp_x, vp_y, width, height);
	glDepthRange(0, 10);
	glClearColor(0.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	Motion background;
	background.position = vec2(w / 2, h / 2);
	background.scale = vec2(w, h);

	// check for retina display
	int frame_buffer_width_px, frame_buffer_height_px;
	glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
																					 //	if (frame_buffer_width_px != window_width_px)
																					 //	{
																					 //		// printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
																					 //		// printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
																					 //		// printf("window width_height = %d,%d\n", window_width_px, window_height_px);
																					 //
																					 //		background.position /= 2;
																					 //		background.scale /= 2;
																					 //	}

	// set transformations
	mat3 projection_2D = createProjectionMatrix(w, h, scaling_factor);
	second_pass.SetMatrix3("projection", projection_2D);

	Transform transform;
	//    transform.scale(res_ratio);
	transform.translate(background.position);
	transform.scale(background.scale);
	second_pass.SetMatrix3("model", transform.mat);

	// Normalize the player position
	vec2 pos = (player_pos * res_ratio * scaling_factor) / background.scale;
	// Reflect about y
	pos.y = 1 - pos.y;

	// Set player texcoord uniform
	second_pass.SetVector2f("player_pos", pos);

	// Set time uniform
	second_pass.SetFloat("time", (float)(glfwGetTime() * 10.0f));
	second_pass.SetInteger("is_phase_anim", phase_success_trans_timer > 0);
    second_pass.SetInteger("is_fail_anim", phase_fail_trans_timer > 0);

	// set spotlight distancde
	second_pass.SetFloat("spot_max", spot_max);

	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	glBindVertexArray(quadVAO);
	gl_has_errors();

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, 6);
	gl_has_errors();

	// Text Render Save/Load when appropriate
	if (draw_load_message_time > 0.0f)
	{
		render_text({1.0f, 1.0f, 1.0f, 1.0f}, h * 0.003f, w * 0.80, h * 0.03, "Loading Game");
	}
	else if (draw_save_message_time > 0.0f)
	{
		render_text({1.0f, 1.0f, 1.0f, 1.0f}, h * 0.003f, w * 0.80, h * 0.03, "Saving Game");
	}
	else if (draw_unable_load_time > 0.0f)
	{
		render_text({1.0f, 1.0f, 1.0f, 1.0f}, h * 0.003f, w * 0.650, h * 0.03, "No save data to load game");
	}

	gl_has_errors();
}

void RenderSystem::initRenderData()
{
	int w, h;

	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	gl_has_errors();

	background.position = vec2(window_width_px / 2, window_height_px / 2);
	background.scale = vec2(window_width_px, window_height_px);

	// check for retina display
	int frame_buffer_width_px, frame_buffer_height_px;
	glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
																					 //	if (frame_buffer_width_px != window_width_px)
																					 //	{
																					 //		// printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
																					 //		// printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
																					 //		// printf("window width_height = %d,%d\n", window_width_px, window_height_px);
																					 //
																					 //		background.position /= 2;
																					 //		background.scale /= 2;
																					 //	}

	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(1, 1, 1, 1.0);
	glClearDepth(10.f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front

	// configure VAO/VBO
	GLuint VBO;

	float vertices[] = {
		// pos      // tex
		-1.f / 2, +1.f / 2, 0.0f, 0.0f,
		-1.f / 2, -1.f / 2, 0.0f, 1.0f,
		+1.f / 2, +1.f / 2, 1.0f, 0.0f,

		+1.f / 2, +1.f / 2, 1.0f, 0.0f,
		-1.f / 2, -1.f / 2, 0.0f, 1.0f,
		+1.f / 2, -1.f / 2, 1.0f, 1.0f};

	gl_has_errors();
	glGenVertexArrays(1, &quadVAO);
	gl_has_errors();
	glGenBuffers(1, &VBO);
	gl_has_errors();

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	gl_has_errors();

	glBindVertexArray(quadVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	gl_has_errors();
}

void RenderSystem::Draw_Controller(GameState gs)
{
	if (gs == Level_Active)
	{
		draw();
	}
	else
	{
		DrawMenu(gs);
	}
}

void RenderSystem::DrawMenu(GameState gs)
{

	// First render to the custom framebuffer
	// glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer); // ENABLES SECOND PASS
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // NO SECOND PASS FOR MENU
	gl_has_errors();

	// Clearing backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gl_has_errors();

	int w, h;

	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	float targetRatio = (float)window_width_px / (float)window_height_px;

	int width = w;
	int height = (int)(width / targetRatio);

	if (height > h)
	{
		height = h;
		width = (int)(height * targetRatio);
	}

	int vp_x = (w / 2) - (width / 2);
	int vp_y = (h / 2) - (height / 2);

	glViewport(vp_x, vp_y, width, height);
	glDepthRange(0, 10);
	glClearColor(0.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat3 projection_2D = createProjectionMatrix(window_width_px, window_height_px);
	ResourceManager::GetShader("Sprite").SetMatrix3("projection", projection_2D, true);
	ResourceManager::GetShader("SpriteSheet").SetMatrix3("projection", projection_2D, true);

	// Draw background first
	Transform button;
	button.translate(background.position);
	button.scale(background.scale);
	if (gs == Pause_Screen || gs == Tutorial_Instruction_Pause_Screen)
	{
        if (gs == Tutorial_Instruction_Pause_Screen) {
            Texture2D help = ResourceManager::GetTexture("Tutorial" + std::to_string(tutorial_instruction_number));
            Renderer->DrawSprite(help, button.mat, glm::vec3(1.0f, 1.0f, 1.0f));
        } else {
            Texture2D help = ResourceManager::GetTexture("Pause_Menu");
		        Renderer->DrawSprite(help, button.mat, glm::vec3(1.0f, 1.0f, 1.0f));
        }
		
		gl_has_errors();

		drawMenuButtons(gs);
	}
	else if (gs == Help_Screen)
	{
		Texture2D help = ResourceManager::GetTexture("Help_Screen");
		Renderer->DrawSprite(help, button.mat, glm::vec3(1.0f, 1.0f, 1.0f));
		gl_has_errors();

		drawMenuButtons(gs);
	}
	else if (gs == Start_Screen)
	{
		Texture2D help = ResourceManager::GetTexture("Title_Screen");
		Renderer->DrawSprite(help, button.mat, glm::vec3(1.0f, 1.0f, 1.0f));
		gl_has_errors();

		drawMainMenuButtons();
	}
	else if (gs == Level_Selection)
	{
		Texture2D help = ResourceManager::GetTexture("Levels_Menu");
		Renderer->DrawSprite(help, button.mat, glm::vec3(1.0f, 1.0f, 1.0f));
		gl_has_errors();

		drawLevelSelectionButtons();
	}
  else if (gs == Tutorial_Instruction)
  {
        Texture2D help = ResourceManager::GetTexture("Tutorial" + std::to_string(tutorial_instruction_number));
        Renderer->DrawSprite(help, button.mat, glm::vec3(1.0f, 1.0f, 1.0f));
        gl_has_errors();
	}
	else if (gs >= End_1)
	{
		Texture2D help = ResourceManager::GetTexture("End_" + std::to_string(gs - End_1 + 1));
		Renderer->DrawSprite(help, button.mat, glm::vec3(1.0f, 1.0f, 1.0f));
		gl_has_errors();
	}
	if (gs < End_1)
	{
		drawText(gs);
	}
	if (draw_unable_load_time > 0.0f)
	{
		render_text({1.0f, 1.0f, 1.0f, 1.0f}, h * 0.003f, w * 0.650, h * 0.03, "No save data to load game");
	}

	glfwSwapBuffers(window);
	gl_has_errors();
}

void RenderSystem::drawMenuButtons(GameState gs)
{
	for (int i = 0; i < registry.menu_btns.components.size(); i++)
	{
		MainMenuElement m = registry.menu_btns.components[i];
		if (((gs == Pause_Screen) && i < MENU_BUTTON_BACK) || ((gs == Help_Screen) && (i >= MENU_BUTTON_BACK)))
		{
			// Blending required for Texture ontop of another texture without Alpha channel
			glEnable(GL_BLEND);
			Transform transform;
			transform.translate(m.position);
			transform.scale(m.size);
			Texture2D t = m.mouseover ? ResourceManager::GetTexture("Generic_Long_Button_Highlight") : ResourceManager::GetTexture("Generic_Long_Button");
			Renderer->DrawSprite(t, transform.mat, glm::vec3(0.8f, 0.9f, 1.0f));
			gl_has_errors();
		}
	}
}

void RenderSystem::drawMainMenuButtons()
{
	for (int i = 0; i < registry.main_menu_btns.components.size() - 3; i++)
	{
		MainMenuElement m = registry.main_menu_btns.components[i];
		if (m.is_active)
		{
			// Blending required for Texture ontop of another texture without Alpha channel
			glEnable(GL_BLEND);
			Transform transform;
			transform.translate(m.position);
			transform.scale(m.size);
			Texture2D t = m.mouseover ? ResourceManager::GetTexture("Generic_Long_Button_Highlight") : ResourceManager::GetTexture("Generic_Long_Button");
			Renderer->DrawSprite(t, transform.mat, glm::vec3(0.8f, 0.9f, 1.0f));
			gl_has_errors();
		}
	}
	for (int i = registry.main_menu_btns.components.size() - 3; i < registry.main_menu_btns.components.size(); i++)
	{
		MainMenuElement m = registry.main_menu_btns.components[i];
		glEnable(GL_BLEND);
		Transform transform;
		transform.translate(m.position);
		transform.scale(m.size);
		Texture2D t = ResourceManager::GetTexture("Score_Board");
		Renderer->DrawSprite(t, transform.mat, glm::vec3(0.8f, 0.8f, 0.8f));
		gl_has_errors();
	}
	for (TextElement t : registry.scores.components)
	{
		for (int i = 0; i < t.str.length(); i++)
		{
			glEnable(GL_BLEND);
			Transform transform;
			// transform.translate(t.position - vec2(t.size.x / 2.f * i, 0.f) + vec2(t.size.x * i, 0.f));
			transform.translate(t.position + vec2(t.size.x * i, 0.f));
			transform.scale(t.size);
			std::string text_char = t.str.substr(i, 1);
			ResourceManager::GetShader("SpriteSheet").SetInteger("moveRight", 0, true);
			// Renderer->DrawSprite(ResourceManager::GetTexture(text_char == ":" ? "Colon" : text_char), transform.mat, glm::vec3(0.8f, 0.8f, 1.0f));
			int index = text_char.c_str()[0] - 33;
			SheetRenderer->DrawSprite(ResourceManager::GetTexture("SpriteSheet_Text"), transform.mat, vec2(index % 16, index / 16), vec2(16, 6), vec3(0.8f, 0.8f, 1.f));
			gl_has_errors();
		}
	}
}

void RenderSystem::drawLevelSelectionButtons()
{
	for (MainMenuElement m : registry.level_selection_btns.components)
	{
		if (m.is_active)
		{
			// Blending required for Texture ontop of another texture without Alpha channel
			glEnable(GL_BLEND);
			Transform transform;
			transform.translate(m.position);
			transform.scale(m.size);
			Texture2D t = m.mouseover ? ResourceManager::GetTexture("Generic_Long_Button_Highlight") : ResourceManager::GetTexture("Generic_Long_Button");
			Renderer->DrawSprite(t, transform.mat, glm::vec3(0.8f, 0.9f, 1.0f));
			gl_has_errors();
		}
	}
	for (int i = 0; i < registry.level_selections.components.size(); i++)
	{
		LevelSelectionElement m = registry.level_selections.components[i];
		// Blending required for Texture ontop of another texture without Alpha channel
		glEnable(GL_BLEND);
		Transform transform;
		transform.translate(m.position);
		Texture2D t;
		if (m.is_implemented)
		{
			if (m.is_unlocked)
			{
				if (m.mouseover)
				{
					transform.scale(m.size + vec2(2.f, 2.f));
					t = ResourceManager::GetTexture("Level_Highlight");
					Renderer->DrawSprite(t, transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));
					gl_has_errors();
				}
			}
			else
			{
				transform.scale(m.size);
				t = ResourceManager::GetTexture("Level_Locked");
				Renderer->DrawSprite(t, transform.mat, m.mouseover ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(1.0f, 1.0f, 1.0f));
				gl_has_errors();
			}
		}
		else
		{
			transform.scale(m.size);
			t = ResourceManager::GetTexture("Level_Empty");
			Renderer->DrawSprite(t, transform.mat, m.mouseover ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(1.0f, 1.0f, 1.0f));
			gl_has_errors();
		}
	}
}

void RenderSystem::drawText(GameState gs)
{
	for (TextElement t : (gs == Pause_Screen ? registry.pause_text.components : (gs == Help_Screen ? registry.help_text.components : registry.texts.components)))
	{
		if (t.is_active)
		{
			for (int i = 0; i < t.str.length(); i++)
			{
				glEnable(GL_BLEND);
				Transform transform;
				// transform.translate(t.position - vec2(t.size.x / 2.f * i, 0.f) + vec2(t.size.x * i, 0.f));
				transform.translate(t.position + vec2(t.size.x * i, 0.f));
				transform.scale(t.size);
				std::string text_char = t.str.substr(i, 1);
				if (text_char != " ")
				{
					ResourceManager::GetShader("SpriteSheet").SetInteger("moveRight", 0, true);
					if (!t.is_highlighted)
					{
						// Renderer->DrawSprite(ResourceManager::GetTexture(text_char == ":" ? "Colon" : text_char), transform.mat, glm::vec3(0.8f, 0.8f, 1.0f));
						int index = text_char.c_str()[0] - 33;
						SheetRenderer->DrawSprite(ResourceManager::GetTexture("SpriteSheet_Text"), transform.mat, vec2(index % 16, index / 16), vec2(16, 6), vec3(0.8f, 0.8f, 1.f));
					}
					else
					{
						// Renderer->DrawSprite(ResourceManager::GetTexture(text_char == ":" ? "Colon" : text_char), transform.mat, glm::vec3(1.0f, 1.f, 1.0f));
						int index = text_char.c_str()[0] - 33;
						SheetRenderer->DrawSprite(ResourceManager::GetTexture("SpriteSheet_Text"), transform.mat, vec2(index % 16, index / 16), vec2(16, 6));
					}
				}
				gl_has_errors();
			}
		}
	}
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;

	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	gl_has_errors();

	//	Motion background;
	//	background.position = vec2(w / 2, h / 2);
	//	background.scale = vec2(w, h);

	// check for retina display
	int frame_buffer_width_px, frame_buffer_height_px;
	glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
																					 //	if (frame_buffer_width_px != window_width_px)
																					 //	{
																					 //		// printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
																					 //		// printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
																					 //		// printf("window width_height = %d,%d\n", window_width_px, window_height_px);
																					 //
																					 //		background.position /= 2;
																					 //		background.scale /= 2;
																					 //	}

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer); // ENABLES SECOND PASS
													 //	 glBindFramebuffer(GL_FRAMEBUFFER, 0); // UNCOMMENT FOR NO SECOND PASS
	gl_has_errors();

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(1, 1, 1, 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix(window_width_px, window_height_px, scaling_factor);

	// Ensure scaled projection matrix is applied to shader
	ResourceManager::GetShader("Sprite").SetMatrix3("projection", projection_2D, true);
	ResourceManager::GetShader("SpriteSheet").SetMatrix3("projection", projection_2D, true);

	// Set the translation correctly for future scaling
	Motion &motion = registry.motions.get(registry.players.entities[0]);
	float scaled_halfway = scaling_factor * scaling_factor * 2.f;

	vec2 window_center = vec2(window_width_px / (scaled_halfway), window_height_px / (scaled_halfway));

	vec2 adjusted_player_pos = vec2(motion.position.x, motion.position.y);

	if (adjusted_player_pos.x < window_center.x)
	{
		adjusted_player_pos.x = window_center.x;
	}
	else if (adjusted_player_pos.x > (window_width_px - window_center.x))
	{
		adjusted_player_pos.x = window_width_px - window_center.x;
	}
	if (adjusted_player_pos.y < window_center.y)
	{
		adjusted_player_pos.y = window_center.y;
	}
	else if (adjusted_player_pos.y > (window_height_px - window_center.y))
	{
		adjusted_player_pos.y = window_height_px - window_center.y;
	}

	move_to_center = window_center - adjusted_player_pos;
	player_pos = motion.position + move_to_center;
	xDiff = (adjusted_player_pos.x / (window_width_px / 2.f)) - 1.f;

	// draw background based on level environment
	if (tilesheet_id == TILESHEET_CAVETHEME)
	{
		draw_cave_background();
		spot_max = 0.3;
	}
	else if (tilesheet_id == TILESHEET_LABTHEME)
	{
		draw_lab_background();
		spot_max = 0.5;
	}
	else if (tilesheet_id == TILESHEET_FORESTTHEME)
	{
		draw_forest_background();
		spot_max = 0.4;
	}

	// Render spikes first
	for (Entity e : registry.spikes.entities)
	{
		if (registry.layers[current_layer]->current_renderRequests.has(e))
		{
			drawTexturedMesh(e, projection_2D);
		}
	}

	// Draw all textured meshes that have a position and size component
	for (Entity entity : registry.layers[current_layer]->current_renderRequests.entities)
	{
		if (!registry.layers[current_layer]->current_motions.has(entity) || registry.players.has(entity))
			continue;
		// Note, its not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize

		if (registry.walls.has(entity))
		{

			if (registry.walls.get(entity).use_shader)
			{
				draw_box(entity, projection_2D, vec3(0, 0, 0));
			}
			else
			{
				int texture_id = registry.layers[current_layer]->current_renderRequests.get(entity).tile_id;
				Motion &motion = registry.layers[current_layer]->current_motions.get(entity);
				ResourceManager::GetShader("SpriteSheet").SetInteger("moveRight", 0, true);

				Texture2D t = ResourceManager::GetTexture(tilesheet_texture_name);
				Transform transform;

				transform = applyTransformations(motion);
				SheetRenderer->DrawSprite(t, transform.mat, vec2(texture_id % tilesheet_width, texture_id / tilesheet_width), vec2(tilesheet_width, tilesheet_height));
			}
		}
		else if (registry.decorations.has(entity) ||  registry.lock_tiles.has(entity) || registry.platforms.has(entity))
		{
			if (!isPresent(entity, registry.lock_tiles))
			{
				continue;
			}
			int texture_id = registry.layers[current_layer]->current_renderRequests.get(entity).tile_id;
			Motion &motion = registry.layers[current_layer]->current_motions.get(entity);
			ResourceManager::GetShader("SpriteSheet").SetInteger("moveRight", 0, true);

			Texture2D t = ResourceManager::GetTexture(tilesheet_texture_name);
			Transform transform;
			transform = applyTransformations(motion);
			SheetRenderer->DrawSprite(t, transform.mat, vec2(texture_id % tilesheet_width, texture_id / tilesheet_width), vec2(tilesheet_width, tilesheet_height));

            if (registry.lock_tiles.has(entity)) {
                t = ResourceManager::GetTexture("Global_Anim");
                SheetRenderer->DrawSprite(t, transform.mat, vec2(7, 3), vec2(8, 9));
            }
		}
		else if (registry.linearInterp.has(entity))
		{
			Texture2D &t = ResourceManager::GetTexture("Global_Anim");
			Motion &motion = registry.layers[current_layer]->current_motions.get(entity);
			LinearInterp &hazard = registry.linearInterp.get(entity);
            ResourceManager::GetShader("SpriteSheet").SetInteger("moveRight", (int) hazard.isFacingRight, true);

			WalkTimer &wt = registry.walkTimers.get(entity);

			Transform transform = applyTransformations(motion);
			//			Transform transform;
			//			transform.translate(motion.position);
			//			transform.rotate(motion.angle);
			//			transform.scale(motion.scale);
			SheetRenderer->DrawSprite(t, transform.mat, vec2(wt.startFrame.x + wt.currentFrame, wt.startFrame.y), vec2(8, 9));
		}
	}
	draw_player();
	draw_animated();
	draw_globalanims();

	// SECOND PASS DRAW CALL
	drawToScreen();

	drawText(Level_Active);

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}


void RenderSystem::draw_forest_background()
{
	Transform transform;
	transform = applyTransformations(background);

	float d5 = 0.0f * xDiff;
	float d4 = 0.03f * xDiff;
	float d3 = 0.06f * xDiff;
	float d2 = 0.08f * xDiff;
	float d1 = 0.1f * xDiff;

	Renderer->shader.SetFloat("parallax", d5, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("forest-1"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));

	Renderer->shader.SetFloat("parallax", d4, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("forest-2"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));

	Renderer->shader.SetFloat("parallax", d3, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("forest-3"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));

	Renderer->shader.SetFloat("parallax", d2, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("forest-4"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));

	point_light(
		d1,
		ResourceManager::GetTexture("forest-5"), 
		ResourceManager::GetTexture("forest-5_n"), 
		vec4(0.4f, 0.4f, 0.9f, 0.4f)
	);

	Renderer->shader.SetFloat("parallax", 0, true);
}

void RenderSystem::draw_cave_background()
{
	Transform transform;
	transform = applyTransformations(background);

	float d6 = 0.0f * xDiff;
	float d5 = 0.02f * xDiff;
	float d4 = 0.04f * xDiff;
	float d3 = 0.06f * xDiff;
	float d2 = 0.08f * xDiff;
	float d1 = 0.1f * xDiff;

	Renderer->shader.SetFloat("parallax", d6, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("cave-6"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));
    gl_has_errors();
	point_light(
		d6,
		ResourceManager::GetTexture("mush_1"), 
		ResourceManager::GetTexture("mush_1_n"), 
		vec4(0.5f, 0.5f, 0.5f, 0.2f)
	);

	Renderer->shader.SetFloat("parallax", d5, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("cave-5"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));
    gl_has_errors();
	point_light(
		d5,
		ResourceManager::GetTexture("mush_2"), 
		ResourceManager::GetTexture("mush_2_n"), 
		vec4(0.6f, 0.6f, 0.6f, 0.2f)
	);

	Renderer->shader.SetFloat("parallax", d4, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("cave-4"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));

	Renderer->shader.SetFloat("parallax", d3, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("cave-3"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));
	point_light(
		d3,
		ResourceManager::GetTexture("mush_3"), 
		ResourceManager::GetTexture("mush_3_n"), 
		vec4(0.7f, 0.7f, 0.7f, 0.2f)
	);

	Renderer->shader.SetFloat("parallax", d2, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("cave-2"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));
	point_light(
		d2,
		ResourceManager::GetTexture("mush_4"),
		ResourceManager::GetTexture("mush_4_n"),
		vec4(0.8f, 0.8f, 0.8f, 0.2f)
	);
	
	Renderer->shader.SetFloat("parallax", d1, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("cave-1"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));

	Renderer->shader.SetFloat("parallax", 0, true);
}

void RenderSystem::draw_lab_background()
{
	Transform transform;
	transform = applyTransformations(background);

	float d6 = 0.0f * xDiff;
	float d5 = 0.02f * xDiff;
	float d4 = 0.04f * xDiff;
	float d3 = 0.06f * xDiff;
	float d2 = 0.08f * xDiff;
	float d1 = 0.1f * xDiff;

	Renderer->shader.SetFloat("parallax", d6, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("lab-1"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));
    gl_has_errors();

	Renderer->shader.SetFloat("parallax", d5, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("lab-2"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));

	Renderer->shader.SetFloat("parallax", d4, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("lab-3"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));

	Renderer->shader.SetFloat("parallax", d3, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("lab-4"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));
	point_light(
		d2,
		ResourceManager::GetTexture("lab_obj_1"), 
		ResourceManager::GetTexture("lab_obj_1_n"), 
		vec4(0.5f, 0.5f, 0.5f, 0.2f)
	);

	Renderer->shader.SetFloat("parallax", d2, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("lab-5"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));
	point_light(
		d1,
		ResourceManager::GetTexture("lab_obj_2"), 
		ResourceManager::GetTexture("lab_obj_2_n"), 
		vec4(0.8f, 0.8f, 0.8f, 0.2f)
	);

	Renderer->shader.SetFloat("parallax", d1, true);
	Renderer->DrawSprite(ResourceManager::GetTexture("lab-6"), transform.mat, glm::vec3(1.0f, 1.0f, 1.0f));

	Renderer->shader.SetFloat("parallax", 0, true);
}

void RenderSystem::point_light(
	float parallax, Texture2D diffuse, Texture2D normal, vec4 ambient
)
{
	vec2 lightPos = player_pos / background.scale;
	gl_has_errors();

	Transform transform = applyTransformations(background);
	
	mat3 projection_2D = createProjectionMatrix(window_width_px, window_height_px, scaling_factor);

	Shader &sh = ResourceManager::GetShader("PointLighting").Use();
	
	sh.SetVector2f("LightPos", lightPos);
	sh.SetMatrix3("model", transform.mat);
	sh.SetMatrix3("projection", projection_2D);
	sh.SetVector2f("Resolution", vec2(window_width_px, window_height_px));
	sh.SetVector4f("diffuse_c", vec4(1.f, 1.f, 1.f, 1.f));
	sh.SetVector3f("Falloff", vec3(.4f, 3.f, 20.f));
	sh.SetFloat("LightSize", 800);
	sh.SetFloat("p", parallax);

	PointLightRenderer->DrawSprite(diffuse, normal, ambient);
}

// For global animated sprites
void RenderSystem::draw_animated()
{
	for (Entity entity : registry.layers[current_layer]->current_animTimers.entities)
	{
		if (!(isPresent(entity, registry.keys) && isPresent(entity, registry.collectibles)))
		{
			continue; // Skip if the current entity is not present
		}
		Motion &motion = registry.layers[current_layer]->current_motions.get((entity));

		AnimTimer &anim = registry.layers[current_layer]->current_animTimers.get(entity);
		ResourceManager::GetShader("SpriteSheet").SetInteger("moveRight", 0, true);

		Texture2D t = ResourceManager::GetTexture("Global_Anim");
		Transform transform;
		transform = applyTransformations(motion);
		SheetRenderer->DrawSprite(t, transform.mat, vec2(anim.startFrame.x + anim.currentFrame, anim.startFrame.y), vec2(8, 9));
	}
}

void RenderSystem::draw_globalanims()
{
	for (Entity entity : registry.global_anims.entities)
	{
		Motion &motion = registry.motions.get((entity));

		AnimTimer &anim = registry.global_anims.get(entity);
		ResourceManager::GetShader("SpriteSheet").SetInteger("moveRight", 0, true);

		Texture2D t = ResourceManager::GetTexture("Global_Anim");
		Transform transform;
		transform = applyTransformations(motion);
		SheetRenderer->DrawSprite(t, transform.mat, vec2(anim.startFrame.x + anim.currentFrame, anim.startFrame.y), vec2(8, 9));
	}
}

void RenderSystem::draw_player()
{
	// Texture2D t = ResourceManager::GetTexture("Player");
	Entity &p = registry.players.entities[0];
	Motion &motion = registry.motions.get(p);

	if (registry.deathTimers.has(p))
	{

		DeathTimer &dt = registry.deathTimers.get(p);

		if (dt.particles_arrived)
		{
			registry.deathTimers.remove(p);
			pg->ResetData();
			return;
		}

		Transform tform;
		tform.translate(registry.deathTimers.get(p).pos);
		tform.translate(move_to_center);
		tform.scale(vec2(10, 10));
		mat3 projection_2D = createProjectionMatrix(window_width_px, window_height_px, scaling_factor);

		// Draw particles on screen
		vec2 avg = pg->Draw(projection_2D, tform.mat, dt.pos);

		motion.position = avg;

		// Update particle positions
		dt.particles_arrived = pg->UpdateVBO(registry.players.components[0].start_pos, dt.pos, dt.countdown, res_ratio.y);

		return;
	}

	Shader &sh = ResourceManager::GetShader("SpriteSheet").Use();
	if (facingRight)
	{
		sh.SetInteger("moveRight", 0);
	}
	else
	{
		sh.SetInteger("moveRight", 1);
	}
	Texture2D t = ResourceManager::GetTexture("Walk");
    Motion scaled_motion = motion;
    scaled_motion.scale *= 1.12;
	Transform transform = applyTransformations(scaled_motion);

	WalkTimer &wt = registry.walkTimers.get(p);

	SheetRenderer->DrawSprite(t, transform.mat, vec2(wt.startFrame.x + wt.currentFrame, wt.startFrame.y), vec2(6, 1));
	// SheetRenderer->DrawSprite(t, transform.mat, glm::vec3(1.0f, 1.0f, 1.0f), vec2(frame, 0), vec2(9, 3));
	//            sh.SetFloat("time", glfwGetTime()*10.f);
}

void RenderSystem::draw_box(Entity entity,
							const mat3 &projection,
							vec3 colour = vec3(0, 0, 0))
{
	Motion &motion = registry.layers[current_layer]->current_motions.get(entity);
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	Shader &sh = ResourceManager::GetShader("Wall"); // Use the shader we compiled
	sh.SetVector3f("color", colour, true);

	GLint transform_uloc = glGetUniformLocation(sh.ID, "transform");
	GLint projection_uloc = glGetUniformLocation(sh.ID, "projection");

	// Set values on the GPU for the rest of our variables that aren't different for each vertex
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float *)&transform.mat);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Bind VAO with the details for our simple box render
	glBindVertexArray(general_VAO);
	gl_has_errors();

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
	glBindVertexArray(0); // Unbind the VAO
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix(int width, int height)
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float)width;
	float bottom = (float)height;
	// float right = (float)width;
	// float bottom = (float)height;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

mat3 RenderSystem::createProjectionMatrix(int width, int height, float scale)
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float)width / scale;
	float bottom = (float)height / scale;
	// float right = (float)width;
	// float bottom = (float)height;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

void RenderSystem::use_tilesheet(int id)
{
	assert(id < TILESHEET_COUNT);
	tilesheet_id = id;
	tilesheet_texture_name = tilesheet_params[id].texture_name;
	tilesheet_width = tilesheet_params[id].width;
	tilesheet_height = tilesheet_params[id].height;
}

// Note: Almost all of this function code comes from the example described in https://github.com/vallentin/glText
void RenderSystem::render_text(vec4 color, float scale, float xpos, float ypos, const char *message)
{
	gltInit();

	GLTtext *text = gltCreateText();
	gltSetText(text, message);

	gltBeginDraw();
	glEnable(GL_BLEND);

	gltColor(color[0], color[1], color[2], color[2]);
    gltDrawText2D(text, xpos, ypos, scale);

	gltDeleteText(text);

	gltTerminate();
}