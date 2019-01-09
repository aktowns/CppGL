#pragma once

#include "Logger.h"
#include "utils/Resource.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader final : public Logger {
	std::filesystem::path _fragmentShader;
	std::filesystem::path _vertexShader;
	GLuint _program;
public:
	Shader(const std::filesystem::path& vertexShader, const std::filesystem::path& fragmentShader)
		: Logger("shader")
	    , _fragmentShader(fragmentShader)
	    , _vertexShader(vertexShader)
	    , _program(0)
	{
	};
	void setup();
	void use() const;

	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setVec3(const std::string &name, const glm::vec3 &value) const;
	void setVec3(const std::string &name, float x, float y, float z) const;
	void setMat4(const std::string &name, const glm::mat4 &mat) const;
	static Shader* fromResource(Resource& resource, const Console& console);

	GLuint program() const
	{
		return _program;
	}
};
