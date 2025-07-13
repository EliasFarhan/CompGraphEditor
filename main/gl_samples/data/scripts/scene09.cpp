#include "wasm/neko2.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))


extern "C"
{
	void WASM_EXPORT scene09_draw(int64_t drawCommand)
	{
		const auto camera = get_scene_camera();
		bind_draw_command(drawCommand);
		if (name_equals(get_name(drawCommand), "model"))
		{
			set_vec3(drawCommand, "cameraPos", get_camera_position(camera));
			set_mat4(drawCommand, "model", glm::mat4(1.0f));
			set_mat4(drawCommand, "normalMatrix", glm::mat4(1.0f));
			set_mat4(drawCommand, "view", get_camera_view(camera));
		}
		else if (name_equals(get_name(drawCommand), "skybox"))
		{
			set_mat4(drawCommand, "view", glm::mat4(glm::mat3(get_camera_view(camera))));
		}
		set_mat4(drawCommand, "projection", get_camera_projection(camera));
		draw(drawCommand);
	}

}