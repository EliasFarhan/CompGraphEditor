#pragma once

#include <string_view>
#include <GL/glew.h>
#include "proto/renderer.pb.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_float4x4.hpp>

#include "renderer/texture.h"

namespace gpr5300
{

struct Shader
{
    void LoadShader(const pb::Shader& shader);
    GLuint name = 0;
    void Destroy();
};

class Pipeline
{
public:
    void Bind() const;
    static void Unbind();
    void LoadRasterizePipeline(const Shader& vertex, const Shader& fragment);
    void LoadComputePipeline(const Shader& compute);
    void Destroy();

    //Uniform functions
    void SetFloat(std::string_view uniformName, float f);

    void SetInt(std::string_view uniformName, int i) ;

    void SetVec2(std::string_view uniformName, glm::vec2 v);

    void SetVec3(std::string_view uniformName, glm::vec3 v);

    void SetVec4(std::string_view uniformName, glm::vec4 v);

    void SetMat4(std::string_view uniformName, const glm::mat4& mat);

    void SetTexture(std::string_view uniformName, const Texture& texture, GLenum textureUnit) ;
private:
    GLuint name = 0;
    inline static GLuint currentBindedPipeline = 0;

    std::unordered_map<std::string, int> uniformMap_;
    int GetUniformLocation(std::string_view uniformName) ;
};

}
