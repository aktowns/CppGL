#pragma once

#include "Logger.hpp"

#include "Camera.hpp"
#include "Shader.hpp"
#include "Model.hpp"
#include "Font.hpp"
#include "Loader.hpp"
#include "Texture.hpp"
#include "PhysXSetup.hpp"
#include "Renderable.hpp"
#include "utils/DebugDrawer.hpp"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Skybox.hpp"
#include "GameUI.hpp"

class Game final : public Logger {
    unsigned int _vao;
    unsigned int _lightVao;
    unsigned int _icosphereVao;
    unsigned int _vbo;
    unsigned int _icosphereVbo;
    unsigned int _icosphereEbo;
    unsigned int _overlayVao;
    unsigned int _overlayVbo;
    Camera _camera;

    Loader<Font> _fontLoader;
    Loader<Shader> _shaderLoader;
    Loader<Model> _modelLoader;
    Loader<Texture> _textureLoader;
    PhysXSetup _physx{};
    //Skybox _skybox;
    GameUI* _ui;
public:
    Game();
    void setup(GLFWwindow* window);
    void processInput(GameObject& gameObject);
    void update(GameObject& gameObject);
    void mouseMoved(GLFWwindow* window, glm::dvec2 pos, glm::dvec2 offset);
    void mouseScrolled(GLFWwindow* window, glm::dvec2 offset);
    void mouseButton(GLFWwindow* window, int button, int action, int mods);
    void render(GameObject& gameObject);
};