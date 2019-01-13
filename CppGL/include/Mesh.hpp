#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "Shader.hpp"
#include "Logger.hpp"
#include "Texture.hpp"

struct Vertex final
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct MeshTexture final
{
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh final : public Logger {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<MeshTexture> textures;
	unsigned int vao{};

	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<MeshTexture>& textures);
	void draw(const Shader* shader);
private:
	unsigned int _vbo{}, _ebo{};
	
	void setupMesh();
};