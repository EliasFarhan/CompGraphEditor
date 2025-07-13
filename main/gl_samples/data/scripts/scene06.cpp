#include "wasm/neko2.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))


extern "C"
{

	static float t = 0.0f;
	static constexpr std::array<glm::vec3, 10> positions{
		{
			glm::vec3(),
			glm::vec3(2.0, 5.0, -15.0),
			glm::vec3(-1.5, -2.2, -2.5),
			glm::vec3(-3.8, -2.0, -12.3),
			glm::vec3(2.4, -0.4, -3.5),
			glm::vec3(-1.7, 3.0, -7.5),
			glm::vec3(1.3, -2.0, -2.5),
			glm::vec3(1.5, 2.0, -2.5),
			glm::vec3(1.5, 0.2, -1.5),
			glm::vec3(-1.3, 1.0, -1.5)
		}};

	void WASM_EXPORT scene06_begin()
	{
		t = 0.0f;
	}

	void WASM_EXPORT scene06_update(float dt)
	{
		t += dt;
	}

	void WASM_EXPORT scene06_draw(int64_t drawCommand)
	{
		bind_draw_command(drawCommand);
		auto view = glm::mat4(1.0f);
		view = glm::translate(view, glm::vec3(0,0,-5));
		set_mat4(drawCommand, "view", view);
		auto projection = glm::perspective(glm::radians(45.0f), get_aspect(), 0.1f, 100.0f);
		set_mat4(drawCommand, "projection", projection);
		for (auto& position : positions)
		{
			auto model = glm::mat4(1.0f);
			model = glm::translate(model, position);
			model = glm::rotate(model, t, glm::vec3(0, 0, 1));
			model = glm::rotate(model, t, glm::vec3(0, 1, 0));
			set_mat4(drawCommand, "model", model);
			draw(drawCommand);
		}
	}

	void WASM_EXPORT scene06_end()
	{

	}
}