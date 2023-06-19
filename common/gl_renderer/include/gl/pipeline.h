#pragma once

#include "renderer/pipeline.h"

#include <GL/glew.h>
#include "proto/renderer.pb.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_float4x4.hpp>

#include <optional>


namespace gl
{
struct Texture;

struct Shader : core::Shader
{
    ~Shader();
    void LoadShader(const core::pb::Shader& shader);
    GLuint name = 0;
    void Destroy();
};

class Pipeline : public core::Pipeline
{
public:

    ~Pipeline() override;
    void Bind() override;
    GLuint GetName() const { return name; }
    static void Unbind();

    void LoadRasterizePipeline(const Shader& vertex, 
        const Shader& fragment,
        std::optional<std::reference_wrapper<Shader>> geometryShader = std::nullopt,
        std::optional<std::reference_wrapper<Shader>> tesselationControlShader = std::nullopt,
        std::optional<std::reference_wrapper<Shader>> tesselationEvalShader = std::nullopt);
    void LoadComputePipeline(const Shader& compute);

    void Destroy();

    //Uniform functions
    void SetFloat(std::string_view uniformName, float f);
    void SetInt(std::string_view uniformName, int i);
    void SetVec2(std::string_view uniformName, glm::vec2 v);
    void SetVec3(std::string_view uniformName, glm::vec3 v);
    void SetVec4(std::string_view uniformName, glm::vec4 v);
    void SetMat4(std::string_view uniformName, const glm::mat4& mat);
    void SetMat3(std::string_view uniformName, const glm::mat3& mat);
    void SetBool(std::string_view uniformName, bool b);

    void SetTexture(std::string_view uniformName, const Texture& texture, GLenum textureUnit);
    void SetTexture(std::string_view uniformName, GLuint textureName, GLenum textureUnit);
    void SetCubemap(std::string_view uniformName, GLuint textureName, GLenum textureUnit);


private:
    GLuint name = 0;
    inline static GLuint currentBindedPipeline = 0;
    std::unordered_map<std::string, int> uniformMap_;
    int GetUniformLocation(std::string_view uniformName);

    
};

}
