#include "gl/debug.h"
#include "utils/log.h"

#include "gl/include.h"
#include <format>

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
        LogError(std::format("KTX File Data Error: {}:{}", file, line));
        break;
    case KTX_FILE_ISPIPE: 
        LogError(std::format("KTX File Is Pipe, {}:{}", file, line));
        break;
    case KTX_FILE_OPEN_FAILED: 
        LogError(std::format("KTX File Open Failed, {}:{}", file, line));
        break;
    case KTX_FILE_OVERFLOW: 
        LogError(std::format("KTX File Overflow, {}:{}", file, line));
        break;
    case KTX_FILE_READ_ERROR:
        LogError(std::format("KTX File Read Error, {}:{}", file, line));
        break;
    case KTX_FILE_SEEK_ERROR: 
        LogError(std::format("KTX File Seek Error, {}:{}", file, line));
        break;
    case KTX_FILE_UNEXPECTED_EOF:
        LogError(std::format("KTX File Unexpected EOF, {}:{}", file, line));
        break;
    case KTX_FILE_WRITE_ERROR:
        LogError(std::format("KTX File Write Error, {}:{}", file, line));
        break;
    case KTX_GL_ERROR:
        LogError(std::format("KTX GL Error, {}:{}", file, line));
        break;
    case KTX_INVALID_OPERATION:
        LogError(std::format("KTX Invalid Operation, {}:{}", file, line));
        break;
    case KTX_INVALID_VALUE:
        LogError(std::format("KTX Invalid Value, {}:{}", file, line));
        break;
    case KTX_NOT_FOUND:
        LogError(std::format("KTX Not Found, {}:{}", file, line));
        break;
    case KTX_OUT_OF_MEMORY:
        LogError(std::format("KTX Out Of Memory, {}:{}", file, line));
        break;
    case KTX_TRANSCODE_FAILED:
        LogError(std::format("KTX Transcode Failed, {}:{}", file, line));
        break;
    case KTX_UNKNOWN_FILE_FORMAT:
        LogError(std::format("KTX Unknown File Format, {}:{}", file, line));
        break;
    case KTX_UNSUPPORTED_TEXTURE_TYPE:
        LogError(std::format("KTX Unsupported Texture Type, {}:{}", file, line));
        break;
    case KTX_UNSUPPORTED_FEATURE:
        LogError(std::format("KTX Unsupported Feature, {}:{}", file, line));
        break;
    case KTX_LIBRARY_NOT_LINKED:
        LogError(std::format("KTX Library Not Linked, {}:{}", file, line));
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
        LogError(std::format("{}, in file: {} line: {}", errorLog, file, line));
    }
}
}
