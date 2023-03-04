#pragma once

#include "proto/renderer.pb.h"
#include <GL/glew.h>

namespace gl
{
constexpr GLenum ConvertDepthCompareOpToGL(core::pb::Pipeline_DepthCompareOp op)
{
    switch (op)
    {
    case core::pb::Pipeline_DepthCompareOp_LESS:
        return (GL_LESS);
    case core::pb::Pipeline_DepthCompareOp_LESS_OR_EQUAL:
        return(GL_LEQUAL);
    case core::pb::Pipeline_DepthCompareOp_EQUAL:
        return(GL_EQUAL);
    case core::pb::Pipeline_DepthCompareOp_GREATER:
        return(GL_GREATER);
    case core::pb::Pipeline_DepthCompareOp_NOT_EQUAL:
        return(GL_NOTEQUAL);
    case core::pb::Pipeline_DepthCompareOp_GREATER_OR_EQUAL:
        return(GL_GEQUAL);
    case core::pb::Pipeline_DepthCompareOp_ALWAYS:
        return(GL_ALWAYS);
    case core::pb::Pipeline_DepthCompareOp_NEVER:
        return(GL_NEVER);
    default:
        return GL_NEVER;
    }
}

constexpr GLenum ConvertBlendFuncToGL(core::pb::Pipeline_BlendFunc blendFunc)
{
    switch (blendFunc)
    {
        case core::pb::Pipeline_BlendFunc_BLEND_ZERO: return GL_ZERO;
        case core::pb::Pipeline_BlendFunc_ONE: return GL_ONE;
        case core::pb::Pipeline_BlendFunc_SRC_COLOR: return GL_SRC_COLOR;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
        case core::pb::Pipeline_BlendFunc_DST_COLOR: return GL_DST_COLOR;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
        case core::pb::Pipeline_BlendFunc_SRC_ALPHA: return GL_SRC_ALPHA;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
        case core::pb::Pipeline_BlendFunc_DST_ALPHA: return GL_DST_ALPHA;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
        case core::pb::Pipeline_BlendFunc_CONSTANT_COLOR: return GL_CONSTANT_COLOR;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_CONSTANT_COLOR: return GL_ONE_MINUS_CONSTANT_COLOR;
        case core::pb::Pipeline_BlendFunc_CONSTANT_ALPHA: return GL_CONSTANT_ALPHA;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_CONSTANT_ALPHA: return GL_ONE_MINUS_CONSTANT_ALPHA;
        case core::pb::Pipeline_BlendFunc_SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
        case core::pb::Pipeline_BlendFunc_SRC1_COLOR: return GL_SRC1_COLOR;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC1_COLOR: return GL_ONE_MINUS_SRC1_COLOR;
        case core::pb::Pipeline_BlendFunc_SRC1_ALPHA: return GL_SRC1_ALPHA;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC1_ALPHA: return GL_ONE_MINUS_SRC1_ALPHA;
        default:;
    }
    return GL_ZERO;
}

constexpr GLenum ConvertStencilOpToGL(core::pb::Pipeline_StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case core::pb::Pipeline_StencilOp_KEEP: return GL_KEEP;
        case core::pb::Pipeline_StencilOp_STENCIL_ZERO: return GL_ZERO;
        case core::pb::Pipeline_StencilOp_REPLACE: return GL_REPLACE;
        case core::pb::Pipeline_StencilOp_INCR: return GL_INCR;
        case core::pb::Pipeline_StencilOp_INCR_WRAP: return GL_INCR_WRAP;
        case core::pb::Pipeline_StencilOp_DECR: return GL_DECR;
        case core::pb::Pipeline_StencilOp_DECR_WRAP: return GL_DECR_WRAP;
        case core::pb::Pipeline_StencilOp_INVERT: return GL_INVERT;
        default:;
    }
    return GL_KEEP;
}

constexpr GLenum ConvertCullFaceToGL(core::pb::Pipeline_CullFace cullFace)
{
    switch (cullFace)
    {
        case core::pb::Pipeline_CullFace_BACK:
            return (GL_BACK);
            break;
        case core::pb::Pipeline_CullFace_FRONT:
            return (GL_FRONT);
            break;
        case core::pb::Pipeline_CullFace_FRONT_AND_BACK:
            return (GL_FRONT_AND_BACK);
            break;
        default:
            break;
    }
    return GL_BACK;
}

constexpr GLenum ConvertFrontFaceToGL(core::pb::Pipeline_FrontFace frontFace)
{
    switch (frontFace)
    {
        case core::pb::Pipeline_FrontFace_COUNTER_CLOCKWISE:
            return (GL_CCW);
            break;
        case core::pb::Pipeline_FrontFace_CLOCKWISE:
            return (GL_CW);
            break;
        default:
            break;
    }
    return GL_CCW;
}

} // namespace gl
