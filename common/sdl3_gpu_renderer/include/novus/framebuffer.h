//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_FRAMEBUFFER_H
#define NEKO2_FRAMEBUFFER_H
#include "renderer/framebuffer.h"

namespace novus
{
class Framebuffer : public core::Framebuffer
{
public:
	std::unique_ptr<core::Image> GetImage(std::string_view attachmentName) override;
};
}
#endif //NEKO2_FRAMEBUFFER_H
