#include <donut/Color.hpp>
#include <donut/File.hpp>
#include <donut/InputFileStream.hpp>
#include <donut/application/Application.hpp>
#include <donut/application/Event.hpp>
#include <donut/application/Input.hpp>
#include <donut/application/InputManager.hpp>
#include <donut/audio/Error.hpp>
#include <donut/audio/Sound.hpp>
#include <donut/audio/SoundStage.hpp>
#include <donut/graphics/Font.hpp>
#include <donut/graphics/Framebuffer.hpp>
#include <donut/graphics/ImageLDR.hpp>
#include <donut/graphics/RenderPass.hpp>
#include <donut/graphics/Renderer.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/json.hpp>

#include <algorithm>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fmt/format.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>

namespace app = donut::application;
namespace audio = donut::audio;
namespace gfx = donut::graphics;
namespace json = donut::json;
using Color = donut::Color;
using File = donut::File;
using InputFileStream = donut::InputFileStream;

namespace {

struct GameOptions {
	app::ApplicationOptions applicationOptions{
		.organizationName = "Donut",
		.applicationName = "ExampleGame",
		.dataDirectoryFilepath = "data",
		.archiveFilenameExtension = "pak",
		.windowTitle = "Example Game",
		.windowWidth = 640,
		.windowHeight = 480,
		.windowResizable = false,
		.maxFps = 240.0f,
	};
	const char* mainMenuMusicFilepath = "sounds/music/donauwalzer.ogg";
	float fieldOfView = 90.0f;
};

class Game final : public app::Application {
public:
	enum class Action {
		CONFIRM,
		CANCEL,
		MOVE_UP,
		MOVE_DOWN,
		MOVE_LEFT,
		MOVE_RIGHT,
		AIM_UP,
		AIM_DOWN,
		AIM_LEFT,
		AIM_RIGHT,
		SPRINT,
		ATTACK,
		SCROLL_UP,
		SCROLL_DOWN,
	};

	Game(const char* programFilepath, const GameOptions& options)
		: app::Application(programFilepath, options.applicationOptions)
		, mainFont(std::make_shared<gfx::Font>("fonts/unscii/unscii-8.ttf"))
		, verticalFieldOfView(2.0f * glm::atan((3.0f / 4.0f) * glm::tan(glm::radians(options.fieldOfView) * 0.5f))) {
		loadBindingsConfiguration();
		initializeSoundStage();
		playMainMenuMusic(options.mainMenuMusicFilepath);
	}

private:
	void resize(glm::ivec2 newWindowSize) override {
		screenViewport.size = newWindowSize;

		inputManager.resize(newWindowSize);

		const glm::vec2 screenViewportPosition{static_cast<float>(screenViewport.position.x), static_cast<float>(screenViewport.position.y)};
		const glm::vec2 screenViewportSize{static_cast<float>(screenViewport.size.x), static_cast<float>(screenViewport.size.y)};
		screenProjectionViewMatrix = glm::ortho(
			screenViewportPosition.x, screenViewportPosition.x + screenViewportSize.x, screenViewportPosition.y + screenViewportSize.y, screenViewportPosition.y);

		worldViewport = {.position{15, 15}, .size{380, 450}};

		const float aspectRatio = static_cast<float>(worldViewport.size.x) / static_cast<float>(worldViewport.size.y);
		worldProjectionViewMatrix = glm::perspective(-verticalFieldOfView, -aspectRatio, 0.1f, 100.0f);
	}

	void beginFrame(const app::FrameInfo& frameInfo) override {
		inputManager.beginFrame();
		if (soundStage) {
			soundStage->beginFrame(frameInfo.deltaTime, soundListenerPosition);
		}
	}

	void handleEvent(const app::FrameInfo& frameInfo, const app::Event& event) override {
		(void)frameInfo; // TODO: Use?

		inputManager.handleEvent(event);
	}

	void update(const app::FrameInfo& frameInfo) override {
		if (inputManager.justPressed(app::Input::KEY_F10)) {
			quit();
		}

		if (inputManager.justPressed(app::Input::KEY_F11) ||
			(inputManager.justPressed(app::Input::KEY_RETURN) && (inputManager.isPressed(app::Input::KEY_LALT) || inputManager.isPressed(app::Input::KEY_RALT)))) {
			setWindowFullscreen(!isWindowFullscreen());
		}

		if (inputManager.justPressed(app::Input::KEY_F2)) {
			if (soundStage) {
				soundStage->stopSound(musicId);
			}
		}

		const float sprintInput = (inputManager.isPressed(Action::SPRINT)) ? 4.0f : 1.0f;

		glm::vec2 movementInput{
			inputManager.getAbsoluteValue(Action::MOVE_RIGHT) - inputManager.getAbsoluteValue(Action::MOVE_LEFT),
			inputManager.getAbsoluteValue(Action::MOVE_DOWN) - inputManager.getAbsoluteValue(Action::MOVE_UP),
		};
		if (const float movementInputLengthSquared = glm::length2(movementInput); movementInputLengthSquared > 1.0f) {
			movementInput /= glm::sqrt(movementInputLengthSquared);
		}
		const float carrotCakeSpeed = 2.0f * sprintInput;
		carrotCakePosition.x += movementInput.x * carrotCakeSpeed * frameInfo.deltaTime;
		carrotCakePosition.y += movementInput.y * carrotCakeSpeed * frameInfo.deltaTime;

		if (inputManager.isPressed(Action::CONFIRM)) {
			const glm::vec2 aimInput{
				inputManager.getRelativeValue(Action::AIM_RIGHT) - inputManager.getRelativeValue(Action::AIM_LEFT),
				inputManager.getRelativeValue(Action::AIM_UP) - inputManager.getRelativeValue(Action::AIM_DOWN),
			};
			carrotCakeScale.x = std::clamp(carrotCakeScale.x + aimInput.x, 0.25f, 4.0f);
			carrotCakeScale.y = std::clamp(carrotCakeScale.y + aimInput.y, 0.25f, 4.0f);
		}

		const float scrollInput = inputManager.getRelativeValue(Action::SCROLL_UP) - inputManager.getRelativeValue(Action::SCROLL_DOWN);
		carrotCakePosition.z -= scrollInput * 0.25f * sprintInput;
	}

	void tick(const app::TickInfo& tickInfo) override {
		(void)tickInfo; // TODO: Use?
	}

	void endFrame(const app::FrameInfo& frameInfo) override {
		(void)frameInfo; // TODO: Use?
	}

	void display(const app::FrameInfo& frameInfo) override {
		renderPass.reset();
		renderPass.setBackgroundColor(Color::PURPLE * 0.25f);
		drawBackground(frameInfo);
		renderer.render(gfx::Framebuffer::getDefault(), renderPass, worldViewport, worldProjectionViewMatrix);

		renderPass.reset();
		drawWorld(frameInfo);
		renderer.render(gfx::Framebuffer::getDefault(), renderPass, worldViewport, worldProjectionViewMatrix);

		renderPass.reset();
		drawUserInterface(frameInfo);
		drawFpsCounter();
		renderer.render(gfx::Framebuffer::getDefault(), renderPass, screenViewport, screenProjectionViewMatrix);
	}

	void loadBindingsConfiguration() {
		const std::unordered_map<std::string_view, Action> actionsByIdentifier{
			{"confirm", Action::CONFIRM},
			{"cancel", Action::CANCEL},
			{"move_up", Action::MOVE_UP},
			{"move_down", Action::MOVE_DOWN},
			{"move_left", Action::MOVE_LEFT},
			{"move_right", Action::MOVE_RIGHT},
			{"aim_up", Action::AIM_UP},
			{"aim_down", Action::AIM_DOWN},
			{"aim_left", Action::AIM_LEFT},
			{"aim_right", Action::AIM_RIGHT},
			{"sprint", Action::SPRINT},
			{"attack", Action::ATTACK},
			{"scroll_up", Action::SCROLL_UP},
			{"scroll_down", Action::SCROLL_DOWN},
		};

		constexpr const char* BINDINGS_CONFIGURATION_FILEPATH = "configuration/bindings.json";
		const std::string bindingsFileContents = InputFileStream::open(BINDINGS_CONFIGURATION_FILEPATH).readAllIntoString();
		try {
			const json::Value bindingsValue = json::Value::parse(bindingsFileContents);
			if (!bindingsValue.is<json::Object>()) {
				throw std::runtime_error{"Invalid bindings type."};
			}
			for (const auto& [inputIdentifier, actions] : bindingsValue.as<json::Object>()) {
				const std::optional<app::Input> input = app::findInput(inputIdentifier);
				if (!input) {
					throw std::runtime_error{fmt::format("Invalid input identifier \"{}\".", inputIdentifier)};
				}
				std::span<const json::Value> actionValues = (actions.is<json::Array>()) ? actions.as<json::Array>() : std::span{&actions, 1};
				for (const json::Value& actionValue : actionValues) {
					if (!actionValue.is<json::String>()) {
						throw std::runtime_error{"Invalid actions type."};
					}
					const auto it = actionsByIdentifier.find(actionValue.as<json::String>());
					if (it == actionsByIdentifier.end()) {
						throw std::runtime_error{fmt::format("Invalid input identifier \"{}\".", inputIdentifier)};
					}
					inputManager.addBinding(*input, it->second);
				}
			}
		} catch (const json::Error& e) {
			throw std::runtime_error{fmt::format("{}:{}:{}: {}", BINDINGS_CONFIGURATION_FILEPATH, e.source.lineNumber, e.source.columnNumber, e.what())};
		} catch (const std::exception& e) {
			throw std::runtime_error{fmt::format("{}: {}", BINDINGS_CONFIGURATION_FILEPATH, e.what())};
		}
	}

	void initializeSoundStage() {
		try {
			soundStage.emplace();
		} catch (const audio::Error& e) {
			// Don't crash on failure, since the user might just not have a working sound card.
			// Just print an error message instead.
			fmt::print(stderr, "{}\n", e.what());
		}
	}

	void playMainMenuMusic(const char* filepath) {
		if (soundStage) {
			if (File::exists(filepath)) {
				music.emplace(filepath,
					audio::SoundOptions{
						.attenuationModel = audio::SoundAttenuationModel::NO_ATTENUATION,
						.volume = 0.1f,
						.listenerRelative = true,
						.looping = true,
					});
				musicId = soundStage->createPausedSoundInBackground(*music);
				soundStage->seekToSoundTime(musicId, 46.7f);
				soundStage->resumeSound(musicId);
			}
		}
	}

	void drawBackground(const app::FrameInfo& frameInfo) {
		constexpr glm::vec3 backgroundOffset{0.0f, -3.5f, -10.0f};
		constexpr glm::vec2 backgroundScale{18.0f, 18.0f};
		constexpr float backgroundAngle = 30.0f;
		constexpr float backgroundSpeed = 2.0f;

		glm::mat4 backgroundTransform = glm::identity<glm::mat4>();
		backgroundTransform = glm::translate(backgroundTransform, backgroundOffset);
		backgroundTransform = glm::rotate(backgroundTransform, glm::radians(backgroundAngle), {1.0f, 0.0f, 0.0f});
		backgroundTransform = glm::scale(backgroundTransform, {backgroundScale, 1.0f});
		backgroundTransform = glm::translate(backgroundTransform, {-0.5f, -0.5f, 0.0f});
		renderPass.draw(gfx::Quad{
			.texture = testTexture,
			.transformation = backgroundTransform,
			.textureOffset{0.0f, -frameInfo.elapsedTime * backgroundSpeed},
			.textureScale = 1000.0f * backgroundScale / testTexture->getSize(),
		});
	}

	void drawWorld(const app::FrameInfo& frameInfo) {
		glm::mat4 carrotCakeTransformation = glm::identity<glm::mat4>();
		carrotCakeTransformation = glm::translate(carrotCakeTransformation, glm::vec3{0.6f, -0.7f, -3.0f} + carrotCakePosition);
		carrotCakeTransformation = glm::scale(carrotCakeTransformation, glm::vec3{5.0f * carrotCakeScale.x, 5.0f * carrotCakeScale.y, 5.0f});
		carrotCakeTransformation = glm::rotate(carrotCakeTransformation, frameInfo.elapsedTime * -1.5f, glm::vec3{0.0f, 1.0f, 0.0f});
		carrotCakeTransformation = glm::rotate(carrotCakeTransformation, frameInfo.elapsedTime * 2.0f, glm::vec3{0.0f, 0.0f, 1.0f});
		carrotCakeTransformation = glm::translate(carrotCakeTransformation, {0.0f, 0.05f, 0.0f});
		renderPass.draw(gfx::Model{
			.scene = carrotCakeModel,
			.transformation = carrotCakeTransformation,
		});
	}

	void drawUserInterface(const app::FrameInfo& frameInfo) {
		renderPass.draw(gfx::Rectangle{
			.texture = testTexture,
			.position{100.0f, 100.0f},
			.size{180.0f, 70.0f},
			.angle = frameInfo.elapsedTime,
			.origin{0.5f, 0.5f},
		});

		renderPass.draw(gfx::ImageSizedRectangle{
			.texture = testTexture,
			.position{200.0f + glm::cos(frameInfo.elapsedTime) * 50.0f, 360.0f + glm::sin(frameInfo.elapsedTime) * 50.0f},
			.scale{0.2f + glm::sin(frameInfo.elapsedTime) * 0.1f, 0.2f + glm::cos(frameInfo.elapsedTime) * 0.1f},
			.origin{0.5f, 0.5f},
		});

		renderPass.draw(gfx::Sprite{
			.atlas = spriteAtlas,
			.id = testSprite,
			.position{450.0f + glm::cos(frameInfo.elapsedTime) * 50.0f, 360.0f + glm::sin(frameInfo.elapsedTime) * 50.0f},
			.scale{0.2f + glm::sin(frameInfo.elapsedTime) * 0.1f, 0.2f + glm::cos(frameInfo.elapsedTime) * 0.1f},
			.origin{0.5f, 0.5f},
		});

		renderPass.draw(gfx::Text{
			.font = mainFont,
			.text = mainFont->shapeText(renderer,
				8,
				u8"The quick brown fox\n"
				"jumps over the lazy dog\n"
				"\n"
				"FLYGANDE BÄCKASINER SÖKA\n"
				"HWILA PÅ MJUKA TUVOR QXZ\n"
				"0123456789\n"
				"\n"
				"+!\"#%&/()=?`@${[]}\\\n"
				"~\'<>|,.-;:_"),
			.position{410.0f, 64.0f},
			.color = Color::LIME,
		});

		renderPass.draw(gfx::Text{
			.font = mainFont,
			.text = mainFont->shapeText(renderer,
				8,
				fmt::format("Position:\n({:.2f}, {:.2f}, {:.2f})\n\nScale:\n({:.2f}, {:.2f})",
					carrotCakePosition.x,
					carrotCakePosition.y,
					carrotCakePosition.z,
					carrotCakeScale.x,
					carrotCakeScale.y)),
			.position{410.0f, 170.0f},
		});
	}

	void drawFpsCounter() {
		const unsigned fps = getLatestMeasuredFps();
		const gfx::Font::ShapedText fpsText = mainFont->shapeText(renderer, 16, fmt::format("FPS: {}", fps));
		const glm::vec2 fpsPosition{static_cast<float>(worldViewport.position.x) + 2.0f, static_cast<float>(worldViewport.position.y) + 20.0f};
		const Color fpsColor = (fps < 60) ? Color::RED : (fps < 120) ? Color::YELLOW : (fps < 240) ? Color::GRAY : Color::LIME;
		renderPass.draw(gfx::Text{.font = mainFont, .text = fpsText, .position = fpsPosition + glm::vec2{1.0f, 1.0f}, .color = Color::BLACK});
		renderPass.draw(gfx::Text{.font = mainFont, .text = fpsText, .position = fpsPosition, .color = fpsColor});
	}

	gfx::Renderer renderer{};
	gfx::RenderPass renderPass{};
	gfx::Viewport screenViewport{};
	gfx::Viewport worldViewport{};
	glm::mat4 screenProjectionViewMatrix{};
	glm::mat4 worldProjectionViewMatrix{};
	std::shared_ptr<gfx::Texture> testTexture = std::make_shared<gfx::Texture>(gfx::ImageLDR{"textures/test.png"});
	std::shared_ptr<gfx::Scene> carrotCakeModel = std::make_shared<gfx::Scene>("models/carrot_cake.obj");
	std::shared_ptr<gfx::SpriteAtlas> spriteAtlas = std::make_shared<gfx::SpriteAtlas>();
	gfx::SpriteAtlas::SpriteId testSprite = spriteAtlas->insert(renderer, gfx::ImageLDR{"textures/test.png"});
	std::shared_ptr<gfx::Font> mainFont;
	app::InputManager inputManager{};
	std::optional<audio::SoundStage> soundStage{};
	std::optional<audio::Sound> music{};
	audio::SoundStage::SoundInstanceId musicId{};
	glm::vec3 soundListenerPosition{0.0f, 0.0f, 0.0f};
	float verticalFieldOfView;
	glm::vec3 carrotCakePosition{0.0f, 0.0f, 0.0f};
	glm::vec2 carrotCakeScale{1.0f, 1.0f};
};

void parseOptionValue(int argc, char* argv[], int& i, std::string_view optionName, const char*& output) {
	if (++i >= argc) {
		throw std::runtime_error{fmt::format("Missing {} value.", optionName)};
	}
	output = argv[i];
}

void parseOptionValue(int argc, char* argv[], int& i, std::string_view optionName, std::integral auto& output) {
	if (++i >= argc) {
		throw std::runtime_error{fmt::format("Missing {} value.", optionName)};
	}
	const std::string_view string{argv[i]};
	if (const std::from_chars_result result = std::from_chars(string.data(), string.data() + string.size(), output); result.ec != std::errc{}) {
		throw std::runtime_error{fmt::format("Invalid {} value \"{}\": {}", optionName, string, std::make_error_code(result.ec).message())};
	}
}

void parseOptionValue(int argc, char* argv[], int& i, std::string_view optionName, float& output) {
	if (++i >= argc) {
		throw std::runtime_error{fmt::format("Missing {} value.", optionName)};
	}
	if (std::sscanf(argv[i], "%f", &output) != 1) {
		throw std::runtime_error{fmt::format("Invalid {} value \"{}\".", optionName, argv[i])};
	}
}

void parseOptionValue(int, char*[], int&, std::string_view, bool& output) {
	output = true;
}

[[nodiscard]] GameOptions parseGameOptions(int argc, char* argv[]) {
	GameOptions result{};
	for (int i = 1; i < argc; ++i) {
		const std::string_view argument{argv[i]};
		if (argument == "-help" || argument == "--help" || argument == "-?" || argument == "/?") {
			throw std::runtime_error{
				"Options:\n"
				"  -help                        Show this information.\n"
				"  -title <string>              Title of the main window.\n"
				"  -width <pixels>              Width of the main window.\n"
				"  -height <pixels>             Height of the main window.\n"
				"  -resizable                   Enable window resizing.\n"
				"  -fullscreen                  Enable fullscreen.\n"
				"  -vsync                       Enable vertical synchronization.\n"
				"  -min-fps <Hz>                Minimum framerate before slowdown.\n"
				"  -max-fps <Hz>                Framerate limit. 0 = unlimited.\n"
				"  -msaa <level>                Level of multisample anti-aliasing.\n"
				"  -main-menu-music <filepath>  Music file to use for the main menu.\n"
				"  -fov <degrees>               Field of view for world rendering."};
		}

		if (argument == "-title") {
			parseOptionValue(argc, argv, i, "title", result.applicationOptions.windowTitle);
		} else if (argument == "-width") {
			parseOptionValue(argc, argv, i, "width", result.applicationOptions.windowWidth);
		} else if (argument == "-height") {
			parseOptionValue(argc, argv, i, "height", result.applicationOptions.windowHeight);
		} else if (argument == "-resizable") {
			parseOptionValue(argc, argv, i, "resizable", result.applicationOptions.windowResizable);
		} else if (argument == "-fullscreen") {
			parseOptionValue(argc, argv, i, "fullscreen", result.applicationOptions.windowFullscreen);
		} else if (argument == "-vsync") {
			parseOptionValue(argc, argv, i, "vsync", result.applicationOptions.windowVSync);
		} else if (argument == "-min-fps") {
			parseOptionValue(argc, argv, i, "min fps", result.applicationOptions.minFps);
		} else if (argument == "-max-fps") {
			parseOptionValue(argc, argv, i, "max fps", result.applicationOptions.maxFps);
		} else if (argument == "-msaa") {
			parseOptionValue(argc, argv, i, "msaa", result.applicationOptions.msaaLevel);
		} else if (argument == "-main-menu-music") {
			parseOptionValue(argc, argv, i, "main menu music file", result.mainMenuMusicFilepath);
		} else if (argument == "-fov") {
			parseOptionValue(argc, argv, i, "fov", result.fieldOfView);
		} else {
			throw std::runtime_error{fmt::format("Unknown option {}. Try -help.", argument)};
		}
	}
	return result;
}

} // namespace

int main(int argc, char* argv[]) {
	try {
		try {
			Game game{argv[0], parseGameOptions(argc, argv)};
			game.run();
		} catch (const std::exception& e) {
			fmt::print(stderr, "{}\n", e.what());
			return EXIT_FAILURE;
		} catch (...) {
			fmt::print(stderr, "Fatal error!\n");
			return EXIT_FAILURE;
		}
	} catch (...) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
