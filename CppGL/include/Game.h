#pragma once

#include "Logger.h"

#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Font.h"
#include "Loader.h"
#include "Texture.h"
#include "PhysXSetup.h"
#include "utils/DebugDrawer.h"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

struct GameObject final
{
	GLFWwindow* window;
	float deltaTime;
	int frameCount;
	int frameRate;
	glm::vec3 mouseCoOrds;
};

class Game final : public Logger {
	unsigned int _vao;
	unsigned int _lightVao;
	unsigned int _skyboxVao;
	unsigned int _vbo;
	unsigned int _skyboxVbo;
	unsigned int _skyboxTexture;
	Camera _camera;

	Loader<Font> _fontLoader;
	Loader<Shader> _shaderLoader;
	Loader<Model> _modelLoader;
	Loader<Texture> _textureLoader;
	PhysXSetup _physx{};
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