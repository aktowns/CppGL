#include <glad/glad.h>

#include "Game.hpp"
#include "Config.hpp"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>

using namespace std;
using namespace glm;

double lastX = SCR_WIDTH / 2.0f;
double lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float previousTime = 0.0f;
float lastFrame = 0.0f;

static GameObject gameObject{};
Game* game;

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    const auto xoffset = xpos - lastX;
    const auto yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    game->mouseMoved(window, dvec2(xpos, ypos), dvec2(xoffset, yoffset));
    gameObject.mouseCoOrds = vec3(xpos, ypos, 0);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    cout << "scroll callback" << endl;
    game->mouseScrolled(window, dvec2(xoffset, yoffset));
}

void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
    cerr << "debug: " << string(message) << endl;
}

void setCursor(GLFWwindow* window)
{
    unsigned char pixels[16 * 16 * 4];
    memset(pixels, 0xff, sizeof(pixels));

    GLFWimage image;
    image.width = 16;
    image.height = 16;
    image.pixels = pixels;

    GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
    glfwSetCursor(window, cursor);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    game->mouseButton(window, button, action, mods);
}

int main()
{
    spdlog::set_level(spdlog::level::debug);
    auto console = spdlog::stdout_color_mt("main");
    console->info("engine initializing.");

    glfwInit();

    const auto primary = glfwGetPrimaryMonitor();
    const auto mode = glfwGetVideoMode(primary);

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    auto window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLRender", nullptr, nullptr);
    if (window == nullptr)
    {
        console->warn("OpenGL 4.6 not supported, falling back to 3.3");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLRender", nullptr, nullptr);
    }
    if (window == nullptr) {
        console->error("glfwCreateWindow failed");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    setCursor(window);
    glfwSetWindowPos(window, (mode->width / 2) - (SCR_WIDTH / 2), (mode->height / 2) - (SCR_HEIGHT / 2));
    glfwSetWindowSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    console->info("Window created");

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        console->error("GLAD load failed");
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    if (GLAD_GL_KHR_debug) {
        console->info("OpenGL debugging enabled");
        //glDebugMessageCallback(debugCallback, nullptr);
    }
    else {
        console->error("OpenGL debugging not available");
    }

    game = new Game();
    game->setup(window);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gameObject.window = window;
    while (!glfwWindowShouldClose(window)) {
        const auto currentFrame = float(glfwGetTime());
        gameObject.deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        gameObject.frameCount++;

        game->processInput(gameObject);

        game->update(gameObject);

        game->render(gameObject);

        if (currentFrame - previousTime >= 1.0) {
            gameObject.frameRate = gameObject.frameCount;
            gameObject.frameCount = 0;
            previousTime = currentFrame;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
