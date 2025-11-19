// --------------------------------------------------
// Camera buffer
// --------------------------------------------------

struct CameraData
{
    float4x4 view;
    float4x4 projection;
};

layout(binding = 0)
ConstantBuffer<CameraData> camera : register(b0, space1);


// --------------------------------------------------
// Model buffer (SSBO)
// --------------------------------------------------

layout(binding = 1)
StructuredBuffer<float4x4> model;


// --------------------------------------------------
// Texture (GLSL-style binding)
// --------------------------------------------------

layout(binding = 2)
Texture2D tex;

layout(binding = 3)
SamplerState texSampler;


// --------------------------------------------------
// Vertex shader
// --------------------------------------------------

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

VSOutput vertexMain(
    layout(location = 0) in float3 aPos,
    layout(location = 1) in float2 aTexCoord,
    uint instanceIndex : SV_InstanceID
)
{
    VSOutput o;

    float4x4 modelMat = model[instanceIndex];

    float4 worldPos = mul(modelMat, float4(aPos, 1.0));
    float4 viewPos  = mul(camera.view, worldPos);
    float4 projPos  = mul(camera.projection, viewPos);

    o.position = projPos;
    o.texCoord = aTexCoord;
    return o;
}


// --------------------------------------------------
// Fragment shader
// --------------------------------------------------

float4 fragmentMain(VSOutput input) : SV_Target
{
    return tex.Sample(texSampler, input.texCoord);
}
