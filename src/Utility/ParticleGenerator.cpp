/*******************************************************************
** This code is developped partially from a tutorial
** found at https://learnopengl.com/
** It has been adapted to fit more easily in to our project
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "Utility/ParticleGenerator.hpp"
#include "common.hpp"

const float GRAVITY_TIME = 750.f;
const float HERMITE_TIME = 1500.f;
// const float ZIP_TIME = 2.5f;

float prev_time = 0.f;

ParticleGenerator::ParticleGenerator()
{
    this->initRenderData();
    this->shader = ResourceManager::LoadShader(std::string(PROJECT_SOURCE_DIR) + "shaders/particles.vs.glsl", std::string(PROJECT_SOURCE_DIR) + "shaders/particles.fs.glsl", "Particles");
}

ParticleGenerator::~ParticleGenerator()
{
    glDeleteVertexArrays(1, &this->quadVAO);
}

glm::vec2 generate_random_velocity()
{
    float rand_angle = ((M_PI / 2.f) * (rand() % 100) / 100) + 5 * (M_PI / 4.f);

    double cs = cos(rand_angle);
    double sn = sin(rand_angle);
    double vel = (rand() % 70) + 50;

    double px = vel * cs;
    double py = vel * sn;

    return vec2(px, py);
}

vec2 ParticleGenerator::Draw(mat3 proj, mat3 player_pos, vec2 player)
{
    glEnable(GL_BLEND);
    // ResourceManager::GetTexture("Enemy").Bind();
    this->shader.Use();
    this->shader.SetMatrix3("projection", proj);
    this->shader.SetMatrix3("player_pos", player_pos);

    glBindVertexArray(quadVAO);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 99); // 99 triangles of 6 vertices each
    glBindVertexArray(0);

    return avg;
}

void ParticleGenerator::UpdateVBO()
{

    vec2 next_velocity_modifier = vec2(0.f, 0.5f);

    for (int i = 0; i < 99; ++i)
    {
        velocity[i] += next_velocity_modifier;
        translations[1 - currentBufferData][i] = translations[currentBufferData][i] + velocity[i] * 0.008f;
    }
    currentBufferData = 1 - currentBufferData;
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 99, NULL, GL_DYNAMIC_DRAW);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 99, &translations[currentBufferData][0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gl_has_errors();
}

vec2 getHermiteLocation(vec2 initial_pos, vec2 initial_vel, vec2 goal_pos, vec2 goal_vel, float time)
{
    float t2 = glm::pow(time, 2);
    float t3 = glm::pow(time, 3);

    return (2 * t3 - 3 * t2 + 1) * initial_pos + 5 * (t3 - 2 * t2 + time) * initial_vel + (-2 * t3 + 3 * t2) * goal_pos + (t2 - t2) * goal_vel / 3.f;
}

// Returns if any particles have yet to arrive at the goal
bool ParticleGenerator::UpdateVBO(glm::vec2 goal, glm::vec2 offset_source, float timer, float res_ratio)
{

    bool far_away_particles = false;
    avg = {0.f, 0.f};
    for (int i = 0; i < 99; ++i)
    {
        vec2 real_pos = offset_source + translations[currentBufferData][i];
        avg += real_pos;
        vec2 target_vector = goal - real_pos;
        float leng = length(target_vector);

        if (timer < GRAVITY_TIME)
        {

            // Apply gravity to particles
            vec2 next_velocity_modifier = vec2(0.f, 0.5f * res_ratio);

            // for (int i = 0; i < 99; ++i)
            // {
            velocity[i] += next_velocity_modifier;
            // translations[1 - currentBufferData][i] = translations[currentBufferData][i] + velocity[i] * (timer - prev_time) / 1000.f;
            translations[1 - currentBufferData][i] = translations[currentBufferData][i] + velocity[i] * 0.08f;
            // }
        }
        // else if (timer < (GRAVITY_TIME + HERMITE_TIME))
        else
        {
            // printf("IM DOING HERMITE\n");
            // Smooth the curve to the goal transition

            if (!hermite_started)
            {

                start_points[i] = real_pos;
                goal_points[i] = goal;
                goal_velocity[i] = (target_vector * 1.f);
            }

            if (!reached_goal[i] && leng > 5.f)
            {

                float step_seconds = (timer - GRAVITY_TIME) / HERMITE_TIME;
                // printf("Step seconds: %f\n", step_seconds);
                far_away_particles = true;

                vec2 hermite_pos = getHermiteLocation(start_points[i], velocity[i], goal_points[i], goal_velocity[i], step_seconds);

                // printf("hermite pos: %f, %f\n", hermite_pos.x, hermite_pos.y);

                translations[1 - currentBufferData][i] = hermite_pos - offset_source;
            }
            else
            {
                reached_goal[i] = true;
                translations[1 - currentBufferData][i] = goal - offset_source;
            }
        }
    }
    if (timer > GRAVITY_TIME)
    {
        hermite_started = true;
    }

    avg /= 99.f;

    prev_time = timer;
    currentBufferData = 1 - currentBufferData;
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 99, NULL, GL_DYNAMIC_DRAW);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 99, &translations[currentBufferData][0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gl_has_errors();

    return timer > (GRAVITY_TIME + HERMITE_TIME) && !far_away_particles;
}

void ParticleGenerator::ResetData()
{
    currentBufferData = 0;
    // unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    // std::shuffle(std::begin(originalVectors), std::end(originalVectors), std::default_random_engine(seed));
    for (int i = 0; i < 99; i++)
    {
        translations[currentBufferData][i] = original[i];

        velocity[i] = generate_random_velocity();
        reached_goal[i] = false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 99, NULL, GL_DYNAMIC_DRAW);

    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 99, &translations[currentBufferData][0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gl_has_errors();

    hermite_started = false;
}

void ParticleGenerator::initRenderData()
{

    int index = 0;
    for (int y = -10; y < 10; y += 2)
    {
        for (int x = -10; x < 10; x += 2)
        {
            if (x == 0 && y == 0)
                continue;

            glm::vec2 translation = vec2((rand() % 30) - 15, (rand() % 30) - 15);

            original[index] = translation;

            reached_goal[index] = false;

            velocity[index] = generate_random_velocity();
            translations[0][index] = translation;
            translations[1][index++] = translation;
        }
    }
    gl_has_errors();
    glGenBuffers(1, &instanceVBO);
    gl_has_errors();
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 99, &translations[0][0], GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    gl_has_errors();

    float vertices[] = {
        // pos      // tex
        -1.f / 6, +1.f / 6, 0.0f, 1.0f,
        -1.f / 6, -1.f / 6, 0.0f, 0.0f,
        +1.f / 6, +1.f / 6, 1.0f, 1.0f,

        +1.f / 6, +1.f / 6, 1.0f, 1.0f,
        -1.f / 6, -1.f / 6, 0.0f, 0.0f,
        +1.f / 6, -1.f / 6, 1.0f, 0.0f};

    GLuint quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    gl_has_errors();

    // also set instance data
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.

    gl_has_errors();
}
