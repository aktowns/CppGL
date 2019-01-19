#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Logger.hpp"
#include "Loader.hpp"

#include <optional>
#include <map>

struct Character final {
	glm::vec2 advance;
	glm::vec2 size;
	glm::vec2 bearing;
	glm::vec2 offset;
};

class Font final : public Logger {
	std::filesystem::path _path;
	unsigned int _size;
	std::map<unsigned long, Character> _characters;
	unsigned int _atlasWidth;
	unsigned int _atlasHeight;
	unsigned int _vao, _vbo;
	GLuint _texture;
public:
	Font(std::filesystem::path path, unsigned int size);

	void setup();

	void draw(const Shader *shader, std::string text, glm::vec2 coOrd, GLfloat scale, glm::vec3 color);

	static std::optional<Font *> fromResource(const Resource &resource, const Console &console);
};

