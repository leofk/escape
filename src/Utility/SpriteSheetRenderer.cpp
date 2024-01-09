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
#include "Utility/SpriteSheetRenderer.hpp"
#include "common.hpp"

SpriteSheetRenderer::SpriteSheetRenderer(Shader &shader)
{
    this->shader = shader;
    this->initRenderData();
}

SpriteSheetRenderer::~SpriteSheetRenderer()
{
    glDeleteVertexArrays(1, &this->quadVAO);
}

void SpriteSheetRenderer::DrawSprite(Texture2D &texture, glm::mat3 modelMatrix, glm::vec2 sprite_frame, glm::vec2 sheet_details, glm::vec3 color)
{
    // Activate shader
    this->shader.Use();

    // Set correct matrix for model
    this->shader.SetMatrix3("model", modelMatrix);

    // colour sprite if needed
    this->shader.SetVector3f("spriteColor", color);

    float column_gap = 1.f / texture.Width;
    float row_gap = 1.f / texture.Height;

    this->shader.SetFloat("column_gap", column_gap);
    this->shader.SetFloat("row_gap", row_gap);

    // Set the position on the sprite sheet to use fo this draw call
    SetIndex(sprite_frame, sheet_details);

    glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    glBindVertexArray(this->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// Sprite render for sprites with normal mapping
void SpriteSheetRenderer::DrawSprite(
    Texture2D &diffuseMap, 
    Texture2D &normalMap,
    glm::vec4 ambient
    )
{
    this->shader.Use();
    this->shader.SetInteger("diffuseMap", 0);
    this->shader.SetInteger("normalMap", 1);
	this->shader.SetVector4f("ambient_c", ambient);

    // If including or view direction
    // lightingShader.setVec3("viewPos", camera.Position);
    // // view/projection transformations
    // glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    // glm::mat4 view = camera.GetViewMatrix();
    // lightingShader.setMat4("projection", projection);
    // lightingShader.setMat4("view", view);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap.ID);
	gl_has_errors();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap.ID);
	gl_has_errors();

    glBindVertexArray(this->quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SpriteSheetRenderer::SetIndex(glm::vec2 sprite_frame, glm::vec2 sheet_details)
{
    this->shader.SetVector2f("BASE", sheet_details);
    this->shader.SetVector2f("POS", sprite_frame);
}

void SpriteSheetRenderer::initRenderData()
{
    // configure VAO/VBO
    GLuint VBO;

    float vertices[] = {
        // pos              // texture coords
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