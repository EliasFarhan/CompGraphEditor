#include "gl/debug.h"
#include "utils/log.h"
#include <fmt/format.h>
#include <GL/glew.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace gl
{
bool CheckKtxError(ktx_error_code_e error_code)
{
    switch(error_code)
    {
    case KTX_SUCCESS: 
        return true;
    case KTX_FILE_DATA_ERROR: 
        LogError("KTX File Data Error");
        break;
    case KTX_FILE_ISPIPE: 
        LogError("KTX File Is Pipe");
        break;
    case KTX_FILE_OPEN_FAILED: 
        LogError("KTX File Open Failed");
        break;
    case KTX_FILE_OVERFLOW: 
        LogError("KTX File Overflow");
        break;
    case KTX_FILE_READ_ERROR:
        LogError("KTX File Read Error");
        break;
    case KTX_FILE_SEEK_ERROR: 
        LogError("KTX File Seek Error");
        break;
    case KTX_FILE_UNEXPECTED_EOF:
        LogError("KTX File Unexpected EOF");
        break;
    case KTX_FILE_WRITE_ERROR:
        LogError("KTX File Write Error");
        break;
    case KTX_GL_ERROR:
        LogError("KTX GL Error");
        break;
    case KTX_INVALID_OPERATION:
        LogError("KTX Invalid Operation");
        break;
    case KTX_INVALID_VALUE:
        LogError("KTX Invalid Value");
        break;
    case KTX_NOT_FOUND:
        LogError("KTX Not Found");
        break;
    case KTX_OUT_OF_MEMORY:
        LogError("KTX Out Of Memory");
        break;
    case KTX_TRANSCODE_FAILED:
        LogError("KTX Transcode Failed");
        break;
    case KTX_UNKNOWN_FILE_FORMAT:
        LogError("KTX Unknown File Format"); break;
    case KTX_UNSUPPORTED_TEXTURE_TYPE:
        LogError("KTX Unsupported Texture Type");
        break;
    case KTX_UNSUPPORTED_FEATURE:
        LogError("KTX Unsupported Feature");
        break;
    case KTX_LIBRARY_NOT_LINKED:
        LogError("KTX Library Not Linked");
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
