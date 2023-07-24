/**
 * \example example_game.cpp
 *
 * \details This example shows a basic game project consisting of a single
 *          source file. The main application class, ExampleGame, is defined at
 *          the top while the main function is defined at the bottom.
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

#include <array>         // std::array
#include <charconv>      // std::from_chars_result, std::from_chars
#include <chrono>        // std::chrono_literals
#include <concepts>      // std::integral
#include <cstddef>       // std::size_t
#include <cstdio>        // stderr, std::sscanf, std::fprintf
#include <cstdlib>       // EXIT_SUCCESS, EXIT_FAILURE
#include <exception>     // std::exception
#include <format>        // std::format
#include <forward_list>  // std::forward_list
#include <optional>      // std::optional
#include <span>          // std::span
#include <stdexcept>     // std::runtime_error
#include <string>        // std::string
#include <string_view>   // std::string_view
#include <system_error>  // std::errc
#include <unordered_map> // std::unordered_map

namespace {

struct GameOptions {
	app::ApplicationOptions applicationOptions{
		.tickRate = 30.0f,
		.maxFrameRate = 240.0f,
	};
	gfx::WindowOptions windowOptions{
		.title = "Example Game",
		.size{640, 480},
		.resizable = true,
	};
	const char* mainMenuMusicFilepath = "sounds/music/donauwalzer.ogg";
	float fieldOfView = 90.0f;
};

class ExampleGame final : public app::Application {
public:
	ExampleGame(Filesystem& filesystem, const GameOptions& options)
		: app::Application(options.applicationOptions)
		, window(options.windowOptions)
		, testTexture(gfx::Image{filesystem, "textures/test.png"})
		, circleTexture(gfx::Image{filesystem, "textures/circle.png"}, {.useLinearFiltering = false, .useMipmap = false})
		, carrotCakeModel(filesystem, "models/carrot_cake.obj")
		, testSprite(spriteAtlas.insert(renderer, gfx::Image{filesystem, "textures/test.png"}))
		, testSubSprite(spriteAtlas.createSubSprite(testSprite, 200, 200, 100, 100, gfx::SpriteAtlas::FLIP_HORIZONTALLY))
		, mainFont(filesystem, "fonts/unscii/unscii-8.ttf")
		, verticalFieldOfView(2.0f * atan((3.0f / 4.0f) * tan(radians(options.fieldOfView) * 0.5f))) {
		loadBindingsConfiguration(filesystem, "configuration/bindings.json");

		initializeSoundStage();
		playMainMenuMusic(filesystem, options.mainMenuMusicFilepath);

		loadCircles();

		resize();
	}

protected:
	void update(app::FrameInfo frameInfo) override {
		using namespace std::chrono_literals;

		inputManager.prepareForEvents();
		for (const events::Event& event : eventPump.pollEvents()) {
			if (event.is<events::ApplicationQuitRequestedEvent>()) {
				quit();
			} else if (event.is<events::WindowSizeChangedEvent>()) {
				resize();
			}
			inputManager.handleEvent(event);
		}

		if (soundStage) {
			soundStage->update(frameInfo.deltaTime, listener);
		}

		if (inputManager.justPressed(events::Input::KEY_F10)) {
			quit();
		}

		if (inputManager.justPressed(events::Input::KEY_F11) ||
			(inputManager.justPressed(events::Input::KEY_RETURN) && (inputManager.isPressed(events::Input::KEY_LALT) || inputManager.isPressed(events::Input::KEY_RALT)))) {
			window.setFullscreen(!window.isFullscreen());
		}

		if (inputManager.justPressed(events::Input::KEY_F2)) {
			if (soundStage) {
				soundStage->stopSound(musicId);
			}
		}

		const float sprintInput = (inputManager.isPressed(Action::SPRINT)) ? 4.0f : 1.0f;

		vec2 movementInput = inputManager.getAbsoluteVector(Action::MOVE_LEFT, Action::MOVE_RIGHT, Action::MOVE_DOWN, Action::MOVE_UP);
		if (const float movementInputLengthSquared = length2(movementInput); movementInputLengthSquared > 1.0f) {
			movementInput /= sqrt(movementInputLengthSquared);
		}
		const float carrotCakeSpeed = 2.0f * sprintInput;
		carrotCakeVelocity = {movementInput * carrotCakeSpeed, 0.0f};

		if (inputManager.isPressed(Action::CONFIRM)) {
			const vec2 aimInput = inputManager.getRelativeVector(Action::AIM_LEFT, Action::AIM_RIGHT, Action::AIM_DOWN, Action::AIM_UP);
			carrotCakeScale = clamp(carrotCakeScale + aimInput * 10.0f, 0.25f, 4.0f);
		}

		const float scrollInput = inputManager.getRelativeVector(Action::SCROLL_DOWN, Action::SCROLL_UP);
		carrotCakeCurrentPosition.z -= scrollInput * 0.25f * sprintInput;

		const bool triggerInput = inputManager.isPressed(Action::CANCEL);
		counterA += timerA.countUpLoop(frameInfo.deltaTime, 1s, triggerInput);
		counterB += timerB.countDownLoop(frameInfo.deltaTime, 1s, triggerInput);
	}

	void tick(app::TickInfo tickInfo) override {
		carrotCakePreviousPosition = carrotCakeCurrentPosition;
		carrotCakeCurrentPosition += carrotCakeVelocity * tickInfo.tickInterval;
	}

	void display(app::TickInfo /*tickInfo*/, app::FrameInfo frameInfo) override {
		carrotCakeDisplayPosition = mix(carrotCakePreviousPosition, carrotCakeCurrentPosition, frameInfo.tickInterpolationAlpha);

		const ExampleShader::PointLight baseLight = {
			.position = carrotCakeDisplayPosition,
			.ambient{0.005f, 0.005f, 0.005f},
			.diffuse{0.5f + 0.5f * sin(frameInfo.elapsedTime), 0.8f, 0.8f},
			.specular{0.8f, 0.8f, 0.8f},
			.constantFalloff = 1.0f,
			.linearFalloff = 0.04f,
			.quadraticFalloff = 0.03f,
		};

		const auto baseLightWithOffset = [&](vec3 offset) -> ExampleShader::PointLight {
			ExampleShader::PointLight result = baseLight;
			result.position += offset;
			return result;
		};

		const std::array<ExampleShader::PointLight, ExampleShader::POINT_LIGHT_COUNT> pointLights{{
			baseLightWithOffset({-2.0f, 0.0f, 0.0f}),
			baseLightWithOffset({0.0f, -2.0f, 0.0f}),
			baseLightWithOffset({0.0f, 2.0f, 0.0f}),
			baseLightWithOffset({0.0f, 0.0f, 2.0f}),
		}};

		const vec3 viewPosition{0.0f, 0.0f, 0.0f};

		exampleShader.setPointLights(pointLights);
		exampleShader.setViewPosition(viewPosition);

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
			drawFrameRateCounter(renderPass);
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

	struct ExampleShader : gfx::Shader3D {
		struct PointLight {
			vec3 position;
			vec3 ambient;
			vec3 diffuse;
			vec3 specular;
			float constantFalloff;
			float linearFalloff;
			float quadraticFalloff;
		};

		struct PointLightParameters {
			PointLightParameters(const gfx::ShaderProgram& program, const char* name)
				: position(program, std::format("{}.position", name).c_str())
				, ambient(program, std::format("{}.ambient", name).c_str())
				, diffuse(program, std::format("{}.diffuse", name).c_str())
				, specular(program, std::format("{}.specular", name).c_str())
				, constantFalloff(program, std::format("{}.constantFalloff", name).c_str())
				, linearFalloff(program, std::format("{}.linearFalloff", name).c_str())
				, quadraticFalloff(program, std::format("{}.quadraticFalloff", name).c_str()) {}

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
			#ifndef GAMMA
			#define GAMMA 2.2
			#endif

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
			uniform sampler2D emissiveMap;
			uniform vec3 diffuseColor;
			uniform vec3 specularColor;
			uniform vec3 normalScale;
			uniform vec3 emissiveColor;
			uniform float specularExponent;
			uniform float dissolveFactor;
			uniform float occlusionFactor;

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

			vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDirection, vec3 ambient, vec3 diffuse, vec3 specular) {
				vec3 lightDifference = light.position - fragmentPosition;
				float lightDistanceSquared = dot(lightDifference, lightDifference);
				float lightDistance = sqrt(lightDistanceSquared);
				vec3 lightDirection = lightDifference * (1.0 / lightDistance);
				float cosine = dot(normal, lightDirection);
				float diffuseFactor = halfLambert(cosine);
				float specularFactor = blinnPhong(normal, lightDirection, viewDirection);
				float attenuation = 1.0 / (light.constantFalloff + light.linearFalloff * lightDistance + light.quadraticFalloff * lightDistanceSquared);
				vec3 ambientTerm = light.ambient * ambient;
				vec3 diffuseTerm = light.diffuse * diffuseFactor * diffuse;
				vec3 specularTerm = light.specular * specularFactor * specular;
				const float visibility = 1.0;
				return attenuation * (ambientTerm * occlusionFactor + (diffuseTerm + specularTerm) * visibility);
			}

			void main() {
				vec4 sampledDiffuse = texture(diffuseMap, fragmentTextureCoordinates);
				vec4 diffuse = fragmentTintColor * vec4(diffuseColor, 1.0 - dissolveFactor) * vec4(pow(sampledDiffuse.rgb, vec3(GAMMA)), sampledDiffuse.a);
				vec3 specular = specularColor * texture(specularMap, fragmentTextureCoordinates).rgb;
				vec3 emissive = emissiveColor * texture(emissiveMap, fragmentTextureCoordinates).rgb;
				
				mat3 TBN = mat3(normalize(fragmentTangent), normalize(fragmentBitangent), normalize(fragmentNormal));
				vec3 surfaceNormal = normalScale * (texture(normalMap, fragmentTextureCoordinates).xyz * 2.0 - vec3(1.0));
				vec3 normal = normalize(TBN * surfaceNormal);

				vec3 viewDirection = normalize(viewPosition - fragmentPosition);

				vec3 color = emissive;
				for (uint i = uint(0); i < uint(POINT_LIGHT_COUNT); ++i) {
					color += calculatePointLight(pointLights[i], normal, viewDirection, vec3(1.0), diffuse.rgb, specular);
				}
				outputColor = vec4(pow(color, vec3(1.0 / GAMMA)), diffuse.a);
			}
		)GLSL";

		ExampleShader()
			: gfx::Shader3D({
				  .definitions = std::format("#define POINT_LIGHT_COUNT {}", POINT_LIGHT_COUNT).c_str(),
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

		void setViewPosition(vec3 position) {
			program.setUniformVec3(viewPosition, position);
		}

	private:
		gfx::ShaderArray<PointLightParameters, POINT_LIGHT_COUNT> pointLights{program, "pointLights"};
		gfx::ShaderParameter viewPosition{program, "viewPosition"};
	};

	void resize() {
		constexpr ivec2 RENDER_RESOLUTION{640, 480};
		constexpr ivec2 WORLD_VIEWPORT_POSITION{15, 15};
		constexpr ivec2 WORLD_VIEWPORT_SIZE{380, 450};

		const ivec2 size = window.getDrawableSize();

		const auto [viewport, scale] = gfx::Viewport::createIntegerScaled(size, RENDER_RESOLUTION);
		screenViewport = viewport;
		screenCamera = gfx::Camera::createOrthographic({
			.offset{0.0f, 0.0f},
			.size = RENDER_RESOLUTION,
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

	void loadBindingsConfiguration(const Filesystem& filesystem, const char* filepath) {
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

		try {
			json::StringParser{filesystem.openFile(filepath).readAllIntoString()}.parseObject(
				json::onElement([&](const json::SourceLocation&, const json::String& key, json::StringParser& parser) -> void {
					if (const events::Input input = events::findInput(key); input != events::Input::UNKNOWN) {
						const auto bindAction = [&](const json::SourceLocation&, const json::String& value) -> void {
							if (const auto it = actionsByIdentifier.find(value); it != actionsByIdentifier.end()) {
								inputManager.addBinding(input, it->second);
							} else {
								throw std::runtime_error{std::format("Invalid action identifier \"{}\".", value)};
							}
						};
						parser.parseValue(json::onArray([&](const json::SourceLocation&, json::StringParser& parser) -> void { parser.parseArray(json::onString(bindAction)); }) |
										  json::onString(bindAction));
					} else {
						throw std::runtime_error{std::format("Invalid input identifier \"{}\".", key)};
					}
				}));
		} catch (const json::Error& e) {
			throw std::runtime_error{std::format("{}:{}:{}: {}", filepath, e.source.lineNumber, e.source.columnNumber, e.what())};
		} catch (const std::exception& e) {
			throw std::runtime_error{std::format("{}: {}", filepath, e.what())};
		}
	}

	void initializeSoundStage() {
		try {
			soundStage.emplace();
		} catch (const audio::Error& e) {
			// Don't crash on failure, since the user might just not have a working sound card.
			// Just print an error message instead.
			std::fprintf(stderr, "%s\n", e.what());
		}
	}

	void playMainMenuMusic(const Filesystem& filesystem, const char* filepath) {
		using namespace std::chrono_literals;

		if (soundStage) {
			if (filesystem.fileExists(filepath)) {
				music.emplace(filesystem, filepath,
					audio::SoundOptions{
						.attenuationModel = audio::SoundAttenuationModel::NO_ATTENUATION,
						.volume = 0.1f,
						.listenerRelative = true,
						.looping = true,
					});
				musicId = soundStage->createPausedSoundInBackground(*music);
				soundStage->seekToSoundTime(musicId, 46700ms);
				soundStage->resumeSound(musicId);
			}
		}
	}

	void loadCircles() {
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
	}

	void drawBackground(gfx::RenderPass& renderPass, const app::FrameInfo& frameInfo) {
		constexpr vec3 BACKGROUND_OFFSET{0.0f, 3.5f, -10.0f};
		constexpr vec2 BACKGROUND_SCALE{18.0f, 18.0f};
		constexpr float BACKGROUND_ANGLE = -30.0f;
		constexpr float BACKGROUND_SPEED = 2.0f;

		renderPass.draw(gfx::QuadInstance{
			.texture = &testTexture,
			.transformation = translate(BACKGROUND_OFFSET) *                            //
		                      orientate4(vec3{radians(BACKGROUND_ANGLE), 0.0f, 0.0f}) * //
		                      scale(vec3{BACKGROUND_SCALE, 1.0f}) *                     //
		                      translate(vec3{-0.5f, -0.5f, 0.0f}),
			.textureOffset{0.0f, frameInfo.elapsedTime * BACKGROUND_SPEED},
			.textureScale = 1000.0f * BACKGROUND_SCALE / testTexture.getSize2D(),
		});
	}

	void drawWorld(gfx::RenderPass& renderPass, const app::FrameInfo& frameInfo) {
		renderPass.draw(gfx::ModelInstance{
			.model = &carrotCakeModel,
			.transformation = translate(vec3{0.6f, 0.7f, -3.0f} + carrotCakeDisplayPosition) *                     //
		                      scale(vec3{5.0f * carrotCakeScale.x, 5.0f * carrotCakeScale.y, 5.0f}) *              //
		                      orientate4(vec3{0.0f, frameInfo.elapsedTime * 1.5f, frameInfo.elapsedTime * 2.0f}) * //
		                      translate(vec3{0.0f, -0.05f, 0.0f}),
		});

		renderPass.draw(gfx::ModelInstance{
			.shader = &exampleShader,
			.model = &carrotCakeModel,
			.transformation = translate(vec3{-0.6f, 0.2f, -3.0f}) *                                                //
		                      scale(vec3{5.0f, 5.0f, 5.0f}) *                                                      //
		                      orientate4(vec3{0.0f, frameInfo.elapsedTime * 1.5f, frameInfo.elapsedTime * 2.0f}) * //
		                      translate(vec3{0.0f, -0.05f, 0.0f}),
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
			.position{200.0f + cos(frameInfo.elapsedTime) * 50.0f, 120.0f + sin(frameInfo.elapsedTime) * 50.0f},
			.scale{0.2f + sin(frameInfo.elapsedTime) * 0.1f, 0.2f + cos(frameInfo.elapsedTime) * 0.1f},
			.origin{0.5f, 0.5f},
		});

		renderPass.draw(gfx::SpriteInstance{
			.atlas = &spriteAtlas,
			.id = testSprite,
			.position{450.0f + cos(frameInfo.elapsedTime) * 50.0f, 120.0f + sin(frameInfo.elapsedTime) * 50.0f},
			.scale{0.2f + sin(frameInfo.elapsedTime) * 0.1f, 0.2f + cos(frameInfo.elapsedTime) * 0.1f},
			.origin{0.5f, 0.5f},
		});

		renderPass.draw(gfx::SpriteInstance{
			.atlas = &spriteAtlas,
			.id = testSubSprite,
			.position{450.0f + cos(frameInfo.elapsedTime) * 50.0f, 320.0f + sin(frameInfo.elapsedTime) * 50.0f},
			.scale{0.2f + sin(frameInfo.elapsedTime) * 0.1f, 0.2f + cos(frameInfo.elapsedTime) * 0.1f},
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
				std::format("Position:\n({:.2f}, {:.2f}, {:.2f})\n\nScale:\n({:.2f}, {:.2f})", carrotCakeDisplayPosition.x, carrotCakeDisplayPosition.y,
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
				std::format("Timer   A: {:.2f}\nCounter A: {}\n\nTimer   B: {:.2f}\nCounter B: {}", static_cast<float>(timerA), counterA, static_cast<float>(timerB), counterB)),
			.position{410.0f, 240.0f},
		});

		if (inputManager.isPressed(events::Input::KEY_SPACE)) {
			constexpr Capsule<2, float> STATIC_CAPSULE{.centerLine{.pointA{80.0f, 80.0f}, .pointB{300.0f, 200.0f}}, .radius = 50.0f};
			constexpr vec2 STATIC_CAPSULE_VECTOR = STATIC_CAPSULE.centerLine.pointB - STATIC_CAPSULE.centerLine.pointA;
			const Circle<float> movingCircle{.center = vec2{200.0f, 50.0f} + vec2{carrotCakeDisplayPosition} * 50.0f, .radius = 32.0f};
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
				.size{length(STATIC_CAPSULE_VECTOR), STATIC_CAPSULE.radius * 2.0f},
				.angle = static_cast<float>(atan2(STATIC_CAPSULE_VECTOR.y, STATIC_CAPSULE_VECTOR.x)),
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
				const vec2 extent = box.max - box.min;
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
				.text = mainFont.shapeText(renderer, 8, std::format("AABB tests: {}\nCircle tests: {}", aabbTestCount, circleTestCount)),
				.position{410.0f, 450.0f},
				.color = Color::BURLY_WOOD,
			});
		}

		if (inputManager.justReleased(events::Input::KEY_SPACE)) {
			inputManager.resetAllInputs();
		}
	}

	void drawFrameRateCounter(gfx::RenderPass& renderPass) {
		const unsigned fps = getLastSecondFrameCount();
		const gfx::Font::ShapedText fpsText = mainFont.shapeText(renderer, 16, std::format("FPS: {}", fps));
		const vec2 fpsPosition{15.0f + 2.0f, 480.0f - 15.0f - 20.0f};
		const Color fpsColor = (fps < 60) ? Color::RED : (fps < 120) ? Color::YELLOW : (fps < 240) ? Color::GRAY : Color::LIME;
		renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = fpsText, .position = fpsPosition + vec2{1.0f, -1.0f}, .color = Color::BLACK});
		renderPass.draw(gfx::TextInstance{.font = &mainFont, .text = fpsText, .position = fpsPosition, .color = fpsColor});
	}

	events::EventPump eventPump{};
	gfx::Window window;
	gfx::Renderer renderer{};
	gfx::Viewport screenViewport{};
	gfx::Viewport worldViewport{};
	gfx::Camera screenCamera{};
	gfx::Camera worldCamera{};
	audio::Listener listener{};
	gfx::SpriteAtlas spriteAtlas{};
	gfx::Texture testTexture;
	gfx::Texture circleTexture;
	gfx::Model carrotCakeModel;
	gfx::SpriteAtlas::SpriteId testSprite;
	gfx::SpriteAtlas::SpriteId testSubSprite;
	gfx::Font mainFont;
	ExampleShader exampleShader{};
	events::InputManager inputManager{};
	std::optional<audio::SoundStage> soundStage{};
	std::optional<audio::Sound> music{};
	audio::SoundStage::SoundInstanceId musicId{};
	float verticalFieldOfView;
	vec3 carrotCakeCurrentPosition{0.0f, 0.0f, 0.0f};
	vec3 carrotCakePreviousPosition{0.0f, 0.0f, 0.0f};
	vec3 carrotCakeDisplayPosition{0.0f, 0.0f, 0.0f};
	vec2 carrotCakeScale{1.0f, 1.0f};
	vec3 carrotCakeVelocity{0.0f, 0.0f, 0.0f};
	Time<float> timerA{};
	Time<float> timerB{};
	std::size_t counterA = 0;
	std::size_t counterB = 0;
	LooseQuadtree<std::forward_list<Circle<float>>> quadtree{
		Box<2, float>{.min{15.0f, 15.0f}, .max{15.0f + 380.0f, 15.0f + 450.0f}},
		vec2{32.0f, 32.0f},
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
				parseOptionValue("min fps", options.applicationOptions.minFrameRate);
			} else if (argument == "-max-fps") {
				parseOptionValue("max fps", options.applicationOptions.maxFrameRate);
			} else if (argument == "-msaa") {
				parseOptionValue("msaa", options.windowOptions.msaaLevel);
			} else if (argument == "-main-menu-music") {
				parseOptionValue("main menu music file", options.mainMenuMusicFilepath);
			} else if (argument == "-fov") {
				parseOptionValue("fov", options.fieldOfView);
			} else {
				throw std::runtime_error{std::format("Unknown option {}. Try -help.", argument)};
			}
			++argumentIndex;
		}
		return options;
	}

private:
	void parseOptionValue(std::string_view optionName, const char*& output) {
		if (++argumentIndex >= argumentCount) {
			throw std::runtime_error{std::format("Missing {} value.", optionName)};
		}
		output = arguments[argumentIndex];
	}

	void parseOptionValue(std::string_view optionName, std::integral auto& output) {
		if (++argumentIndex >= argumentCount) {
			throw std::runtime_error{std::format("Missing {} value.", optionName)};
		}
		const std::string_view string{arguments[argumentIndex]};
		if (const std::from_chars_result result = std::from_chars(string.data(), string.data() + string.size(), output); result.ec != std::errc{}) {
			throw std::runtime_error{std::format("Invalid {} value \"{}\": {}", optionName, string, std::make_error_code(result.ec).message())};
		}
	}

	void parseOptionValue(std::string_view optionName, float& output) {
		if (++argumentIndex >= argumentCount) {
			throw std::runtime_error{std::format("Missing {} value.", optionName)};
		}
		if (std::sscanf(arguments[argumentIndex], "%f", &output) != 1) {
			throw std::runtime_error{std::format("Invalid {} value \"{}\".", optionName, arguments[argumentIndex])};
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
		Filesystem filesystem{argv[0],
			FilesystemOptions{
				.organizationName = "Donut",
				.applicationName = "ExampleGame",
				.dataDirectory = "data",
				.archiveSearchPath = ".",
				.archiveSearchFileExtension = "pk3",
			}};

		const Variant<GameOptions, std::string> options = OptionsParser{argc, argv}.parseGameOptions();
		if (options.is<std::string>()) {
			std::fprintf(stderr, "%s\n", options.as<std::string>().c_str());
			return EXIT_SUCCESS;
		}

		ExampleGame game{filesystem, options.as<GameOptions>()};

		game.run();
	} catch (const std::exception& e) {
		std::fprintf(stderr, "%s\n", e.what());
		events::MessageBox::show(events::MessageBox::Type::ERROR_MESSAGE, "Error", e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
