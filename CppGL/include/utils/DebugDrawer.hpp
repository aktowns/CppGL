#pragma once

#include "Shader.hpp"
#include "Loader.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxRenderBuffer.h>

struct DebugPoint final
{
    float x, y, z;
    float r, g, b;
};

class DebugDrawer final {
    unsigned int _vao, _vbo;
    std::vector<DebugPoint> _lines;
    Shader* _debugShader;
    physx::PxScene* _scene;
public:
    explicit DebugDrawer(Loader<Shader> shaderLoader, physx::PxScene* scene)
        : _vao(0), _vbo(0), _scene(scene)
    {
        _debugShader = shaderLoader.load("debugline").value();
    }

    void setup()
    {
        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
    }

    void drawLine(const physx::PxVec3 &from,
        const physx::PxVec3 &to,
        const physx::PxU32 &color1,
        const physx::PxU32 &color2)
    {
        DebugPoint start{};
        DebugPoint end{};
        start.x = from.x;
        start.y = from.y;
        start.z = from.z;
        start.r = static_cast<uint8_t>(color1 >> 16);
        start.g = static_cast<uint8_t>(color1 >> 8);
        start.b = static_cast<uint8_t>(color1);
        _lines.push_back(start);
        end.x = to.x;
        end.y = to.y;
        end.z = to.z;
        end.r = static_cast<uint8_t>(color2 >> 16);
        end.g = static_cast<uint8_t>(color2 >> 8);
        end.b = static_cast<uint8_t>(color2);
        _lines.push_back(end);
    }
    void linesFromScene()
    {
        const physx::PxRenderBuffer& rb = _scene->getRenderBuffer();
        for (physx::PxU32 i = 0; i < rb.getNbLines(); i++)
        {
            const physx::PxDebugLine& line = rb.getLines()[i];
            drawLine(line.pos0, line.pos1, line.color0, line.color1);
        }
    }
    void draw(Camera &camera) {
        if (!_lines.empty()) {
            glBindVertexArray(_vao);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _lines.size() * 6, &_lines[0], GL_DYNAMIC_DRAW);

            // Position
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);
            // Colour
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            _debugShader->use();

            const glm::mat4 view = camera.getViewMatrix();
            const glm::mat4 projection = glm::perspective(glm::radians(camera.zoom()),
                static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
            _debugShader->setMat4("projection", projection);
            _debugShader->setMat4("view", view);

            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(_lines.size() * 6));

            _lines.clear();
        }
    }
};

