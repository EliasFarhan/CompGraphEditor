#pragma once

#include "renderer/mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <vector>
#include <memory>

namespace core
{
/**
 * @brief Model is a class containing Mesh
 */
class Model final
{
public:

private:
    std::vector<std::unique_ptr<Mesh>> meshes_;
};
/**
 * ModelManager is a manager that manages models both for OpenGL and Vulkan
 */
class ModelManager final
{
public:
    void ImportModel(std::string_view modelPath);
protected:
    void ImportScene(aiScene* scene);

    Assimp::Importer importer_;
};
} // namespace core
