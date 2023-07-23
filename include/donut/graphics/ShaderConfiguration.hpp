#ifndef DONUT_GRAPHICS_SHADER_CONFIGURATION_HPP
#define DONUT_GRAPHICS_SHADER_CONFIGURATION_HPP

#include <compare> // std::strong_ordering
#include <cstdint> // std::int32_t, std::uint8_t, std::uint32_t

namespace donut::graphics {

/**
 * Depth buffer mode used in a ShaderConfiguration.
 *
 * \sa ShaderConfiguration::depthBufferMode
 */
enum class DepthBufferMode : std::uint8_t {
	/**
	 * Ignore the depth buffer.
	 */
	IGNORE,

	/**
	 * Evaluate the depth test defined by the DepthTestPredicate to determine
	 * whether the pixel should be rendered or discarded.
	 *
	 * If the test fails, the new pixel is discarded and will not be rendered.
	 *
	 * \note In 3D, using the depth test with DepthTestPredicate::LESS is
	 *       necessary to avoid 3D objects and faces being incorrectly
	 *       rendered on top of each other. However, for 2D shaders, the
	 *       depth test should typically be disabled in favor of using the
	 *       painter's algorithm instead.
	 */
	USE_DEPTH_TEST,
};

/**
 * Depth test predicate used in a ShaderConfiguration.
 *
 * \sa ShaderConfiguration::depthTestPredicate
 */
enum class DepthTestPredicate : std::uint32_t {
	/**
	 * The depth test always fails.
	 */
	NEVER_PASS = 0x0200,

	/**
	 * The depth test passes if and only if the new depth value is less than the
	 * old depth value.
	 */
	LESS = 0x0201,

	/**
	 * The depth test passes if and only if the new depth value is less than or
	 * equal to the old depth value.
	 */
	LESS_OR_EQUAL = 0x0203,

	/**
	 * The depth test passes if and only if the new depth value is greater than
	 * the old depth value.
	 */
	GREATER = 0x0204,

	/**
	 * The depth test passes if and only if the new depth value is greater than
	 * or equal to the old depth value.
	 */
	GREATER_OR_EQUAL = 0x0206,

	/**
	 * The depth test passes if and only if the new depth value is equal to the
	 * old depth value.
	 */
	EQUAL = 0x0202,

	/**
	 * The depth test passes if and only if the new depth value is not equal to
	 * the old depth value.
	 */
	NOT_EQUAL = 0x0205,

	/**
	 * The depth test always passes.
	 */
	ALWAYS_PASS = 0x0207,
};

/**
 * Stencil buffer mode used in a ShaderConfiguration.
 *
 * \sa ShaderConfiguration::stencilBufferMode
 */
enum class StencilBufferMode : std::uint8_t {
	/**
	 * Ignore the stencil buffer.
	 */
	IGNORE,

	/**
	 * Evaluate the stencil test defined by the StencilTestPredicate to
	 * determine whether the pixel should be rendered or discarded, then perform
	 * the corresponding StencilBufferOperation on the stencil buffer value.
	 * 
	 * If the test fails, the new pixel is discarded and will not be rendered.
	 */
	USE_STENCIL_TEST,
};

/**
 * Stencil test predicate used in a ShaderConfiguration.
 *
 * \sa ShaderConfiguration::stencilTestPredicate
 */
enum class StencilTestPredicate : std::uint32_t {
	/**
	 * The stencil test always fails.
	 */
	NEVER_PASS = 0x0200,

	/**
	 * The stencil test passes if and only if the given reference value is less
	 * than the current value in the stencil buffer.
	 *
	 * The stencil value and the reference value are both masked with the given
	 * mask before performing the check.
	 */
	LESS = 0x0201,

	/**
	 * The stencil test passes if and only if the given reference value is less
	 * than or equal to the current value in the stencil buffer.
	 *
	 * The stencil value and the reference value are both masked with the given
	 * mask before performing the check.
	 */
	LESS_OR_EQUAL = 0x0203,

	/**
	 * The stencil test passes if and only if the given reference value is
	 * greater than the current value in the stencil buffer.
	 *
	 * The stencil value and the reference value are both masked with the given
	 * mask before performing the check.
	 */
	GREATER = 0x0204,

	/**
	 * The stencil test passes if and only if the given reference value is
	 * greater than or equal to the current value in the stencil buffer.
	 *
	 * The stencil value and the reference value are both masked with the given
	 * mask before performing the check.
	 */
	GREATER_OR_EQUAL = 0x0206,

	/**
	 * The stencil test passes if and only if the given reference value is equal
	 * to the current value in the stencil buffer.
	 *
	 * The stencil value and the reference value are both masked with the given
	 * mask before performing the check.
	 */
	EQUAL = 0x0202,

	/**
	 * The stencil test passes if and only if the given reference value is not
	 * equal to the current value in the stencil buffer.
	 *
	 * The stencil value and the reference value are both masked with the given
	 * mask before performing the check.
	 */
	NOT_EQUAL = 0x0205,

	/**
	 * The stencil test always passes.
	 */
	ALWAYS_PASS = 0x0207,
};

/**
 * Operation to perform after evaluating the stencil test in a
 * ShaderConfiguration.
 *
 * \sa ShaderConfiguration::stencilBufferOperationOnStencilTestFail
 * \sa ShaderConfiguration::stencilBufferOperationOnDepthTestFail
 * \sa ShaderConfiguration::stencilBufferOperationOnPass
 */
enum class StencilBufferOperation : std::uint32_t {
	/**
	 * Keep the current value in the stencil buffer.
	 */
	KEEP = 0x1E00,

	/**
	 * Set the stencil buffer value to 0.
	 */
	SET_TO_ZERO = 0,

	/**
	 * Set the stencil buffer value to the given reference value.
	 */
	REPLACE = 0x1E01,

	/**
	 * Increment the stencil buffer value by 1, unless it is already maxed out.
	 */
	INCREMENT_CLAMP = 0x1E02,

	/**
	 * Increment the stencil buffer value by 1, or wrap around to 0 if it was
	 * maxed out.
	 */
	INCREMENT_WRAP = 0x8507,

	/**
	 * Decrement the stencil buffer value by 1, unless it is already 0.
	 */
	DECREMENT_CLAMP = 0x1E03,

	/**
	 * Decrement the stencil buffer value by 1, or wrap around to the maximum
	 * value if it was 0.
	 */
	DECREMENT_WRAP = 0x8508,

	/**
	 * Toggle each bit in the stencil buffer value.
	 */
	BITWISE_INVERT = 0x150A,
};

/**
 * Face culling mode used in a ShaderConfiguration.
 *
 * \sa ShaderConfiguration::faceCullingMode
 */
enum class FaceCullingMode : std::uint8_t {
	/**
	 * Ignore facing.
	 */
	IGNORE,

	/**
	 * Discard back-facing faces.
	 */
	CULL_BACK_FACES,

	/**
	 * Discard front-facing faces.
	 */
	CULL_FRONT_FACES,

	/**
	 * Discard all faces, only render primitives without faces, such as lines
	 * and points.
	 */
	CULL_FRONT_AND_BACK_FACES,
};

/**
 * Front face used in a ShaderConfiguration.
 *
 * \sa ShaderConfiguration::frontFace
 */
enum class FrontFace : std::uint32_t {
	/**
	 * Consider faces with clockwise winding order as front-facing.
	 */
	CLOCKWISE = 0x0900,

	/**
	 * Consider faces with counterclockwise winding order as front-facing.
	 */
	COUNTERCLOCKWISE = 0x0901,
};

/**
 * Alpha mode used in a ShaderConfiguration.
 *
 * \sa ShaderConfiguration::alphaMode
 */
enum class AlphaMode : std::uint8_t {
	/**
	 * Ignore the alpha channel value of the rendered pixel color.
	 */
	IGNORE,

	/**
	 * Blend the old and new pixel colors depending on the alpha value of
	 * the new pixel according to the standard "over" compositing operator
	 * for transparency.
	 */
	USE_ALPHA_BLENDING,
};

/**
 * Base configuration options for a shader.
 */
struct ShaderConfiguration {
	/**
	 * How to treat the depth buffer for each pixel being rendered.
	 *
	 * \sa #depthTestPredicate
	 */
	DepthBufferMode depthBufferMode = DepthBufferMode::USE_DEPTH_TEST;

	/**
	 * The condition to check when evaluating the depth test.
	 *
	 * \sa #depthBufferMode
	 */
	DepthTestPredicate depthTestPredicate = DepthTestPredicate::LESS;

	/**
	 * How to treat the stencil buffer for each pixel being rendered.
	 *
	 * \sa #stencilTestPredicate
	 * \sa #stencilTestReferenceValue
	 * \sa #stencilTestMask
	 * \sa #stencilBufferOperationOnStencilTestFail
	 * \sa #stencilBufferOperationOnDepthTestFail
	 * \sa #stencilBufferOperationOnPass
	 */
	StencilBufferMode stencilBufferMode = StencilBufferMode::IGNORE;

	/**
	 * The condition to check when evaluating the stencil test.
	 *
	 * \sa #stencilBufferMode
	 * \sa #stencilTestReferenceValue
	 * \sa #stencilTestMask
	 * \sa #stencilBufferOperationOnStencilTestFail
	 * \sa #stencilBufferOperationOnDepthTestFail
	 * \sa #stencilBufferOperationOnPass
	 */
	StencilTestPredicate stencilTestPredicate = StencilTestPredicate::ALWAYS_PASS;

	/**
	 * The reference value to compare the stencil buffer value against when
	 * evaluating the stencil test.
	 *
	 * \sa #stencilBufferMode
	 * \sa #stencilTestPredicate
	 * \sa #stencilTestMask
	 * \sa #stencilBufferOperationOnStencilTestFail
	 * \sa #stencilBufferOperationOnDepthTestFail
	 * \sa #stencilBufferOperationOnPass
	 */
	std::int32_t stencilTestReferenceValue = 0;

	/**
	 * The bit pattern to mask the reference value and stencil value with
	 * before performing the stencil test.
	 *
	 * The set bits in the mask indicate the relevant bits that will be
	 * compared.
	 *
	 * \sa #stencilBufferMode
	 * \sa #stencilTestPredicate
	 * \sa #stencilTestReferenceValue
	 * \sa #stencilBufferOperationOnStencilTestFail
	 * \sa #stencilBufferOperationOnDepthTestFail
	 * \sa #stencilBufferOperationOnPass
	 */
	std::uint32_t stencilTestMask = 0xFFFFFFFF;

	/**
	 * The operation to perform on the stencil buffer if the stencil test fails.
	 *
	 * \sa #stencilBufferMode
	 * \sa #stencilTestPredicate
	 * \sa #stencilTestReferenceValue
	 * \sa #stencilTestMask
	 * \sa #stencilBufferOperationOnDepthTestFail
	 * \sa #stencilBufferOperationOnPass
	 */
	StencilBufferOperation stencilBufferOperationOnStencilTestFail = StencilBufferOperation::KEEP;

	/**
	 * The operation to perform on the stencil buffer if the stencil test
	 * passes, but the depth test fails.
	 *
	 * \sa #stencilBufferMode
	 * \sa #stencilTestPredicate
	 * \sa #stencilTestReferenceValue
	 * \sa #stencilTestMask
	 * \sa #stencilBufferOperationOnStencilTestFail
	 * \sa #stencilBufferOperationOnPass
	 */
	StencilBufferOperation stencilBufferOperationOnDepthTestFail = StencilBufferOperation::KEEP;

	/**
	 * The operation to perform on the stencil buffer if the stencil test and
	 * the depth test both pass.
	 *
	 * \sa #stencilBufferMode
	 * \sa #stencilTestPredicate
	 * \sa #stencilTestReferenceValue
	 * \sa #stencilTestMask
	 * \sa #stencilBufferOperationOnStencilTestFail
	 * \sa #stencilBufferOperationOnDepthTestFail
	 */
	StencilBufferOperation stencilBufferOperationOnPass = StencilBufferOperation::KEEP;

	/**
	 * How to treat the facing of primitives while rendering.
	 *
	 * The facing is determined by the winding order of the vertices on each
	 * rendered face. The face is considered to be front-facing if it has the
	 * winding order specified by #frontFace.
	 *
	 * \sa #frontFace
	 */
	FaceCullingMode faceCullingMode = FaceCullingMode::CULL_BACK_FACES;

	/**
	 * The winding order of front-facing faces.
	 *
	 * \sa #faceCullingMode
	 */
	FrontFace frontFace = FrontFace::COUNTERCLOCKWISE;

	/**
	 * How to treat the alpha channel of the output pixel color while rendering.
	 */
	AlphaMode alphaMode = AlphaMode::IGNORE;

	/**
	 * Compare two sets of shader options for equality.
	 *
	 * \param other the shader options to compare these to.
	 *
	 * \return true if the configurations are equal, false otherwise.
	 */
	[[nodiscard]] constexpr bool operator==(const ShaderConfiguration& other) const = default;

	/**
	 * Compare two sets of shader options.
	 *
	 * \param other the shader options to compare these to.
	 *
	 * \return a strong ordering between the two configurations.
	 */
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const ShaderConfiguration& other) const = default;
};

} // namespace donut::graphics

#endif
