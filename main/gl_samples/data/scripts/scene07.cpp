
#include "wasm/neko2.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))


extern "C"
{
	void WASM_EXPORT scene07_draw(int64_t drawCommand)
	{
		const auto camera = get_scene_camera();
		bind_draw_command(drawCommand);
		set_mat4(drawCommand, "view", get_view(camera));
		set_mat4(drawCommand, "projection", get_projection(camera));
		glm::mat4 model = glm::mat4(1.0f);
		set_mat4(drawCommand, "model", &model);
		draw(drawCommand);
	}

}