#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#define NK_IMPLEMENTATION
#include <nuklear.h>
#undef NK_IMPLEMENTATION

#include "GameUI.hpp"
#include <cassert>
#include <functional>

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)

#ifndef NK_GLFW_DOUBLE_CLICK_LO
#define NK_GLFW_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_HI
#define NK_GLFW_DOUBLE_CLICK_HI 0.2
#endif

GameUI* GameUI::_trampoline = nullptr;

GameUI::GameUI(GLFWwindow* win) : Logger("GameUI"), _win(win)
{
    nk_init_default(&_ctx, nullptr);
    _trampoline = this;
    _ctx.clip.copy = &GameUI::clipboardCopyTrampoline;
    _ctx.clip.paste = &GameUI::clipboardPasteTrampoline;
    _ctx.clip.userdata = nk_handle_ptr(nullptr);
    _lastButtonClick = 0;
    deviceCreate();
    _isDoubleClickDown = nk_false;
    _doubleClickPos = nk_vec2(0, 0);
}

void GameUI::draw(const GameObject& game, const RenderObject& render)
{
    struct nk_buffer vbuf, ebuf;
    GLfloat ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f,1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= static_cast<GLfloat>(_width);
    ortho[1][1] /= static_cast<GLfloat>(_height);

    /* setup global state */
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* setup program */
    glUseProgram(_ogl.prog);
    glUniform1i(_ogl.uniformTex, 0);
    glUniformMatrix4fv(_ogl.uniformProj, 1, GL_FALSE, &ortho[0][0]);
    glViewport(0, 0, static_cast<GLsizei>(_displayWidth), static_cast<GLsizei>(_displayHeight));
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        const nk_draw_index *offset = nullptr;

        /* allocate vertex and element buffer */
        glBindVertexArray(_ogl.vao);
        glBindBuffer(GL_ARRAY_BUFFER, _ogl.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ogl.ebo);

        glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_BUFFER, nullptr, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_ELEMENT_BUFFER, nullptr, GL_STREAM_DRAW);

        /* load draw vertices & elements directly into vertex + element buffer */
        const auto vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        const auto elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(glfwVertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(glfwVertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(glfwVertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            memset(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(glfwVertex);
            config.vertex_alignment = NK_ALIGNOF(glfwVertex);
            config.null = _ogl.null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = NK_ANTI_ALIASING_ON;
            config.line_AA = NK_ANTI_ALIASING_ON;

            /* setup buffers to load vertices and elements */
            nk_buffer_init_fixed(&vbuf, vertices, static_cast<size_t>(MAX_VERTEX_BUFFER));
            nk_buffer_init_fixed(&ebuf, elements, static_cast<size_t>(MAX_ELEMENT_BUFFER));
            nk_convert(&_ctx, &_ogl.cmds, &vbuf, &ebuf, &config);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, &_ctx, &_ogl.cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(cmd->texture.id));
            glScissor(
                static_cast<GLint>(cmd->clip_rect.x * _fbScale.x),
                static_cast<GLint>((_height - static_cast<GLint>(cmd->clip_rect.y + cmd->clip_rect.h)) * _fbScale.y),
                static_cast<GLint>(cmd->clip_rect.w * _fbScale.x),
                static_cast<GLint>(cmd->clip_rect.h * _fbScale.y));
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cmd->elem_count), GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&_ctx);
    }

    /* default OpenGL state */
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

void GameUI::fontStashBegin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&_atlas);
    nk_font_atlas_begin(&_atlas);
    *atlas = &_atlas;
}

void GameUI::fontStashEnd(void)
{
    int w, h;
    const auto image = nk_font_atlas_bake(&_atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    deviceUploadAtlas(image, w, h);
    nk_font_atlas_end(&_atlas, nk_handle_id(static_cast<int>(_ogl.fontTex)), &_ogl.null);
    if (_atlas.default_font)
        nk_style_set_font(&_ctx, &_atlas.default_font->handle);
}

void GameUI::newFrame(void)
{
    double x, y;

    glfwGetWindowSize(_win, &_width, &_height);
    glfwGetFramebufferSize(_win, &_displayWidth, &_displayHeight);
    _fbScale.x = static_cast<float>(_displayWidth) / static_cast<float>(_width);
    _fbScale.y = static_cast<float>(_displayHeight) / static_cast<float>(_height);

    nk_input_begin(&_ctx);
    for (auto i = 0; i < _textLen; ++i)
        nk_input_unicode(&_ctx, _text[i]);

    // MARK: Mouse grabbing
    /* optional grabbing behavior */
    if (_ctx.input.mouse.grab)
        glfwSetInputMode(_win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    else if (_ctx.input.mouse.ungrab)
        glfwSetInputMode(_win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    nk_input_key(&_ctx, NK_KEY_DEL, glfwGetKey(_win, GLFW_KEY_DELETE) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_ENTER, glfwGetKey(_win, GLFW_KEY_ENTER) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_TAB, glfwGetKey(_win, GLFW_KEY_TAB) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_BACKSPACE, glfwGetKey(_win, GLFW_KEY_BACKSPACE) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_UP, glfwGetKey(_win, GLFW_KEY_UP) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_DOWN, glfwGetKey(_win, GLFW_KEY_DOWN) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_TEXT_START, glfwGetKey(_win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_TEXT_END, glfwGetKey(_win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_SCROLL_START, glfwGetKey(_win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_SCROLL_END, glfwGetKey(_win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_SCROLL_DOWN, glfwGetKey(_win, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_SCROLL_UP, glfwGetKey(_win, GLFW_KEY_PAGE_UP) == GLFW_PRESS);
    nk_input_key(&_ctx, NK_KEY_SHIFT, glfwGetKey(_win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(_win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    if (glfwGetKey(_win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(_win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        nk_input_key(&_ctx, NK_KEY_COPY, glfwGetKey(_win, GLFW_KEY_C) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_PASTE, glfwGetKey(_win, GLFW_KEY_V) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_CUT, glfwGetKey(_win, GLFW_KEY_X) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_TEXT_UNDO, glfwGetKey(_win, GLFW_KEY_Z) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_TEXT_REDO, glfwGetKey(_win, GLFW_KEY_R) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_TEXT_WORD_LEFT, glfwGetKey(_win, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_TEXT_WORD_RIGHT, glfwGetKey(_win, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_TEXT_LINE_START, glfwGetKey(_win, GLFW_KEY_B) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_TEXT_LINE_END, glfwGetKey(_win, GLFW_KEY_E) == GLFW_PRESS);
    }
    else {
        nk_input_key(&_ctx, NK_KEY_LEFT, glfwGetKey(_win, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_RIGHT, glfwGetKey(_win, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(&_ctx, NK_KEY_COPY, 0);
        nk_input_key(&_ctx, NK_KEY_PASTE, 0);
        nk_input_key(&_ctx, NK_KEY_CUT, 0);
        nk_input_key(&_ctx, NK_KEY_SHIFT, 0);
    }

    glfwGetCursorPos(_win, &x, &y);
    nk_input_motion(&_ctx, static_cast<int>(x), static_cast<int>(y));
    // MARK: Mouse Grabbing
    if (_ctx.input.mouse.grabbed) {
        glfwSetCursorPos(_win, _ctx.input.mouse.prev.x, _ctx.input.mouse.prev.y);
        _ctx.input.mouse.pos.x = _ctx.input.mouse.prev.x;
        _ctx.input.mouse.pos.y = _ctx.input.mouse.prev.y;
    }
    nk_input_button(&_ctx, NK_BUTTON_LEFT, static_cast<int>(x), static_cast<int>(y), glfwGetMouseButton(_win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    nk_input_button(&_ctx, NK_BUTTON_MIDDLE, static_cast<int>(x), static_cast<int>(y), glfwGetMouseButton(_win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    nk_input_button(&_ctx, NK_BUTTON_RIGHT, static_cast<int>(x), static_cast<int>(y), glfwGetMouseButton(_win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    nk_input_button(&_ctx, NK_BUTTON_DOUBLE, static_cast<int>(_doubleClickPos.x), static_cast<int>(_doubleClickPos.y), _isDoubleClickDown);
    nk_input_scroll(&_ctx, _scroll);
    nk_input_end(&_ctx);
    _textLen = 0;
    _scroll = nk_vec2(0, 0);
}

void GameUI::drawUI(const GameObject& gameObject, const RenderObject& render, const std::function<void(nk_context*)> f) 
{
    newFrame();
    f(&_ctx);
    nk_end(&_ctx);
    draw(gameObject, render);
}

void GameUI::clipboardCopyTrampoline(nk_handle usr, const char *text, int len)
{
    assert(_trampoline != nullptr);
    _trampoline->clipboardCopy(usr, text, len);
}

void GameUI::clipboardCopy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = static_cast<char*>(malloc(static_cast<size_t>(len) + 1));
    if (!str) return;
    memcpy(str, text, static_cast<size_t>(len));
    str[len] = '\0';
    glfwSetClipboardString(_win, str);
    free(str);
}

void GameUI::clipboardPasteTrampoline(nk_handle usr, struct nk_text_edit *edit)
{
    assert(_trampoline != nullptr);
    _trampoline->clipboardPaste(usr, edit);
}

void GameUI::clipboardPaste(nk_handle usr, struct nk_text_edit *edit)
{
    const auto text = glfwGetClipboardString(_win);
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

void GameUI::deviceCreate()
{
    GLint status;
    static auto vertexShader =
        "#version 300 es\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 TexCoord;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main() {\n"
        "   Frag_UV = TexCoord;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
        "}\n";
    static auto fragmentShader =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    auto dev = &_ogl;
    nk_buffer_init_default(&dev->cmds);
    dev->prog = glCreateProgram();
    dev->vertShdr = glCreateShader(GL_VERTEX_SHADER);
    dev->fragShdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(dev->vertShdr, 1, &vertexShader, 0);
    glShaderSource(dev->fragShdr, 1, &fragmentShader, 0);
    glCompileShader(dev->vertShdr);
    glCompileShader(dev->fragShdr);
    glGetShaderiv(dev->vertShdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glGetShaderiv(dev->fragShdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glAttachShader(dev->prog, dev->vertShdr);
    glAttachShader(dev->prog, dev->fragShdr);
    glLinkProgram(dev->prog);
    glGetProgramiv(dev->prog, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);

    dev->uniformTex = glGetUniformLocation(dev->prog, "Texture");
    dev->uniformProj = glGetUniformLocation(dev->prog, "ProjMtx");
    dev->attribPos = glGetAttribLocation(dev->prog, "Position");
    dev->attribUv = glGetAttribLocation(dev->prog, "TexCoord");
    dev->attribCol = glGetAttribLocation(dev->prog, "Color");

    {
        /* buffer setup */
        const GLsizei vs = sizeof(glfwVertex);
        const auto vp = offsetof(glfwVertex, position);
        const auto vt = offsetof(glfwVertex, uv);
        const auto vc = offsetof(glfwVertex, col);

        glGenBuffers(1, &dev->vbo);
        glGenBuffers(1, &dev->ebo);
        glGenVertexArrays(1, &dev->vao);

        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glEnableVertexAttribArray(static_cast<GLuint>(dev->attribPos));
        glEnableVertexAttribArray(static_cast<GLuint>(dev->attribUv));
        glEnableVertexAttribArray(static_cast<GLuint>(dev->attribCol));

        glVertexAttribPointer(static_cast<GLuint>(dev->attribPos), 2, GL_FLOAT, GL_FALSE, vs, reinterpret_cast<void*>(vp));
        glVertexAttribPointer(static_cast<GLuint>(dev->attribUv), 2, GL_FLOAT, GL_FALSE, vs, reinterpret_cast<void*>(vt));
        glVertexAttribPointer(static_cast<GLuint>(dev->attribCol), 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, reinterpret_cast<void*>(vc));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GameUI::deviceDestroy()
{
    auto dev = &_ogl;
    glDetachShader(dev->prog, dev->vertShdr);
    glDetachShader(dev->prog, dev->fragShdr);
    glDeleteShader(dev->vertShdr);
    glDeleteShader(dev->fragShdr);
    glDeleteProgram(dev->prog);
    glDeleteTextures(1, &dev->fontTex);
    glDeleteBuffers(1, &dev->vbo);
    glDeleteBuffers(1, &dev->ebo);
    nk_buffer_free(&dev->cmds);
}

void GameUI::deviceUploadAtlas(const void *image, const int width, const int height)
{
    auto dev = &_ogl;
    glGenTextures(1, &dev->fontTex);
    glBindTexture(GL_TEXTURE_2D, dev->fontTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image);

}

void GameUI::charCallback(GLFWwindow *win, unsigned int codepoint)
{
    (void)win;
    if (_textLen < NK_GLFW_TEXT_MAX)
        _text[_textLen++] = codepoint;
}

void GameUI::scrollCallback(GLFWwindow *win, double xoff, double yoff)
{
    (void)win; (void)xoff;
    _scroll.x += static_cast<float>(xoff);
    _scroll.y += static_cast<float>(yoff);
}

void GameUI::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    double x, y;
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS) {
        const auto dt = glfwGetTime() - _lastButtonClick;
        if (dt > NK_GLFW_DOUBLE_CLICK_LO && dt < NK_GLFW_DOUBLE_CLICK_HI) {
            _isDoubleClickDown = nk_true;
            _doubleClickPos = nk_vec2(static_cast<float>(x), static_cast<float>(y));
        }
        _lastButtonClick = glfwGetTime();
    }
    else _isDoubleClickDown = nk_false;
}

void GameUI::shutdown(void)
{
    nk_font_atlas_clear(&_atlas);
    nk_free(&_ctx);
    deviceDestroy();
}
