#pragma once

#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

#include "Mesh.h"
#include "Shader.h"
#include "Logger.h"
#include "Model.h"

class Model final : public Logger {
    const std::filesystem::path _path;
    bool _gamma;
public:
	std::vector<MeshTexture> texturesLoaded;
	std::vector<Mesh> meshes;
	bool gammaCorrection;

    explicit Model(std::filesystem::path path, bool gamma = false);
	void setup();
	void draw(const Shader* shader);
	static Model* Model::fromResource(Resource& resource, const Console& console);
private:
	void loadModel(std::filesystem::path const &path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
	glm::vec3 _minBounds;
	glm::vec3 _maxBounds;
};