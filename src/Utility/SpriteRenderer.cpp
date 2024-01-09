/*******************************************************************
** This code is adapted from a class in a version the Breakout tutorial
** found at https://learnopengl.com/In-Practice/2D-Game/Breakout
** It has been adapted to fit more easily in to our project
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "Utility/SpriteRenderer.hpp"
#include "common.hpp"

SpriteRenderer::SpriteRenderer(Shader &shader)
{
    this->shader = shader;
    this->initRenderData();
}

SpriteRenderer::~SpriteRenderer()
{
    glDeleteVertexArrays(1, &this->quadVAO);
}

void SpriteRenderer::DrawSprite(Texture2D &texture, glm::mat3 modelMatrix, glm::vec3 color)
{
    // prepare transformations
    this->shader.Use();

    this->shader.SetMatrix3("model", modelMatrix);

    // render textured quad
    this->shader.SetVector3f("spriteColor", color);

        glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    glBindVertexArray(this->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SpriteRenderer::initRenderData()
{
    // this->shader.SetFloat("parallax", 0.f);
    // configure VAO/VBO
    GLuint VBO;
    // float vertices[] = {
    //     // pos      // tex
    //     0.0f, 1.0f, 0.0f, 1.0f,
    //     1.0f, 0.0f, 1.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f, 0.0f,

    //     0.0f, 1.0f, 0.0f, 1.0f,
    //     1.0f, 1.0f, 1.0f, 1.0f,
    //     1.0f, 0.0f, 1.0f, 0.0f};

    float vertices[] = {
        // pos      // tex
        -1.f / 2, +1.f / 2, 0.0f, 1.0f,
        -1.f / 2, -1.f / 2, 0.0f, 0.0f,
        +1.f / 2, +1.f / 2, 1.0f, 1.0f,

        +1.f / 2, +1.f / 2, 1.0f, 1.0f,
        -1.f / 2, -1.f / 2, 0.0f, 0.0f,
        +1.f / 2, -1.f / 2, 1.0f, 0.0f};

    gl_has_errors();
    glGenVertexArrays(1, &this->quadVAO);
    gl_has_errors();
    glGenBuffers(1, &VBO);
    gl_has_errors();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    gl_has_errors();

    glBindVertexArray(this->quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    gl_has_errors();
}