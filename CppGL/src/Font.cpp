#include "Font.hpp"
#include "Config.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>

#include <utility>
#include <string>
#include <functional>
#include <algorithm>

using namespace std;
using namespace glm;

// IS there an easier way?
#define IS_BASIC_LAT(x) (x >= 0x0020 && x <= 0x007E)
#define IS_EXT_LAT(x) (x >= 0x00A0 && x <= 0x00FF)
#define IS_PRIV(x) (x >= 0xE000 && x <= 0xF8FF)

#define PADDING 1

Font::Font(filesystem::path path, const unsigned int size)
    : Logger("font"), _path(std::move(path)), _size(size), _atlasWidth(0), _atlasHeight(0), _vao(0), _vbo(0)
{
}

void Font::setup()
{
    console->info("loading font {} with size {}", _path.string(), _size);

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        console->error("failed to initialize freetype");

    FT_Face face;
    if (FT_New_Face(ft, _path.string().c_str(), 0, &face))
        console->error("failed to load font file {}", _path.string());

    FT_Set_Pixel_Sizes(face, 0, _size);

    FT_UInt index;

    // Get the atlas size height of highest glyph and width of all the glyphs
    auto c = FT_Get_First_Char(face, &index);
    while (index) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            console->error("failed to load glyph at index: {}", index);
            continue;
        }
        if (IS_BASIC_LAT(c) || IS_PRIV(c)) {
            _atlasWidth += face->glyph->bitmap.width + PADDING;
            _atlasHeight = std::max(_atlasHeight, face->glyph->bitmap.rows);
        }
        c = FT_Get_Next_Char(face, c, &index);
    }

    console->debug("created texture {} by {}", _atlasWidth, _atlasHeight);

    // Build the atlas
    // Sanity check the font atlas we're creating is less than the maximum texture size we can use.
    //GLint maxTextureSize;
    //glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize); 
    //assert((_atlasWidth * _atlasHeight) < static_cast<unsigned int>(maxTextureSize));

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);

    unsigned char *pixels = new unsigned char[_atlasWidth*_atlasHeight];
    memset(pixels, 0, sizeof(*pixels));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _atlasWidth, _atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    index = 0;

    c = FT_Get_First_Char(face, &index);
    auto x = 0, y = 0;
    while (index) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            console->error("failed to load font glyph at index {}", index);
            continue;
        }
        if (IS_BASIC_LAT(c) || IS_PRIV(c)) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED,
                GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            Character chr{};
            chr.advance = vec2(face->glyph->advance.x >> 6, face->glyph->advance.y >> 6);
            chr.size = vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            chr.bearing = vec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
            chr.offset = vec2(x / static_cast<float>(_atlasWidth), y / static_cast<float>(_atlasHeight));

            _characters.insert(pair<unsigned long, Character>(c, chr));

            x += face->glyph->bitmap.width + PADDING;
        }
        c = FT_Get_Next_Char(face, c, &index);
    }

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
}

struct Point final
{
    GLfloat x;
    GLfloat y;
    GLfloat s;
    GLfloat t;
    Point(const GLfloat x, const GLfloat y, const GLfloat s, const GLfloat t) : x(x), y(y), s(s), t(t) {}
};


void Font::draw(const Shader *shader, string text, vec2 coOrd, const GLfloat scale_, const vec3 color)
{
    vector<Point> coords{};
    auto scale = vec2(scale_, scale_);
    for (string::const_iterator c = text.begin(); c != text.end(); ++c) {
        auto it = _characters.find(*c);
        assert(it != _characters.end());

        const auto ch = it->second;

        auto x2 = coOrd.x + ch.bearing.x * scale.x;
        auto y2 = -coOrd.y - ch.bearing.y * scale.y;
        auto w = ch.size.x * scale.x;
        auto h = ch.size.y * scale.y;

        coOrd.x += ch.advance.x * scale.x;
        coOrd.y += ch.advance.y * scale.y;

        if (w == 0 && h == 0) continue;

        coords.emplace_back(x2, -y2, ch.offset.x, ch.offset.y);
        coords.emplace_back(x2 + w, -y2, ch.offset.x + ch.size.x / _atlasWidth, ch.offset.y);
        coords.emplace_back(x2, -y2 - h, ch.offset.x, ch.offset.y + ch.size.y / _atlasHeight);
        coords.emplace_back(x2 + w, -y2, ch.offset.x + ch.size.x / _atlasWidth, ch.offset.y);
        coords.emplace_back(x2, -y2 - h, ch.offset.x, ch.offset.y + ch.size.y / _atlasHeight);
        coords.emplace_back(x2 + w, -y2 - h, ch.offset.x + ch.size.x / _atlasWidth, ch.offset.y + ch.size.y / _atlasHeight);
    }

    shader->use();
    shader->setVec3("textColor", color);
    shader->setInt("text", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(_vao);

    glBindTexture(GL_TEXTURE_2D, _texture);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * coords.size(), &coords[0], GL_DYNAMIC_DRAW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(coords.size()));

    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

optional<Font*> Font::fromResource(const Resource& resource, const Console& console)
{
    auto rSize = resource["size"];
    if (rSize->empty())
    {
        console->error("font expects a size argument, when loading {}", resource.path().string());
    }

    assert(rSize.has_value());

    const auto size = stoi(resource["size"].value());

    return new Font(FONTS_DIR / resource.path(), size);
}
