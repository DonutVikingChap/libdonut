#ifndef DONUT_COLOR_HPP
#define DONUT_COLOR_HPP

#include <glm/glm.hpp> // glm::...

namespace donut {

/**
 * Normalized floating-point RGBA color type with 32 bits per component.
 */
class Color {
public:
	static const Color INVISIBLE;
	static const Color ALICE_BLUE;
	static const Color ANTIQUE_WHITE;
	static const Color AQUA;
	static const Color AQUAMARINE;
	static const Color AZURE;
	static const Color BEIGE;
	static const Color BISQUE;
	static const Color BLACK;
	static const Color BLANCHED_ALMOND;
	static const Color BLUE;
	static const Color BLUE_VIOLET;
	static const Color BROWN;
	static const Color BURLY_WOOD;
	static const Color CADET_BLUE;
	static const Color CHARTREUSE;
	static const Color CHOCOLATE;
	static const Color CORAL;
	static const Color CORNFLOWER_BLUE;
	static const Color CORNSILK;
	static const Color CRIMSON;
	static const Color CYAN;
	static const Color DARK_BLUE;
	static const Color DARK_CYAN;
	static const Color DARK_GOLDEN_ROD;
	static const Color DARK_GRAY;
	static const Color DARK_GREY;
	static const Color DARK_GREEN;
	static const Color DARK_KHAKI;
	static const Color DARK_MAGENTA;
	static const Color DARK_OLIVE_GREEN;
	static const Color DARK_ORANGE;
	static const Color DARK_ORCHID;
	static const Color DARK_RED;
	static const Color DARK_SALMON;
	static const Color DARK_SEA_GREEN;
	static const Color DARK_SLATE_BLUE;
	static const Color DARK_SLATE_GRAY;
	static const Color DARK_SLATE_GREY;
	static const Color DARK_TURQUOISE;
	static const Color DARK_VIOLET;
	static const Color DEEP_PINK;
	static const Color DEEP_SKY_BLUE;
	static const Color DIM_GRAY;
	static const Color DIM_GREY;
	static const Color DODGER_BLUE;
	static const Color FIRE_BRICK;
	static const Color FLORAL_WHITE;
	static const Color FOREST_GREEN;
	static const Color FUCHSIA;
	static const Color GAINSBORO;
	static const Color GHOST_WHITE;
	static const Color GOLD;
	static const Color GOLDEN_ROD;
	static const Color GRAY;
	static const Color GREY;
	static const Color GREEN;
	static const Color GREEN_YELLOW;
	static const Color HONEY_DEW;
	static const Color HOT_PINK;
	static const Color INDIAN_RED;
	static const Color INDIGO;
	static const Color IVORY;
	static const Color KHAKI;
	static const Color LAVENDER;
	static const Color LAVENDER_BLUSH;
	static const Color LAWN_GREEN;
	static const Color LEMON_CHIFFON;
	static const Color LIGHT_BLUE;
	static const Color LIGHT_CORAL;
	static const Color LIGHT_CYAN;
	static const Color LIGHT_GOLDEN_ROD_YELLOW;
	static const Color LIGHT_GRAY;
	static const Color LIGHT_GREY;
	static const Color LIGHT_GREEN;
	static const Color LIGHT_PINK;
	static const Color LIGHT_SALMON;
	static const Color LIGHT_SEA_GREEN;
	static const Color LIGHT_SKY_BLUE;
	static const Color LIGHT_SLATE_GRAY;
	static const Color LIGHT_SLATE_GREY;
	static const Color LIGHT_STEEL_BLUE;
	static const Color LIGHT_YELLOW;
	static const Color LIME;
	static const Color LIME_GREEN;
	static const Color LINEN;
	static const Color MAGENTA;
	static const Color MAROON;
	static const Color MEDIUM_AQUA_MARINE;
	static const Color MEDIUM_BLUE;
	static const Color MEDIUM_ORCHID;
	static const Color MEDIUM_PURPLE;
	static const Color MEDIUM_SEA_GREEN;
	static const Color MEDIUM_SLATE_BLUE;
	static const Color MEDIUM_SPRING_GREEN;
	static const Color MEDIUM_TURQUOISE;
	static const Color MEDIUM_VIOLET_RED;
	static const Color MIDNIGHT_BLUE;
	static const Color MINT_CREAM;
	static const Color MISTY_ROSE;
	static const Color MOCCASIN;
	static const Color NAVAJO_WHITE;
	static const Color NAVY;
	static const Color OLD_LACE;
	static const Color OLIVE;
	static const Color OLIVE_DRAB;
	static const Color ORANGE;
	static const Color ORANGE_RED;
	static const Color ORCHID;
	static const Color PALE_GOLDEN_ROD;
	static const Color PALE_GREEN;
	static const Color PALE_TURQUOISE;
	static const Color PALE_VIOLET_RED;
	static const Color PAPAYA_WHIP;
	static const Color PEACH_PUFF;
	static const Color PERU;
	static const Color PINK;
	static const Color PLUM;
	static const Color POWDER_BLUE;
	static const Color PURPLE;
	static const Color REBECCA_PURPLE;
	static const Color RED;
	static const Color ROSY_BROWN;
	static const Color ROYAL_BLUE;
	static const Color SADDLE_BROWN;
	static const Color SALMON;
	static const Color SANDY_BROWN;
	static const Color SEA_GREEN;
	static const Color SEA_SHELL;
	static const Color SIENNA;
	static const Color SILVER;
	static const Color SKY_BLUE;
	static const Color SLATE_BLUE;
	static const Color SLATE_GRAY;
	static const Color SLATE_GREY;
	static const Color SNOW;
	static const Color SPRING_GREEN;
	static const Color STEEL_BLUE;
	static const Color TAN;
	static const Color TEAL;
	static const Color THISTLE;
	static const Color TOMATO;
	static const Color TURQUOISE;
	static const Color VIOLET;
	static const Color WHEAT;
	static const Color WHITE;
	static const Color WHITE_SMOKE;
	static const Color YELLOW;
	static const Color YELLOW_GREEN;

	/**
	 * Construct a transparent color with a value of 0 in all components.
	 */
	constexpr Color() noexcept = default;

	/**
	 * Construct a color with given values for each component.
	 *
	 * \param r value of the red color component.
	 * \param g value of the green color component.
	 * \param b value of the blue color component.
	 * \param a value of the alpha component. Defaults to fully opaque, i.e. a
	 *        value of 1.
	 */
	constexpr Color(float r, float g, float b, float a = 1.0f) noexcept
		: rgba(r, g, b, a) {}

	/**
	 * Construct a color with given values for each component.
	 *
	 * \param r value of the red color component.
	 * \param g value of the green color component.
	 * \param b value of the blue color component.
	 * \param a value of the alpha component. Defaults to fully opaque, i.e. a
	 *        value of 1.
	 */
	constexpr Color(double r, double g, double b, double a = 1.0) noexcept
		: rgba(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a)) {}

	/**
	 * Construct a color with given integer values for each component in the
	 * range 0-255, which will be normalized to the range 0-1.
	 *
	 * \param r value of the red color component, before normalization.
	 * \param g value of the green color component.
	 * \param b value of the blue color component.
	 * \param a value of the alpha component. Defaults to fully opaque, i.e. a
	 *        value of 1.
	 */
	constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) noexcept
		: Color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f) {}

	/**
	 * Construct a color with given integer values for each component in the
	 * range 0-255, which will be normalized to the range 0-1.
	 *
	 * \param r value of the red color component, before normalization.
	 * \param g value of the green color component.
	 * \param b value of the blue color component.
	 * \param a value of the alpha component. Defaults to fully opaque, i.e. a
	 *        value of 1.
	 */
	constexpr Color(int r, int g, int b, int a = 255) noexcept
		: Color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f) {}

	/**
	 * Construct a color from a vector with 3 components, XYZ, that map to the
	 * color components RGB, respectively.
	 *
	 * The alpha component is set to fully opaque, i.e. a value of 1.
	 *
	 * \param rgb input vector containing values for the red, green, and blue
	 *        components.
	 */
	constexpr Color(glm::vec3 rgb) noexcept
		: rgba(rgb.x, rgb.y, rgb.z, 1.0f) {}

	/**
	 * Construct a color from a vector with 4 components, XYZW, that map to the
	 * color components RGBA, respectively.
	 *
	 * \param rgba input vector containing values for the red, green, blue and
	 *        alpha components.
	 */
	constexpr Color(glm::vec4 rgba) noexcept
		: rgba(rgba.x, rgba.y, rgba.z, rgba.w) {}

	/**
	 * Construct a color from a vector with 3 integer components in the range
	 * 0-255, XYZ, that map to the normalized 0-1 color components RGB,
	 * respectively.
	 *
	 * The alpha component is set to fully opaque, i.e. a value of 1.
	 *
	 * \param rgb input vector containing values for the red, green, and blue
	 *        components.
	 */
	constexpr Color(glm::u8vec3 rgb) noexcept
		: rgba(rgb.x, rgb.y, rgb.z, 255) {}

	/**
	 * Construct a color from a vector with 4 integer components in the range
	 * 0-255, XYZW, that map to the normalized 0-1 color components RGBA,
	 * respectively.
	 *
	 * \param rgba input vector containing values for the red, green, blue and
	 *        alpha components.
	 */
	constexpr Color(glm::u8vec4 rgba) noexcept
		: rgba(rgba.x, rgba.y, rgba.z, rgba.w) {}

	/**
	 * Convert the color to a vector with 3 components, XYZ, that are mapped
	 * from the color components RGB, respectively.
	 *
	 * \return the RGB components of the color as a vector.
	 */
	constexpr operator glm::vec3() const noexcept {
		return glm::vec3{rgba};
	}

	/**
	 * Convert the color to a vector with 4 components, XYZW, that are mapped
	 * from the color components RGBA, respectively.
	 *
	 * \return the RGBA components of the color as a vector.
	 */
	constexpr operator glm::vec4() const noexcept {
		return rgba;
	}

	/**
	 * Convert the color to a vector with 3 components, XYZ, that are mapped
	 * from the color components RGB, respectively.
	 *
	 * \return the RGB components of the color as a vector.
	 */
	constexpr operator glm::dvec3() const noexcept {
		return glm::dvec3{
			static_cast<double>(rgba.x),
			static_cast<double>(rgba.y),
			static_cast<double>(rgba.z),
		};
	}

	/**
	 * Convert the color to a vector with 4 components, XYZW, that are mapped
	 * from the color components RGBA, respectively.
	 *
	 * \return the RGBA components of the color as a vector.
	 */
	constexpr operator glm::dvec4() const noexcept {
		return glm::dvec4{
			static_cast<double>(rgba.x),
			static_cast<double>(rgba.y),
			static_cast<double>(rgba.z),
			static_cast<double>(rgba.w),
		};
	}

	/**
	 * Convert the color to a vector with 3 components, XYZ, that are mapped
	 * from the color components RGB, respectively, which are clamped to the
	 * range 0-1 before being converted to 0-255.
	 *
	 * \return the RGB components of the color as a vector.
	 */
	constexpr operator glm::u8vec3() const noexcept {
		return glm::u8vec3{
			static_cast<std::uint8_t>(glm::clamp(rgba.x, 0.0f, 1.0f) * 255.0f),
			static_cast<std::uint8_t>(glm::clamp(rgba.y, 0.0f, 1.0f) * 255.0f),
			static_cast<std::uint8_t>(glm::clamp(rgba.z, 0.0f, 1.0f) * 255.0f),
		};
	}

	/**
	 * Convert the color to a vector with 4 components, XYZW, that are mapped
	 * from the normalized color components RGBA, respectively, which are
	 * clamped to the range 0-1 before being converted to 0-255.
	 *
	 * \return the RGBA components of the color as a vector.
	 */
	constexpr operator glm::u8vec4() const noexcept {
		return glm::u8vec4{
			static_cast<std::uint8_t>(glm::clamp(rgba.x, 0.0f, 1.0f) * 255.0f),
			static_cast<std::uint8_t>(glm::clamp(rgba.y, 0.0f, 1.0f) * 255.0f),
			static_cast<std::uint8_t>(glm::clamp(rgba.z, 0.0f, 1.0f) * 255.0f),
			static_cast<std::uint8_t>(glm::clamp(rgba.w, 0.0f, 1.0f) * 255.0f),
		};
	}

	/**
	 * Add the component values of another color to the respective component
	 * values of this color.
	 *
	 * \param other the other color to add to this color.
	 *
	 * \return `*this`, for chaining.
	 */
	constexpr Color& operator+=(const Color& other) noexcept {
		rgba += other.rgba;
		return *this;
	}

	/**
	 * Subtract the component values of another color from the respective
	 * component values of this color.
	 *
	 * \param other the other color to subtract from this color.
	 *
	 * \return `*this`, for chaining.
	 */
	constexpr Color& operator-=(const Color& other) noexcept {
		rgba -= other.rgba;
		return *this;
	}

	/**
	 * Multiply the component values of this color with the respective component
	 * values of another color.
	 *
	 * \param other the other color to multiply this color by.
	 *
	 * \return `*this`, for chaining.
	 */
	constexpr Color& operator*=(const Color& other) noexcept {
		rgba *= other.rgba;
		return *this;
	}

	/**
	 * Divide the component values of this color with the respective component
	 * values of another color.
	 *
	 * \param other the other color to divide this color by.
	 *
	 * \return `*this`, for chaining.
	 */
	constexpr Color& operator/=(const Color& other) noexcept {
		rgba /= other.rgba;
		return *this;
	}

	/**
	 * Multiply each of the component values of this color by a scalar value.
	 *
	 * \param scalar the scalar value to multiply this color by.
	 *
	 * \return `*this`, for chaining.
	 */
	constexpr Color& operator*=(float scalar) noexcept {
		rgba *= scalar;
		return *this;
	}

	/**
	 * Divide each of the component values of this color by a scalar value.
	 *
	 * \param scalar the scalar value to divide this color by.
	 *
	 * \return `*this`, for chaining.
	 */
	constexpr Color& operator/=(float scalar) noexcept {
		rgba /= scalar;
		return *this;
	}

	/**
	 * Get the component-wise additive identity of a color.
	 *
	 * \param a color to get the additive identity of.
	 *
	 * \return the additive identity of the given color.
	 */
	[[nodiscard]] friend Color operator+(const Color& a) {
		return Color{+a.rgba};
	}

	/**
	 * Get the component-wise additive inverse of a color.
	 *
	 * \param a color to get the additive inverse of.
	 *
	 * \return the additive inverse of the given color.
	 */
	[[nodiscard]] friend Color operator-(const Color& a) {
		return Color{-a.rgba};
	}

	/**
	 * Get the result of component-wise addition between two colors.
	 *
	 * \param a left-hand side of the addition.
	 * \param b right-hand side of the addition.
	 *
	 * \return a color containing a + b, component-wise.
	 */
	[[nodiscard]] friend Color operator+(const Color& a, const Color& b) {
		return Color{a.rgba + b.rgba};
	}

	/**
	 * Get the result of component-wise subtraction between two colors.
	 *
	 * \param a left-hand side of the subtraction.
	 * \param b right-hand side of the subtraction.
	 *
	 * \return a color containing a - b, component-wise.
	 */
	[[nodiscard]] friend Color operator-(const Color& a, const Color& b) {
		return Color{a.rgba - b.rgba};
	}

	/**
	 * Get the result of component-wise multiplication between two colors.
	 *
	 * \param a left-hand side of the multiplication.
	 * \param b right-hand side of the multiplication.
	 *
	 * \return a color containing a * b, component-wise.
	 */
	[[nodiscard]] friend Color operator*(const Color& a, const Color& b) {
		return Color{a.rgba * b.rgba};
	}

	/**
	 * Get the result of component-wise division between two colors.
	 *
	 * \param a left-hand side of the division.
	 * \param b right-hand side of the division.
	 *
	 * \return a color containing a / b, component-wise.
	 */
	[[nodiscard]] friend Color operator/(const Color& a, const Color& b) {
		return Color{a.rgba / b.rgba};
	}

	/**
	 * Get the result of multiplication between a color and a scalar.
	 *
	 * \param a left-hand side of the multiplication.
	 * \param b right-hand side of the multiplication.
	 *
	 * \return a color containing the result of multiplying each component of a
	 *         by b.
	 */
	[[nodiscard]] friend Color operator*(const Color& a, float b) {
		return Color{a.rgba * b};
	}

	/**
	 * Get the result of multiplication between a scalar and a color.
	 *
	 * \param a left-hand side of the multiplication.
	 * \param b right-hand side of the multiplication.
	 *
	 * \return a color containing the result of multiplying a by each component
	 *         of b.
	 */
	[[nodiscard]] friend Color operator*(float a, const Color& b) {
		return Color{a * b.rgba};
	}

	/**
	 * Get the result of division between a color and a scalar.
	 *
	 * \param a left-hand side of the division.
	 * \param b right-hand side of the division.
	 *
	 * \return a color containing the result of dividing each component of a by
	 *         b.
	 */
	[[nodiscard]] friend Color operator/(const Color& a, float b) {
		return Color{a.rgba / b};
	}

	/**
	 * Get the result of division between a scalar and a color.
	 *
	 * \param a left-hand side of the division.
	 * \param b right-hand side of the division.
	 *
	 * \return a color containing the result of dividing a by each component of
	 *         b.
	 */
	[[nodiscard]] friend Color operator/(float a, const Color& b) {
		return Color{a / b.rgba};
	}

	/**
	 * Set the value of the red component of this color.
	 *
	 * \param newValue new value to set the red color component to.
	 */
	constexpr void setRedComponent(float newValue) noexcept {
		rgba.x = newValue;
	}

	/**
	 * Set the value of the green component of this color.
	 *
	 * \param newValue new value to set the green color component to.
	 */
	constexpr void setGreenComponent(float newValue) noexcept {
		rgba.y = newValue;
	}

	/**
	 * Set the value of the blue component of this color.
	 *
	 * \param newValue new value to set the blue color component to.
	 */
	constexpr void setBlueComponent(float newValue) noexcept {
		rgba.z = newValue;
	}

	/**
	 * Set the value of the alpha component of this color.
	 *
	 * \param newValue new value to set the alpha component to.
	 */
	constexpr void setAlphaComponent(float newValue) noexcept {
		rgba.w = newValue;
	}

	/**
	 * Get the value of the red component of this color.
	 *
	 * \return the value of the red color component.
	 */
	[[nodiscard]] constexpr float getRedComponent() const noexcept {
		return rgba.x;
	}

	/**
	 * Get the value of the green component of this color.
	 *
	 * \return the value of the green color component.
	 */
	[[nodiscard]] constexpr float getGreenComponent() const noexcept {
		return rgba.y;
	}

	/**
	 * Get the value of the blue component of this color.
	 *
	 * \return the value of the blue color component.
	 */
	[[nodiscard]] constexpr float getBlueComponent() const noexcept {
		return rgba.z;
	}

	/**
	 * Get the value of the alpha component of this color.
	 *
	 * \return the value of the alpha component.
	 */
	[[nodiscard]] constexpr float getAlphaComponent() const noexcept {
		return rgba.w;
	}

private:
	glm::vec4 rgba{0.0f, 0.0f, 0.0f, 0.0f};
};

// clang-format off
inline constexpr Color Color::INVISIBLE                 {  0,   0,   0,   0};
inline constexpr Color Color::ALICE_BLUE                {240, 248, 255, 255}; // #F0F8FF
inline constexpr Color Color::ANTIQUE_WHITE             {250, 235, 215, 255}; // #FAEBD7
inline constexpr Color Color::AQUA                      {  0, 255, 255, 255}; // #00FFFF
inline constexpr Color Color::AQUAMARINE                {127, 255, 212, 255}; // #7FFFD4
inline constexpr Color Color::AZURE                     {240, 255, 255, 255}; // #F0FFFF
inline constexpr Color Color::BEIGE                     {245, 245, 220, 255}; // #F5F5DC
inline constexpr Color Color::BISQUE                    {255, 228, 196, 255}; // #FFE4C4
inline constexpr Color Color::BLACK                     {  0,   0,   0, 255}; // #000000
inline constexpr Color Color::BLANCHED_ALMOND           {255, 235, 205, 255}; // #FFEBCD
inline constexpr Color Color::BLUE                      {  0,   0, 255, 255}; // #0000FF
inline constexpr Color Color::BLUE_VIOLET               {138,  43, 226, 255}; // #8A2BE2
inline constexpr Color Color::BROWN                     {165,  42,  42, 255}; // #A52A2A
inline constexpr Color Color::BURLY_WOOD                {222, 184, 135, 255}; // #DEB887
inline constexpr Color Color::CADET_BLUE                { 95, 158, 160, 255}; // #5F9EA0
inline constexpr Color Color::CHARTREUSE                {127, 255,   0, 255}; // #7FFF00
inline constexpr Color Color::CHOCOLATE                 {210, 105,  30, 255}; // #D2691E
inline constexpr Color Color::CORAL                     {255, 127,  80, 255}; // #FF7F50
inline constexpr Color Color::CORNFLOWER_BLUE           {100, 149, 237, 255}; // #6495ED
inline constexpr Color Color::CORNSILK                  {255, 248, 220, 255}; // #FFF8DC
inline constexpr Color Color::CRIMSON                   {220,  20,  60, 255}; // #DC143C
inline constexpr Color Color::CYAN                      {  0, 255, 255, 255}; // #00FFFF
inline constexpr Color Color::DARK_BLUE                 {  0,   0, 139, 255}; // #00008B
inline constexpr Color Color::DARK_CYAN                 {  0, 139, 139, 255}; // #008B8B
inline constexpr Color Color::DARK_GOLDEN_ROD           {184, 134,  11, 255}; // #B8860B
inline constexpr Color Color::DARK_GRAY                 {169, 169, 169, 255}; // #A9A9A9
inline constexpr Color Color::DARK_GREY                 {169, 169, 169, 255}; // #A9A9A9
inline constexpr Color Color::DARK_GREEN                {  0, 100,   0, 255}; // #006400
inline constexpr Color Color::DARK_KHAKI                {189, 183, 107, 255}; // #BDB76B
inline constexpr Color Color::DARK_MAGENTA              {139,   0, 139, 255}; // #8B008B
inline constexpr Color Color::DARK_OLIVE_GREEN          { 85, 107,  47, 255}; // #556B2F
inline constexpr Color Color::DARK_ORANGE               {255, 140,   0, 255}; // #FF8C00
inline constexpr Color Color::DARK_ORCHID               {153,  50, 204, 255}; // #9932CC
inline constexpr Color Color::DARK_RED                  {139,   0,   0, 255}; // #8B0000
inline constexpr Color Color::DARK_SALMON               {233, 150, 122, 255}; // #E9967A
inline constexpr Color Color::DARK_SEA_GREEN            {143, 188, 143, 255}; // #8FBC8F
inline constexpr Color Color::DARK_SLATE_BLUE           { 72,  61, 139, 255}; // #483D8B
inline constexpr Color Color::DARK_SLATE_GRAY           { 47,  79,  79, 255}; // #2F4F4F
inline constexpr Color Color::DARK_SLATE_GREY           { 47,  79,  79, 255}; // #2F4F4F
inline constexpr Color Color::DARK_TURQUOISE            {  0, 206, 209, 255}; // #00CED1
inline constexpr Color Color::DARK_VIOLET               {148,   0, 211, 255}; // #9400D3
inline constexpr Color Color::DEEP_PINK                 {255,  20, 147, 255}; // #FF1493
inline constexpr Color Color::DEEP_SKY_BLUE             {  0, 191, 255, 255}; // #00BFFF
inline constexpr Color Color::DIM_GRAY                  {105, 105, 105, 255}; // #696969
inline constexpr Color Color::DIM_GREY                  {105, 105, 105, 255}; // #696969
inline constexpr Color Color::DODGER_BLUE               { 30, 144, 255, 255}; // #1E90FF
inline constexpr Color Color::FIRE_BRICK                {178,  34,  34, 255}; // #B22222
inline constexpr Color Color::FLORAL_WHITE              {255, 250, 240, 255}; // #FFFAF0
inline constexpr Color Color::FOREST_GREEN              { 34, 139,  34, 255}; // #228B22
inline constexpr Color Color::FUCHSIA                   {255,   0, 255, 255}; // #FF00FF
inline constexpr Color Color::GAINSBORO                 {220, 220, 220, 255}; // #DCDCDC
inline constexpr Color Color::GHOST_WHITE               {248, 248, 255, 255}; // #F8F8FF
inline constexpr Color Color::GOLD                      {255, 215,   0, 255}; // #FFD700
inline constexpr Color Color::GOLDEN_ROD                {218, 165,  32, 255}; // #DAA520
inline constexpr Color Color::GRAY                      {128, 128, 128, 255}; // #808080
inline constexpr Color Color::GREY                      {128, 128, 128, 255}; // #808080
inline constexpr Color Color::GREEN                     {  0, 128,   0, 255}; // #008000
inline constexpr Color Color::GREEN_YELLOW              {173, 255,  47, 255}; // #ADFF2F
inline constexpr Color Color::HONEY_DEW                 {240, 255, 240, 255}; // #F0FFF0
inline constexpr Color Color::HOT_PINK                  {255, 105, 180, 255}; // #FF69B4
inline constexpr Color Color::INDIAN_RED                {205,  92,  92, 255}; // #CD5C5C
inline constexpr Color Color::INDIGO                    { 75,   0, 130, 255}; // #4B0082
inline constexpr Color Color::IVORY                     {255, 255, 240, 255}; // #FFFFF0
inline constexpr Color Color::KHAKI                     {240, 230, 140, 255}; // #F0E68C
inline constexpr Color Color::LAVENDER                  {230, 230, 250, 255}; // #E6E6FA
inline constexpr Color Color::LAVENDER_BLUSH            {255, 240, 245, 255}; // #FFF0F5
inline constexpr Color Color::LAWN_GREEN                {124, 252,   0, 255}; // #7CFC00
inline constexpr Color Color::LEMON_CHIFFON             {255, 250, 205, 255}; // #FFFACD
inline constexpr Color Color::LIGHT_BLUE                {173, 216, 230, 255}; // #ADD8E6
inline constexpr Color Color::LIGHT_CORAL               {240, 128, 128, 255}; // #F08080
inline constexpr Color Color::LIGHT_CYAN                {224, 255, 255, 255}; // #E0FFFF
inline constexpr Color Color::LIGHT_GOLDEN_ROD_YELLOW   {250, 250, 210, 255}; // #FAFAD2
inline constexpr Color Color::LIGHT_GRAY                {211, 211, 211, 255}; // #D3D3D3
inline constexpr Color Color::LIGHT_GREY                {211, 211, 211, 255}; // #D3D3D3
inline constexpr Color Color::LIGHT_GREEN               {144, 238, 144, 255}; // #90EE90
inline constexpr Color Color::LIGHT_PINK                {255, 182, 193, 255}; // #FFB6C1
inline constexpr Color Color::LIGHT_SALMON              {255, 160, 122, 255}; // #FFA07A
inline constexpr Color Color::LIGHT_SEA_GREEN           { 32, 178, 170, 255}; // #20B2AA
inline constexpr Color Color::LIGHT_SKY_BLUE            {135, 206, 250, 255}; // #87CEFA
inline constexpr Color Color::LIGHT_SLATE_GRAY          {119, 136, 153, 255}; // #778899
inline constexpr Color Color::LIGHT_SLATE_GREY          {119, 136, 153, 255}; // #778899
inline constexpr Color Color::LIGHT_STEEL_BLUE          {176, 196, 222, 255}; // #B0C4DE
inline constexpr Color Color::LIGHT_YELLOW              {255, 255, 224, 255}; // #FFFFE0
inline constexpr Color Color::LIME                      {  0, 255,   0, 255}; // #00FF00
inline constexpr Color Color::LIME_GREEN                { 50, 205,  50, 255}; // #32CD32
inline constexpr Color Color::LINEN                     {250, 240, 230, 255}; // #FAF0E6
inline constexpr Color Color::MAGENTA                   {255,   0, 255, 255}; // #FF00FF
inline constexpr Color Color::MAROON                    {128,   0,   0, 255}; // #800000
inline constexpr Color Color::MEDIUM_AQUA_MARINE        {102, 205, 170, 255}; // #66CDAA
inline constexpr Color Color::MEDIUM_BLUE               {  0,   0, 205, 255}; // #0000CD
inline constexpr Color Color::MEDIUM_ORCHID             {186,  85, 211, 255}; // #BA55D3
inline constexpr Color Color::MEDIUM_PURPLE             {147, 112, 219, 255}; // #9370DB
inline constexpr Color Color::MEDIUM_SEA_GREEN          { 60, 179, 113, 255}; // #3CB371
inline constexpr Color Color::MEDIUM_SLATE_BLUE         {123, 104, 238, 255}; // #7B68EE
inline constexpr Color Color::MEDIUM_SPRING_GREEN       {  0, 250, 154, 255}; // #00FA9A
inline constexpr Color Color::MEDIUM_TURQUOISE          { 72, 209, 204, 255}; // #48D1CC
inline constexpr Color Color::MEDIUM_VIOLET_RED         {199,  21, 133, 255}; // #C71585
inline constexpr Color Color::MIDNIGHT_BLUE             { 25,  25, 112, 255}; // #191970
inline constexpr Color Color::MINT_CREAM                {245, 255, 250, 255}; // #F5FFFA
inline constexpr Color Color::MISTY_ROSE                {255, 228, 225, 255}; // #FFE4E1
inline constexpr Color Color::MOCCASIN                  {255, 228, 181, 255}; // #FFE4B5
inline constexpr Color Color::NAVAJO_WHITE              {255, 222, 173, 255}; // #FFDEAD
inline constexpr Color Color::NAVY                      {  0,   0, 128, 255}; // #000080
inline constexpr Color Color::OLD_LACE                  {253, 245, 230, 255}; // #FDF5E6
inline constexpr Color Color::OLIVE                     {128, 128,   0, 255}; // #808000
inline constexpr Color Color::OLIVE_DRAB                {107, 142,  35, 255}; // #6B8E23
inline constexpr Color Color::ORANGE                    {255, 165,   0, 255}; // #FFA500
inline constexpr Color Color::ORANGE_RED                {255,  69,   0, 255}; // #FF4500
inline constexpr Color Color::ORCHID                    {218, 112, 214, 255}; // #DA70D6
inline constexpr Color Color::PALE_GOLDEN_ROD           {238, 232, 170, 255}; // #EEE8AA
inline constexpr Color Color::PALE_GREEN                {152, 251, 152, 255}; // #98FB98
inline constexpr Color Color::PALE_TURQUOISE            {175, 238, 238, 255}; // #AFEEEE
inline constexpr Color Color::PALE_VIOLET_RED           {219, 112, 147, 255}; // #DB7093
inline constexpr Color Color::PAPAYA_WHIP               {255, 239, 213, 255}; // #FFEFD5
inline constexpr Color Color::PEACH_PUFF                {255, 218, 185, 255}; // #FFDAB9
inline constexpr Color Color::PERU                      {205, 133,  63, 255}; // #CD853F
inline constexpr Color Color::PINK                      {255, 192, 203, 255}; // #FFC0CB
inline constexpr Color Color::PLUM                      {221, 160, 221, 255}; // #DDA0DD
inline constexpr Color Color::POWDER_BLUE               {176, 224, 230, 255}; // #B0E0E6
inline constexpr Color Color::PURPLE                    {128,   0, 128, 255}; // #800080
inline constexpr Color Color::REBECCA_PURPLE            {102,  51, 153, 255}; // #663399
inline constexpr Color Color::RED                       {255,   0,   0, 255}; // #FF0000
inline constexpr Color Color::ROSY_BROWN                {188, 143, 143, 255}; // #BC8F8F
inline constexpr Color Color::ROYAL_BLUE                { 65, 105, 225, 255}; // #4169E1
inline constexpr Color Color::SADDLE_BROWN              {139,  69,  19, 255}; // #8B4513
inline constexpr Color Color::SALMON                    {250, 128, 114, 255}; // #FA8072
inline constexpr Color Color::SANDY_BROWN               {244, 164,  96, 255}; // #F4A460
inline constexpr Color Color::SEA_GREEN                 { 46, 139,  87, 255}; // #2E8B57
inline constexpr Color Color::SEA_SHELL                 {255, 245, 238, 255}; // #FFF5EE
inline constexpr Color Color::SIENNA                    {160,  82,  45, 255}; // #A0522D
inline constexpr Color Color::SILVER                    {192, 192, 192, 255}; // #C0C0C0
inline constexpr Color Color::SKY_BLUE                  {135, 206, 235, 255}; // #87CEEB
inline constexpr Color Color::SLATE_BLUE                {106,  90, 205, 255}; // #6A5ACD
inline constexpr Color Color::SLATE_GRAY                {112, 128, 144, 255}; // #708090
inline constexpr Color Color::SLATE_GREY                {112, 128, 144, 255}; // #708090
inline constexpr Color Color::SNOW                      {255, 250, 250, 255}; // #FFFAFA
inline constexpr Color Color::SPRING_GREEN              {  0, 255, 127, 255}; // #00FF7F
inline constexpr Color Color::STEEL_BLUE                { 70, 130, 180, 255}; // #4682B4
inline constexpr Color Color::TAN                       {210, 180, 140, 255}; // #D2B48C
inline constexpr Color Color::TEAL                      {  0, 128, 128, 255}; // #008080
inline constexpr Color Color::THISTLE                   {216, 191, 216, 255}; // #D8BFD8
inline constexpr Color Color::TOMATO                    {255,  99,  71, 255}; // #FF6347
inline constexpr Color Color::TURQUOISE                 { 64, 224, 208, 255}; // #40E0D0
inline constexpr Color Color::VIOLET                    {238, 130, 238, 255}; // #EE82EE
inline constexpr Color Color::WHEAT                     {245, 222, 179, 255}; // #F5DEB3
inline constexpr Color Color::WHITE                     {255, 255, 255, 255}; // #FFFFFF
inline constexpr Color Color::WHITE_SMOKE               {245, 245, 245, 255}; // #F5F5F5
inline constexpr Color Color::YELLOW                    {255, 255,   0, 255}; // #FFFF00
inline constexpr Color Color::YELLOW_GREEN              {154, 205,  50, 255}; // #9ACD32
// clang-format on

} // namespace donut

#endif
