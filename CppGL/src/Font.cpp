#include "Font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <utility>
#include <string>
#include <functional>
#include <algorithm>

using namespace std;
using namespace glm;


Font::Font(filesystem::path path, const unsigned int size)
	: Logger("font"), _path(std::move(path)), _size(size), _vao(0), _vbo(0)
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

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (GLubyte c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		    console->error("failed to load font glyph");

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows,
			0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<GLuint>(face->glyph->advance.x)
		};
		_characters.insert(pair<GLchar, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Font::draw(const Shader *shader, string text, vec2 coOrd, const GLfloat scale, const vec3 color)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
	shader->use();
	shader->setVec3("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(_vao);

	for (string::const_iterator c = text.begin(); c != text.end(); ++c) {
		const auto ch = _characters[*c];

		const auto xpos = coOrd.x + ch.bearing.x * scale;
		const auto ypos = coOrd.y - (ch.size.y - ch.bearing.y) * scale;

		const auto w = ch.size.x * scale;
		const auto h = ch.size.y * scale;

		GLfloat vertices[6][4] = {
			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos, ypos, 0.0, 1.0 },
			{ xpos + w, ypos, 1.0, 1.0 },
			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos + w, ypos, 1.0, 1.0 },
			{ xpos + w, ypos + h, 1.0, 0.0 }
		};

		glBindTexture(GL_TEXTURE_2D, ch.textureId);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		coOrd.x += (ch.advance >> 6) * scale;
	}

	glDisable(GL_BLEND);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

optional<Font*> Font::fromResource(const Resource& resource, const Console& console)
{
	const auto base = filesystem::path("resources") / "fonts";
	auto rSize = resource["size"];
	if (rSize->empty())
	{
		console->error("font expects a size argument, when loading {}", resource.path().string());
	}

	assert(rSize.has_value());

	const auto size = stoi(resource["size"].value());
	
	return new Font(base / resource.path(), size);
}
