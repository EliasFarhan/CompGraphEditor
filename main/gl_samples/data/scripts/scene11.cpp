#include "wasm/neko2.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <vector>
#include <random>
#include <format>
#include <numbers>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "wasm/camera.h"
#include "wasm/buffer.h"
#include "wasm/draw_command.h"

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility ("default")))

static std::vector<glm::vec4> positions{};
static constexpr float gravity_const = 1000.0f;
static constexpr float center_mass = 1000.0f;
static constexpr float planet_mass = 1.0f;
static constexpr size_t planet_count = 10000;

extern "C"
{

void WASM_EXPORT scene11_begin()
{
	// Seed with a real random value, if available
	std::random_device r;

	std::default_random_engine e1(r());
	std::uniform_real_distribution<float> uniform_dist_angle(0.0f, 2.0f * std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> uniform_dist_radius(20.0f, 50.0f);
	positions.resize(planet_count);
	for (size_t i = 0; i < planet_count; i++)
	{
	    const auto angle = uniform_dist_angle(e1);
		positions[i] = glm::vec4(std::sin(angle), 0.0f, std::cos(angle), 0.0f)*uniform_dist_radius(e1);
	}
    const auto buffer = script::GetBuffer("positions");
    buffer.CopyData(positions);

    const auto camera = script::GetSystemCamera();
    static constexpr auto cameraPos = glm::vec3(100.0f);
    camera.SetPosition(cameraPos);
    camera.SetDirection(-glm::normalize(cameraPos));
    camera.SetFar(500.0f);


}
void WASM_EXPORT scene11_update(float dt)
{
	for (auto& pos: positions)
	{
		const auto delta = -pos;
		const auto r2 = glm::length2(delta);
		const auto force_value = gravity_const * center_mass * planet_mass / 2;
		const auto vel_dir = glm::normalize(glm::vec4(-delta.z, 0.0f, delta.x, 0.0f));
	    const auto r = std::sqrt(r2);
		const auto speed = std::sqrt(force_value / planet_mass * r);
		const auto vel = vel_dir * speed;
		pos += vel * dt;
	}

    const auto buffer = script::GetBuffer("positions");
    buffer.CopyData(positions);

}
void WASM_EXPORT scene11_draw(int64_t drawCommandId)
{
    script::DrawCommand drawCommand(drawCommandId);
	const auto camera = script::GetSceneCamera();

    drawCommand.Bind();
    drawCommand.SetMat4("view", camera.GetView());
	drawCommand.SetMat4("projection", camera.GetProjection());

	drawCommand.Draw(positions.size());
}

void WASM_EXPORT scene11_end()
{
	positions.resize(0);
}
}