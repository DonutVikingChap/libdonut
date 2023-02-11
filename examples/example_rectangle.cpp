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

		renderer.clearFramebufferColor(framebuffer, Color::BLACK);

		gfx::RenderPass renderPass{};
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
};

} // namespace

int main(int /*argc*/, char* argv[]) {
	RectangleApplication application{argv[0]};
	application.run();
}
