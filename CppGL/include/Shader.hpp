#pragma once

#include <glad/glad.h>

#include "Logger.hpp"
#include "utils/Resource.hpp"

#include <glm/glm.hpp>
#include <string>

class Shader final : public Logger {
	std::filesystem::path _fragmentShader;
	std::filesystem::path _vertexShader;
    std::optional<std::filesystem::path> _geometryShader;
	GLuint _program;
public:
    explicit Shader(std::filesystem::path vertexShader, std::filesystem::path fragmentShader);
    explicit Shader(std::filesystem::path vertexShader, std::filesystem::path fragmentShader, std::filesystem::path geometryShader);
	void use() const;

	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setVec3(const std::string &name, const glm::vec3 &value) const;
	void setVec3(const std::string &name, float x, float y, float z) const;
	void setVec4(const std::string &name, const glm::vec4 &value) const;
	void setVec4(const std::string &name, float x, float y, float z, float w) const;
	void setMat4(const std::string &name, const glm::mat4 &mat) const;
	static std::optional<Shader*> fromResource(const Resource& resource, const Console& console);

	GLuint program() const
	{
		return _program;
	}
private:
    enum ShaderType
    {
        Vertex = GL_VERTEX_SHADER, 
        Fragment = GL_FRAGMENT_SHADER, 
        Geometry = GL_GEOMETRY_SHADER
    };
    GLuint compileShader(ShaderType type, std::filesystem::path shader);
    void linkProgram(std::vector<GLuint> shaders);
};
