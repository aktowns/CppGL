#include "Game.h"

#include "Model.h"
#include "Shader.h"
#include "Font.h"
#include "Config.h"
#include "Loader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>

#include <string>

#include <iostream>
#include <stb_image.h>
#include <PxPhysics.h>
#include <foundation/PxMat44.h>
#include <PxFoundation.h>
#include <PxRigidDynamic.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear.h>
#include "nuklear_glfw_gl3.h"

#include "PhysXSetup.h"
#include <PxPhysics.h>
#include <PxScene.h>
#include <PxRenderBuffer.h>
#include "../../third/PhysX/kaplademo/source/kaplaDemo/PhysXMacros.h"

using namespace std;
using namespace glm;
using namespace physx;

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)

unsigned int loadCubemap(vector<string> faces)
{
	unsigned int textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

	int width, height, nrChannels;
	for (auto i =0; i < faces.size(); i++)
	{
		cout << "loading: " << faces[i] << endl;
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			cout << "failed to load cubemap: " << faces[i] << endl;
		}
	}
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureId;
}

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
    vec3(-0.6f,  3.0f, -0.9f),
    vec3(-0.3f,  8.0f, -2.3f),
    vec3(-5.2f,  1.5f, -12.5f),
    vec3(-4.3f,  50.0f, -4.5f),
};
quat cubeRotations[15];

vec3 pointLightPositions[] = {
    vec3( 0.7f,  0.2f,  2.0f),
    vec3( 2.3f, -3.3f, -4.0f),
    vec3(-4.0f,  2.0f, -12.0f),
    vec3( 0.0f,  0.0f, -3.0f)
};

float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

//btDiscreteDynamicsWorld* dynamicsWorld;

Game::Game() :
	Logger("game"),
	_camera(glm::vec3(0.0f, 0.0f, 3.0f)),
	_fontLoader(Font::fromResource),
	_shaderLoader(Shader::fromResource),
	_modelLoader(Model::fromResource),
	_textureLoader(textureFromResource)
{
	
	/*
	const auto collisionConfiguration = new btDefaultCollisionConfiguration();
	const auto dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	const auto solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -1.0, 0));
	*/
}

struct nk_context *ctx;
DebugDrawer* drawer;

#define DEFAULT_FONT "arial.ttf?size=18"

void Game::setup(GLFWwindow* window)
{
	_physx.initPhysics();
	console->info("Loading models");
	auto _model = _modelLoader.load("nanosuit/nanosuit.obj").value();
	auto _link = _modelLoader.load("link/link.obj").value();
	//_model->setup();
	//_link->setup();

	console->info("Loading shaders");
	auto _shader = _shaderLoader.load("shader").value();
	auto _modelShader = _shaderLoader.load("model").value();
	auto _textShader = _shaderLoader.load("text").value();
	auto _lightingShader = _shaderLoader.load("multiple_lights").value();
	auto _lampShader = _shaderLoader.load("lamp").value();
	auto _skyboxShader = _shaderLoader.load("skybox").value();

	_lampShader->setup();
	_lightingShader->setup();
	_textShader->setup();
	_modelShader->setup();
	_shader->setup();
	_skyboxShader->setup();

	console->info("Loading fonts");
	_fontLoader.load(DEFAULT_FONT).value()->setup();

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_MULTISAMPLE);

	// Load skybox
    glGenVertexArrays(1, &_skyboxVao);
    glGenBuffers(1, &_skyboxVbo);
    glBindVertexArray(_skyboxVao);
    glBindBuffer(GL_ARRAY_BUFFER, _skyboxVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	_skyboxTexture = loadCubemap(cubeMapFaces);

	// 

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);

	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	// normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Texture CoOrds
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    glGenVertexArrays(1, &_lightVao);
    glBindVertexArray(_lightVao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


	//auto texture1 = _textureLoader.load("container.png");
	auto texture1 = _textureLoader.load("container2.png");
	_textureLoader.load("container2_specular.png");
	auto texture2 = _textureLoader.load("awesomeface.png");

	_shader->use();
	_shader->setInt("texture1", texture1.value()->id);
	_shader->setInt("texture2", texture2.value()->id);

    _skyboxShader->use();
    _skyboxShader->setInt("skybox", 0);

    _lightingShader->use();
    _lightingShader->setInt("material.diffuse", 0);
    _lightingShader->setInt("material.specular", 1);
    ctx = nk_glfw3_init(window, NK_GLFW3_DEFAULT);

	{struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
    nk_glfw3_font_stash_end();
    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
/*nk_style_set_font(ctx, &droid->handle);*/}

	
	drawer = new DebugDrawer(_shaderLoader, _physx.scene);
	drawer->setup();
	_physx.stepPhysics(0);
}

auto clearColour = vec3(0.5f, 0.5f, 0.5f);
auto doStep = false;
bool showTextures = true;
bool showPhysicsDebug = true;


bool toggles[1024] = { false };

void Game::processInput(GameObject& gameObject)
{
	if (glfwGetKey(gameObject.window, GLFW_KEY_W) == GLFW_PRESS)
		_camera.processKeyboard(CameraMovement::Forward, gameObject.deltaTime);
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
	


	auto keySpace = glfwGetKey(gameObject.window, GLFW_KEY_SPACE);
	if (keySpace == GLFW_PRESS && !toggles[GLFW_KEY_SPACE])
	{
		doStep = !doStep;
		toggles[GLFW_KEY_SPACE] = true;
	}
	else if (keySpace == GLFW_RELEASE && toggles[GLFW_KEY_SPACE]) {
		toggles[GLFW_KEY_SPACE] = false;
	}

	if (glfwGetKey(gameObject.window, GLFW_KEY_C) == GLFW_PRESS) {
		if (glfwGetInputMode(gameObject.window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			glfwSetInputMode(gameObject.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(gameObject.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
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
	nk_gflw3_scroll_callback(window, offset.x, offset.y);
}

void Game::mouseButton(GLFWwindow* window, int button, int action, int mods)
{
	nk_glfw3_mouse_button_callback(window, button, action, mods);
}

bool once = true;

PxRigidDynamic* bodies[15];

vec3 pointLightColours[] = {
	vec3(0.0f, 0.0f, 0.0f),
	vec3(0.0f, 0.0f, 0.5f),
	vec3(0.5f, 0.0f, 0.0f),
	vec3(0.0f, 0.5f, 0.0f)
};

inline PxVec3 vec3ToPx(const vec3& in) { return PxVec3(in.x, in.y, in.z); }
inline vec3 vec3ToGlm(const PxVec3& in) { return vec3(in.x, in.y, in.z); }
inline PxQuat quatToPx(const quat& in) { return PxQuat(in.x, in.y, in.z, in.w); }
inline quat quatToGlm(const PxQuat& in) { return quat(in.w, in.x, in.y, in.z); }
inline PxMat44 mat44ToPx(const mat4& in) {
	PxMat44 mat;
	mat[0][0] = in[0][0];
	mat[0][1] = in[0][1];
	mat[0][2] = in[0][2];
	mat[0][3] = in[0][3];
	mat[1][0] = in[1][0];
	mat[1][1] = in[1][1];
	mat[1][2] = in[1][2];
	mat[1][3] = in[1][3];
	mat[2][0] = in[2][0];
	mat[2][1] = in[2][1];
	mat[2][2] = in[2][2];
	mat[2][3] = in[2][3];
	mat[3][0] = in[3][0];
	mat[3][1] = in[3][1];
	mat[3][2] = in[3][2];
	mat[3][3] = in[3][3];
	return mat;
}
inline mat4 mat44ToGlm(const PxMat44& in) {
	mat4 mat;
	mat[0][0] = in[0][0];
	mat[0][1] = in[0][1];
	mat[0][2] = in[0][2];
	mat[0][3] = in[0][3];
	mat[1][0] = in[1][0];
	mat[1][1] = in[1][1];
	mat[1][2] = in[1][2];
	mat[1][3] = in[1][3];
	mat[2][0] = in[2][0];
	mat[2][1] = in[2][1];
	mat[2][2] = in[2][2];
	mat[2][3] = in[2][3];
	mat[3][0] = in[3][0];
	mat[3][1] = in[3][1];
	mat[3][2] = in[3][2];
	mat[3][3] = in[3][3];
	return mat;
}


void Game::render(GameObject& gameObject) 
{
	//dynamicsWorld->debugDrawWorld();

	//auto _shader = _shaderLoader.load("shader").value();
	auto _modelShader = _shaderLoader.load("model").value();
	auto _textShader = _shaderLoader.load("text").value();
	auto _lightingShader = _shaderLoader.load("multiple_lights").value();
	auto _lampShader = _shaderLoader.load("lamp").value();
	auto _skyboxShader = _shaderLoader.load("skybox").value();
	auto _model = _modelLoader.load("nanosuit/nanosuit.obj").value();
	auto _link = _modelLoader.load("link/link.obj").value();

	string version(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	string vendor(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
	string renderer(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    glClearColor(clearColour.x, clearColour.y, clearColour.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	if (showTextures) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _textureLoader.load("container2.png").value()->id);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _textureLoader.load("container2_specular.png").value()->id);
	}

	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, _textureLoader.load("awesomeface.png").value()->id);

    _lightingShader->use();
    _lightingShader->setVec3("viewPos", _camera.position());
    _lightingShader->setFloat("material.shininess", 32.0f);

    // directional light
    _lightingShader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    _lightingShader->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    _lightingShader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    _lightingShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // point light 1
    _lightingShader->setVec3("pointLights[0].position", pointLightPositions[0]);
	_lightingShader->setVec3("pointLights[0].ambient", pointLightColours[0].x,
		pointLightColours[0].y, pointLightColours[0].z);
    //_lightingShader->setVec3("pointLights[0].ambient", 0.55f, 0.05f, 0.05f);
    _lightingShader->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    _lightingShader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    _lightingShader->setFloat("pointLights[0].constant", 1.0f);
    _lightingShader->setFloat("pointLights[0].linear", 0.22);
    _lightingShader->setFloat("pointLights[0].quadratic", 0.020);
	// point light 2
    _lightingShader->setVec3("pointLights[1].position", pointLightPositions[1]);
	_lightingShader->setVec3("pointLights[1].ambient", pointLightColours[1].x,
		pointLightColours[1].y, pointLightColours[1].z);
    //_lightingShader->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    _lightingShader->setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
    _lightingShader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    _lightingShader->setFloat("pointLights[1].constant", 1.0f);
    _lightingShader->setFloat("pointLights[1].linear", 0.09);
    _lightingShader->setFloat("pointLights[1].quadratic", 0.032);
    // point light 3
    _lightingShader->setVec3("pointLights[2].position", pointLightPositions[2]);
    //_lightingShader->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	_lightingShader->setVec3("pointLights[2].ambient", pointLightColours[2].x,
		pointLightColours[2].y, pointLightColours[2].z);
    _lightingShader->setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
    _lightingShader->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    _lightingShader->setFloat("pointLights[2].constant", 1.0f);
    _lightingShader->setFloat("pointLights[2].linear", 0.09);
    _lightingShader->setFloat("pointLights[2].quadratic", 0.032);
    // point light 4
    _lightingShader->setVec3("pointLights[3].position", pointLightPositions[3]);
    _lightingShader->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
    _lightingShader->setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
    _lightingShader->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    _lightingShader->setFloat("pointLights[3].constant", 1.0f);
    _lightingShader->setFloat("pointLights[3].linear", 0.09);
    _lightingShader->setFloat("pointLights[3].quadratic", 0.032);
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

	// Use camera object to navigate the scene
	auto view = _camera.getViewMatrix();
	auto projection = perspective(radians(_camera.zoom())
		, static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);

	_lightingShader->setMat4("projection", projection);
	_lightingShader->setMat4("view", view);
	mat4 worldTransform(1.0f);
	_lightingShader->setMat4("model", worldTransform);

	//_shader->use();
	//_shader->setMat4("projection", projection);
	//_shader->setMat4("view", view);

	/*
	if (bodies[0])
	{
		btTransform trans;
		for (int i = 0; i < 10; i++)
		{
			bodies[i]->getMotionState()->getWorldTransform(trans);
			cubePositions[i].x = trans.getOrigin().x();
			cubePositions[i].y = trans.getOrigin().y();
			cubePositions[i].z = trans.getOrigin().z();
		}
	}
	*/

	if (once) {
		for (unsigned int i = 0; i < 15; i++) {
			//PxMat44 nvModel = PxMat44(PxIdentity);
			//nvModel.transform(GlmtoPx(cubePositions[i]));

			auto modelm = mat4(1.0f);
			modelm = translate(modelm, cubePositions[i]);
			auto angle = 20.0f * i;
			modelm = rotate(modelm, radians(angle), vec3(1.0f, 0.3f, 0.5f));
			auto m = quat_cast(modelm);
			//physx::PxTransform localTm(physx::PxVec3(cubePositions[i].x, cubePositions[i].y, cubePositions[i].z));
			//auto z = physx::PxTransform(m.x, m.y, m.z);
			bodies[i] = _physx.createActor(PxTransform(mat44ToPx(modelm)));
//				localTm.transform(z));
		}
		/*
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(10.), btScalar(50.)));
		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -26, 0));
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btVector3 localInertia(0, 0, 0);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		body->setFriction(1.0);
		body->setRestitution(1.0);
		dynamicsWorld->addRigidBody(body);

		for (unsigned int i = 0; i < 10; i++) {
			auto modelm = mat4(1.0f);
			modelm = translate(modelm, cubePositions[i]);
			auto angle = 20.0f * i;
			modelm = rotate(modelm, radians(angle), vec3(1.0f, 0.3f, 0.5f));

			auto m = quat_cast(modelm);
			auto motionstate = new btDefaultMotionState(btTransform(
				btQuaternion(m.x, m.y, m.z, m.w), 
				btVector3(cubePositions[i].x, cubePositions[i].y, cubePositions[i].z)
			));

			auto box = new btBoxShape(btVector3(0.53f, 0.53f, 0.53f));

			auto r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			auto body = new btRigidBody(r, motionstate, box);
			bodies[i] = body;
			body->setRestitution(1.0);
			body->setFriction(1.0);
			body->setRollingFriction(1.0);

			dynamicsWorld->addRigidBody(body);
		}
		// Crysis model physics (bounding box)
		mat4 modelm = mat4(1.0f);
		modelm = translate(modelm, vec3(2.0f, -1.75f, 0.0f));
		modelm = scale(modelm, vec3(0.2f, 0.2f, 0.2f));

		model.BoundingBox(mat4());

		quat m2 = glm::quat_cast(modelm); // object space <-> physics space?
		btDefaultMotionState* motionstate2 = new btDefaultMotionState(btTransform(
			btQuaternion(m2.x, m2.y, m2.z, m2.w), 
			btVector3(2.0f, -1.75f, 0.0f)
		));

		auto modelTestBody = new btRigidBody(0.01, motionstate2, model.box);

		dynamicsWorld->addRigidBody(modelTestBody);
		*/
		once = false;
	}

	for (unsigned int i = 0; i < 15; i++) {
		PxRigidDynamic* body = bodies[i];
		auto pose = body->getGlobalPose().p;
		auto rota = body->getGlobalPose().q;
		cubePositions[i] = vec3ToGlm(pose);
		cubeRotations[i] = quatToGlm(rota);
	}

	glBindVertexArray(_vao);
	for (unsigned int i = 0; i < 15; i++) {
		auto modelm = translate(mat4(1.0f), cubePositions[i]) * toMat4(cubeRotations[i]);

		_lightingShader->use();
		_lightingShader->setMat4("model", modelm);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

    _lampShader->use();
    _lampShader->setMat4("projection", projection);
    _lampShader->setMat4("view", view);
    
	glBindVertexArray(_lightVao);
	for (int i = 0; i < 4; i++)
	{
		_lampShader->setVec3("colour", pointLightColours[i]);
		auto modelm = mat4(1.0f);
		modelm = glm::translate(modelm, pointLightPositions[i]);
		modelm = glm::scale(modelm, glm::vec3(0.2f)); // Make it a smaller cube
		_lampShader->setMat4("model", modelm);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		pointLightColours[i].x += 0.01f;
		if (pointLightColours[i].x > 1.0f)
			pointLightColours[i].x = 0;
		pointLightColours[i].y += 0.01f;
		if (pointLightColours[i].y > 1.0f)
			pointLightColours[i].y = 0;
		pointLightColours[i].z += 0.01f;
		if (pointLightColours[i].z > 1.0f)
			pointLightColours[i].z = 0;
	}

    glBindVertexArray(0);

	// Draw skybox
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    _skyboxShader->use();
    view = glm::mat4(glm::mat3(_camera.getViewMatrix())); // remove translation from the view matrix
    _skyboxShader->setMat4("view", view);
    _skyboxShader->setMat4("projection", projection);
    // skybox cube
    glBindVertexArray(_skyboxVao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
	// end draw skybox

	glBindTexture(GL_TEXTURE_2D, 0);
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

	_textShader->use();
	const auto textProj = ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	_textShader->setMat4("projection", textProj);

	auto _arial = _fontLoader.load(DEFAULT_FONT).value();
	_arial->draw(_textShader, "vendor: " + vendor, vec2(25.0f, SCR_HEIGHT - 45.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));
	_arial->draw(_textShader, "renderer: " + renderer, vec2(25.0f, SCR_HEIGHT - 60.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));
	_arial->draw(_textShader, "version: " + version, vec2(25.0f, SCR_HEIGHT - 75.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));
	_arial->draw(_textShader, "fps: " + to_string(gameObject.frameRate), vec2(25.0f, SCR_HEIGHT - 25.0f), 1.0f, vec3(0.5f, 0.8f, 0.2f));

	if (showPhysicsDebug) {
		drawer->linesFromScene();
		drawer->draw(_camera);
	}

	glDisable(GL_DEPTH_TEST);

	nk_glfw3_new_frame();
	if (nk_begin(ctx, "Inspector", nk_rect(SCR_WIDTH - 240, 10, 230, 250), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
	{
		auto client = to_string(gameObject.mouseCoOrds.x) + ',' + to_string(gameObject.mouseCoOrds.y);
		nk_layout_row_dynamic(ctx, 20, 1);
		nk_label(ctx, "client", NK_TEXT_ALIGN_LEFT);
		nk_text(ctx, client.c_str(), static_cast<int>(client.length()), NK_TEXT_ALIGN_RIGHT);
		auto objectSpace = unProject(gameObject.mouseCoOrds, view, projection, vec4(0, 0, SCR_WIDTH, SCR_HEIGHT));
		auto obj = to_string(objectSpace.x) + ',' + to_string(objectSpace.y) + ',' + to_string(objectSpace.z);
		nk_label(ctx, "object", NK_TEXT_ALIGN_LEFT);
		nk_text(ctx, obj.c_str(), static_cast<int>(obj.length()), NK_TEXT_ALIGN_RIGHT);
	}
	nk_end(ctx);

	nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
}

void Game::update(GameObject& gameObject) {
	
	if (doStep) {
		_physx.stepPhysics(gameObject.deltaTime);
	}
}

