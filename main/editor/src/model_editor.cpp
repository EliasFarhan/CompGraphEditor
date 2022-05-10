#include "model_editor.h"
#include <array>
#include "utils/log.h"
#include <fmt/format.h>
#include <tiny_obj_loader.h>

#include "editor.h"
#include "mesh_editor.h"
#include "resource_manager.h"
#include "texture_editor.h"
#include "engine/filesystem.h"

#include <fstream>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace py = pybind11;

namespace gpr5300
{
void ModelEditor::AddResource(const Resource& resource)
{
    const auto extension = GetFileExtension(resource.path);
    if(extension == ".model")
    {
        ModelInfo modelInfo{};
        modelInfo.resourceId = resource.resourceId;
        modelInfo.filename = GetFilename(resource.path);

        const auto& fileSystem = FilesystemLocator::get();
        if (!fileSystem.IsRegularFile(resource.path))
        {
            LogWarning(fmt::format("Could not find model file: {}", resource.path));
            return;
        }
        std::ifstream fileIn(resource.path, std::ios::binary);
        if (!modelInfo.info.ParseFromIstream(&fileIn))
        {
            LogWarning(fmt::format("Could not open protobuf file: {}", resource.path));
            return;
        }
        tinyobj::ObjReaderConfig readerConfig{};
        readerConfig.triangulate = true;
        readerConfig.vertex_color = false;
        const auto& modelPath = modelInfo.info.model_path();

        if (!modelInfo.reader.ParseFromFile(modelPath, readerConfig))
        {
            if (!modelInfo.reader.Error().empty())
            {
                LogError(fmt::format("Error parsing obj file: {}", modelPath));
                return;
            }
        }
        modelInfo.path = resource.path;
        modelInfos_.push_back(modelInfo);
    }
    else if(extension == ".obj")
    {
        const auto baseFolder = GetFolder(resource.path);
        const auto modelInfoPath = fmt::format("{}/{}.model", baseFolder, GetFilename(resource.path, false));
        const auto& fileSystem = FilesystemLocator::get();
        if (!fileSystem.IsRegularFile(modelInfoPath))
        {
            LogWarning(fmt::format("Could not find model file: {} of obj {}", modelInfoPath, resource.path));
            return;
        }
    }
}

void ModelEditor::RemoveResource(const Resource& resource)
{
}

void ModelEditor::UpdateExistingResource(const Resource& resource)
{
}


void ModelEditor::DrawMainView()
{
}

void ModelEditor::DrawInspector()
{
    if(currentIndex_ >= modelInfos_.size())
    {
        return;
    }

    const auto& currentModelInfo = modelInfos_[currentIndex_];
    ImGui::Text("Path: %s", currentModelInfo.info.model_path());

    if (ImGui::BeginListBox("Meshes"))
    {
        for (int i = 0; i < currentModelInfo.info.meshes_size(); i++)
        {
            const auto& meshInfo = currentModelInfo.info.meshes(i);
            const auto& text = meshInfo.mesh_name();
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }

    if (ImGui::BeginListBox("Materials"))
    {
        for (int i = 0; i < currentModelInfo.info.materials_size(); i++)
        {
            const auto& materialInfo = currentModelInfo.info.materials(i);
            const auto& text = materialInfo.material_name();
            ImGui::Selectable(text.c_str(), false);
        }
        ImGui::EndListBox();
    }
}

bool ModelEditor::DrawContentList(bool unfocus)
{
    bool wasFocused = false;
    if (unfocus)
        currentIndex_ = modelInfos_.size();
    for (std::size_t i = 0; i < modelInfos_.size(); i++)
    {
        const auto& meshInfo = modelInfos_[i];
        if (ImGui::Selectable(meshInfo.filename.data(), currentIndex_ == i))
        {
            currentIndex_ = i;
            wasFocused = true;
        }
    }
    return wasFocused;
}

std::string_view ModelEditor::GetSubFolder()
{
    return "models/";
}

EditorType ModelEditor::GetEditorType()
{
    return EditorType::MODEL;
}

void ModelEditor::Save()
{
}

void ModelEditor::ReloadId()
{
}

void ModelEditor::Delete()
{
}

std::span<const std::string_view> ModelEditor::GetExtensions() const
{
    static constexpr std::array<std::string_view, 2> extensions = { ".obj", ".model"};
    return std::span{ extensions };
}

ModelInfo* ModelEditor::GetModel(ResourceId resourceId)
{
    auto it = std::ranges::find_if(modelInfos_, [&resourceId](const auto& modelInfo)
        {
            return modelInfo.resourceId == resourceId;
        });
    if(it != modelInfos_.end())
    {
        return &*it;
    }
    return nullptr;
}

void ModelEditor::ImportResource(std::string_view path)
{
    auto& filesystem = FilesystemLocator::get();
    if(GetFileExtension(path) != ".obj")
    {
        LogError("Can only import obj file");
        return;
    }

    tinyobj::ObjReaderConfig readerConfig{};
    readerConfig.triangulate = true;
    readerConfig.vertex_color = false;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path.data(), readerConfig))
    {
        if (!reader.Error().empty())
        {
            LogError(fmt::format("Error parsing obj file: {}", path));
            return;
        }
    }
    std::vector<std::string> mtlFiles;
    try
    {
        py::function getMaterialsFromObjFunc = py::module_::import("scripts.obj_parser").attr("get_materials_path");
        std::string result = static_cast<py::str>(getMaterialsFromObjFunc(path));
        auto materialsJson = json::parse(result);
        auto mtlList = materialsJson["materials"];
        for(auto& mtlPath: mtlList)
        {
            mtlFiles.push_back(mtlPath);
        }
    }
    catch(py::error_already_set& e)
    {
        LogError("Could not parse obj to get mtl files");
    }
    catch(json::parse_error& e)
    {
        LogError("Could not parse oobj info from script");
    }
    const auto srcFolder = GetFolder(path);
    const auto dstFolder = fmt::format("{}{}{}/", ResourceManager::dataFolder, GetSubFolder(), GetFilename(path, false));
    auto* editor = Editor::GetInstance();
    auto& resourceManager = editor->GetResourceManager();
    CreateNewDirectory(dstFolder);

    pb::Model newModel;
    newModel.set_model_path(path.data());
    auto findTextureInModelFunc = [&newModel](const std::string_view texture)
    {
        for(int i = 0; i < newModel.textures_size(); i++)
        {
            if(fs::equivalent(newModel.textures(i).texture_path(),texture))
            {
                return i;
            }
        }
        return -1;
    };
    auto& materials = reader.GetMaterials();
    LogDebug(fmt::format("Newly imported model contains {} materials", materials.size()));

    auto loadMaterialTextureFunc = [&]
    (std::string_view textureName, pb::TextureType textureType, pb::ModelMaterial* material)
    {
        if(textureName.empty())
        {
            return;
        }
        const auto textureSrcPath = fmt::format("{}/{}", srcFolder, textureName);
        const auto textureDstPath = fmt::format("{}{}", dstFolder, textureName);

        int index = -1;
        if(filesystem.FileExists(textureDstPath))
        {
            index = findTextureInModelFunc(textureDstPath);
        }
        else
        {
            if (!CopyFileFromTo(textureSrcPath, textureDstPath))
            {
                LogError(fmt::format("Could not copy texture from {} to {}", textureSrcPath, textureDstPath));
                return;
            }
            resourceManager.AddResource(textureDstPath);
            index = newModel.textures_size();
            auto* newTexture = newModel.add_textures();
            newTexture->set_type(textureType);
            newTexture->set_texture_path(textureDstPath);
            //change the actual texture info to the type to be linked automatically when binding pipeline
            const auto textureId = resourceManager.FindResourceByPath(textureDstPath);
            auto* textureEditor = dynamic_cast<TextureEditor*>(editor->GetEditorSystem(EditorType::TEXTURE));
            auto* texture = textureEditor->GetTexture(textureId);
            texture->info.set_type(textureType);
        }
        material->add_texture_indices(index);
    };

    for (auto& material : materials)
    {
        LogDebug(fmt::format("Material: {}, diffuse: {} specular: {}", material.name, material.diffuse_texname, material.specular_texname));

        auto* newMaterial = newModel.add_materials();
        newMaterial->set_material_name(material.name);
        
        loadMaterialTextureFunc(material.ambient_texname, pb::TextureType::AMBIENT, newMaterial);
        loadMaterialTextureFunc(material.diffuse_texname, pb::TextureType::DIFFUSE, newMaterial);
        loadMaterialTextureFunc(material.specular_texname, pb::TextureType::SPECULAR, newMaterial);
        loadMaterialTextureFunc(material.specular_highlight_texname, pb::TextureType::SPECULAR_HIGHLIGHT, newMaterial);
        loadMaterialTextureFunc(material.bump_texname, pb::TextureType::BUMP, newMaterial);
        loadMaterialTextureFunc(material.displacement_texname, pb::TextureType::DISPLACEMENT, newMaterial);
        loadMaterialTextureFunc(material.alpha_texname, pb::TextureType::ALPHA, newMaterial);
        loadMaterialTextureFunc(material.reflection_texname, pb::TextureType::REFLECTION, newMaterial);
        loadMaterialTextureFunc(material.roughness_texname, pb::TextureType::ROUGHNESS, newMaterial);
        loadMaterialTextureFunc(material.metallic_texname, pb::TextureType::METALLIC, newMaterial);
        loadMaterialTextureFunc(material.sheen_texname, pb::TextureType::SHEEN, newMaterial);
        loadMaterialTextureFunc(material.emissive_texname, pb::TextureType::EMISSIVE, newMaterial);
        loadMaterialTextureFunc(material.normal_texname, pb::TextureType::NORMAL, newMaterial);
    }

    auto& shapes = reader.GetShapes();
    LogDebug(fmt::format("Newly imported model containes {} meshes", shapes.size()));
    for (auto& shape : shapes)
    {
        LogDebug(fmt::format("Mesh: {} contains {} vertices", shape.name, shape.mesh.indices.size()));

        auto meshInfoDstPath = fmt::format("{}{}.mesh", dstFolder, shape.name);
        auto meshInfoId = resourceManager.FindResourceByPath(meshInfoDstPath);
        if (meshInfoId == INVALID_RESOURCE_ID)
        {
            editor->CreateNewFile(meshInfoDstPath, EditorType::MESH);
        }
        meshInfoId = resourceManager.FindResourceByPath(meshInfoDstPath);
        auto* meshEditor = dynamic_cast<MeshEditor*>(editor->GetEditorSystem(EditorType::MESH));
        auto* meshInfo = meshEditor->GetMesh(meshInfoId);
        meshInfo->info.set_primitve_type(pb::Mesh_PrimitveType_MODEL);

        auto* newMesh = newModel.add_meshes();
        newMesh->set_mesh_name(shape.name);
        newMesh->set_material_name(newModel.materials(shape.mesh.material_ids[0]).material_name());
        newMesh->set_mesh_path(meshInfoDstPath);
    }

    auto modelInfoPath = fmt::format("{}{}.model", dstFolder, GetFilename(path, false));
    filesystem.WriteString(modelInfoPath, newModel.SerializeAsString());
    resourceManager.AddResource(modelInfoPath);

    for(std::string_view mtlPath: mtlFiles)
    {
        auto mtlSrcPath = fmt::format("{}/{}", srcFolder, GetFilename(mtlPath));
        auto mtlDstPath = fmt::format("{}{}", dstFolder, GetFilename(mtlPath));
        CopyFileFromTo(mtlSrcPath, mtlDstPath);
    }

    auto modelDstPath = fmt::format("{}{}", dstFolder, GetFilename(path));
    CopyFileFromTo(path, modelDstPath);
    resourceManager.AddResource(modelDstPath);

}
}
