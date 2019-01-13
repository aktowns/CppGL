#pragma once

#include <glad/glad.h>

#include "Logger.hpp"
#include "Loader.hpp"
#include "Renderable.hpp"
#include "Shader.hpp"

class Skybox final : Renderable, Logger
{
    std::vector<std::string> _faces;
    GLuint _textureId, _vao, _vbo;
    Shader *_shader;
public:
    explicit Skybox(std::vector<std::string> faces, Loader<Shader>& shaderLoader);
    void draw(const GameObject& game, const RenderObject& render) override;
private:
    void loadCubemap();
};
