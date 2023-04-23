#pragma once

#include "proto/renderer.pb.h"
#include "renderer/mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>


#include <span>
#include <array>
#include <vector>

namespace core
{


struct ModelMaterial
{
    std::string name;
    std::array<std::string, core::pb::TextureType::LENGTH> textures;
};



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
    [[nodiscard]] std::span<Mesh> GetMeshes() { return meshes_; }
    [[nodiscard]] std::span<const Mesh> GetMeshes() const { return std::span{meshes_.cbegin(), meshes_.cend()}; }
    [[nodiscard]] std::span<ModelMaterial> GetMaterials() { return materials_; }
    [[nodiscard]] std::span<const ModelMaterial> GetMaterials() const { return materials_; }
    const Mesh& GetMesh(std::string_view meshName);

protected:
    void LoadFromNode(const aiScene* scene, const aiNode* node);
    void LoadMaterials(const aiScene* scene);
    void LoadMesh(const aiMesh* aiMesh);
    friend class ModelManager;
    std::vector<Mesh> meshes_;
    std::vector<ModelMaterial> materials_;
};
/**
 * ModelManager is a manager that manages models both for OpenGL and Vulkan
 */
class ModelManager final
{
public:
    ModelManager();
    ModelIndex ImportModel(std::string_view modelPath);
    [[nodiscard]] Model& GetModel(ModelIndex index) { return models_[index.index]; }
    [[nodiscard]] const Model& GetModel(ModelIndex index) const { return models_[index.index]; }
    void Clear();

protected:
    ModelIndex ImportScene(const aiScene* scene);
    std::unordered_map<std::string, ModelIndex> modelNamesMap_;
    std::vector<Model> models_;
    Assimp::Importer importer_;
};
} // namespace core
