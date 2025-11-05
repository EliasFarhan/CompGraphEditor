#include "utils/sdl_fb_impl.h"
#include "generated/renderer_generated.h"

namespace flatbuffers {
novus::internal::Color Pack(const SDL_FColor& obj)
{
    return {obj.r, obj.g, obj.b, obj.a};
}

SDL_FColor UnPack(const novus::internal::Color& obj)
{
    return {obj.r(), obj.g(), obj.b(), obj.a()};
}
}
