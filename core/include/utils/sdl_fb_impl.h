#ifndef NOVUS_UTILS_SDL_FB_IMPL_H
#define NOVUS_UTILS_SDL_FB_IMPL_H

#include <generated/internal_renderer_generated.h>


#include "SDL3/SDL_pixels.h"

namespace novus::renderer
{
struct Color;
}

namespace flatbuffers {
novus::internal::Color Pack(const SDL_FColor& obj);

SDL_FColor UnPack(const novus::internal::Color& obj);
}

#endif //NOVUS_UTILS_SDL_FB_IMPL_H