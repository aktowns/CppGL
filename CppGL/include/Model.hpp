#pragma once

#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

#include "Mesh.hpp"
#include "Shader.hpp"
#include "Logger.hpp"
#include "Model.hpp"

class Model final : public Logger {
    const std::filesystem::path _path;
    bool _gamma;
public:
    std::vector<MeshTexture> texturesLoaded;
    std::vector<Mesh> meshes;
    bool gammaCorrection;

    explicit Model(std::filesystem::path path, bool gamma = false);
    void draw(const Shader* shader);
    static std::optional<Model*> fromResource(const Resource& resource, const Console& console);
private:
    void loadModel(std::filesystem::path const &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
    glm::vec3 _minBounds;
    glm::vec3 _maxBounds;
};
