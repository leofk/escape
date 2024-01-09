/*******************************************************************
** This code is built based on the OpenGL tutorial at https://learnopengl.com/
******************************************************************/
#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H

#include <gl3w.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <random> // std::default_random_engine
#include <chrono> // std::chrono::system_clock

#include "ParticleGenerator.hpp"
#include "Shader.hpp"
#include "Utility/ResourceManager.hpp"

class ParticleGenerator
{
public:
    // Constructor (inits shaders/shapes)
    ParticleGenerator();
    // Destructor
    ~ParticleGenerator();
    // Renders a defined quad textured with given sprite
    glm::vec2 Draw(glm::mat3 proj, glm::mat3 player_pos, glm::vec2 player);

    void UpdateVBO();
    bool UpdateVBO(glm::vec2 goal, glm::vec2 offset_source, float timer, float res_ratio);

    void ResetData();

private:
    // Render state
    GLuint quadVAO;
    GLuint instanceVBO;

    glm::vec2 original[99];
    glm::vec2 translations[2][99];

    glm::vec2 velocity[99];

    glm::vec2 start_points[99];
    // glm::vec2 start_velocity[99];
    glm::vec2 goal_points[99];
    glm::vec2 goal_velocity[99];

    bool reached_goal[99];

    bool hermite_started = false;

    int currentBufferData = 0;

    Shader shader;

    // Initializes and configures the quad's buffer and vertex attributes
    void initRenderData();

    glm::vec2 avg;
};

#endif