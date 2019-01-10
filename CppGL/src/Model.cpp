#include "Model.h"
#include "utils/FmtExtensions.h"

#include <glm/glm.hpp>
#include <utility>

using namespace std;
using namespace glm;

Model::Model(const std::filesystem::path path, const bool gamma) 
	: Logger("model")
    , _path(path)
    , _gamma(gamma)
    , gammaCorrection(gamma)
    , _minBounds(FLT_MAX)
    , _maxBounds(FLT_MIN)
{
}

void Model::setup()
{
	loadModel(_path);
}

void Model::draw(const Shader* shader)
{
	for (auto& mesh : meshes)
		mesh.draw(shader);
}

void Model::loadModel(const std::filesystem::path& path)
{
	console->info("loading model {}", path);
	Assimp::Importer importer;
	const auto scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		console->error("failed to load model: {}", importer.GetErrorString());
		return;
	}

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode * node, const aiScene * scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		const auto mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}

	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		auto mesh = scene->mMeshes[node->mMeshes[i]];
		for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
			aiVector3D _pos = mesh->mVertices[j];
			vec3 pos(_pos.x, _pos.y, _pos.z);

			// BUG: bounding box calculation is wrong.
			// TODO: Check how expensive convex hull collision boxes are?
			_minBounds = glm::min(_minBounds, pos);
			_maxBounds = glm::max(_maxBounds, pos);
		}
	}

}

Mesh Model::processMesh(aiMesh * mesh, const aiScene * scene)
{
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<MeshTexture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex{};
		vec3 vect;
		// positions
		vect.x = mesh->mVertices[i].x;
		vect.y = mesh->mVertices[i].y;
		vect.z = mesh->mVertices[i].z;
		vertex.position = vect;
		// normals
		vect.x = mesh->mNormals[i].x;
		vect.y = mesh->mNormals[i].y;
		vect.z = mesh->mNormals[i].z;
		vertex.normal = vect;
		// texture coords
		if (mesh->mTextureCoords[0]) {
			vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoords = vec;
		}
		else {
			vertex.texCoords = vec2(0.0f, 0.0f);
		}
		// tangent 
		vect.x = mesh->mTangents[i].x;
		vect.y = mesh->mTangents[i].y;
		vect.z = mesh->mTangents[i].z;
		vertex.tangent = vect;
		// bitangent
		vect.x = mesh->mBitangents[i].x;
		vect.y = mesh->mBitangents[i].y;
		vect.z = mesh->mBitangents[i].z;
		vertex.bitangent = vect;
		vertices.push_back(vertex);
	}

	// Faces
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// Materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	
	// diffuse maps
	vector<MeshTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

	// specular maps
	vector<MeshTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	// normal maps
	vector<MeshTexture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	// height maps
	vector<MeshTexture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	return Mesh(vertices, indices, textures);
}

vector<MeshTexture> Model::loadMaterialTextures(aiMaterial * mat, const aiTextureType type, const string& typeName)
{
	vector<MeshTexture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		auto skip = false;
		
		for (auto &j : texturesLoaded) {
			if (strcmp(j.path.data(), str.C_Str()) == 0) {
				textures.push_back(j);
				skip = true;
				break;
			}
		}
		if (!skip) {
			auto filepath = _path.parent_path() / string(str.C_Str());

			MeshTexture texture;
			texture.id = textureFromFile(filepath);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			texturesLoaded.push_back(texture);
		}

	}
	return textures;
}

optional<Model*> Model::fromResource(const Resource& resource, const Console& console)
{
	const auto fn = filesystem::path("resources") / "models" / resource.path();
	
	return new Model(fn);
}
