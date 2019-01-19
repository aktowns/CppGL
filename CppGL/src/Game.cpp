#include "Game.hpp"

#include "Model.hpp"
#include "Shader.hpp"
#include "Font.hpp"
#include "Config.hpp"
#include "Loader.hpp"
#include "Icosphere.hpp"
#include "utils/Conversions.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>

#include <string>

#include <iostream>

#include "GameUI.hpp"

#include <foundation/PxMat44.h>
#include <PxRigidDynamic.h>

#include "PhysXSetup.hpp"

using namespace std;
using namespace glm;
using namespace physx;

vector<std::string> cubeMapFaces
{
    "resources/textures/skybox/right.png",
    "resources/textures/skybox/left.png",
    "resources/textures/skybox/top.png",
    "resources/textures/skybox/bottom.png",
    "resources/textures/skybox/front.png",
    "resources/textures/skybox/back.png"
};

float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};

vec3 cubePositions[] = {
    vec3( 0.0f,  0.0f,  0.0f),
    vec3( 2.0f,  5.0f, -15.0f),
    vec3(-1.5f, -2.2f, -2.5f),
    vec3(-3.8f, -2.0f, -12.3f),
    vec3( 2.4f, -0.4f, -3.5f),
    vec3(-1.7f,  3.0f, -7.5f),
    vec3( 1.3f, -2.0f, -2.5f),
    vec3( 1.5f,  2.0f, -2.5f),
    vec3( 1.5f,  0.2f, -1.5f),
    vec3(-1.3f,  1.0f, -1.2f),
    vec3(-1.1f,  4.0f, -2.9f),
    vec3(0.0f,  -200.0f, 0.0f),
    vec3(-0.3f,  8.0f, -2.3f),
    vec3(-5.2f,  1.5f, -12.5f),
    vec3(-4.3f, -100.0f, -4.5f),
};
quat cubeRotations[15];

vec3 pointLightPositions[] = {
    vec3( 0.7f,  0.2f,  2.0f),
    vec3( 2.3f, -3.3f, -4.0f),
    vec3(-2.0f,  2.0f, -1.0f),
    vec3( 0.0f,  0.0f, -3.0f)
};

vec3 initialPointLightPositions[] = {
    vec3( 0.7f,  0.2f,  2.0f),
    vec3( 2.3f, -3.3f, -4.0f),
    vec3(-4.0f,  2.0f, -12.0f),
    vec3( 0.0f,  0.0f, -3.0f)
};

Game::Game() :
    Logger("game"), _vao(0), _lightVao(0), _icosphereVao(0), _vbo(0), _icosphereVbo(0), _icosphereEbo(0),
    _camera(glm::vec3(0.0f, 0.0f, 3.0f)),
    _fontLoader(Font::fromResource),
    _shaderLoader(Shader::fromResource),
    _modelLoader(Model::fromResource),
    _textureLoader(textureFromResource)//,
   // _skybox(Skybox(cubeMapFaces, _shaderLoader)), _ui(nullptr)
{
}

static const GLfloat overlayVertices[] = {
         -0.5f, -0.5f, 0.0f,  0.0,  0.0,
          0.5f, -0.5f, 0.0f,  1.0,  0.0,
         -0.5f,  0.5f, 0.0f,  0.0,  1.0,
          0.5f,  0.5f, 0.0f,  1.0,  1.0
};

DebugDrawer* drawer;

vector<vec3> icospherePos{};
vector<uint32> icosphereIndex{};

void Game::setup(GLFWwindow* window)
{
	//generateIcosphereMesh(5, icosphereIndex, icospherePos);
	//console->info("icosphere has {} indexes and {} vertices", icosphereIndex.size(), icospherePos.size());
	//_physx.initPhysics();

	//console->info("Loading models");
	//auto _model = _modelLoader.load("nanosuit/nanosuit.obj").value();
	//auto _link = _modelLoader.load("link/link.obj").value();
	//_model->setup();
	//_link->setup();
    //_modelLoader.load("spaceship/spaceship.fbx");
	//auto _spacetruck = _modelLoader.load("nanosuit/nanosuit.obj").value();

	//console->info("Loading shaders");
    //const auto overlayShader = _shaderLoader.load("overlay").value();
    //const auto overlayTextShader = _shaderLoader.load("overlaytext").value();
    //const auto shader = _shaderLoader.load("shader").value();
	//_shaderLoader.load("model");
	_shaderLoader.load("text");
    //const auto lightingShader = _shaderLoader.load("multiple_lights").value();
	//_shaderLoader.load("lamp");
	//_shaderLoader.load("basic");

	console->info("Loading fonts");
	_fontLoader.load(DEFAULT_FONT).value()->setup();
    //_fontLoader.load(GAME_FONT).value()->setup();

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_MULTISAMPLE);

    /*
    // Overlay start

    glGenVertexArrays(1, &_overlayVao);
    glGenBuffers(1, &_overlayVbo);
    glBindVertexArray(_overlayVao);

    glBindBuffer(GL_ARRAY_BUFFER, _overlayVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(overlayVertices), overlayVertices, GL_DYNAMIC_DRAW);

    // positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	// tex coords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
    */
    // Overlay end

	//glGenVertexArrays(1, &_icosphereVao);
	//glGenBuffers(1, &_icosphereVbo);
	//glGenBuffers(1, &_icosphereEbo);
	//glBindVertexArray(_icosphereVao);

	//glBindBuffer(GL_ARRAY_BUFFER, _icosphereVbo);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * icospherePos.size(), &icospherePos[0], GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _icosphereEbo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * icosphereIndex.size(), &icosphereIndex[0], GL_STATIC_DRAW);

	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	//glEnableVertexAttribArray(0);
	//

	//glGenVertexArrays(1, &_vao);
	//glGenBuffers(1, &_vbo);

	//glBindVertexArray(_vao);

	//glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
	//glEnableVertexAttribArray(0);

	// normals
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);

	// Texture CoOrds
    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    //glGenVertexArrays(1, &_lightVao);
    //glBindVertexArray(_lightVao);

    //glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void*>(nullptr));
    //glEnableVertexAttribArray(0);

	//auto texture1 = _textureLoader.load("container.png");
	//_textureLoader.load("container2.png");
	//_textureLoader.load("container2_specular.png");
    //_textureLoader.load("lamp.png?repeat=false");

    /*
	overlayShader->use();
	overlayShader->setInt("texture1", 0);

    lightingShader->use();
    lightingShader->setInt("material.diffuse", 0);
    lightingShader->setInt("material.specular", 1);
    */

    // UI
    //_ui = new GameUI(window);
    //struct nk_font_atlas *atlas;
    //_ui->fontStashBegin(&atlas);
    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
    //_ui->fontStashEnd();
	
    // Physx Debug
	//drawer = new DebugDrawer(_shaderLoader, _physx.scene);
	//drawer->setup();
}

auto clearColour = vec3(0.5f, 0.5f, 0.5f);
auto doStep = false;
bool showTextures = true;
bool showPhysicsDebug = true;
bool showSkybox = true;

bool toggles[1024] = { false };

PxRigidDynamic* cameraBody;

void Game::processInput(GameObject& gameObject)
{
	if (glfwGetKey(gameObject.window, GLFW_KEY_W) == GLFW_PRESS) {
		//cameraBody->getGlobalPose().transform(PxVec3(0, 1, 0));
		_camera.processKeyboard(CameraMovement::Forward, gameObject.deltaTime);
	}
	if (glfwGetKey(gameObject.window, GLFW_KEY_S) == GLFW_PRESS)
		_camera.processKeyboard(CameraMovement::Backward, gameObject.deltaTime);
	if (glfwGetKey(gameObject.window, GLFW_KEY_A) == GLFW_PRESS)
		_camera.processKeyboard(CameraMovement::Left, gameObject.deltaTime);
	if (glfwGetKey(gameObject.window, GLFW_KEY_D) == GLFW_PRESS)
		_camera.processKeyboard(CameraMovement::Right, gameObject.deltaTime);
	if (glfwGetKey(gameObject.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(gameObject.window, true);

	if (glfwGetKey(gameObject.window, GLFW_KEY_H) == GLFW_PRESS)
		clearColour -= 0.01;
	if (glfwGetKey(gameObject.window, GLFW_KEY_J) == GLFW_PRESS)
		clearColour += 0.01;

	auto keyK = glfwGetKey(gameObject.window, GLFW_KEY_K);
	if (keyK == GLFW_PRESS && !toggles[GLFW_KEY_K]) {
		showTextures = !showTextures;
		toggles[GLFW_KEY_K] = true;
	}
	else if (keyK == GLFW_RELEASE && toggles[GLFW_KEY_K]) {
		toggles[GLFW_KEY_K] = false;
	}

	auto keyL = glfwGetKey(gameObject.window, GLFW_KEY_L);
	if (keyL == GLFW_PRESS && !toggles[GLFW_KEY_L]) {
		showPhysicsDebug = !showPhysicsDebug;
		toggles[GLFW_KEY_L] = true;
	}
	else if (keyL == GLFW_RELEASE && toggles[GLFW_KEY_L]) {
		toggles[GLFW_KEY_L] = false;
	}

	auto keyP = glfwGetKey(gameObject.window, GLFW_KEY_P);
	if (keyP == GLFW_PRESS && !toggles[GLFW_KEY_P]) {
		showSkybox = !showSkybox;
		toggles[GLFW_KEY_P] = true;
	}
	else if (keyP == GLFW_RELEASE && toggles[GLFW_KEY_P]) {
		toggles[GLFW_KEY_P] = false;
	}

	auto keyM = glfwGetKey(gameObject.window, GLFW_KEY_M);
	if (keyM == GLFW_PRESS && !toggles[GLFW_KEY_M]) {
        _shaderLoader.clear();
		toggles[GLFW_KEY_M] = true;
	}
	else if (keyM == GLFW_RELEASE && toggles[GLFW_KEY_M]) {
		toggles[GLFW_KEY_M] = false;
	}

	auto keySpace = glfwGetKey(gameObject.window, GLFW_KEY_SPACE);
	if (keySpace == GLFW_PRESS && !toggles[GLFW_KEY_SPACE])
	{
		doStep = !doStep;
		toggles[GLFW_KEY_SPACE] = true;
	}
	else if (keySpace == GLFW_RELEASE && toggles[GLFW_KEY_SPACE]) {
		toggles[GLFW_KEY_SPACE] = false;
	}

	auto keyC = glfwGetKey(gameObject.window, GLFW_KEY_C);
	if (keyC == GLFW_PRESS && !toggles[GLFW_KEY_C]) {
		if (glfwGetInputMode(gameObject.window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			glfwSetInputMode(gameObject.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(gameObject.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		toggles[GLFW_KEY_C] = true;
	}
	else if (keyC == GLFW_RELEASE && toggles[GLFW_KEY_C]) {
		toggles[GLFW_KEY_C] = false;
	}
}

void Game::mouseMoved(GLFWwindow * window, glm::dvec2 pos, glm::dvec2 offset)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
		_camera.processMouseMovement(static_cast<float>(offset.x)
			, static_cast<float>(offset.y));
	}
	else {
	}
}

void Game::mouseScrolled(GLFWwindow * window, glm::dvec2 offset)
{
	_camera.processMouseScroll(static_cast<float>(offset.y));
    _ui->scrollCallback(window, offset.x, offset.y);
}

void Game::mouseButton(GLFWwindow* window, int button, int action, int mods)
{
    _ui->mouseButtonCallback(window, button, action, mods);
}

bool once = true;

PxRigidDynamic* bodies[15];

vec3 pointLightColours[] = {
	vec3(0.0f, 0.0f, 0.0f),
	vec3(0.0f, 0.0f, 0.5f),
	vec3(0.5f, 0.0f, 0.0f),
	vec3(0.0f, 0.5f, 0.0f)
};

void Game::render(GameObject& gameObject) 
{
    glClearColor(clearColour.x, clearColour.y, clearColour.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

    const auto textShader = _shaderLoader.load("text").value();
	textShader->use();
	const auto textProj = ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	textShader->setMat4("projection", textProj);
	auto arial = _fontLoader.load(DEFAULT_FONT).value();
	arial->draw(textShader, "fps: " + to_string(gameObject.frameRate), vec2(25.0f, SCR_HEIGHT - 45.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));

	arial->draw(textShader, "the quick brown fox jumps over the lazy dog", vec2(25.0f, SCR_HEIGHT - 85.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));
	arial->draw(textShader, "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG", vec2(65.0f, SCR_HEIGHT - 125.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));
}

void renderOld(GameObject& gameObject) 
{
    //const auto overlayShader = _shaderLoader.load("overlay").value();
    //const auto overlayTextShader = _shaderLoader.load("overlaytext").value();
    //const auto shader = _shaderLoader.load("shader").value();
    //const auto modelShader = _shaderLoader.load("model").value();
    //const auto textShader = _shaderLoader.load("text").value();
    //const auto lightingShader = _shaderLoader.load("multiple_lights").value();
    //const auto lampShader = _shaderLoader.load("lamp").value();
	//auto _model = _modelLoader.load("nanosuit/nanosuit.obj").value();
	//auto _link = _modelLoader.load("link/link.obj").value();
	//auto _spacetruck = _modelLoader.load("nanosuit/nanosuit.obj").value();
	//auto spacetruck = _modelLoader.load("spaceship/spaceship.fbx").value();
    //const auto basic = _shaderLoader.load("basic").value();

    //const string version(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    //const string vendor(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    //const string renderer(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    glClearColor(clearColour.x, clearColour.y, clearColour.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	//if (showPhysicsDebug) {
	//	drawer->linesFromScene();
	//	drawer->draw(_camera);
	//}

	// Use camera object to navigate the scene
    /*
	auto view = _camera.getViewMatrix();
	auto projection = perspective(radians(_camera.zoom())
		, static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);

    const auto render = RenderObject(view, projection);
    */

    // Overlay?
    /*
	glBindVertexArray(_overlayVao);
    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _textureLoader.load("lamp.png?repeat=false").value()->id);
    overlayShader->use();
    overlayShader->setInt("texture1", 0);
	overlayShader->setMat4("projection", projection);
	overlayShader->setMat4("view", view);
    overlayShader->setVec3("cameraUp", _camera.up());
    overlayShader->setVec3("cameraRight", _camera.right());
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    */
    /*
    for (auto pointLightPosition : pointLightPositions)
    {
        overlayShader->setVec3("position", pointLightPosition);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    */

    /*
	glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);

    overlayTextShader->use();
	overlayTextShader->setMat4("projection", projection);
	overlayTextShader->setMat4("view", view);
    overlayTextShader->setVec3("cameraUp", _camera.up());
    overlayTextShader->setVec3("cameraRight", _camera.right());
    overlayTextShader->setVec3("position", vec3(10.0f, 10.0f, 10.0f));
	auto game = _fontLoader.load(GAME_FONT).value();
    game->draw(overlayTextShader, "Testing", vec2(1.0, 1.0), 0.1f, vec3(1.0, 1.0, 1.0));
    overlayTextShader->setVec3("position", vec3(10.0f, 9.8f, 10.0f));
    game->draw(overlayTextShader, "some other text", vec2(1.0, 1.0), 0.1f, vec3(0.0, 0.0, 0.5));
    overlayTextShader->setVec3("position", vec3(10.0f, 9.6f, 10.0f));
    game->draw(overlayTextShader, "font rendering is borked", vec2(1.0, 1.0), 0.1f, vec3(0.0, 0.5, 0.0));
    */

    /*
	if (showTextures) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _textureLoader.load("container2.png").value()->id);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _textureLoader.load("container2_specular.png").value()->id);
	}
    */

    /*
	basic->use();
	basic->setMat4("projection", projection);
	basic->setMat4("view", view);
	basic->setFloat("unDT", gameObject.deltaTime);
	basic->setVec3("unColor", vec3(1.0, 1.0, 1.0));
	basic->setVec3("unCenterDir", vec3(0, 0, 0));
	basic->setFloat("unRadius", 0.3f);
	glBindVertexArray(_icosphereVao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _icosphereEbo);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(icosphereIndex.size()), GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	auto icom = mat4(1.0f);
	icom = glm::translate(icom, vec3(10.0, 10.0, 10.0));
	basic->setMat4("model", icom);
    */

	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, _textureLoader.load("awesomeface.png").value()->id);

    /*
    lightingShader->use();
    lightingShader->setVec3("viewPos", _camera.position());
    lightingShader->setFloat("material.shininess", 32.0f);
    */

    // Orbit the lights just to test..
    /*
    for (auto i = 0; i < 4; i++)
    {
       const auto deg = 3.14159 / 180.0;
        
       pointLightPositions[i].x = initialPointLightPositions[i].x * (2.0f * sinf(glfwGetTime() * deg * 20.0));
       pointLightPositions[i].z = initialPointLightPositions[i].z * (2.0f * cosf(glfwGetTime() * deg * 20.0));
    }
    */

    // directional light
    /*
    lightingShader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    lightingShader->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    lightingShader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    lightingShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    */

    // point light 1
    /*
    lightingShader->setVec3("pointLights[0].position", pointLightPositions[0]);
    lightingShader->setVec3("pointLights[0].ambient", 0.55f, 0.05f, 0.05f);
    lightingShader->setVec3("pointLights[0].diffuse", 1.0f, 0.0f, 0.0f);
    lightingShader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    lightingShader->setFloat("pointLights[0].constant", 1.0f);
    lightingShader->setFloat("pointLights[0].linear", 0.09f);
    lightingShader->setFloat("pointLights[0].quadratic", 0.032f);
	// point light 2
    lightingShader->setVec3("pointLights[1].position", pointLightPositions[1]);
    lightingShader->setVec3("pointLights[1].ambient", 0.55f, 0.05f, 0.05f);
    lightingShader->setVec3("pointLights[1].diffuse", 0.0f, 1.0f, 0.0f);
    lightingShader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    lightingShader->setFloat("pointLights[1].constant", 1.0f);
    lightingShader->setFloat("pointLights[1].linear", 0.09f);
    lightingShader->setFloat("pointLights[1].quadratic", 0.032f);
    // point light 3
    lightingShader->setVec3("pointLights[2].position", pointLightPositions[2]);
    lightingShader->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader->setVec3("pointLights[2].diffuse", 0.0f, 0.0f, 1.0f);
    lightingShader->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    lightingShader->setFloat("pointLights[2].constant", 1.0f);
    lightingShader->setFloat("pointLights[2].linear", 0.09f);
    lightingShader->setFloat("pointLights[2].quadratic", 0.032f);
    // point light 4
    lightingShader->setVec3("pointLights[3].position", pointLightPositions[3]);
    lightingShader->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader->setVec3("pointLights[3].diffuse", 1.0f, 1.0f, 0.0f);
    lightingShader->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    lightingShader->setFloat("pointLights[3].constant", 1.0f);
    lightingShader->setFloat("pointLights[3].linear", 0.09f);
    lightingShader->setFloat("pointLights[3].quadratic", 0.032f);
	// spotLight
    //_lightingShader->setVec3("spotLight.position", _camera.position());
    //_lightingShader->setVec3("spotLight.direction", _camera.front());
    //_lightingShader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    //_lightingShader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    //_lightingShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    //_lightingShader->setFloat("spotLight.constant", 1.0f);
    //_lightingShader->setFloat("spotLight.linear", 0.09);
    //_lightingShader->setFloat("spotLight.quadratic", 0.032);
    //_lightingShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    //_lightingShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));  

	lightingShader->setMat4("projection", projection);
	lightingShader->setMat4("view", view);
    const mat4 worldTransform(1.0f);
	lightingShader->setMat4("model", worldTransform);

	//_shader->use();
	//_shader->setMat4("projection", projection);
	//_shader->setMat4("view", view);
    */

    /*
	if (once) {
		for (unsigned int i = 0; i < 15; i++) {
			auto modelm = mat4(1.0f);
			modelm = translate(modelm, cubePositions[i]);
            const auto angle = 20.0f * i;
			modelm = rotate(modelm, radians(angle), vec3(1.0f, 0.3f, 0.5f));
			bodies[i] = _physx.createActor(PxTransform(mat44ToPx(modelm)));
		}
		
		{
			auto modelm = mat4(1.0f);
			modelm = translate(modelm, _camera.position());
			cameraBody = _physx.createActor(PxTransform(mat44ToPx(modelm)));
		}

		once = false;
	}
    */

	//_camera.setPosition(vec3ToGlm(cameraBody->getGlobalPose().p));

    /*
	for (unsigned int i = 0; i < 15; i++) {
        auto body = bodies[i];
		auto pose = body->getGlobalPose().p;
		auto rota = body->getGlobalPose().q;
		cubePositions[i] = vec3ToGlm(pose);
		cubeRotations[i] = quatToGlm(rota);
	}

	glBindVertexArray(_vao);
	for (unsigned int i = 0; i < 15; i++) {
		auto modelm = translate(mat4(1.0f), cubePositions[i]) * toMat4(cubeRotations[i]);

		lightingShader->use();
		lightingShader->setMat4("model", modelm);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

    */
    //lampShader->use();
    //lampShader->setMat4("projection", projection);
    //lampShader->setMat4("view", view);
    
	//glBindVertexArray(_lightVao);

	//for (auto i = 0; i < 4; i++)
	//{
	//	lampShader->setVec3("colour", pointLightColours[i]);
	//	auto modelm = mat4(1.0f);
	//	modelm = glm::translate(modelm, pointLightPositions[i]);
	//	lampShader->setMat4("model", modelm);
	//}
	//glBindTexture(GL_TEXTURE_2D, 0);
    //glBindVertexArray(0);


    /*
	modelShader->use();
	modelShader->setMat4("projection", projection);
	modelShader->setMat4("view", view);

	auto modelm = mat4(1.0f);
	modelm = translate(modelm, vec3(2.0f, -1.75f, 0.0f));
	modelm = scale(modelm, vec3(0.2f, 0.2f, 0.2f));
	modelShader->setMat4("model", modelm);
	spacetruck->draw(modelShader);
    */

	//glBindTexture(GL_TEXTURE_2D, 0);

    if (showSkybox) {
       // _skybox.draw(gameObject, render);
    }

    /*
	// Draw Crysis Model
	_modelShader->use();
	_modelShader->setMat4("projection", projection);
	_modelShader->setMat4("view", view);

	auto modelm = mat4(1.0f);
	modelm = translate(modelm, vec3(2.0f, -1.75f, 0.0f));
	modelm = scale(modelm, vec3(0.2f, 0.2f, 0.2f));
	_modelShader->setMat4("model", modelm);
	_model->draw(_modelShader);
	*/
	// Draw Link
	/*
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	
	modelm = mat4(1.0f);
	modelm = translate(modelm, vec3(-2.0f, -1.75f, 0.0f));
	modelm = scale(modelm, vec3(0.2f, 0.2f, 0.2f));
	_modelShader->setMat4("model", modelm);
	_link->draw(_modelShader);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	*/

    /*
	textShader->use();
	const auto textProj = ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	textShader->setMat4("projection", textProj);

	auto arial = _fontLoader.load(DEFAULT_FONT).value();
	arial->draw(textShader, "vendor: " + vendor, vec2(25.0f, SCR_HEIGHT - 45.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));
	arial->draw(textShader, "renderer: " + renderer, vec2(25.0f, SCR_HEIGHT - 60.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));
	arial->draw(textShader, "version: " + version, vec2(25.0f, SCR_HEIGHT - 75.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));
	arial->draw(textShader, "fps: " + to_string(gameObject.frameRate), vec2(25.0f, SCR_HEIGHT - 25.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));
    */

	//glDisable(GL_DEPTH_TEST);
    /*

    _ui->drawUI(gameObject, render, [gameObject, view, projection](nk_context* ctx)
    {
        if (nk_begin(ctx, "Inspector", nk_rect(SCR_WIDTH - 240, 10, 230, 250), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
            NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
        {
            auto client = to_string(gameObject.mouseCoOrds.x) + ',' + to_string(gameObject.mouseCoOrds.y);
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "client", NK_TEXT_ALIGN_LEFT);
            nk_text(ctx, client.c_str(), static_cast<int>(client.length()), NK_TEXT_ALIGN_RIGHT);
            const auto objectSpace = unProject(gameObject.mouseCoOrds, view, projection, vec4(0, 0, SCR_WIDTH, SCR_HEIGHT));
            auto obj = to_string(objectSpace.x) + ',' + to_string(objectSpace.y) + ',' + to_string(objectSpace.z);
            nk_label(ctx, "object", NK_TEXT_ALIGN_LEFT);
            nk_text(ctx, obj.c_str(), static_cast<int>(obj.length()), NK_TEXT_ALIGN_RIGHT);
        }
    });
    */

    //glEnable(GL_DEPTH_TEST);
}

void Game::update(GameObject& gameObject) {
	
	if (doStep) {
		_physx.stepPhysics(gameObject.deltaTime);
	}
}

