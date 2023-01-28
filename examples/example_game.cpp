/**
 * \example example_game.cpp
 *
 * \details This example shows a basic game project consisting of a single
 *          source file. The main application class, Game, is defined at the top
 *          while the main function is defined at the bottom.
 *
 *          This can be used to study how various libdonut features are combined
 *          to form a working application. Note however that for a real project,
 *          the code that this example represents would typically be split
 *          across multiple files to make the main application file less
 *          cluttered.
 *
 *          The game uses the included `examples/data/` folder as its main
 *          resource directory for all asset files that are loaded at runtime.
 */

#include <donut/Color.hpp>
#include <donut/File.hpp>
#include <donut/InputFileStream.hpp>
#include <donut/Timer.hpp>
#include <donut/Variant.hpp>
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
#include <donut/graphics/Shader.hpp>
#include <donut/graphics/Shader3D.hpp>
#include <donut/graphics/Texture.hpp>
#include <donut/graphics/Viewport.hpp>
#include <donut/json.hpp>

#include <array>                         // std::array
#include <charconv>                      // std::from_chars_result, std::from_chars
#include <concepts>                      // std::integral
#include <cstddef>                       // std::size_t
#include <cstdio>                        // stderr, std::sscanf
#include <cstdlib>                       // EXIT_SUCCESS, EXIT_FAILURE
#include <exception>                     // std::exception
#include <fmt/format.h>                  // fmt::format, fmt::print
#include <glm/ext/matrix_clip_space.hpp> // glm::ortho, glm::perspective
#include <glm/ext/matrix_transform.hpp>  // glm::identity, glm::translate, glm::rotate, glm::scale
#include <glm/glm.hpp>                   // glm::...
#include <glm/gtx/norm.hpp>              // glm::length2
#include <optional>                      // std::optional
#include <span>                          // std::span
#include <stdexcept>                     // std::runtime_error
#include <string>                        // std::string
#include <string_view>                   // std::string_view
#include <system_error>                  // std::errc
#include <unordered_map>                 // std::unordered_map

namespace app = donut::application;
namespace audio = donut::audio;
namespace gfx = donut::graphics;
namespace json = donut::json;
using Color = donut::Color;
using File = donut::File;
using InputFileStream = donut::InputFileStream;
using Timer = donut::Timer<float>;
template <typename... Ts>
using Variant = donut::Variant<Ts...>;

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
		.windowResizable = true,
		.tickRate = 60.0f,
		.maxFps = 240.0f,
	};
	const char* mainMenuMusicFilepath = "sounds/music/donauwalzer.ogg";
	float fieldOfView = 90.0f;
	std::string messageToShowAndExit{};
};

class Game final : public app::Application {
public:
	Game(const char* programFilepath, const GameOptions& options)
		: app::Application(programFilepath, options.applicationOptions)
		, verticalFieldOfView(2.0f * glm::atan((3.0f / 4.0f) * glm::tan(glm::radians(options.fieldOfView) * 0.5f))) {
		loadBindingsConfiguration("configuration/bindings.json");
		initializeSoundStage();
		playMainMenuMusic(options.mainMenuMusicFilepath);
	}

protected:
	void resize(glm::ivec2 newWindowSize) override {
		constexpr glm::ivec2 RENDER_RESOLUTION{640, 480};
		constexpr glm::ivec2 WORLD_VIEWPORT_POSITION{15, 15};
		constexpr glm::ivec2 WORLD_VIEWPORT_SIZE{380, 450};

		const auto [viewport, scale] = gfx::Viewport::createIntegerScaled(newWindowSize, RENDER_RESOLUTION);
		screenViewport = viewport;
		screenProjectionViewMatrix = glm::ortho(0.0f, static_cast<float>(RENDER_RESOLUTION.x), 0.0f, static_cast<float>(RENDER_RESOLUTION.y));

		worldViewport = {.position = screenViewport.position + WORLD_VIEWPORT_POSITION * scale, .size = WORLD_VIEWPORT_SIZE * scale};
		const float aspectRatio = static_cast<float>(worldViewport.size.x) / static_cast<float>(worldViewport.size.y);
		worldProjectionViewMatrix = glm::perspective(verticalFieldOfView, aspectRatio, 0.1f, 100.0f);
	}

	void prepareForEvents(app::FrameInfo frameInfo) override {
		(void)frameInfo; // TODO: Use?

		inputManager.prepareForEvents();
	}

	void handleEvent(app::FrameInfo frameInfo, const app::Event& event) override {
		(void)frameInfo; // TODO: Use?

		inputManager.handleEvent(event);
	}

	void update(app::FrameInfo frameInfo) override {
		if (soundStage) {
			soundStage->update(frameInfo.deltaTime, {.position = soundListenerPosition});
		}

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

		glm::vec2 movementInput = inputManager.getAbsoluteVector(Action::MOVE_LEFT, Action::MOVE_RIGHT, Action::MOVE_DOWN, Action::MOVE_UP);
		if (const float movementInputLengthSquared = glm::length2(movementInput); movementInputLengthSquared > 1.0f) {
			movementInput /= glm::sqrt(movementInputLengthSquared);
		}
		const float carrotCakeSpeed = 2.0f * sprintInput;
		carrotCakeVelocity.x = movementInput.x * carrotCakeSpeed;
		carrotCakeVelocity.y = movementInput.y * carrotCakeSpeed;
		carrotCakeVelocity.z = 0.0f;

		if (inputManager.isPressed(Action::CONFIRM)) {
			const glm::vec2 aimInput = inputManager.getRelativeVector(Action::AIM_LEFT, Action::AIM_RIGHT, Action::AIM_DOWN, Action::AIM_UP);
			carrotCakeScale.x = glm::clamp(carrotCakeScale.x + aimInput.x, 0.25f, 4.0f);
			carrotCakeScale.y = glm::clamp(carrotCakeScale.y + aimInput.y, 0.25f, 4.0f);
		}

		const float scrollInput = inputManager.getRelativeVector(Action::SCROLL_DOWN, Action::SCROLL_UP);
		carrotCakeCurrentPosition.z -= scrollInput * 0.25f * sprintInput;

		const bool triggerInput = inputManager.isPressed(Action::CANCEL);
		counterA += timerA.countUpLoopTrigger(frameInfo.deltaTime, 1.0f, triggerInput);
		counterB += timerB.countDownLoopTrigger(frameInfo.deltaTime, 1.0f, triggerInput);
	}

	void tick(app::TickInfo tickInfo) override {
		carrotCakePreviousPosition = carrotCakeCurrentPosition;
		carrotCakeCurrentPosition += carrotCakeVelocity * tickInfo.tickInterval;
	}

	void prepareForDisplay(app::FrameInfo frameInfo) override {
		carrotCakeDisplayPosition = glm::mix(carrotCakePreviousPosition, carrotCakeCurrentPosition, frameInfo.tickInterpolationAlpha);

		const TestShader3D::PointLight baseLight = {
			.position = carrotCakeDisplayPosition,
			.ambient{0.2f, 0.2f, 0.2f},
			.diffuse{0.5f + 0.5f * glm::sin(frameInfo.elapsedTime), 0.8f, 0.8f},
			.specular{0.8f, 0.8f, 0.8f},
			.constantFalloff = 1.0f,
			.linearFalloff = 0.04f,
			.quadraticFalloff = 0.03f,
		};

		const auto baseLightWithOffset = [&](glm::vec3 offset) -> TestShader3D::PointLight {
			TestShader3D::PointLight result = baseLight;
			result.position += offset;
			return result;
		};

		const std::array<TestShader3D::PointLight, TestShader3D::POINT_LIGHT_COUNT> pointLights{{
			baseLightWithOffset({-2.0f, 0.0f, 0.0f}),
			baseLightWithOffset({0.0f, -2.0f, 0.0f}),
			baseLightWithOffset({0.0f, 2.0f, 0.0f}),
			baseLightWithOffset({0.0f, 0.0f, 2.0f}),
		}};

		const glm::vec3 viewPosition{0.0f, 0.0f, 0.0f};

		testShader3D.setPointLights(pointLights);
		testShader3D.setViewPosition(viewPosition);
	}

	void display(app::FrameInfo frameInfo) override {
		renderer.clearFramebufferColorAndDepth(framebuffer, Color::PURPLE * 0.25f);

		{
			gfx::RenderPass renderPass{};
			drawBackground(renderPass, frameInfo);
			renderer.render(framebuffer, renderPass, worldViewport, worldProjectionViewMatrix);
		}

		{
			gfx::RenderPass renderPass{};
			drawWorld(renderPass, frameInfo);
			renderer.render(framebuffer, renderPass, worldViewport, worldProjectionViewMatrix);
		}

		{
			gfx::RenderPass renderPass{};
			drawUserInterface(renderPass, frameInfo);
			drawFpsCounter(renderPass);
			renderer.render(framebuffer, renderPass, screenViewport, screenProjectionViewMatrix);
		}
	}

private:
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

	struct TestShader3D : gfx::Shader3D {
		struct PointLight {
			glm::vec3 position;
			glm::vec3 ambient;
			glm::vec3 diffuse;
			glm::vec3 specular;
			float constantFalloff;
			float linearFalloff;
			float quadraticFalloff;
		};

		struct PointLightUniform {
			PointLightUniform(const gfx::ShaderProgram& program, const char* name)
				: position(program, fmt::format("{}.position", name).c_str())
				, ambient(program, fmt::format("{}.ambient", name).c_str())
				, diffuse(program, fmt::format("{}.diffuse", name).c_str())
				, specular(program, fmt::format("{}.specular", name).c_str())
				, constantFalloff(program, fmt::format("{}.constantFalloff", name).c_str())
				, linearFalloff(program, fmt::format("{}.linearFalloff", name).c_str())
				, quadraticFalloff(program, fmt::format("{}.quadraticFalloff", name).c_str()) {}

			gfx::ShaderUniform position;
			gfx::ShaderUniform ambient;
			gfx::ShaderUniform diffuse;
			gfx::ShaderUniform specular;
			gfx::ShaderUniform constantFalloff;
			gfx::ShaderUniform linearFalloff;
			gfx::ShaderUniform quadraticFalloff;
		};

		static constexpr std::size_t POINT_LIGHT_COUNT = 4;
		static constexpr const char* FRAGMENT_SHADER_SOURCE_CODE = R"GLSL(
			struct PointLight {
				vec3 position;
				vec3 ambient;
				vec3 diffuse;
				vec3 specular;
				float constantFalloff;
				float linearFalloff;
				float quadraticFalloff;
			};

			in vec3 ioFragmentPosition;
			in vec3 ioNormal;
			in vec3 ioTangent;
			in vec3 ioBitangent;
			in vec2 ioTextureCoordinates;
			in vec4 ioTintColor;

			out vec4 outFragmentColor;

			uniform sampler2D diffuseMap;
			uniform sampler2D specularMap;
			uniform sampler2D normalMap;
			uniform float specularExponent;

			uniform PointLight pointLights[POINT_LIGHT_COUNT];
			uniform vec3 viewPosition;

			float halfLambert(float cosine) {
				float factor = 0.5 + 0.5 * cosine;
				return factor * factor;
			}

			float blinnPhong(vec3 normal, vec3 lightDirection, vec3 viewDirection) {
				vec3 halfwayDirection = normalize(lightDirection + viewDirection);
				return pow(max(dot(normal, halfwayDirection), 0.0), specularExponent);
			}

			vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDirection, vec3 ambientColor, vec3 diffuseColor, vec3 specularColor) {
				vec3 lightDifference = light.position - ioFragmentPosition;
				float lightDistanceSquared = dot(lightDifference, lightDifference);
				float lightDistance = sqrt(lightDistanceSquared);
				vec3 lightDirection = lightDifference * (1.0 / lightDistance);
				float cosine = dot(normal, lightDirection);
				float diffuseFactor = halfLambert(cosine);
				float specularFactor = blinnPhong(normal, lightDirection, viewDirection);
				float attenuation = 1.0 / (light.constantFalloff + light.linearFalloff * lightDistance + light.quadraticFalloff * lightDistanceSquared);
				vec3 ambientTerm = light.ambient * ambientColor;
				vec3 diffuseTerm = light.diffuse * diffuseFactor * diffuseColor;
				vec3 specularTerm = light.specular * specularFactor * specularColor;
				const float visibility = 1.0;
				return attenuation * (ambientTerm + (diffuseTerm + specularTerm) * visibility);
			}

			void main() {
				vec4 diffuseColor = ioTintColor * texture(diffuseMap, ioTextureCoordinates);
				vec3 specularColor = texture(specularMap, ioTextureCoordinates).rgb;
				
				mat3 TBN = mat3(normalize(ioTangent), normalize(ioBitangent), normalize(ioNormal));
				vec3 surfaceNormal = texture(normalMap, ioTextureCoordinates).xyz * 2.0 - vec3(1.0);
				vec3 normal = normalize(TBN * surfaceNormal);

				vec3 viewDirection = normalize(viewPosition - ioFragmentPosition);

				vec3 result = vec3(0.0, 0.0, 0.0);
				for (uint i = uint(0); i < uint(POINT_LIGHT_COUNT); ++i) {
					result += calculatePointLight(pointLights[i], normal, viewDirection, diffuseColor.rgb, diffuseColor.rgb, specularColor);
				}
				outFragmentColor = vec4(result, diffuseColor.a);
			}
		)GLSL";

		TestShader3D()
			: gfx::Shader3D({
				  .definitions = fmt::format("#define POINT_LIGHT_COUNT {}", POINT_LIGHT_COUNT).c_str(),
				  .vertexShaderSourceCode = gfx::Shader3D::vertexShaderSourceCodeInstancedModel,
				  .fragmentShaderSourceCode = FRAGMENT_SHADER_SOURCE_CODE,
			  }) {}

		void setPointLights(std::span<const PointLight, POINT_LIGHT_COUNT> values) {
			for (std::size_t i = 0; i < POINT_LIGHT_COUNT; ++i) {
				program.setUniformVec3(pointLights[i].position, values[i].position);
				program.setUniformVec3(pointLights[i].ambient, values[i].ambient);
				program.setUniformVec3(pointLights[i].diffuse, values[i].diffuse);
				program.setUniformVec3(pointLights[i].specular, values[i].specular);
				program.setUniformFloat(pointLights[i].constantFalloff, values[i].constantFalloff);
				program.setUniformFloat(pointLights[i].linearFalloff, values[i].linearFalloff);
				program.setUniformFloat(pointLights[i].quadraticFalloff, values[i].quadraticFalloff);
			}
		}

		void setViewPosition(glm::vec3 position) {
			program.setUniformVec3(viewPosition, position);
		}

	private:
		gfx::ShaderArray<PointLightUniform, POINT_LIGHT_COUNT> pointLights{program, "pointLights"};
		gfx::ShaderUniform viewPosition{program, "viewPosition"};
	};

	void loadBindingsConfiguration(const char* filepath) {
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

		const std::string bindingsFileContents = InputFileStream::open(filepath).readAllIntoString();
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
				const std::span<const json::Value> actionValues = (actions.is<json::Array>()) ? actions.as<json::Array>() : std::span{&actions, 1};
				for (const json::Value& actionValue : actionValues) {
					if (!actionValue.is<json::String>()) {
						throw std::runtime_error{"Invalid actions type."};
					}
					const json::String& actionIdentifier = actionValue.as<json::String>();
					const auto it = actionsByIdentifier.find(actionIdentifier);
					if (it == actionsByIdentifier.end()) {
						throw std::runtime_error{fmt::format("Invalid action identifier \"{}\".", actionIdentifier)};
					}
					inputManager.addBinding(*input, it->second);
				}
			}
		} catch (const json::Error& e) {
			throw std::runtime_error{fmt::format("{}:{}:{}: {}", filepath, e.source.lineNumber, e.source.columnNumber, e.what())};
		} catch (const std::exception& e) {
			throw std::runtime_error{fmt::format("{}: {}", filepath, e.what())};
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

	void drawBackground(gfx::RenderPass& renderPass, const app::FrameInfo& frameInfo) {
		constexpr glm::vec3 BACKGROUND_OFFSET{0.0f, 3.5f, -10.0f};
		constexpr glm::vec2 BACKGROUND_SCALE{18.0f, 18.0f};
		constexpr float BACKGROUND_ANGLE = -30.0f;
		constexpr float BACKGROUND_SPEED = 2.0f;

		glm::mat4 backgroundTransform = glm::identity<glm::mat4>();
		backgroundTransform = glm::translate(backgroundTransform, BACKGROUND_OFFSET);
		backgroundTransform = glm::rotate(backgroundTransform, glm::radians(BACKGROUND_ANGLE), {1.0f, 0.0f, 0.0f});
		backgroundTransform = glm::scale(backgroundTransform, {BACKGROUND_SCALE, 1.0f});
		backgroundTransform = glm::translate(backgroundTransform, {-0.5f, -0.5f, 0.0f});
		renderPass.draw(gfx::QuadInstance{
			.texture = &testTexture,
			.transformation = backgroundTransform,
			.textureOffset{0.0f, frameInfo.elapsedTime * BACKGROUND_SPEED},
			.textureScale = 1000.0f * BACKGROUND_SCALE / testTexture.getSize2D(),
		});
	}

	void drawWorld(gfx::RenderPass& renderPass, const app::FrameInfo& frameInfo) {
		glm::mat4 carrotCakeTransformation = glm::identity<glm::mat4>();
		carrotCakeTransformation = glm::translate(carrotCakeTransformation, glm::vec3{0.6f, 0.7f, -3.0f} + carrotCakeDisplayPosition);
		carrotCakeTransformation = glm::scale(carrotCakeTransformation, {5.0f * carrotCakeScale.x, 5.0f * carrotCakeScale.y, 5.0f});
		carrotCakeTransformation = glm::rotate(carrotCakeTransformation, frameInfo.elapsedTime * 1.5f, {0.0f, 1.0f, 0.0f});
		carrotCakeTransformation = glm::rotate(carrotCakeTransformation, frameInfo.elapsedTime * 2.0f, {0.0f, 0.0f, 1.0f});
		carrotCakeTransformation = glm::translate(carrotCakeTransformation, {0.0f, -0.05f, 0.0f});
		renderPass.draw(gfx::ModelInstance{
			.model = &carrotCakeModel,
			.transformation = carrotCakeTransformation,
		});

		glm::mat4 shadedCarrotCakeTransformation = glm::identity<glm::mat4>();
		shadedCarrotCakeTransformation = glm::translate(shadedCarrotCakeTransformation, {-0.6f, 0.2f, -3.0f});
		shadedCarrotCakeTransformation = glm::scale(shadedCarrotCakeTransformation, {5.0f, 5.0f, 5.0f});
		shadedCarrotCakeTransformation = glm::rotate(shadedCarrotCakeTransformation, frameInfo.elapsedTime * 1.5f, {0.0f, 1.0f, 0.0f});
		shadedCarrotCakeTransformation = glm::rotate(shadedCarrotCakeTransformation, frameInfo.elapsedTime * 2.0f, {0.0f, 0.0f, 1.0f});
		shadedCarrotCakeTransformation = glm::translate(shadedCarrotCakeTransformation, {0.0f, -0.05f, 0.0f});
		renderPass.draw(gfx::ModelInstance{
			.shader = &testShader3D,
			.model = &carrotCakeModel,
			.transformation = shadedCarrotCakeTransformation,
		});
	}

	void drawUserInterface(gfx::RenderPass& renderPass, const app::FrameInfo& frameInfo) {
		renderPass.draw(gfx::RectangleInstance{
			.texture = &testTexture,
			.position{100.0f, 380.0f},
			.size{180.0f, 70.0f},
			.angle = frameInfo.elapsedTime,
			.origin{0.5f, 0.5f},
		});

		renderPass.draw(gfx::TextureInstance{
			.texture = &testTexture,
			.position{200.0f + glm::cos(frameInfo.elapsedTime) * 50.0f, 120.0f + glm::sin(frameInfo.elapsedTime) * 50.0f},
			.scale{0.2f + glm::sin(frameInfo.elapsedTime) * 0.1f, 0.2f + glm::cos(frameInfo.elapsedTime) * 0.1f},
			.origin{0.5f, 0.5f},
		});

		renderPass.draw(gfx::SpriteInstance{
			.atlas = &spriteAtlas,
			.id = testSprite,
			.position{450.0f + glm::cos(frameInfo.elapsedTime) * 50.0f, 120.0f + glm::sin(frameInfo.elapsedTime) * 50.0f},
			.scale{0.2f + glm::sin(frameInfo.elapsedTime) * 0.1f, 0.2f + glm::cos(frameInfo.elapsedTime) * 0.1f},
			.origin{0.5f, 0.5f},
		});

		renderPass.draw(gfx::TextInstance{
			.font = &mainFont,
			.text = mainFont.shapeText(renderer,
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
			.position{410.0f, 416.0f},
			.color = Color::LIME,
		});

		renderPass.draw(gfx::TextInstance{
			.font = &mainFont,
			.text = mainFont.shapeText(renderer,
				8,
				fmt::format("Position:\n({:.2f}, {:.2f}, {:.2f})\n\nScale:\n({:.2f}, {:.2f})",
					carrotCakeDisplayPosition.x,
					carrotCakeDisplayPosition.y,
					carrotCakeDisplayPosition.z,
					carrotCakeScale.x,
					carrotCakeScale.y)),
			.position{410.0f, 310.0f},
		});

		if (inputManager.isPressed(Action::MOVE_UP) || inputManager.justPressed(Action::MOVE_UP)) {
			renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = mainFont.shapeText(renderer, 8, "^"), .position{590.0f, 320.0f}});
		}
		if (inputManager.isPressed(Action::MOVE_DOWN) || inputManager.justPressed(Action::MOVE_DOWN)) {
			renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = mainFont.shapeText(renderer, 8, "v"), .position{590.0f, 300.0f}});
		}
		if (inputManager.isPressed(Action::MOVE_LEFT) || inputManager.justPressed(Action::MOVE_LEFT)) {
			renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = mainFont.shapeText(renderer, 8, "<"), .position{580.0f, 310.0f}});
		}
		if (inputManager.isPressed(Action::MOVE_RIGHT) || inputManager.justPressed(Action::MOVE_RIGHT)) {
			renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = mainFont.shapeText(renderer, 8, ">"), .position{600.0f, 310.0f}});
		}

		if (inputManager.isPressed(Action::AIM_UP) || inputManager.justPressed(Action::AIM_UP)) {
			renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = mainFont.shapeText(renderer, 8, "^"), .position{590.0f, 280.0f}});
		}
		if (inputManager.isPressed(Action::AIM_DOWN) || inputManager.justPressed(Action::AIM_DOWN)) {
			renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = mainFont.shapeText(renderer, 8, "v"), .position{590.0f, 260.0f}});
		}
		if (inputManager.isPressed(Action::AIM_LEFT) || inputManager.justPressed(Action::AIM_LEFT)) {
			renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = mainFont.shapeText(renderer, 8, "<"), .position{580.0f, 270.0f}});
		}
		if (inputManager.isPressed(Action::AIM_RIGHT) || inputManager.justPressed(Action::AIM_RIGHT)) {
			renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = mainFont.shapeText(renderer, 8, ">"), .position{600.0f, 270.0f}});
		}

		renderPass.draw(gfx::TextInstance{
			.font = &mainFont,
			.text = mainFont.shapeText(
				renderer, 8, fmt::format("Timer   A: {:.2f}\nCounter A: {}\n\nTimer   B: {:.2f}\nCounter B: {}", timerA.getTime(), counterA, timerB.getTime(), counterB)),
			.position{410.0f, 240.0f},
		});
	}

	void drawFpsCounter(gfx::RenderPass& renderPass) {
		const unsigned fps = getLatestMeasuredFps();
		const gfx::Font::ShapedText fpsText = mainFont.shapeText(renderer, 16, fmt::format("FPS: {}", fps));
		const glm::vec2 fpsPosition{15.0f + 2.0f, 480.0f - 15.0f - 20.0f};
		const Color fpsColor = (fps < 60) ? Color::RED : (fps < 120) ? Color::YELLOW : (fps < 240) ? Color::GRAY : Color::LIME;
		renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = fpsText, .position = fpsPosition + glm::vec2{1.0f, -1.0f}, .color = Color::BLACK});
		renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = fpsText, .position = fpsPosition, .color = fpsColor});
	}

	gfx::Framebuffer framebuffer = gfx::Framebuffer::getDefault();
	gfx::Renderer renderer{};
	gfx::Viewport screenViewport{};
	gfx::Viewport worldViewport{};
	glm::mat4 screenProjectionViewMatrix{};
	glm::mat4 worldProjectionViewMatrix{};
	gfx::Texture testTexture{gfx::ImageLDR{"textures/test.png", {.flipVertically = true}}};
	gfx::Model carrotCakeModel{"models/carrot_cake.obj"};
	gfx::SpriteAtlas spriteAtlas{};
	gfx::SpriteAtlas::SpriteId testSprite = spriteAtlas.insert(renderer, gfx::ImageLDR{"textures/test.png", {.flipVertically = true}});
	gfx::Font mainFont{"fonts/unscii/unscii-8.ttf"};
	TestShader3D testShader3D{};
	app::InputManager inputManager{};
	std::optional<audio::SoundStage> soundStage{};
	std::optional<audio::Sound> music{};
	audio::SoundStage::SoundInstanceId musicId{};
	glm::vec3 soundListenerPosition{0.0f, 0.0f, 0.0f};
	float verticalFieldOfView;
	glm::vec3 carrotCakeCurrentPosition{0.0f, 0.0f, 0.0f};
	glm::vec3 carrotCakePreviousPosition{0.0f, 0.0f, 0.0f};
	glm::vec3 carrotCakeDisplayPosition{0.0f, 0.0f, 0.0f};
	glm::vec2 carrotCakeScale{1.0f, 1.0f};
	glm::vec3 carrotCakeVelocity{0.0f, 0.0f, 0.0f};
	Timer timerA{};
	Timer timerB{};
	unsigned counterA = 0u;
	unsigned counterB = 0u;
};

class OptionsParser {
public:
	OptionsParser(int argc, char* argv[]) noexcept
		: argumentCount(argc)
		, arguments(argv) {
		assert(argc > 0);
	}

	[[nodiscard]] Variant<GameOptions, std::string> parseGameOptions() {
		GameOptions options{};
		while (argumentIndex < argumentCount) {
			const std::string_view argument{arguments[argumentIndex]};
			if (argument == "-help" || argument == "--help" || argument == "-?" || argument == "/?") {
				return "Options:\n"
					   "  -help                        Show this information.\n"
					   "  -title <string>              Title of the main window.\n"
					   "  -width <pixels>              Width of the main window.\n"
					   "  -height <pixels>             Height of the main window.\n"
					   "  -resizable                   Enable window resizing.\n"
					   "  -fullscreen                  Enable fullscreen.\n"
					   "  -vsync                       Enable vertical synchronization.\n"
					   "  -min-fps <Hz>                Minimum frame rate before slowdown.\n"
					   "  -max-fps <Hz>                Frame rate limit. 0 = unlimited.\n"
					   "  -msaa <level>                Level of multisample anti-aliasing.\n"
					   "  -main-menu-music <filepath>  Music file to use for the main menu.\n"
					   "  -fov <degrees>               Field of view for world rendering.";
			}

			if (argument == "-title") {
				parseOptionValue("title", options.applicationOptions.windowTitle);
			} else if (argument == "-width") {
				parseOptionValue("width", options.applicationOptions.windowWidth);
			} else if (argument == "-height") {
				parseOptionValue("height", options.applicationOptions.windowHeight);
			} else if (argument == "-resizable") {
				parseOptionValue("resizable", options.applicationOptions.windowResizable);
			} else if (argument == "-fullscreen") {
				parseOptionValue("fullscreen", options.applicationOptions.windowFullscreen);
			} else if (argument == "-vsync") {
				parseOptionValue("vsync", options.applicationOptions.windowVSync);
			} else if (argument == "-min-fps") {
				parseOptionValue("min fps", options.applicationOptions.minFps);
			} else if (argument == "-max-fps") {
				parseOptionValue("max fps", options.applicationOptions.maxFps);
			} else if (argument == "-msaa") {
				parseOptionValue("msaa", options.applicationOptions.msaaLevel);
			} else if (argument == "-main-menu-music") {
				parseOptionValue("main menu music file", options.mainMenuMusicFilepath);
			} else if (argument == "-fov") {
				parseOptionValue("fov", options.fieldOfView);
			} else {
				throw std::runtime_error{fmt::format("Unknown option {}. Try -help.", argument)};
			}
			++argumentIndex;
		}
		return options;
	}

private:
	void parseOptionValue(std::string_view optionName, const char*& output) {
		if (++argumentIndex >= argumentCount) {
			throw std::runtime_error{fmt::format("Missing {} value.", optionName)};
		}
		output = arguments[argumentIndex];
	}

	void parseOptionValue(std::string_view optionName, std::integral auto& output) {
		if (++argumentIndex >= argumentCount) {
			throw std::runtime_error{fmt::format("Missing {} value.", optionName)};
		}
		const std::string_view string{arguments[argumentIndex]};
		if (const std::from_chars_result result = std::from_chars(string.data(), string.data() + string.size(), output); result.ec != std::errc{}) {
			throw std::runtime_error{fmt::format("Invalid {} value \"{}\": {}", optionName, string, std::make_error_code(result.ec).message())};
		}
	}

	void parseOptionValue(std::string_view optionName, float& output) {
		if (++argumentIndex >= argumentCount) {
			throw std::runtime_error{fmt::format("Missing {} value.", optionName)};
		}
		if (std::sscanf(arguments[argumentIndex], "%f", &output) != 1) {
			throw std::runtime_error{fmt::format("Invalid {} value \"{}\".", optionName, arguments[argumentIndex])};
		}
	}

	void parseOptionValue(std::string_view /*optionName*/, bool& output) {
		output = true;
	}

	const int argumentCount;
	char** const arguments;
	int argumentIndex = 1;
};

} // namespace

int main(int argc, char* argv[]) {
	try {
		try {
			match(OptionsParser{argc, argv}.parseGameOptions())(
				[&](const GameOptions& options) -> void {
					Game game{argv[0], options};
					game.run();
				},
				[&](const std::string& string) -> void { fmt::print(stderr, "{}\n", string); });
		} catch (const std::exception& e) {
			fmt::print(stderr, "{}\n", e.what());
			Game::showSimpleMessageBox(Game::MessageBoxType::ERROR_MESSAGE, "Error", e.what());
			return EXIT_FAILURE;
		}
	} catch (...) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
