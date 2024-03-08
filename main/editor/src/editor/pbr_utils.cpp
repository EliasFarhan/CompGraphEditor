#include "pbr_utils.h"

#include "proto/renderer.pb.h"
#include "editor_filesystem.h"
#include "engine/filesystem.h"
#include "utils/log.h"
#include "renderer/pipeline.h"


#include <renderer/framebuffer.h>

#include "gl/buffer.h"
#include "gl/framebuffer.h"
#include "gl/pipeline.h"
#include "renderer/mesh.h"
#include "scene_editor.h"
#include "editor.h"

#include <fmt/format.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <stb_image.h>
#include <stb_image_write.h>

#include "GL/glew.h"
#include "gl/debug.h"


namespace editor
{

void GeneratePreComputeBrdfLUT()
{
    auto* sceneEditor = GetSceneEditor();
    const auto* currentScene = sceneEditor->GetCurrentSceneInfo();
    if(currentScene == nullptr)
    {
        LogWarning("Can not create BRDF LUT texture without a scene.");
        return;
    }
    constexpr int texW = 512;
    // dimensions of the image
    constexpr int texH = 512;
    GLuint texOutput;
    glGenTextures(1, &texOutput);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texOutput);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texW, texH, 0, GL_RGBA, GL_FLOAT,
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindImageTexture(1, texOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glCheckError();

    core::pb::Shader shaderInfo;
    shaderInfo.set_path("shaders/pre_compute_brdf.comp");
    shaderInfo.set_type(core::pb::COMPUTE);

    gl::Shader shader;
    shader.LoadShader(shaderInfo);
    gl::Pipeline pipeline;
    pipeline.LoadComputePipeline(shader);

    shader.Destroy();
    // launch compute shaders!
    pipeline.Bind();
    glCheckError();
    glDispatchCompute(static_cast<GLuint>(texW), static_cast<GLuint>(texH), 1);

    // make sure writing to image has finished before read
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glCheckError();
    auto* buffer = static_cast<float*>(std::calloc(texH * texW, 4 * sizeof(float)));
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);
    glCheckError();
    stbi_flip_vertically_on_write(true);
    const auto path = fmt::format("data/{}/textures/brdf_lut.hdr", currentScene->info.name());
    if (!stbi_write_hdr(path.data(), texW, texH, 4, buffer))
    {
        //Error
        LogError("Error while exporting BRDF LUT to hdr texture");
    }
    else
    {
        auto& resourceManager = GetResourceManager();
        resourceManager.AddResource(path);
    }
    std::free(buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texOutput);
    pipeline.Unbind();
    pipeline.Destroy();
    glCheckError();
}


void GenerateIrradianceMap(std::string_view path)
{
    const auto baseDir = GetFolder(path);
    const auto filename = GetFilename(path, false);
    const auto irradianceMapPath = fmt::format("{}/{}_irrmap.hdr", baseDir, filename);
    const auto irradianceKtxMapPath = fmt::format("{}/{}_irrmap.ktx", baseDir, filename);

    auto& filesystem = core::FilesystemLocator::get();
    auto envMapFile = filesystem.LoadFile(path);
    int texW;
    int texH;
    int channel;

    stbi_set_flip_vertically_on_load(true);
    auto* envMapData = stbi_loadf_from_memory(envMapFile.data, envMapFile.size, &texW, &texH, &channel, 4);

    unsigned int envMap;
    if (envMapData)
    {
        glGenTextures(1, &envMap);
        glBindTexture(GL_TEXTURE_2D, envMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texW, texH, 0, GL_RGBA, GL_FLOAT, envMapData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glBindImageTexture(0, envMap, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(envMapData);

    }
    else
    {
        //Error loading hdr
        return;
    }
    glCheckError();
    const auto cubeMesh = core::GenerateCube(glm::vec3(2.0f), glm::vec3(0.0f));
    gl::VertexInputBuffer cube{};
    cube.CreateFromMesh(cubeMesh);

    core::pb::FrameBuffer captureFboInfo;
    captureFboInfo.set_name("captureFBO");
    auto* captureCubemap = captureFboInfo.add_color_attachments();
    captureCubemap->set_cubemap(true);
    captureCubemap->set_type(core::pb::RenderTarget_Type_FLOAT);
    captureCubemap->set_format(core::pb::RenderTarget_Format_RGBA);
    captureCubemap->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_32);
    captureCubemap->set_size_type(core::pb::RenderTarget_Size_FIXED_SIZE);
    captureCubemap->mutable_target_size()->set_x(512);
    captureCubemap->mutable_target_size()->set_y(512);
    static constexpr std::string_view envCubemapName = "envCubeName";
    captureCubemap->set_name(envCubemapName.data());

    gl::Framebuffer captureFbo;
    captureFbo.Load(captureFboInfo);

    //Generate environment cubemap
    //from equirectangle to cubemap
    core::pb::Shader cubemapShaderInfo;
    cubemapShaderInfo.set_path("shaders/cubemap.vert");
    cubemapShaderInfo.set_type(core::pb::VERTEX);

    gl::Shader cubemapShader;
    cubemapShader.LoadShader(cubemapShaderInfo);

    core::pb::Shader equirectangleToCubemapShaderInfo;
    equirectangleToCubemapShaderInfo.set_path("shaders/equirectangle_to_cubemap.frag");
    equirectangleToCubemapShaderInfo.set_type(core::pb::FRAGMENT);

    gl::Shader equirectangleToCubemapShader;
    equirectangleToCubemapShader.LoadShader(equirectangleToCubemapShaderInfo);

    gl::Pipeline equirectangleToCubemap;
    equirectangleToCubemap.LoadRasterizePipeline(cubemapShader, equirectangleToCubemapShader);
    equirectangleToCubemapShader.Destroy();

    equirectangleToCubemap.Bind();
    equirectangleToCubemap.SetTexture("equirectangularMap", envMap, 0);
    cube.Bind();
    glCheckError();
    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    equirectangleToCubemap.SetMat4("projection", captureProjection);

    glViewport(0, 0, 512, 512);
    captureFbo.Bind();
    glCheckError();
    auto envCubemap = captureFbo.GetTextureName(envCubemapName);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glCheckError();
        equirectangleToCubemap.SetMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glCheckError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCheckError();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        glCheckError();
    }
    captureFbo.Unbind();

    //generate irradiance cubemap
    static constexpr int irradianceMapSize = 32;
    core::pb::FrameBuffer irradianceFboInfo;
    irradianceFboInfo.set_name("irradianceFbo");
    auto* irradianceAttachmentInfo = irradianceFboInfo.add_color_attachments();
    irradianceAttachmentInfo->set_type(core::pb::RenderTarget_Type_FLOAT);
    irradianceAttachmentInfo->set_format(core::pb::RenderTarget_Format_RGBA);
    irradianceAttachmentInfo->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_32);
    irradianceAttachmentInfo->set_size_type(core::pb::RenderTarget_Size_FIXED_SIZE);
    irradianceAttachmentInfo->set_cubemap(true);
    static constexpr std::string_view irradianceMapName = "irradiance";
    irradianceAttachmentInfo->set_name(irradianceMapName.data());
    irradianceAttachmentInfo->mutable_target_size()->set_x(irradianceMapSize);
    irradianceAttachmentInfo->mutable_target_size()->set_y(irradianceMapSize);

    gl::Framebuffer irradianceFbo;
    irradianceFbo.Load(irradianceFboInfo);

    core::pb::Shader irradianceConvolutionShaderInfo;
    irradianceConvolutionShaderInfo.set_type(core::pb::FRAGMENT);
    irradianceConvolutionShaderInfo.set_path("shaders/irradiance_convolution.frag");

    gl::Shader irradianceConvlutionShader;
    irradianceConvlutionShader.LoadShader(irradianceConvolutionShaderInfo);

    gl::Pipeline irradianceConvolution;
    irradianceConvolution.LoadRasterizePipeline(cubemapShader, irradianceConvlutionShader);

    irradianceFbo.Bind();
    irradianceConvolution.Bind();
    irradianceConvolution.SetMat4("projection", captureProjection);
    cube.Bind();
    irradianceConvolution.SetInt("environmentMap", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, captureFbo.GetTextureName(envCubemapName));
    glViewport(0, 0, irradianceMapSize, irradianceMapSize);
    glCheckError();
    const auto irradianceMap = irradianceFbo.GetTextureName(irradianceMapName);
    for (int face = 0; face < 6; face++)
    {
        irradianceConvolution.SetMat4("view", captureViews[face]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, irradianceMap, 0);
        glCheckError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCheckError();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        glCheckError();
    }
    irradianceFbo.Unbind();
    irradianceConvolution.Unbind();

    //Export capture as cubemap ktx
    
    ktxTexture1* texture;                 // For KTX
    ktxTextureCreateInfo createInfo;
    KTX_error_code result;
    
    createInfo.glInternalformat = GL_RGBA32F;   // Ignored if creating a ktxTexture2.
    createInfo.baseWidth = 32;
    createInfo.baseHeight = 32;
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 6;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;

    result = ktxTexture1_Create(&createInfo,
        KTX_TEXTURE_CREATE_ALLOC_STORAGE,
        &texture);
    ktxCheckError(result);

    std::size_t faceSize = irradianceMapSize * irradianceMapSize * 4 * 4;
    void* faceBuffer = std::malloc(faceSize);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glCheckError();
    for(int face = 0; face < 6; face++)
    {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, GL_FLOAT, faceBuffer);
        glCheckError();
        result = ktxTexture_SetImageFromMemory(ktxTexture(texture),
            0, 0, face,
            static_cast<const ktx_uint8_t*>(faceBuffer), faceSize);
        ktxCheckError(result);
    }
    std::free(faceBuffer);
    ktxTexture_WriteToNamedFile(ktxTexture(texture), irradianceKtxMapPath.data());
    ktxTexture_Destroy(ktxTexture(texture));
    //from cubemap to equirectangle

    core::pb::Shader equirectangleVertShaderInfo;
    equirectangleVertShaderInfo.set_path("shaders/equirectangle.vert");
    equirectangleVertShaderInfo.set_type(core::pb::VERTEX);

    core::pb::Shader equirectangleFragShaderInfo;
    equirectangleFragShaderInfo.set_path("shaders/equirectangle.frag");
    equirectangleFragShaderInfo.set_type(core::pb::FRAGMENT);

    gl::Shader equirectangleVertShader;
    equirectangleVertShader.LoadShader(equirectangleVertShaderInfo);

    gl::Shader equirectangleFragShader;
    equirectangleFragShader.LoadShader(equirectangleFragShaderInfo);

    gl::Pipeline equirectangle;
    equirectangle.LoadRasterizePipeline(equirectangleVertShader, equirectangleFragShader);

    auto resultW = irradianceMapSize * 4;
    auto resultH = irradianceMapSize * 2;

    core::pb::FrameBuffer equirectangleFboInfo;
    auto* equirectangleColorAttachment = equirectangleFboInfo.add_color_attachments();
    equirectangleColorAttachment->set_name("irradiance");
    equirectangleColorAttachment->set_type(core::pb::RenderTarget_Type_FLOAT);
    equirectangleColorAttachment->set_format(core::pb::RenderTarget_Format_RGBA);
    equirectangleColorAttachment->set_size_type(core::pb::RenderTarget_Size_FIXED_SIZE);
    equirectangleColorAttachment->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_32);
    equirectangleColorAttachment->mutable_target_size()->set_x(resultW);
    equirectangleColorAttachment->mutable_target_size()->set_y(resultH);

    auto quadMesh = core::GenerateQuad(glm::vec3(2.0f), glm::vec3(0.0f));
    gl::VertexInputBuffer quad{};
    quad.CreateFromMesh(quadMesh);
    gl::Framebuffer equirectangleFbo;
    equirectangleFbo.Load(equirectangleFboInfo);

    equirectangleFbo.Bind();
    equirectangle.Bind();
    quad.Bind();
    glViewport(0, 0, resultW, resultH);

    equirectangle.SetInt("irradiance", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    equirectangleFbo.Unbind();
    equirectangle.Unbind();

    //Export as KTX?
    //export as hdr
    glBindTexture(GL_TEXTURE_2D, equirectangleFbo.GetTextureName("irradiance"));
    auto* buffer = static_cast<float*>(std::calloc(resultW * resultH, 4 * sizeof(float)));
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);
    glCheckError();
    stbi_flip_vertically_on_write(true);
    //TODO give proper name
    if (!stbi_write_hdr(irradianceMapPath.data(), resultW, resultH, 4, buffer))
    {
        //Error
        LogError("Error while exporting Irradiance map to hdr texture");
    }
    std::free(buffer);

    //TODO clean textures
    equirectangle.Destroy();
    equirectangleFbo.Destroy();
    irradianceConvolution.Destroy();
    irradianceConvlutionShader.Destroy();
    cubemapShader.Destroy();
    equirectangleToCubemap.Destroy();
    captureFbo.Destroy();
    irradianceFbo.Destroy();
    glDeleteTextures(1, &envMap);
    cube.Destroy();
    quad.Destroy();
    glCheckError();
}

void GeneratePreFilterEnvMap(std::string_view path)
{
    const auto baseDir = GetFolder(path);
    const auto filename = GetFilename(path, false);
    const auto preFilterEnvMapPath = fmt::format("{}/{}_prefilter.ktx", baseDir, filename);

    auto& filesystem = core::FilesystemLocator::get();
    auto envMapFile = filesystem.LoadFile(path);
    int texW;
    int texH;
    int channel;

    stbi_set_flip_vertically_on_load(true);
    auto* envMapData = stbi_loadf_from_memory(envMapFile.data, envMapFile.size, &texW, &texH, &channel, 4);

    unsigned int envMap;
    if (envMapData)
    {
        glGenTextures(1, &envMap);
        glBindTexture(GL_TEXTURE_2D, envMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texW, texH, 0, GL_RGBA, GL_FLOAT, envMapData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glBindImageTexture(0, envMap, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(envMapData);

    }
    else
    {
        //Error loading hdr
        return;
    }
    glCheckError();
    auto cubeMesh = core::GenerateCube(glm::vec3(2.0f), glm::vec3(0.0f));
    gl::VertexInputBuffer cube{};
    cube.CreateFromMesh(cubeMesh);
    core::pb::FrameBuffer captureFboInfo;
    captureFboInfo.set_name("captureFBO");
    auto* captureCubemap = captureFboInfo.add_color_attachments();
    captureCubemap->set_cubemap(true);
    captureCubemap->set_type(core::pb::RenderTarget_Type_FLOAT);
    captureCubemap->set_format(core::pb::RenderTarget_Format_RGBA);
    captureCubemap->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_32);
    captureCubemap->set_size_type(core::pb::RenderTarget_Size_FIXED_SIZE);
    captureCubemap->mutable_target_size()->set_x(512);
    captureCubemap->mutable_target_size()->set_y(512);
    static constexpr std::string_view envCubemapName = "envCubeName";
    captureCubemap->set_name(envCubemapName.data());

    gl::Framebuffer captureFbo;
    captureFbo.Load(captureFboInfo);

    //Generate environment cubemap
    //from equirectangle to cubemap
    core::pb::Shader cubemapShaderInfo;
    cubemapShaderInfo.set_path("shaders/cubemap.vert");
    cubemapShaderInfo.set_type(core::pb::VERTEX);

    gl::Shader cubemapShader;
    cubemapShader.LoadShader(cubemapShaderInfo);

    core::pb::Shader equirectangleToCubemapShaderInfo;
    equirectangleToCubemapShaderInfo.set_path("shaders/equirectangle_to_cubemap.frag");
    equirectangleToCubemapShaderInfo.set_type(core::pb::FRAGMENT);

    gl::Shader equirectangleToCubemapShader;
    equirectangleToCubemapShader.LoadShader(equirectangleToCubemapShaderInfo);

    gl::Pipeline equirectangleToCubemap;
    equirectangleToCubemap.LoadRasterizePipeline(cubemapShader, equirectangleToCubemapShader);
    equirectangleToCubemapShader.Destroy();

    equirectangleToCubemap.Bind();
    equirectangleToCubemap.SetTexture("equirectangularMap", envMap, 0);
    cube.Bind();
    glCheckError();
    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    equirectangleToCubemap.SetMat4("projection", captureProjection);

    glViewport(0, 0, 512, 512);
    captureFbo.Bind();
    glCheckError();
    const auto envCubemap = captureFbo.GetTextureName(envCubemapName);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glCheckError();
        equirectangleToCubemap.SetMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, 
            GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            envCubemap, 0);
        glCheckError();
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        glCheckError();
    }
    captureFbo.Unbind();
    equirectangleToCubemap.Destroy();
    equirectangleToCubemapShader.Destroy();

    //generate prefilter env map map
    core::pb::Shader preFilterShaderInfo;
    preFilterShaderInfo.set_type(core::pb::FRAGMENT);
    preFilterShaderInfo.set_path("shaders/prefilter.frag");

    gl::Shader preFilterShader;
    preFilterShader.LoadShader(preFilterShaderInfo);

    gl::Pipeline prefilterPipeline;
    prefilterPipeline.LoadRasterizePipeline(cubemapShader, preFilterShader);
    preFilterShader.Destroy();
    cubemapShader.Destroy();

    core::pb::FrameBuffer prefilterFramebuffer;
    prefilterFramebuffer.set_name("prefilterFBO");

    constexpr auto width = 128;
    constexpr auto height = 128;

    constexpr GLint maxMipLevels = 5;
    auto* prefilterTarget = prefilterFramebuffer.add_color_attachments();
    prefilterTarget->set_cubemap(true);
    prefilterTarget->set_format(core::pb::RenderTarget_Format_RGBA);
    prefilterTarget->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_32);
    prefilterTarget->set_size_type(core::pb::RenderTarget_Size_FIXED_SIZE);
    auto* targetSize = prefilterTarget->mutable_target_size();
    targetSize->set_x(width);
    targetSize->set_y(height);
    prefilterTarget->set_type(core::pb::RenderTarget_Type_FLOAT);
    prefilterTarget->set_name(envCubemapName.data());
    prefilterTarget->set_mipcount(maxMipLevels);

    gl::Framebuffer prefilterFBO;
    prefilterFBO.Load(prefilterFramebuffer);


    const auto prefilterMap = prefilterFBO.GetTextureName(envCubemapName);

    prefilterPipeline.Bind();
    prefilterFBO.Bind();
    prefilterPipeline.SetCubemap("environmentMap", envCubemap, 0);
    for(GLint mip = 0; mip < maxMipLevels; mip++)
    {
        LogDebug(fmt::format("Draw pre filter mip level: {}", mip));
        const auto mipWidth = static_cast<GLsizei>(width * std::pow(0.5, static_cast<double>(mip)));
        const auto mipHeight = static_cast<GLsizei>(height * std::pow(0.5, static_cast<double>(mip)));

        glViewport(0, 0, mipWidth, mipHeight);
        float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        prefilterPipeline.SetFloat("roughness", roughness);
        prefilterPipeline.SetMat4("projection", captureProjection);
        for(int i = 0; i < 6; i++)
        {
            LogDebug(fmt::format("Draw pre filter face: {}", i));
            prefilterPipeline.SetMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            
            gl::CheckFramebufferStatus();
            glClear(GL_COLOR_BUFFER_BIT);

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            glCheckError();
        }
    }
    prefilterFBO.Unbind();
    prefilterPipeline.Destroy();
    
    glCheckError();

    //export to KTX
    ktxTexture1* texture;
    ktxTextureCreateInfo createInfo;
    KTX_error_code result;

    createInfo.glInternalformat = GL_RGBA32F;
    createInfo.baseWidth = 128;
    createInfo.baseHeight = 128;
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = maxMipLevels;
    createInfo.numLayers = 1;
    createInfo.numFaces = 6;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;


    result = ktxTexture1_Create(&createInfo,
        KTX_TEXTURE_CREATE_ALLOC_STORAGE,
        &texture);
    if (!ktxCheckError(result))
    {
        return;
    }
    constexpr int maxSize = 128 * 128 * 4 * 4;
    void* buffer = std::malloc(maxSize);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (GLint mip = 0; mip < maxMipLevels; mip++)
    {
        auto mipWidth = static_cast<GLsizei>(128.0 * std::pow(0.5, static_cast<double>(mip)));
        auto mipHeight = static_cast<GLsizei>(128.0 * std::pow(0.5, static_cast<double>(mip)));
        const auto size = mipWidth * mipHeight * 4 * 4;
        for (int faceIndex = 0; faceIndex < 6; faceIndex++)
        {
            glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, mip, GL_RGBA, GL_FLOAT, buffer);
            result = ktxTexture_SetImageFromMemory(ktxTexture(texture),
                mip, 0, faceIndex,
                static_cast<const ktx_uint8_t*>(buffer), size);
            ktxCheckError(result);
            glCheckError();
        }
    }

    std::free(buffer);


    // Repeat for the other 15 slices of the base level and all other levels
    // up to createInfo.numLevels.

    ktxTexture_WriteToNamedFile(ktxTexture(texture), preFilterEnvMapPath.data());
    ktxTexture_Destroy(ktxTexture(texture));


    prefilterFBO.Destroy();
    captureFbo.Destroy();
    cube.Destroy();

}
} // namespace gpr5300
