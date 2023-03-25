#pragma once

#include "renderer/pipeline.h"

#include <GL/glew.h>
#include "proto/renderer.pb.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_float4x4.hpp>


namespace gl
{
struct Texture;

struct Shader
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

    void LoadRasterizePipeline(const Shader& vertex, const Shader& fragment);
    void LoadComputePipeline(const Shader& compute);

    void Destroy();

    //Uniform functions



private:
    GLuint name = 0;
    inline static GLuint currentBindedPipeline = 0;

    
};

}
