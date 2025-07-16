
#include "wasm/neko2.h"
#include "wasm/camera.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "wasm/draw_command.h"

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))


extern "C"
{
	void WASM_EXPORT scene07_draw(int64_t drawCommandId)
	{
	    script::DrawCommand drawCommand(drawCommandId);
		const auto camera = script::GetSceneCamera();
		drawCommand.Draw();
		drawCommand.SetMat4("view", camera.GetView());
		drawCommand.SetMat4("projection", camera.GetProjection());
		glm::mat4 model = glm::mat4(1.0f);
		drawCommand.SetMat4("model", model);
		drawCommand.Draw();
	}

}