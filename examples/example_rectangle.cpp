/**
 * \example example_rectangle.cpp
 *
 * \details This example shows a very basic application that renders a lime green
 *          rectangle at a fixed size in the middle of a resizable window.
 */

#include <donut/aliases.hpp>
#include <donut/donut.hpp>

namespace {

class RectangleApplication final : public app::Application {
public:
	RectangleApplication() {
		resize();
	}

protected:
	void update(app::FrameInfo /*frameInfo*/) override {
		for (const events::Event& event : eventPump.pollEvents()) {
			if (event.is<events::ApplicationQuitRequestedEvent>()) {
				quit();
			} else if (event.is<events::WindowSizeChangedEvent>()) {
				resize();
			}
		}
	}

	void display(app::TickInfo /*tickInfo*/, app::FrameInfo /*frameInfo*/) override {
		constexpr vec2 RECTANGLE_SIZE{100.0f, 60.0f};

		gfx::Framebuffer& framebuffer = window.getFramebuffer();

		renderer.clearFramebufferColor(framebuffer, Color::BLACK);

		gfx::RenderPass renderPass{};
		renderPass.draw(gfx::RectangleInstance{
			.position = vec2{viewport.size / 2} - RECTANGLE_SIZE * 0.5f,
			.size = RECTANGLE_SIZE,
			.tintColor = Color::LIME,
		});

		renderer.render(framebuffer, renderPass, viewport, camera);

		window.present();
	}

private:
	void resize() {
		const ivec2 size = window.getDrawableSize();
		viewport = {.position{0, 0}, .size = size};
		camera = gfx::Camera::createOrthographic({.offset{0.0f, 0.0f}, .size = size});
	}

	events::EventPump eventPump{};
	gfx::Window window{{.title = "Rectangle"}};
	gfx::Viewport viewport{};
	gfx::Camera camera{};
	gfx::Renderer renderer{};
};

} // namespace

int main() {
	RectangleApplication application{};
	application.run();
}
