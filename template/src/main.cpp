#include <donut/aliases.hpp>
#include <donut/donut.hpp>

#include <glm/glm.hpp>

class Application final : public app::Application {
public:
	explicit Application(const char* programFilepath)
		: app::Application(programFilepath, {}) {}

protected:
	void prepareForEvents(app::FrameInfo /*frameInfo*/) override {
		// TODO
	}

	void handleEvent(app::FrameInfo /*frameInfo*/, const app::Event& /*event*/) override {
		// TODO
	}

	void update(app::FrameInfo /*frameInfo*/) override {
		// TODO
	}

	void tick(app::TickInfo /*tickInfo*/) override {
		// TODO
	}

	void display(app::FrameInfo /*frameInfo*/) override {
		// TODO
	}
};

int main(int /*argc*/, char* argv[]) {
	Application application{argv[0]};
	application.run();
}
