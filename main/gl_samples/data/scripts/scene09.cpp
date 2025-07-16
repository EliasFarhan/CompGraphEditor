#include "wasm/neko2.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "wasm/camera.h"
#include "wasm/draw_command.h"

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))


extern "C"
{
	void WASM_EXPORT scene09_draw(int64_t drawCommandId)
	{
	    script::DrawCommand drawCommand(drawCommandId);
		const auto camera = script::GetSceneCamera();
		drawCommand.Bind();
		if (drawCommand.EqualsName("model"))
		{
			drawCommand.SetVec3("cameraPos", camera.GetPosition());
			drawCommand.SetMat4("model", glm::mat4(1.0f));
			drawCommand.SetMat4("normalMatrix", glm::mat4(1.0f));
			drawCommand.SetMat4("view", camera.GetView());
		}
		else if (drawCommand.EqualsName("skybox"))
		{
			drawCommand.SetMat4( "view", glm::mat4(glm::mat3(camera.GetView())));
		}
		drawCommand.SetMat4("projection", camera.GetProjection());
		drawCommand.Draw();
	}

}