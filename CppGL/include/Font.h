#pragma once

#include <map>

#include <glm/glm.hpp>

#include <glad/glad.h>

#include "Shader.h"
#include "Logger.h"
#include "Loader.h"

#include <optional>

struct Character final
{
	GLuint textureId;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint advance;
};

class Font final : public Logger {
	std::filesystem::path _path;
	unsigned int _size;
	std::map<GLchar, Character> _characters;
	unsigned int _vao, _vbo;
public:
	Font(std::filesystem::path path, unsigned int size);

	void setup();
	void draw(const Shader *shader, std::string text, glm::vec2 coOrd, GLfloat scale, glm::vec3 color);
  static std::optional<Font*> fromResource(const Resource& resource, const Console& console);
};

