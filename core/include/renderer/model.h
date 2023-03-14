#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <span>

namespace core
{

namespace refactor
{
struct Mesh
{
    std::string name;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::vector<unsigned> indices;
};
Mesh GenerateQuad(glm::vec3 scale, glm::vec3 offset);
Mesh GenerateCube(glm::vec3 scale, glm::vec3 offset);
Mesh GenerateSphere(float scale, glm::vec3 offset);
}

struct ModelIndex
{
    std::size_t index = std::numeric_limits<std::size_t>::max();
    constexpr bool operator==(ModelIndex other) const
    {
        return index == other.index;
    }
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
    const refactor::Mesh& GetMesh(std::string_view meshName);

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
    void Clear();

protected:
    ModelIndex ImportScene(const aiScene* scene);
    std::vector<Model> models_;
    Assimp::Importer importer_;
};
} // namespace core
