#include <msdfgen.h>

#include "Font.hpp"
#include "Config.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>

#include <utility>
#include <string>
#include <functional>
#include <algorithm>
#include "include/freetype/ftoutln.h"

using namespace std;
using namespace glm;

// IS there an easier way?
#define IS_BASIC_LAT(x) (x >= 0x0020 && x <= 0x007E)
#define IS_EXT_LAT(x) (x >= 0x00A0 && x <= 0x00FF)
#define IS_PRIV(x) (x >= 0xE000 && x <= 0xF8FF)

#define PADDING 2
#define WIDTH 1024

Font::Font(filesystem::path path, const unsigned int size)
    : Logger("font"), _path(std::move(path)), _size(size), _atlasWidth(0), _atlasHeight(0), _vao(0), _vbo(0)
{
}

struct FtContext {
    msdfgen::Point2 position;
    msdfgen::Shape *shape;
    msdfgen::Contour *contour;
};

static msdfgen::Point2 ftPoint2(const FT_Vector &vector) {
    return msdfgen::Point2(vector.x / 64., vector.y / 64.);
}

struct Bitmapped
{
    vector<uint8_t> bitmap;
    unsigned int width;
    unsigned int height;
};

Bitmapped msdfg(FT_Face& face, FT_ULong chr)
{
    //FT_Error error = FT_Load_Char(face, chr, FT_LOAD_NO_SCALE);
    //assert(!error);
    msdfgen::Shape shape{};
    shape.contours.clear();
    shape.inverseYAxis = false;

    FtContext context{};
    context.shape = &shape;
    FT_Outline_Funcs ftFunctions;
    ftFunctions.move_to = [](const FT_Vector *to, void *user)
    {
        auto context = reinterpret_cast<FtContext *>(user);
        context->contour = &context->shape->addContour();
        context->position = ftPoint2(*to);
        return 0;
    };
    ftFunctions.line_to = [](const FT_Vector *to, void *user)
    {
        auto context = reinterpret_cast<FtContext *>(user);
        context->contour->addEdge(new msdfgen::LinearSegment(context->position, ftPoint2(*to)));
        context->position = ftPoint2(*to);
        return 0;
    };
    ftFunctions.conic_to = [](const FT_Vector *control, const FT_Vector *to, void *user)
    {
        auto context = reinterpret_cast<FtContext *>(user);
        context->contour->addEdge(new msdfgen::QuadraticSegment(context->position, ftPoint2(*control), ftPoint2(*to)));
        context->position = ftPoint2(*to);
        return 0;
    };
    ftFunctions.cubic_to = [](const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user)
    {
        auto context = reinterpret_cast<FtContext *>(user);
        context->contour->addEdge(new msdfgen::CubicSegment(context->position, ftPoint2(*control1), ftPoint2(*control2), ftPoint2(*to)));
        context->position = ftPoint2(*to);
        return 0;
    };
    ftFunctions.shift = 0;
    ftFunctions.delta = 0;
    FT_Outline_Decompose(&face->glyph->outline, &ftFunctions, &context);

    shape.normalize();

    msdfgen::edgeColoringSimple(shape, 3.0);
    msdfgen::Bitmap<msdfgen::FloatRGB> msdf(32, 32);
    msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));

    vector<uint8_t> res{};
    for (auto y = 0; y < msdf.width(); ++y)
    {
        for (auto x = 0; x < msdf.height(); ++x)
        {
            const auto v = msdf(x, y);
            res.push_back(static_cast<uint8_t>(std::clamp(int(v.r * 0x100), 0, 0xff)));
            res.push_back(static_cast<uint8_t>(std::clamp(int(v.g * 0x100), 0, 0xff)));
            res.push_back(static_cast<uint8_t>(std::clamp(int(v.b * 0x100), 0, 0xff)));
        }
    }

    Bitmapped bitmap{};
    bitmap.bitmap = res;
    bitmap.width = msdf.width();
    bitmap.height = msdf.height();

    return bitmap;
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
    //FT_Set_Char_Size(face, 0, 32 * 64, 96, 0);

    FT_UInt index;
    unsigned int roww = 0;
    unsigned int rowh = 0;

    // Get the atlas size height of highest glyph and width of all the glyphs
    auto c = FT_Get_First_Char(face, &index);
    while (index) {
        if (FT_Load_Char(face, c, FT_LOAD_DEFAULT)) {
            console->error("failed to load glyph at index: {}", index);
            c = FT_Get_Next_Char(face, c, &index);
            continue;
        }
        if (IS_BASIC_LAT(c) || IS_PRIV(c)) {
            auto m = msdfg(face, c);
            if (roww + m.width + PADDING + 1 >= WIDTH)
            {
                _atlasWidth = std::max(_atlasWidth, roww);
                _atlasHeight += rowh;
                roww = 0;
                rowh = 0;
            }
            roww += m.width + PADDING; //face->glyph->bitmap.width + PADDING;
            rowh = std::max(rowh, m.height); //std::max(rowh, face->glyph->bitmap.rows);
        }
        c = FT_Get_Next_Char(face, c, &index);
    }
    _atlasWidth = std::max(_atlasWidth, rowh);
    _atlasHeight += rowh;

    console->debug("created texture {} by {}", _atlasWidth, _atlasHeight);

    // Build the atlas
    // Sanity check the font atlas we're creating is less than the maximum texture size we can use.
    //GLint maxTextureSize;
    //glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize); 
    //assert((_atlasWidth * _atlasHeight) < static_cast<unsigned int>(maxTextureSize));

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, _atlasWidth, _atlasHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    index = 0;

    rowh = 0;

    c = FT_Get_First_Char(face, &index);
    auto x = 0, y = 0;
    while (index) {
        if (FT_Load_Char(face, c, FT_LOAD_DEFAULT)) {
            console->error("failed to load font glyph at index {}", index);
            continue;
        }
        if (IS_BASIC_LAT(c) || IS_PRIV(c)) {
            auto m = msdfg(face, c);
            if (x + m.width + PADDING >= WIDTH)
            {
                y += rowh;
                rowh = 0;
                x = 0;
            }
            //glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RGB,
            //    GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
            //glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RGB,
            //    GL_UNSIGNED_BYTE, &m.bitmap[0]);
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, m.width, m.height, GL_RGB,
                GL_UNSIGNED_BYTE, &m.bitmap[0]);

            Character chr{};
            chr.advance = vec2(face->glyph->advance.x >> 6, face->glyph->advance.y >> 6);
            chr.size = vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            //chr.size = vec2(m.width, m.height); //vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
            chr.bearing = vec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
            chr.offset = vec2(x / static_cast<float>(_atlasWidth), y / static_cast<float>(_atlasHeight));

            console->debug("chr={} advance={},{} size={},{} bearing={},{} offset={},{}",
                (char)c, chr.advance.x, chr.advance.y, chr.size.x, chr.size.y, chr.bearing.x, chr.bearing.y,
                chr.offset.x, chr.offset.y);

            _characters.insert(pair<unsigned long, Character>(c, chr));

            rowh = std::max(rowh, m.height);
            x += m.width + PADDING; //face->glyph->bitmap.width + PADDING;
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
    shader->setVec4("fgColor", vec4(color.r, color.g, color.b, 1.0));
    shader->setVec4("bgColor", vec4(0.0, 0.0, 0.0, 0.1));
    shader->setInt("msdf", 0);
    shader->setFloat("pxRange", 4.0);

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
