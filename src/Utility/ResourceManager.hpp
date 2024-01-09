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
#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <string>

#include <gl3w.h>
#include <GLFW/glfw3.h>

#include "Texture.hpp"
#include "Shader.hpp"

// A static singleton ResourceManager class that hosts several
// functions to load Textures and Shaders. Each loaded texture
// and/or shader is also stored for future reference by string
// handles. All functions and resources are static and no
// public constructor is defined.
class ResourceManager
{
public:
    // resource storage
    static std::map<std::string, Shader> Shaders;
    static std::map<std::string, Texture2D> Textures;
    // loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code. If gShaderFile is not nullptr, it also loads a geometry shader
    static Shader &LoadShader(std::string vShaderFile, std::string fShaderFile, std::string name);
    // retrieves a stored sader
    static Shader &GetShader(std::string name);
    // loads (and generates) a texture from file
    static Texture2D &LoadTexture(const char *file, bool alpha, std::string name);
    // retrieves a stored texture
    static Texture2D &GetTexture(std::string name);
    // properly de-allocates all loaded resources
    static void Clear();

private:
    // private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
    ResourceManager() {}
    // loads and generates a shader from file
    static Shader loadShaderFromFile(std::string vShaderFile, std::string fShaderFile);
    // loads a single texture from file
    static Texture2D loadTextureFromFile(const char *file, bool alpha);
};

#endif