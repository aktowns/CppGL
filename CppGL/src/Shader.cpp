#include "Shader.hpp"
#include "utils/Resource.hpp"
#include "utils/FmtExtensions.hpp"

#include <fstream>

using namespace std;

Shader::Shader(const std::filesystem::path& vertexShader, const std::filesystem::path& fragmentShader)
    : Logger("shader")
    , _fragmentShader(fragmentShader)
    , _vertexShader(vertexShader)
    , _program(0)
{
    console->info("loading vertex shader {}", _vertexShader);
    auto vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    auto fragmentHandle = glCreateShader(GL_FRAGMENT_SHADER);

    if (!filesystem::exists(_vertexShader))
    {
        console->error("vertex file {} does not exist.", _vertexShader);
    }

    ifstream vertexFile(_vertexShader);
    string vertexSource((istreambuf_iterator<char>(vertexFile)), (istreambuf_iterator<char>()));

    auto cVertexSource = vertexSource.c_str();
    glShaderSource(vertexHandle, 1, &cVertexSource, nullptr);
    glCompileShader(vertexHandle);

    GLint vertexCompiled;
    glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &vertexCompiled);
    if (vertexCompiled != GL_TRUE) {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetShaderInfoLog(vertexHandle, 1024, &logLength, message);

        console->error("vertex shader compilation failed: {}", string(message));
    }

    console->info("loading fragment shader {}", _fragmentShader);

    if (!filesystem::exists(_fragmentShader))
    {
        console->error("fragment file {} does not exist.", _fragmentShader);
    }

    ifstream fragmentFile(_fragmentShader);
    string fragmentSource((istreambuf_iterator<char>(fragmentFile)), (istreambuf_iterator<char>()));

    auto cFragmentSource = fragmentSource.c_str();
    glShaderSource(fragmentHandle, 1, &cFragmentSource, nullptr);
    glCompileShader(fragmentHandle);

    GLint fragmentCompiled;
    glGetShaderiv(fragmentHandle, GL_COMPILE_STATUS, &fragmentCompiled);
    if (fragmentCompiled != GL_TRUE) {
        auto log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(fragmentHandle, 1024, &log_length, message);

        console->error("fragment shader compilation failed: {}", string(message));
    }

    _program = glCreateProgram();
    glAttachShader(_program, vertexHandle);
    glAttachShader(_program, fragmentHandle);
    glLinkProgram(_program);

    GLint programLinked;
    glGetProgramiv(_program, GL_LINK_STATUS, &programLinked);
    if (programLinked != GL_TRUE) {
        auto logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(_program, 1024, &logLength, message);

        console->error("failed to link shader with files {} and {}: {}", _fragmentShader, _vertexShader,
            string(message));
    }

    glDeleteShader(vertexHandle);
    glDeleteShader(fragmentHandle);
}

void Shader::setBool(const std::string & name, const bool value) const
{
	glUniform1i(glGetUniformLocation(_program, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string & name, const int value) const
{
	glUniform1i(glGetUniformLocation(_program, name.c_str()), value);
}

void Shader::setFloat(const std::string & name, const float value) const
{
	glUniform1f(glGetUniformLocation(_program, name.c_str()), value);
}

void Shader::setVec3(const std::string & name, const glm::vec3 & value) const
{
	glUniform3fv(glGetUniformLocation(_program, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string & name, const float x, const float y, const float z) const
{
	glUniform3f(glGetUniformLocation(_program, name.c_str()), x, y, z);
}

void Shader::setMat4(const std::string & name, const glm::mat4 & mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(_program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::use() const
{
	glUseProgram(_program);
}

optional<Shader*> Shader::fromResource(const Resource& resource, const Console& console)
{
	const auto base = filesystem::path("resources") / "shaders";

	const auto vertexShader = (base / resource.path()).replace_extension("vert");
	const auto fragmentShader = (base / resource.path()).replace_extension("frag");
	
	return new Shader(vertexShader, fragmentShader);
}
