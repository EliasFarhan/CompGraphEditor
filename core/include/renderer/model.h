#pragma once

#include "renderer/mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <vector>

namespace core
{

namespace refactor
{
struct Mesh
{
    std::string name;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> biTangents;
    std::vector<unsigned> indices;
};
}

struct ModelIndex
{
    std::size_t index = std::numeric_limits<std::size_t>::max();
};

static constexpr ModelIndex INVALID_MODEL_INDEX = {};
/**
 * @brief Model is a class containing Mesh
 */
class Model
{
public:
    [[nodiscard]] std::span<refactor::Mesh> GetMeshes() { return meshes_; }
    [[nodiscard]] std::span<const refactor::Mesh> GetMeshes() const { return std::span{meshes_.cbegin(), meshes_.cend()}; }
protected:
    void LoadFromNode(const aiScene* scene, const aiNode* node);
    void LoadMesh(const aiMesh* aiMesh);
    friend class ModelManager;
    std::vector<refactor::Mesh> meshes_;
};
/**
 * ModelManager is a manager that manages models both for OpenGL and Vulkan
 */
class ModelManager final
{
public:
    ModelIndex ImportModel(std::string_view modelPath);
    [[nodiscard]] Model& GetModel(ModelIndex index) { return models_[index.index]; }
    [[nodiscard]] const Model& GetModel(ModelIndex index) const { return models_[index.index]; }
protected:
    ModelIndex ImportScene(const aiScene* scene);
    std::vector<Model> models_;
    Assimp::Importer importer_;
};
} // namespace core
