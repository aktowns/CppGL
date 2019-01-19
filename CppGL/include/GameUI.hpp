#pragma once

#include <glad/glad.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear.h>

#include <GLFW/glfw3.h>

#include "Logger.hpp"
#include "Renderable.hpp"
#include <functional>

#ifndef NK_GLFW_TEXT_MAX
#define NK_GLFW_TEXT_MAX 256
#endif

struct OpenGLContext {
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vertShdr;
    GLuint fragShdr;
    GLint attribPos;
    GLint attribUv;
    GLint attribCol;
    GLint uniformTex;
    GLint uniformProj;
    GLuint fontTex;
};

struct glfwVertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

class GameUI final : Logger, Renderable
{
    GLFWwindow* _win;
    struct nk_context _ctx;
    double _lastButtonClick;
    int _isDoubleClickDown;
    struct nk_vec2 _doubleClickPos;
    OpenGLContext _ogl;
    struct nk_font_atlas _atlas;
    int _width, _height;
    int _displayWidth, _displayHeight;
    struct nk_vec2 _fbScale;
    int _textLen;
    unsigned int _text[NK_GLFW_TEXT_MAX];
    struct nk_vec2 _scroll;
    static GameUI* _trampoline;
public:
    explicit GameUI(GLFWwindow* win);
    void draw(const GameObject& game, const RenderObject& render) override;

    void fontStashBegin(struct nk_font_atlas **atlas);
    void fontStashEnd();

    void scrollCallback(GLFWwindow *win, double xoff, double yoff);
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    void newFrame();

    void drawUI(const GameObject& game, const RenderObject& render, std::function<void(nk_context*)> f);

private:
    static void clipboardCopyTrampoline(nk_handle usr, const char *text, int len);
    void clipboardCopy(nk_handle usr, const char *text, int len);
    static void clipboardPasteTrampoline(nk_handle usr, struct nk_text_edit *edit);
    void clipboardPaste(nk_handle usr, struct nk_text_edit *edit);

    void deviceCreate();
    void deviceDestroy();
    void deviceUploadAtlas(const void *image, const int width, const int height);

    void charCallback(GLFWwindow *win, unsigned int codepoint);

    void shutdown();
};
