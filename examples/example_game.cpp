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

#include <donut/aliases.hpp>
#include <donut/donut.hpp>

#include <array>                        // std::array
#include <charconv>                     // std::from_chars_result, std::from_chars
#include <cmath>                        // std::atan2
#include <concepts>                     // std::integral
#include <cstddef>                      // std::size_t
#include <cstdio>                       // stderr, std::sscanf
#include <cstdlib>                      // EXIT_SUCCESS, EXIT_FAILURE
#include <exception>                    // std::exception
#include <fmt/format.h>                 // fmt::format, fmt::print
#include <forward_list>                 // std::forward_list
#include <glm/ext/matrix_transform.hpp> // glm::identity
#include <glm/glm.hpp>                  // glm::...
#include <glm/gtx/euler_angles.hpp>     // glm::orientate4
#include <glm/gtx/norm.hpp>             // glm::length2
#include <glm/gtx/transform.hpp>        // glm::translate, glm::scale
#include <optional>                     // std::optional
#include <span>                         // std::span
#include <stdexcept>                    // std::runtime_error
#include <string>                       // std::string
#include <string_view>                  // std::string_view
#include <system_error>                 // std::errc
#include <unordered_map>                // std::unordered_map

namespace {

struct GameOptions {
	app::ApplicationOptions applicationOptions{
		.organizationName = "Donut",
		.applicationName = "ExampleGame",
		.dataDirectoryFilepath = "data",
		.archiveFilenameExtension = "pak",
		.tickRate = 60.0f,
		.maxFps = 240.0f,
	};
	gfx::WindowOptions windowOptions{
		.title = "Example Game",
		.size{640, 480},
		.resizable = true,
	};
	const char* mainMenuMusicFilepath = "sounds/music/donauwalzer.ogg";
	float fieldOfView = 90.0f;
	std::string messageToShowAndExit{};
};

class Game final : public app::Application {
public:
	Game(const char* programFilepath, const GameOptions& options)
		: app::Application(programFilepath, options.applicationOptions)
		, window(options.windowOptions)
		, verticalFieldOfView(2.0f * glm::atan((3.0f / 4.0f) * glm::tan(glm::radians(options.fieldOfView) * 0.5f))) {
		constexpr Circle<float> CIRCLE_A{.center{60.0f, 80.0f}, .radius = 20.0f};
		constexpr Circle<float> CIRCLE_B{.center{50.0f, 90.0f}, .radius = 20.0f};
		constexpr Circle<float> CIRCLE_C{.center{60.0f, 120.0f}, .radius = 20.0f};
		constexpr Circle<float> CIRCLE_D{.center{300.0f, 100.0f}, .radius = 10.0f};
		constexpr Circle<float> CIRCLE_E{.center{200.0f, 180.0f}, .radius = 30.0f};
		constexpr Circle<float> CIRCLE_F{.center{140.0f, 440.0f}, .radius = 20.0f};
		quadtree[getAabbOf(CIRCLE_A)].push_front(CIRCLE_A);
		quadtree[getAabbOf(CIRCLE_B)].push_front(CIRCLE_B);
		quadtree[getAabbOf(CIRCLE_C)].push_front(CIRCLE_C);
		quadtree[getAabbOf(CIRCLE_D)].push_front(CIRCLE_D);
		quadtree[getAabbOf(CIRCLE_E)].push_front(CIRCLE_E);
		quadtree[getAabbOf(CIRCLE_F)].push_front(CIRCLE_F);

		loadBindingsConfiguration("configuration/bindings.json");
		initializeSoundStage();
		playMainMenuMusic(options.mainMenuMusicFilepath);

		resize(window.getDrawableSize());
	}

protected:
	void prepareForEvents(app::FrameInfo /*frameInfo*/) override {
		inputManager.prepareForEvents();
	}

	void handleEvent(app::FrameInfo /*frameInfo*/, const app::Event& event) override {
		if (event.is<app::WindowSizeChangedEvent>()) {
			resize(window.getDrawableSize());
		}
		inputManager.handleEvent(event);
	}

	void update(app::FrameInfo frameInfo) override {
		if (soundStage) {
			soundStage->update(frameInfo.deltaTime, listener);
		}

		if (inputManager.justPressed(app::Input::KEY_F10)) {
			quit();
		}

		if (inputManager.justPressed(app::Input::KEY_F11) ||
			(inputManager.justPressed(app::Input::KEY_RETURN) && (inputManager.isPressed(app::Input::KEY_LALT) || inputManager.isPressed(app::Input::KEY_RALT)))) {
			window.setFullscreen(!window.isFullscreen());
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

	void display(app::FrameInfo frameInfo) override {
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

		gfx::Framebuffer& framebuffer = window.getFramebuffer();

		renderer.clearFramebufferColorAndDepth(framebuffer, Color::PURPLE * 0.25f);

		{
			gfx::RenderPass renderPass{};
			drawBackground(renderPass, frameInfo);
			renderer.render(framebuffer, renderPass, worldViewport, worldCamera);
		}

		{
			gfx::RenderPass renderPass{};
			drawWorld(renderPass, frameInfo);
			renderer.render(framebuffer, renderPass, worldViewport, worldCamera);
		}

		{
			gfx::RenderPass renderPass{};
			drawUserInterface(renderPass, frameInfo);
			drawFpsCounter(renderPass);
			renderer.render(framebuffer, renderPass, screenViewport, screenCamera);
		}

		window.present();
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

		struct PointLightParameters {
			PointLightParameters(const gfx::ShaderProgram& program, const char* name)
				: position(program, fmt::format("{}.position", name).c_str())
				, ambient(program, fmt::format("{}.ambient", name).c_str())
				, diffuse(program, fmt::format("{}.diffuse", name).c_str())
				, specular(program, fmt::format("{}.specular", name).c_str())
				, constantFalloff(program, fmt::format("{}.constantFalloff", name).c_str())
				, linearFalloff(program, fmt::format("{}.linearFalloff", name).c_str())
				, quadraticFalloff(program, fmt::format("{}.quadraticFalloff", name).c_str()) {}

			gfx::ShaderParameter position;
			gfx::ShaderParameter ambient;
			gfx::ShaderParameter diffuse;
			gfx::ShaderParameter specular;
			gfx::ShaderParameter constantFalloff;
			gfx::ShaderParameter linearFalloff;
			gfx::ShaderParameter quadraticFalloff;
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

			in vec3 fragmentPosition;
			in vec3 fragmentNormal;
			in vec3 fragmentTangent;
			in vec3 fragmentBitangent;
			in vec2 fragmentTextureCoordinates;
			in vec4 fragmentTintColor;

			out vec4 outputColor;

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
				vec3 lightDifference = light.position - fragmentPosition;
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
				vec4 diffuseColor = fragmentTintColor * texture(diffuseMap, fragmentTextureCoordinates);
				vec3 specularColor = texture(specularMap, fragmentTextureCoordinates).rgb;
				
				mat3 TBN = mat3(normalize(fragmentTangent), normalize(fragmentBitangent), normalize(fragmentNormal));
				vec3 surfaceNormal = texture(normalMap, fragmentTextureCoordinates).xyz * 2.0 - vec3(1.0);
				vec3 normal = normalize(TBN * surfaceNormal);

				vec3 viewDirection = normalize(viewPosition - fragmentPosition);

				vec3 color = vec3(0.0, 0.0, 0.0);
				for (uint i = uint(0); i < uint(POINT_LIGHT_COUNT); ++i) {
					color += calculatePointLight(pointLights[i], normal, viewDirection, diffuseColor.rgb, diffuseColor.rgb, specularColor);
				}
				outputColor = vec4(color, diffuseColor.a);
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
		gfx::ShaderArray<PointLightParameters, POINT_LIGHT_COUNT> pointLights{program, "pointLights"};
		gfx::ShaderParameter viewPosition{program, "viewPosition"};
	};

	void resize(glm::ivec2 newWindowSize) {
		constexpr glm::ivec2 RENDER_RESOLUTION{640, 480};
		constexpr glm::ivec2 WORLD_VIEWPORT_POSITION{15, 15};
		constexpr glm::ivec2 WORLD_VIEWPORT_SIZE{380, 450};

		const auto [viewport, scale] = gfx::Viewport::createIntegerScaled(newWindowSize, RENDER_RESOLUTION);
		screenViewport = viewport;
		screenCamera = gfx::Camera::createOrthographic({
			.offset{0.0f, 0.0f},
			.size{static_cast<float>(RENDER_RESOLUTION.x), static_cast<float>(RENDER_RESOLUTION.y)},
		});

		worldViewport = {
			.position = screenViewport.position + WORLD_VIEWPORT_POSITION * scale,
			.size = WORLD_VIEWPORT_SIZE * scale,
		};
		worldCamera = gfx::Camera::createPerspective({
			.verticalFieldOfView = verticalFieldOfView,
			.aspectRatio = static_cast<float>(worldViewport.size.x) / static_cast<float>(worldViewport.size.y),
			.nearZ = 0.1f,
			.farZ = 100.0f,
		});
	}

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

		renderPass.draw(gfx::QuadInstance{
			.texture = &testTexture,
			.transformation = glm::translate(BACKGROUND_OFFSET) *                                      //
		                      glm::orientate4(glm::vec3{glm::radians(BACKGROUND_ANGLE), 0.0f, 0.0f}) * //
		                      glm::scale(glm::vec3{BACKGROUND_SCALE, 1.0f}) *                          //
		                      glm::translate(glm::vec3{-0.5f, -0.5f, 0.0f}),
			.textureOffset{0.0f, frameInfo.elapsedTime * BACKGROUND_SPEED},
			.textureScale = 1000.0f * BACKGROUND_SCALE / testTexture.getSize2D(),
		});
	}

	void drawWorld(gfx::RenderPass& renderPass, const app::FrameInfo& frameInfo) {
		renderPass.draw(gfx::ModelInstance{
			.model = &carrotCakeModel,
			.transformation = glm::translate(glm::vec3{0.6f, 0.7f, -3.0f} + carrotCakeDisplayPosition) *                     //
		                      glm::scale(glm::vec3{5.0f * carrotCakeScale.x, 5.0f * carrotCakeScale.y, 5.0f}) *              //
		                      glm::orientate4(glm::vec3{0.0f, frameInfo.elapsedTime * 1.5f, frameInfo.elapsedTime * 2.0f}) * //
		                      glm::translate(glm::vec3{0.0f, -0.05f, 0.0f}),
		});

		renderPass.draw(gfx::ModelInstance{
			.shader = &testShader3D,
			.model = &carrotCakeModel,
			.transformation = glm::translate(glm::vec3{-0.6f, 0.2f, -3.0f}) *                                                //
		                      glm::scale(glm::vec3{5.0f, 5.0f, 5.0f}) *                                                      //
		                      glm::orientate4(glm::vec3{0.0f, frameInfo.elapsedTime * 1.5f, frameInfo.elapsedTime * 2.0f}) * //
		                      glm::translate(glm::vec3{0.0f, -0.05f, 0.0f}),
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

		renderPass.draw(gfx::SpriteInstance{
			.atlas = &spriteAtlas,
			.id = testSubSprite,
			.position{450.0f + glm::cos(frameInfo.elapsedTime) * 50.0f, 320.0f + glm::sin(frameInfo.elapsedTime) * 50.0f},
			.scale{0.2f + glm::sin(frameInfo.elapsedTime) * 0.1f, 0.2f + glm::cos(frameInfo.elapsedTime) * 0.1f},
			.origin{0.5f, 0.5f},
		});

		renderPass.draw(gfx::TextInstance{
			.font = &mainFont,
			.text = mainFont.shapeText(renderer, 8,
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
			.text = mainFont.shapeText(renderer, 8,
				fmt::format("Position:\n({:.2f}, {:.2f}, {:.2f})\n\nScale:\n({:.2f}, {:.2f})", carrotCakeDisplayPosition.x, carrotCakeDisplayPosition.y,
					carrotCakeDisplayPosition.z, carrotCakeScale.x, carrotCakeScale.y)),
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
			.text = mainFont.shapeText(renderer, 8,
				fmt::format("Timer   A: {:.2f}\nCounter A: {}\n\nTimer   B: {:.2f}\nCounter B: {}", timerA.getTime(), counterA, timerB.getTime(), counterB)),
			.position{410.0f, 240.0f},
		});

		if (inputManager.isPressed(app::Input::KEY_SPACE)) {
			constexpr Capsule<2, float> STATIC_CAPSULE{.centerLine{.pointA{80.0f, 80.0f}, .pointB{300.0f, 200.0f}}, .radius = 50.0f};
			constexpr glm::vec2 STATIC_CAPSULE_VECTOR = STATIC_CAPSULE.centerLine.pointB - STATIC_CAPSULE.centerLine.pointA;
			const Circle<float> movingCircle{.center = glm::vec2{200.0f, 50.0f} + glm::vec2{carrotCakeDisplayPosition} * 50.0f, .radius = 32.0f};
			const Color movingCircleColor = (intersects(movingCircle, STATIC_CAPSULE)) ? Color::RED : Color::YELLOW;

			renderPass.draw(gfx::RectangleInstance{
				.texture = &circleTexture,
				.position = STATIC_CAPSULE.centerLine.pointA,
				.size{STATIC_CAPSULE.radius * 2.0f, STATIC_CAPSULE.radius * 2.0f},
				.origin{0.5f, 0.5f},
				.tintColor = Color::GREEN,
			});
			renderPass.draw(gfx::RectangleInstance{
				.texture = &circleTexture,
				.position = STATIC_CAPSULE.centerLine.pointB,
				.size{STATIC_CAPSULE.radius * 2.0f, STATIC_CAPSULE.radius * 2.0f},
				.origin{0.5f, 0.5f},
				.tintColor = Color::GREEN,
			});
			renderPass.draw(gfx::RectangleInstance{
				.position = STATIC_CAPSULE.centerLine.pointA,
				.size{glm::length(STATIC_CAPSULE_VECTOR), STATIC_CAPSULE.radius * 2.0f},
				.angle = std::atan2(STATIC_CAPSULE_VECTOR.y, STATIC_CAPSULE_VECTOR.x),
				.origin{0.0f, 0.5f},
				.tintColor = Color::GREEN,
			});

			renderPass.draw(gfx::RectangleInstance{
				.texture = &circleTexture,
				.position = movingCircle.center,
				.size{movingCircle.radius * 2.0f, movingCircle.radius * 2.0f},
				.origin{0.5f, 0.5f},
				.tintColor = movingCircleColor,
			});

			const auto drawBorder = [&](const Box<2, float>& box, float lineThickness, Color color) -> void {
				const glm::vec2 extent = box.max - box.min;
				renderPass.draw(gfx::RectangleInstance{.position = box.min, .size{extent.x, lineThickness}, .origin{0.0f, 0.0f}, .tintColor = color});
				renderPass.draw(gfx::RectangleInstance{.position{box.min.x, box.max.y}, .size{extent.x, lineThickness}, .origin{0.0f, 1.0f}, .tintColor = color});
				renderPass.draw(gfx::RectangleInstance{.position = box.min, .size{lineThickness, extent.y}, .origin{0.0f, 0.0f}, .tintColor = color});
				renderPass.draw(gfx::RectangleInstance{.position{box.max.x, box.min.y}, .size{lineThickness, extent.y}, .origin{1.0f, 0.0f}, .tintColor = color});
			};

			quadtree.traverseActiveNodes([&](const Box<2, float>& looseBounds, const std::forward_list<Circle<float>>* circles) -> void {
				drawBorder(looseBounds, 2.0f, Color::BLANCHED_ALMOND);
				if (circles) {
					for (const Circle<float>& circle : *circles) {
						renderPass.draw(gfx::RectangleInstance{
							.texture = &circleTexture,
							.position = circle.center,
							.size{circle.radius * 2.0f, circle.radius * 2.0f},
							.origin{0.5f, 0.5f},
							.tintColor = Color::BLUE,
						});
					}
				}
			});
			const Box<2, float> movingCircleAabb = getAabbOf(movingCircle);
			std::size_t aabbTestCount = 0;
			std::size_t circleTestCount = 0;
			quadtree.traverseActiveNodes(
				[&](const Box<2, float>& looseBounds, const std::forward_list<Circle<float>>* circles) -> void {
					drawBorder(looseBounds, 2.0f, Color::DARK_BLUE);
					if (circles) {
						for (const Circle<float>& circle : *circles) {
							++circleTestCount;
							if (intersects(circle, movingCircle)) {
								renderPass.draw(gfx::RectangleInstance{
									.texture = &circleTexture,
									.position = circle.center,
									.size{circle.radius * 2.0f, circle.radius * 2.0f},
									.origin{0.5f, 0.5f},
									.tintColor = Color::DARK_GOLDEN_ROD,
								});
							}
						}
					}
				},
				[&](const Box<2, float>& looseBounds) -> bool {
					++aabbTestCount;
					return intersects(movingCircleAabb, looseBounds);
				});

			renderPass.draw(gfx::TextInstance{
				.font = &mainFont,
				.text = mainFont.shapeText(renderer, 8, fmt::format("AABB tests: {}\nCircle tests: {}", aabbTestCount, circleTestCount)),
				.position{410.0f, 450.0f},
				.color = Color::BURLY_WOOD,
			});
		}

		if (inputManager.justReleased(app::Input::KEY_SPACE)) {
			inputManager.resetAllInputs();
		}
	}

	void drawFpsCounter(gfx::RenderPass& renderPass) {
		const unsigned fps = getLatestMeasuredFps();
		const gfx::Font::ShapedText fpsText = mainFont.shapeText(renderer, 16, fmt::format("FPS: {}", fps));
		const glm::vec2 fpsPosition{15.0f + 2.0f, 480.0f - 15.0f - 20.0f};
		const Color fpsColor = (fps < 60) ? Color::RED : (fps < 120) ? Color::YELLOW : (fps < 240) ? Color::GRAY : Color::LIME;
		renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = fpsText, .position = fpsPosition + glm::vec2{1.0f, -1.0f}, .color = Color::BLACK});
		renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = fpsText, .position = fpsPosition, .color = fpsColor});
	}

	gfx::Window window;
	gfx::Renderer renderer{};
	gfx::Viewport screenViewport{};
	gfx::Viewport worldViewport{};
	gfx::Camera screenCamera{};
	gfx::Camera worldCamera{};
	audio::Listener listener{};
	gfx::Texture testTexture{gfx::Image{"textures/test.png"}};
	gfx::Texture circleTexture{gfx::Image{"textures/circle.png"}, {.useLinearFiltering = false, .useMipmap = false}};
	gfx::Model carrotCakeModel{"models/carrot_cake.obj"};
	gfx::SpriteAtlas spriteAtlas{};
	gfx::SpriteAtlas::SpriteId testSprite = spriteAtlas.insert(renderer, gfx::Image{"textures/test.png"});
	gfx::SpriteAtlas::SpriteId testSubSprite = spriteAtlas.createSubSprite(testSprite, 200, 200, 100, 100, gfx::SpriteAtlas::FLIP_HORIZONTALLY);
	gfx::Font mainFont{"fonts/unscii/unscii-8.ttf"};
	TestShader3D testShader3D{};
	app::InputManager inputManager{};
	std::optional<audio::SoundStage> soundStage{};
	std::optional<audio::Sound> music{};
	audio::SoundStage::SoundInstanceId musicId{};
	float verticalFieldOfView;
	glm::vec3 carrotCakeCurrentPosition{0.0f, 0.0f, 0.0f};
	glm::vec3 carrotCakePreviousPosition{0.0f, 0.0f, 0.0f};
	glm::vec3 carrotCakeDisplayPosition{0.0f, 0.0f, 0.0f};
	glm::vec2 carrotCakeScale{1.0f, 1.0f};
	glm::vec3 carrotCakeVelocity{0.0f, 0.0f, 0.0f};
	Timer<float> timerA{};
	Timer<float> timerB{};
	unsigned counterA = 0u;
	unsigned counterB = 0u;
	LooseQuadtree<std::forward_list<Circle<float>>> quadtree{
		Box<2, float>{.min{15.0f, 15.0f}, .max{15.0f + 380.0f, 15.0f + 450.0f}},
		glm::vec2{32.0f, 32.0f},
	};
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
				parseOptionValue("title", options.windowOptions.title);
			} else if (argument == "-width") {
				parseOptionValue("width", options.windowOptions.size.x);
			} else if (argument == "-height") {
				parseOptionValue("height", options.windowOptions.size.y);
			} else if (argument == "-resizable") {
				parseOptionValue("resizable", options.windowOptions.resizable);
			} else if (argument == "-fullscreen") {
				parseOptionValue("fullscreen", options.windowOptions.fullscreen);
			} else if (argument == "-vsync") {
				parseOptionValue("vsync", options.windowOptions.vSync);
			} else if (argument == "-min-fps") {
				parseOptionValue("min fps", options.applicationOptions.minFps);
			} else if (argument == "-max-fps") {
				parseOptionValue("max fps", options.applicationOptions.maxFps);
			} else if (argument == "-msaa") {
				parseOptionValue("msaa", options.windowOptions.msaaLevel);
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
