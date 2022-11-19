#pragma once
#include <ktx.h>
#include <string>

namespace gpr5300::gl
{
bool CheckKtxError(KTX_error_code error_code);
std::string GetGlError(GLenum err);
void CheckGlError(const char* file, int line);
#define glCheckError() CheckGlError(__FILE__, __LINE__)
}
