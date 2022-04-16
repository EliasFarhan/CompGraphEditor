#pragma once

namespace gpr5300
{
void CheckGlError(const char* file, int line);
#define glCheckError() CheckGlError(__FILE__, __LINE__)
}