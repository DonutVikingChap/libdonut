#include <donut/aliases.hpp>
#include <donut/donut.hpp>

#include <cstdio>
#include <cstdlib>

struct ApplicationOptions {
	// TODO: Decide initial option values.
	// TODO: See the documentation for more information about each options struct.
	app::ApplicationOptions applicationOptions{};
	gfx::WindowOptions windowOptions{};
	gfx::RendererOptions rendererOptions{};
	audio::SoundStageOptions soundStageOptions{};
	events::InputManagerOptions inputManagerOptions{};
};

class Application final : public app::Application {
public:
	Application(Filesystem& filesystem, const ApplicationOptions& options)
		: app::Application(options.applicationOptions)
		, window(options.windowOptions)
		, renderer(options.rendererOptions)
		, soundStage(options.soundStageOptions)
		, inputManager(options.inputManagerOptions) {
		resize();

		(void)filesystem; // TODO: Load assets, initialize state, etc.
	}

protected:
	void update(app::FrameInfo frameInfo) override {
		inputManager.prepareForEvents();
		for (const events::Event& event : eventPump.pollEvents()) {
			if (event.is<events::ApplicationQuitRequestedEvent>()) {
				quit();
			} else if (event.is<events::WindowSizeChangedEvent>()) {
				resize();
			}
			inputManager.handleEvent(event);
		}

		soundStage.update(frameInfo.deltaTime, listener);

		(void)frameInfo; // TODO: Update movement directions, aim angles, etc.
	}

	void tick(app::TickInfo tickInfo) override {
		(void)tickInfo; // TODO: Step physics simulation, etc.
	}

	void display(app::TickInfo tickInfo, app::FrameInfo frameInfo) override {
		(void)tickInfo, (void)frameInfo; // TODO: Interpolate animations, camera, listener, etc.

		gfx::Framebuffer& framebuffer = window.getFramebuffer();

		renderer.clearFramebufferColor(framebuffer, Color::BLACK);

		{
			gfx::RenderPass renderPass{};

			// TODO: Draw the world to the render pass.

			renderer.render(framebuffer, renderPass, viewport, worldCamera);
		}

		{
			gfx::RenderPass renderPass{};

			// TODO: Draw the user interface to the render pass.

			renderer.render(framebuffer, renderPass, viewport, uiCamera);
		}

		window.present();
	}

private:
	void resize() {
		const ivec2 size = window.getDrawableSize();
		viewport = {.position{0, 0}, .size = size};
		worldCamera = gfx::Camera::createPerspective({.verticalFieldOfView = radians(74.0f)});
		uiCamera = gfx::Camera::createOrthographic({.offset{0.0f, 0.0f}, .size = size});
	}

	events::EventPump eventPump{};
	gfx::Window window;
	gfx::Viewport viewport{};
	gfx::Camera worldCamera{};
	gfx::Camera uiCamera{};
	gfx::Renderer renderer;
	audio::SoundStage soundStage;
	audio::Listener listener{};
	events::InputManager inputManager;
};

int main(int /*argc*/, char* argv[]) {
	try {
		FilesystemOptions filesystemOptions{};
		Filesystem filesystem{argv[0], filesystemOptions};

		ApplicationOptions applicationOptions{}; // TODO: Override options based on command line, configuration files, etc.
		Application application{filesystem, applicationOptions};

		application.run();
	} catch (const std::exception& e) {
		std::fprintf(stderr, "%s\n", e.what());
		events::MessageBox::show(events::MessageBox::Type::ERROR_MESSAGE, "Error", e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
