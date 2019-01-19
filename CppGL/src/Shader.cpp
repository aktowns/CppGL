#include "Shader.hpp"
#include "Config.hpp"
#include "utils/Resource.hpp"
#include "utils/FmtExtensions.hpp"

#include <fstream>
#include <utility>

using namespace std;

GLuint Shader::compileShader(ShaderType type, std::filesystem::path shader)
{
    console->info("loading shader {}", shader);
    const auto handle = glCreateShader(type);

    if (!filesystem::exists(shader))
    {
        console->error("file {} does not exist.", shader);
    }
    assert(filesystem::exists(shader));

    ifstream file(shader);
    string source((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));

    auto cSource = source.c_str();
    glShaderSource(handle, 1, &cSource, nullptr);
    glCompileShader(handle);

    GLint compiled;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetShaderInfoLog(handle, 1024, &logLength, message);

        console->error("shader compilation for {} failed: {}", shader, string(message));
    }
    assert(compiled == GL_TRUE);

    return handle;
}

void Shader::linkProgram(std::vector<GLuint> shaders)
{
    _program = glCreateProgram();
    for (auto shader : shaders)
    {
        glAttachShader(_program, shader);
    }
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
}

Shader::Shader(filesystem::path vertexShader, filesystem::path fragmentShader)
    : Logger("shader")
    , _fragmentShader(std::move(fragmentShader))
    , _vertexShader(std::move(vertexShader))
    , _geometryShader(nullopt)
    , _program(0)
{
    const auto vertex = compileShader(Vertex, _vertexShader);
    const auto fragment = compileShader(Fragment, _fragmentShader);

    linkProgram({ vertex, fragment });

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::Shader(std::filesystem::path vertexShader, std::filesystem::path fragmentShader,
    std::filesystem::path geometryShader)
    : Logger("shader")
    , _fragmentShader(std::move(fragmentShader))
    , _vertexShader(std::move(vertexShader))
    , _geometryShader(std::move(geometryShader))
    , _program(0)
{
    const auto vertex = compileShader(Vertex, _vertexShader);
    const auto fragment = compileShader(Fragment, _fragmentShader);
    const auto geometry = compileShader(Geometry, _geometryShader.value());

    linkProgram({ vertex, fragment, geometry });

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    glDeleteShader(geometry);
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

void Shader::setVec4(const std::string & name, const glm::vec4 & value) const
{
    glUniform4fv(glGetUniformLocation(_program, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string & name, const float x, const float y, const float z, const float w) const
{
    glUniform4f(glGetUniformLocation(_program, name.c_str()), x, y, z, w);
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
    const auto vertexShader = (SHADERS_DIR / resource.path()).replace_extension("vert");
    const auto fragmentShader = (SHADERS_DIR / resource.path()).replace_extension("frag");
    const auto geometryShader = (SHADERS_DIR / resource.path()).replace_extension("geom");

    if (filesystem::exists(geometryShader))
    {
        return new Shader(vertexShader, fragmentShader, geometryShader);
    }

    return new Shader(vertexShader, fragmentShader);
}

