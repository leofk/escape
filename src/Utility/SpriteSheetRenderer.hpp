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
#ifndef SPRITE_SHEET_RENDERER_H
#define SPRITE_SHEET_RENDERER_H

#include <gl3w.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture.hpp"
#include "Shader.hpp"

class SpriteSheetRenderer
{
public:
    // Constructor (inits shaders/shapes)
    SpriteSheetRenderer(Shader &shader);
    // Destructor
    ~SpriteSheetRenderer();
    // Renders a defined quad textured with given sprite
    void DrawSprite(Texture2D &texture, glm::mat3 modelMatrix, glm::vec2 sprite_frame, glm::vec2 sheet_details, glm::vec3 color = glm::vec3(1, 1, 1));
    void DrawSprite(Texture2D &diffuseMap, Texture2D &normalMap, glm::vec4 ambient);

private:
    // Render state
    Shader shader;
    GLuint quadVAO;

    // Initializes and configures the quad's buffer and vertex attributes
    void initRenderData();

    void SetIndex(glm::vec2 sprite_frame, glm::vec2 sheet_details);
};

#endif