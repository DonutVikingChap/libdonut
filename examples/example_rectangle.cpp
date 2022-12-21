/**
 * \example example_rectangle.cpp
 *
 * \details This example shows a very basic application that renders a lime green
 *          rectangle at a fixed size in the middle of the main window.
 */

#include <donut/Color.hpp>
#include <donut/application/Application.hpp>
#include <donut/application/Event.hpp>
#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/RenderPass.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/Viewport.hpp>

#include <glm/ext/matrix_clip_space.hpp> // glm::ortho
#include <glm/glm.hpp>                   // glm::...

namespace app = donut::application;
namespace gfx = donut::graphics;
using Color = donut::Color;

class Application : public app::Application {
public:
	explicit Application(const char* programFilepath)
		: app::Application(programFilepath, {.windowTitle = "Rectangle"}) {}

protected:
	void resize(glm::ivec2 newWindowSize) override {
		viewport = {.position{0, 0}, .size = newWindowSize};
		projectionViewMatrix = glm::ortho(0.0f, static_cast<float>(newWindowSize.x), 0.0f, static_cast<float>(newWindowSize.y));
	}

	void prepareForEvents(app::FrameInfo /*frameInfo*/) override {}
	void handleEvent(app::FrameInfo /*frameInfo*/, const app::Event& /*event*/) override {}
	void update(app::FrameInfo /*frameInfo*/) override {}
	void tick(app::TickInfo /*tickInfo*/) override {}
	void prepareForDisplay(app::FrameInfo /*frameInfo*/) override {}

	void display(app::FrameInfo /*frameInfo*/) override {
		constexpr glm::vec2 RECTANGLE_SIZE{100.0f, 60.0f};

		renderPass.reset();
		renderPass.setBackgroundColor(Color::BLACK);
		renderPass.draw(gfx::RectangleInstance{
			.position = glm::vec2{viewport.size / 2} - RECTANGLE_SIZE * 0.5f,
			.size = RECTANGLE_SIZE,
			.tintColor = Color::LIME,
		});
		renderer.render(framebuffer, renderPass, viewport, projectionViewMatrix);
	}

private:
	gfx::Framebuffer framebuffer = gfx::Framebuffer::getDefault();
	gfx::Viewport viewport{};
	glm::mat4 projectionViewMatrix{};
	gfx::Renderer renderer{};
	gfx::RenderPass renderPass{};
};

int main(int /*argc*/, char* argv[]) {
	Application application{argv[0]};
	application.run();
}
