#ifndef NOVUS_UTILS_SDL_FB_IMPL_H
#define NOVUS_UTILS_SDL_FB_IMPL_H


#include "SDL3/SDL_pixels.h"
#include <generated/color_generated.h>

namespace novus::internal
{
struct Color;
}

namespace flatbuffers {
novus::internal::Color Pack(const SDL_FColor& obj);

SDL_FColor UnPack(const novus::internal::Color& obj);
}

#endif //NOVUS_UTILS_SDL_FB_IMPL_H