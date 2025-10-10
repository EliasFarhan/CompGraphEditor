#include "wasm/neko2.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <random>
#include <format>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "wasm/camera.h"
#include "wasm/draw_command.h"

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))

static std::vector<glm::vec3> positions{};
static constexpr float gravity_const = 1000.0f;
static constexpr float center_mass = 1000.0f;
static constexpr float planet_mass = 1.0f;
static constexpr size_t planet_count = 1000;

extern "C"
{

void WASM_EXPORT scene08_begin()
{
	// Seed with a real random value, if available
	std::random_device r;

	std::default_random_engine e1(r());
	std::uniform_real_distribution<float> uniform_dist(10.0f, 50.0f);
	positions.resize(planet_count);
	for (auto& pos: positions)
	{
		pos = glm::vec3(uniform_dist(e1), 0.0f, uniform_dist(e1));
	}

    const auto camera = script::GetSystemCamera();
    static constexpr auto cameraPos = glm::vec3(100.0f);
    camera.SetPosition(cameraPos);
    camera.SetDirection(-glm::normalize(cameraPos));
    camera.SetFar(500.0f);


}
void WASM_EXPORT scene08_update(float dt)
{
	for (auto& pos: positions)
	{
		const auto delta = -pos;
		const auto r = glm::length(delta);
		const auto force_value = gravity_const * center_mass * planet_mass / (r * r);
		const auto vel_dir = glm::normalize(glm::vec3(-delta.z, 0.0f, delta.x));
		const auto speed = std::sqrt(force_value / planet_mass * r);
		const auto vel = vel_dir * speed;
		pos += vel * dt;
	}
}
void WASM_EXPORT scene08_draw(int64_t drawCommandId)
{
    script::DrawCommand drawCommand(drawCommandId);
	const auto camera = script::GetSceneCamera();

    drawCommand.Bind();
    drawCommand.SetMat4("ubo.view", camera.GetView());
	drawCommand.SetMat4("ubo.projection", camera.GetProjection());

	for (size_t i = 0; i < positions.size(); ++i)
	{
		std::string uniformName = std::format("ubo.pos[{}]", i);
		drawCommand.SetVec3(uniformName.c_str(), positions[i]);
	}

	drawCommand.Draw(positions.size());
}

void WASM_EXPORT scene08_end()
{
	positions.resize(0);
}
}