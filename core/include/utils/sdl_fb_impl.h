#ifndef NOVUS_UTILS_SDL_FB_IMPL_H
#define NOVUS_UTILS_SDL_FB_IMPL_H

#include "SDL3/SDL_pixels.h"

namespace novus::renderer
{
struct Color;
}

namespace flatbuffers {
novus::renderer::Color Pack(const SDL_FColor& obj);

SDL_FColor UnPack(const novus::renderer::Color& obj);
}

#endif //NOVUS_UTILS_SDL_FB_IMPL_H