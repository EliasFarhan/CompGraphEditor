#include "gl/debug.h"
#include "utils/log.h"
#include <fmt/format.h>
#include <GL/glew.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace gl
{
bool CheckKtxError(ktx_error_code_e error_code, const char* file, int line)
{
    switch(error_code)
    {
    case KTX_SUCCESS: 
        return true;
    case KTX_FILE_DATA_ERROR: 
        LogError(fmt::format("KTX File Data Error: {}:{}", file, line));
        break;
    case KTX_FILE_ISPIPE: 
        LogError(fmt::format("KTX File Is Pipe, {}:{}", file, line));
        break;
    case KTX_FILE_OPEN_FAILED: 
        LogError(fmt::format("KTX File Open Failed, {}:{}", file, line));
        break;
    case KTX_FILE_OVERFLOW: 
        LogError(fmt::format("KTX File Overflow, {}:{}", file, line));
        break;
    case KTX_FILE_READ_ERROR:
        LogError(fmt::format("KTX File Read Error, {}:{}", file, line));
        break;
    case KTX_FILE_SEEK_ERROR: 
        LogError(fmt::format("KTX File Seek Error, {}:{}", file, line));
        break;
    case KTX_FILE_UNEXPECTED_EOF:
        LogError(fmt::format("KTX File Unexpected EOF, {}:{}", file, line));
        break;
    case KTX_FILE_WRITE_ERROR:
        LogError(fmt::format("KTX File Write Error, {}:{}", file, line));
        break;
    case KTX_GL_ERROR:
        LogError(fmt::format("KTX GL Error, {}:{}", file, line));
        break;
    case KTX_INVALID_OPERATION:
        LogError(fmt::format("KTX Invalid Operation, {}:{}", file, line));
        break;
    case KTX_INVALID_VALUE:
        LogError(fmt::format("KTX Invalid Value, {}:{}", file, line));
        break;
    case KTX_NOT_FOUND:
        LogError(fmt::format("KTX Not Found, {}:{}", file, line));
        break;
    case KTX_OUT_OF_MEMORY:
        LogError(fmt::format("KTX Out Of Memory, {}:{}", file, line));
        break;
    case KTX_TRANSCODE_FAILED:
        LogError(fmt::format("KTX Transcode Failed, {}:{}", file, line));
        break;
    case KTX_UNKNOWN_FILE_FORMAT:
        LogError(fmt::format("KTX Unknown File Format, {}:{}", file, line));
        break;
    case KTX_UNSUPPORTED_TEXTURE_TYPE:
        LogError(fmt::format("KTX Unsupported Texture Type, {}:{}", file, line));
        break;
    case KTX_UNSUPPORTED_FEATURE:
        LogError(fmt::format("KTX Unsupported Feature, {}:{}", file, line));
        break;
    case KTX_LIBRARY_NOT_LINKED:
        LogError(fmt::format("KTX Library Not Linked, {}:{}", file, line));
        break;
    default: break;
    }
    return false;
}

std::string GetGlError(GLenum err)
{
    std::string log;
    // Process/log the error.
    switch (err)
    {
    case GL_INVALID_ENUM:
        log = "GL Invalid Enum";
        break;
    case GL_INVALID_VALUE:
        log = "GL Invalid Value";
        break;
    case GL_INVALID_OPERATION:
        log = "GL Invalid Operation";
        break;
    case GL_OUT_OF_MEMORY:
        log = "GL Out Of Memory";
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        log = "GL Invalid Framebuffer Operation";
        break;
    default:
        break;
    }
    return log;
}

void CheckGlError(const char *file, int line)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        auto errorLog = GetGlError(err);
        if (errorLog.empty())
            continue;
        LogError(fmt::format("{}, in file: {} line: {}", errorLog, file, line));
    }
}
}
