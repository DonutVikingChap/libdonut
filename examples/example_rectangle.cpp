/**
 * \example example_rectangle.cpp
 *
 * \details This example shows a very basic application that renders a lime green
 *          rectangle at a fixed size in the middle of the main window.
 */

#include <donut/aliases.hpp>
#include <donut/donut.hpp>

#include <glm/ext/matrix_clip_space.hpp> // glm::ortho
#include <glm/glm.hpp>                   // glm::...

namespace {

class RectangleApplication final : public app::Application {
public:
	explicit RectangleApplication(const char* programFilepath)
		: app::Application(programFilepath, {})
		, window({.title = "Rectangle"}) {
		resize(window.getDrawableSize());
	}

protected:
	void prepareForEvents(app::FrameInfo /*frameInfo*/) override {}

	void handleEvent(app::FrameInfo /*frameInfo*/, const app::Event& event) override {
		if (event.is<app::WindowSizeChangedEvent>()) {
			resize(window.getDrawableSize());
		}
	}

	void update(app::FrameInfo /*frameInfo*/) override {}

	void tick(app::TickInfo /*tickInfo*/) override {}

	void display(app::FrameInfo /*frameInfo*/) override {
		constexpr glm::vec2 RECTANGLE_SIZE{100.0f, 60.0f};

		gfx::Framebuffer& framebuffer = window.getFramebuffer();

		renderer.clearFramebufferColor(framebuffer, Color::BLACK);

		gfx::RenderPass renderPass{};
		renderPass.draw(gfx::RectangleInstance{
			.position = glm::vec2{viewport.size / 2} - RECTANGLE_SIZE * 0.5f,
			.size = RECTANGLE_SIZE,
			.tintColor = Color::LIME,
		});

		renderer.render(framebuffer, renderPass, viewport, projectionViewMatrix);

		window.present();
	}

private:
	void resize(glm::ivec2 newWindowSize) {
		viewport = {.position{0, 0}, .size = newWindowSize};
		projectionViewMatrix = glm::ortho(0.0f, static_cast<float>(newWindowSize.x), 0.0f, static_cast<float>(newWindowSize.y));
	}

	gfx::Window window;
	gfx::Viewport viewport{};
	glm::mat4 projectionViewMatrix{};
	gfx::Renderer renderer{};
};

} // namespace

int main(int /*argc*/, char* argv[]) {
	RectangleApplication application{argv[0]};
	application.run();
}
