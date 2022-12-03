#ifndef DONUT_COLOR_HPP
#define DONUT_COLOR_HPP

#include <glm/glm.hpp>

namespace donut {

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

	constexpr Color() noexcept = default;

	constexpr Color(float r, float g, float b, float a = 1.0f)
		: rgba(r, g, b, a) {}

	constexpr Color(glm::vec3 rgb)
		: rgba(rgb, 1.0f) {}

	constexpr Color(glm::vec4 rgba)
		: rgba(rgba) {}

	constexpr operator glm::vec4() const noexcept {
		return rgba;
	}

	constexpr Color& operator+=(const Color& other) noexcept {
		rgba += other.rgba;
		return *this;
	}

	constexpr Color& operator-=(const Color& other) noexcept {
		rgba -= other.rgba;
		return *this;
	}

	constexpr Color& operator*=(const Color& other) noexcept {
		rgba *= other.rgba;
		return *this;
	}

	constexpr Color& operator/=(const Color& other) noexcept {
		rgba /= other.rgba;
		return *this;
	}

	constexpr Color& operator*=(float scalar) noexcept {
		rgba *= scalar;
		return *this;
	}

	constexpr Color& operator/=(float scalar) noexcept {
		rgba /= scalar;
		return *this;
	}

	[[nodiscard]] friend Color operator+(const Color& a) {
		return a;
	}

	[[nodiscard]] friend Color operator-(const Color& a) {
		return Color{-a.rgba};
	}

	[[nodiscard]] friend Color operator+(const Color& a, const Color& b) {
		return Color{a.rgba + b.rgba};
	}

	[[nodiscard]] friend Color operator-(const Color& a, const Color& b) {
		return Color{a.rgba - b.rgba};
	}

	[[nodiscard]] friend Color operator*(const Color& a, const Color& b) {
		return Color{a.rgba * b.rgba};
	}

	[[nodiscard]] friend Color operator/(const Color& a, const Color& b) {
		return Color{a.rgba / b.rgba};
	}

	[[nodiscard]] friend Color operator*(const Color& a, float b) {
		return Color{a.rgba * b};
	}

	[[nodiscard]] friend Color operator*(float a, const Color& b) {
		return Color{a * b.rgba};
	}

	[[nodiscard]] friend Color operator/(const Color& a, float b) {
		return Color{a.rgba / b};
	}

	[[nodiscard]] friend Color operator/(float a, const Color& b) {
		return Color{a / b.rgba};
	}

	[[nodiscard]] constexpr float getRedComponent() const noexcept {
		return rgba.x;
	}

	[[nodiscard]] constexpr float getGreenComponent() const noexcept {
		return rgba.y;
	}

	[[nodiscard]] constexpr float getBlueComponent() const noexcept {
		return rgba.z;
	}

	[[nodiscard]] constexpr float getAlphaComponent() const noexcept {
		return rgba.w;
	}

private:
	glm::vec4 rgba{0.0f, 0.0f, 0.0f, 0.0f};
};

// clang-format off
inline constexpr Color Color::INVISIBLE                 {0.0f,      0.0f,      0.0f,      0.0f};
inline constexpr Color Color::ALICE_BLUE                {0.941176f, 0.972549f, 1.0f,      1.0f}; // #F0F8FF (240, 248, 255)
inline constexpr Color Color::ANTIQUE_WHITE             {0.980392f, 0.921569f, 0.843137f, 1.0f}; // #FAEBD7 (250, 235, 215)
inline constexpr Color Color::AQUA                      {0.0f,      1.0f,      1.0f,      1.0f}; // #00FFFF (  0, 255, 255)
inline constexpr Color Color::AQUAMARINE                {0.498039f, 1.0f,      0.831373f, 1.0f}; // #7FFFD4 (127, 255, 212)
inline constexpr Color Color::AZURE                     {0.941176f, 1.0f,      1.0f,      1.0f}; // #F0FFFF (240, 255, 255)
inline constexpr Color Color::BEIGE                     {0.960784f, 0.960784f, 0.862745f, 1.0f}; // #F5F5DC (245, 245, 220)
inline constexpr Color Color::BISQUE                    {1.0f,      0.894118f, 0.768627f, 1.0f}; // #FFE4C4 (255, 228, 196)
inline constexpr Color Color::BLACK                     {0.0f,      0.0f,      0.0f,      1.0f}; // #000000 (  0,   0,   0)
inline constexpr Color Color::BLANCHED_ALMOND           {1.0f,      0.921569f, 0.803922f, 1.0f}; // #FFEBCD (255, 235, 205)
inline constexpr Color Color::BLUE                      {0.0f,      0.0f,      1.0f,      1.0f}; // #0000FF (  0,   0, 255)
inline constexpr Color Color::BLUE_VIOLET               {0.541176f, 0.168627f, 0.886275f, 1.0f}; // #8A2BE2 (138,  43, 226)
inline constexpr Color Color::BROWN                     {0.647059f, 0.164706f, 0.164706f, 1.0f}; // #A52A2A (165,  42,  42)
inline constexpr Color Color::BURLY_WOOD                {0.870588f, 0.721569f, 0.529412f, 1.0f}; // #DEB887 (222, 184, 135)
inline constexpr Color Color::CADET_BLUE                {0.372549f, 0.619608f, 0.627451f, 1.0f}; // #5F9EA0 ( 95, 158, 160)
inline constexpr Color Color::CHARTREUSE                {0.498039f, 1.0f,      0.0f,      1.0f}; // #7FFF00 (127, 255,   0)
inline constexpr Color Color::CHOCOLATE                 {0.823529f, 0.411765f, 0.117647f, 1.0f}; // #D2691E (210, 105,  30)
inline constexpr Color Color::CORAL                     {1.0f,      0.498039f, 0.313725f, 1.0f}; // #FF7F50 (255, 127,  80)
inline constexpr Color Color::CORNFLOWER_BLUE           {0.392157f, 0.584314f, 0.929412f, 1.0f}; // #6495ED (100, 149, 237)
inline constexpr Color Color::CORNSILK                  {1.0f,      0.972549f, 0.862745f, 1.0f}; // #FFF8DC (255, 248, 220)
inline constexpr Color Color::CRIMSON                   {0.862745f, 0.078431f, 0.235294f, 1.0f}; // #DC143C (220,  20,  60)
inline constexpr Color Color::CYAN                      {0.0f,      1.0f,      1.0f,      1.0f}; // #00FFFF (  0, 255, 255)
inline constexpr Color Color::DARK_BLUE                 {0.0f,      0.0f,      0.545098f, 1.0f}; // #00008B (  0,   0, 139)
inline constexpr Color Color::DARK_CYAN                 {0.0f,      0.545098f, 0.545098f, 1.0f}; // #008B8B (  0, 139, 139)
inline constexpr Color Color::DARK_GOLDEN_ROD           {0.721569f, 0.52549f,  0.043137f, 1.0f}; // #B8860B (184, 134,  11)
inline constexpr Color Color::DARK_GRAY                 {0.662745f, 0.662745f, 0.662745f, 1.0f}; // #A9A9A9 (169, 169, 169)
inline constexpr Color Color::DARK_GREY                 {0.662745f, 0.662745f, 0.662745f, 1.0f}; // #A9A9A9 (169, 169, 169)
inline constexpr Color Color::DARK_GREEN                {0.0f,      0.392157f, 0.0f,      1.0f}; // #006400 (  0, 100,   0)
inline constexpr Color Color::DARK_KHAKI                {0.741176f, 0.717647f, 0.419608f, 1.0f}; // #BDB76B (189, 183, 107)
inline constexpr Color Color::DARK_MAGENTA              {0.545098f, 0.0f,      0.545098f, 1.0f}; // #8B008B (139,   0, 139)
inline constexpr Color Color::DARK_OLIVE_GREEN          {0.333333f, 0.419608f, 0.184314f, 1.0f}; // #556B2F ( 85, 107,  47)
inline constexpr Color Color::DARK_ORANGE               {1.0f,      0.54902f,  0.0f,      1.0f}; // #FF8C00 (255, 140,   0)
inline constexpr Color Color::DARK_ORCHID               {0.6f,      0.196078f, 0.8f,      1.0f}; // #9932CC (153,  50, 204)
inline constexpr Color Color::DARK_RED                  {0.545098f, 0.0f,      0.0f,      1.0f}; // #8B0000 (139,   0,   0)
inline constexpr Color Color::DARK_SALMON               {0.913725f, 0.588235f, 0.478431f, 1.0f}; // #E9967A (233, 150, 122)
inline constexpr Color Color::DARK_SEA_GREEN            {0.560784f, 0.737255f, 0.560784f, 1.0f}; // #8FBC8F (143, 188, 143)
inline constexpr Color Color::DARK_SLATE_BLUE           {0.282353f, 0.239216f, 0.545098f, 1.0f}; // #483D8B ( 72,  61, 139)
inline constexpr Color Color::DARK_SLATE_GRAY           {0.184314f, 0.309804f, 0.309804f, 1.0f}; // #2F4F4F ( 47,  79,  79)
inline constexpr Color Color::DARK_SLATE_GREY           {0.184314f, 0.309804f, 0.309804f, 1.0f}; // #2F4F4F ( 47,  79,  79)
inline constexpr Color Color::DARK_TURQUOISE            {0.0f,      0.807843f, 0.819608f, 1.0f}; // #00CED1 (  0, 206, 209)
inline constexpr Color Color::DARK_VIOLET               {0.580392f, 0.0f,      0.827451f, 1.0f}; // #9400D3 (148,   0, 211)
inline constexpr Color Color::DEEP_PINK                 {1.0f,      0.078431f, 0.576471f, 1.0f}; // #FF1493 (255,  20, 147)
inline constexpr Color Color::DEEP_SKY_BLUE             {0.0f,      0.74902f,  1.0f,      1.0f}; // #00BFFF (  0, 191, 255)
inline constexpr Color Color::DIM_GRAY                  {0.411765f, 0.411765f, 0.411765f, 1.0f}; // #696969 (105, 105, 105)
inline constexpr Color Color::DIM_GREY                  {0.411765f, 0.411765f, 0.411765f, 1.0f}; // #696969 (105, 105, 105)
inline constexpr Color Color::DODGER_BLUE               {0.117647f, 0.564706f, 1.0f,      1.0f}; // #1E90FF ( 30, 144, 255)
inline constexpr Color Color::FIRE_BRICK                {0.698039f, 0.133333f, 0.133333f, 1.0f}; // #B22222 (178,  34,  34)
inline constexpr Color Color::FLORAL_WHITE              {1.0f,      0.980392f, 0.941176f, 1.0f}; // #FFFAF0 (255, 250, 240)
inline constexpr Color Color::FOREST_GREEN              {0.133333f, 0.545098f, 0.133333f, 1.0f}; // #228B22 ( 34, 139,  34)
inline constexpr Color Color::FUCHSIA                   {1.0f,      0.0f,      1.0f,      1.0f}; // #FF00FF (255,   0, 255)
inline constexpr Color Color::GAINSBORO                 {0.862745f, 0.862745f, 0.862745f, 1.0f}; // #DCDCDC (220, 220, 220)
inline constexpr Color Color::GHOST_WHITE               {0.972549f, 0.972549f, 1.0f,      1.0f}; // #F8F8FF (248, 248, 255)
inline constexpr Color Color::GOLD                      {1.0f,      0.843137f, 0.0f,      1.0f}; // #FFD700 (255, 215,   0)
inline constexpr Color Color::GOLDEN_ROD                {0.854902f, 0.647059f, 0.12549f,  1.0f}; // #DAA520 (218, 165,  32)
inline constexpr Color Color::GRAY                      {0.501961f, 0.501961f, 0.501961f, 1.0f}; // #808080 (128, 128, 128)
inline constexpr Color Color::GREY                      {0.501961f, 0.501961f, 0.501961f, 1.0f}; // #808080 (128, 128, 128)
inline constexpr Color Color::GREEN                     {0.0f,      0.501961f, 0.0f,      1.0f}; // #008000 (  0, 128,   0)
inline constexpr Color Color::GREEN_YELLOW              {0.678431f, 1.0f,      0.184314f, 1.0f}; // #ADFF2F (173, 255,  47)
inline constexpr Color Color::HONEY_DEW                 {0.941176f, 1.0f,      0.941176f, 1.0f}; // #F0FFF0 (240, 255, 240)
inline constexpr Color Color::HOT_PINK                  {1.0f,      0.411765f, 0.705882f, 1.0f}; // #FF69B4 (255, 105, 180)
inline constexpr Color Color::INDIAN_RED                {0.803922f, 0.360784f, 0.360784f, 1.0f}; // #CD5C5C (205,  92,  92)
inline constexpr Color Color::INDIGO                    {0.294118f, 0.0f,      0.509804f, 1.0f}; // #4B0082 ( 75,   0, 130)
inline constexpr Color Color::IVORY                     {1.0f,      1.0f,      0.941176f, 1.0f}; // #FFFFF0 (255, 255, 240)
inline constexpr Color Color::KHAKI                     {0.941176f, 0.901961f, 0.54902f,  1.0f}; // #F0E68C (240, 230, 140)
inline constexpr Color Color::LAVENDER                  {0.901961f, 0.901961f, 0.980392f, 1.0f}; // #E6E6FA (230, 230, 250)
inline constexpr Color Color::LAVENDER_BLUSH            {1.0f,      0.941176f, 0.960784f, 1.0f}; // #FFF0F5 (255, 240, 245)
inline constexpr Color Color::LAWN_GREEN                {0.486275f, 0.988235f, 0.0f,      1.0f}; // #7CFC00 (124, 252,   0)
inline constexpr Color Color::LEMON_CHIFFON             {1.0f,      0.980392f, 0.803922f, 1.0f}; // #FFFACD (255, 250, 205)
inline constexpr Color Color::LIGHT_BLUE                {0.678431f, 0.847059f, 0.901961f, 1.0f}; // #ADD8E6 (173, 216, 230)
inline constexpr Color Color::LIGHT_CORAL               {0.941176f, 0.501961f, 0.501961f, 1.0f}; // #F08080 (240, 128, 128)
inline constexpr Color Color::LIGHT_CYAN                {0.878431f, 1.0f,      1.0f,      1.0f}; // #E0FFFF (224, 255, 255)
inline constexpr Color Color::LIGHT_GOLDEN_ROD_YELLOW   {0.980392f, 0.980392f, 0.823529f, 1.0f}; // #FAFAD2 (250, 250, 210)
inline constexpr Color Color::LIGHT_GRAY                {0.827451f, 0.827451f, 0.827451f, 1.0f}; // #D3D3D3 (211, 211, 211)
inline constexpr Color Color::LIGHT_GREY                {0.827451f, 0.827451f, 0.827451f, 1.0f}; // #D3D3D3 (211, 211, 211)
inline constexpr Color Color::LIGHT_GREEN               {0.564706f, 0.933333f, 0.564706f, 1.0f}; // #90EE90 (144, 238, 144)
inline constexpr Color Color::LIGHT_PINK                {1.0f,      0.713725f, 0.756863f, 1.0f}; // #FFB6C1 (255, 182, 193)
inline constexpr Color Color::LIGHT_SALMON              {1.0f,      0.627451f, 0.478431f, 1.0f}; // #FFA07A (255, 160, 122)
inline constexpr Color Color::LIGHT_SEA_GREEN           {0.12549f,  0.698039f, 0.666667f, 1.0f}; // #20B2AA ( 32, 178, 170)
inline constexpr Color Color::LIGHT_SKY_BLUE            {0.529412f, 0.807843f, 0.980392f, 1.0f}; // #87CEFA (135, 206, 250)
inline constexpr Color Color::LIGHT_SLATE_GRAY          {0.466667f, 0.533333f, 0.6f,      1.0f}; // #778899 (119, 136, 153)
inline constexpr Color Color::LIGHT_SLATE_GREY          {0.466667f, 0.533333f, 0.6f,      1.0f}; // #778899 (119, 136, 153)
inline constexpr Color Color::LIGHT_STEEL_BLUE          {0.690196f, 0.768627f, 0.870588f, 1.0f}; // #B0C4DE (176, 196, 222)
inline constexpr Color Color::LIGHT_YELLOW              {1.0f,      1.0f,      0.878431f, 1.0f}; // #FFFFE0 (255, 255, 224)
inline constexpr Color Color::LIME                      {0.0f,      1.0f,      0.0f,      1.0f}; // #00FF00 (  0, 255,   0)
inline constexpr Color Color::LIME_GREEN                {0.196078f, 0.803922f, 0.196078f, 1.0f}; // #32CD32 ( 50, 205,  50)
inline constexpr Color Color::LINEN                     {0.980392f, 0.941176f, 0.901961f, 1.0f}; // #FAF0E6 (250, 240, 230)
inline constexpr Color Color::MAGENTA                   {1.0f,      0.0f,      1.0f,      1.0f}; // #FF00FF (255,   0, 255)
inline constexpr Color Color::MAROON                    {0.501961f, 0.0f,      0.0f,      1.0f}; // #800000 (128,   0,   0)
inline constexpr Color Color::MEDIUM_AQUA_MARINE        {0.4f,      0.803922f, 0.666667f, 1.0f}; // #66CDAA (102, 205, 170)
inline constexpr Color Color::MEDIUM_BLUE               {0.0f,      0.0f,      0.803922f, 1.0f}; // #0000CD (  0,   0, 205)
inline constexpr Color Color::MEDIUM_ORCHID             {0.729412f, 0.333333f, 0.827451f, 1.0f}; // #BA55D3 (186,  85, 211)
inline constexpr Color Color::MEDIUM_PURPLE             {0.576471f, 0.439216f, 0.858824f, 1.0f}; // #9370DB (147, 112, 219)
inline constexpr Color Color::MEDIUM_SEA_GREEN          {0.235294f, 0.701961f, 0.443137f, 1.0f}; // #3CB371 ( 60, 179, 113)
inline constexpr Color Color::MEDIUM_SLATE_BLUE         {0.482353f, 0.407843f, 0.933333f, 1.0f}; // #7B68EE (123, 104, 238)
inline constexpr Color Color::MEDIUM_SPRING_GREEN       {0.0f,      0.980392f, 0.603922f, 1.0f}; // #00FA9A (  0, 250, 154)
inline constexpr Color Color::MEDIUM_TURQUOISE          {0.282353f, 0.819608f, 0.8f,      1.0f}; // #48D1CC ( 72, 209, 204)
inline constexpr Color Color::MEDIUM_VIOLET_RED         {0.780392f, 0.082353f, 0.521569f, 1.0f}; // #C71585 (199,  21, 133)
inline constexpr Color Color::MIDNIGHT_BLUE             {0.098039f, 0.098039f, 0.439216f, 1.0f}; // #191970 ( 25,  25, 112)
inline constexpr Color Color::MINT_CREAM                {0.960784f, 1.0f,      0.980392f, 1.0f}; // #F5FFFA (245, 255, 250)
inline constexpr Color Color::MISTY_ROSE                {1.0f,      0.894118f, 0.882353f, 1.0f}; // #FFE4E1 (255, 228, 225)
inline constexpr Color Color::MOCCASIN                  {1.0f,      0.894118f, 0.709804f, 1.0f}; // #FFE4B5 (255, 228, 181)
inline constexpr Color Color::NAVAJO_WHITE              {1.0f,      0.870588f, 0.678431f, 1.0f}; // #FFDEAD (255, 222, 173)
inline constexpr Color Color::NAVY                      {0.0f,      0.0f,      0.501961f, 1.0f}; // #000080 (  0,   0, 128)
inline constexpr Color Color::OLD_LACE                  {0.992157f, 0.960784f, 0.901961f, 1.0f}; // #FDF5E6 (253, 245, 230)
inline constexpr Color Color::OLIVE                     {0.501961f, 0.501961f, 0.0f,      1.0f}; // #808000 (128, 128,   0)
inline constexpr Color Color::OLIVE_DRAB                {0.419608f, 0.556863f, 0.137255f, 1.0f}; // #6B8E23 (107, 142,  35)
inline constexpr Color Color::ORANGE                    {1.0f,      0.647059f, 0.0f,      1.0f}; // #FFA500 (255, 165,   0)
inline constexpr Color Color::ORANGE_RED                {1.0f,      0.270588f, 0.0f,      1.0f}; // #FF4500 (255,  69,   0)
inline constexpr Color Color::ORCHID                    {0.854902f, 0.439216f, 0.839216f, 1.0f}; // #DA70D6 (218, 112, 214)
inline constexpr Color Color::PALE_GOLDEN_ROD           {0.933333f, 0.909804f, 0.666667f, 1.0f}; // #EEE8AA (238, 232, 170)
inline constexpr Color Color::PALE_GREEN                {0.596078f, 0.984314f, 0.596078f, 1.0f}; // #98FB98 (152, 251, 152)
inline constexpr Color Color::PALE_TURQUOISE            {0.686275f, 0.933333f, 0.933333f, 1.0f}; // #AFEEEE (175, 238, 238)
inline constexpr Color Color::PALE_VIOLET_RED           {0.858824f, 0.439216f, 0.576471f, 1.0f}; // #DB7093 (219, 112, 147)
inline constexpr Color Color::PAPAYA_WHIP               {1.0f,      0.937255f, 0.835294f, 1.0f}; // #FFEFD5 (255, 239, 213)
inline constexpr Color Color::PEACH_PUFF                {1.0f,      0.854902f, 0.72549f,  1.0f}; // #FFDAB9 (255, 218, 185)
inline constexpr Color Color::PERU                      {0.803922f, 0.521569f, 0.247059f, 1.0f}; // #CD853F (205, 133,  63)
inline constexpr Color Color::PINK                      {1.0f,      0.752941f, 0.796078f, 1.0f}; // #FFC0CB (255, 192, 203)
inline constexpr Color Color::PLUM                      {0.866667f, 0.627451f, 0.866667f, 1.0f}; // #DDA0DD (221, 160, 221)
inline constexpr Color Color::POWDER_BLUE               {0.690196f, 0.878431f, 0.901961f, 1.0f}; // #B0E0E6 (176, 224, 230)
inline constexpr Color Color::PURPLE                    {0.501961f, 0.0f,      0.501961f, 1.0f}; // #800080 (128,   0, 128)
inline constexpr Color Color::REBECCA_PURPLE            {0.4f,      0.2f,      0.6f,      1.0f}; // #663399 (102,  51, 153)
inline constexpr Color Color::RED                       {1.0f,      0.0f,      0.0f,      1.0f}; // #FF0000 (255,   0,   0)
inline constexpr Color Color::ROSY_BROWN                {0.737255f, 0.560784f, 0.560784f, 1.0f}; // #BC8F8F (188, 143, 143)
inline constexpr Color Color::ROYAL_BLUE                {0.254902f, 0.411765f, 0.882353f, 1.0f}; // #4169E1 ( 65, 105, 225)
inline constexpr Color Color::SADDLE_BROWN              {0.545098f, 0.270588f, 0.07451f,  1.0f}; // #8B4513 (139,  69,  19)
inline constexpr Color Color::SALMON                    {0.980392f, 0.501961f, 0.447059f, 1.0f}; // #FA8072 (250, 128, 114)
inline constexpr Color Color::SANDY_BROWN               {0.956863f, 0.643137f, 0.376471f, 1.0f}; // #F4A460 (244, 164,  96)
inline constexpr Color Color::SEA_GREEN                 {0.180392f, 0.545098f, 0.341176f, 1.0f}; // #2E8B57 ( 46, 139,  87)
inline constexpr Color Color::SEA_SHELL                 {1.0f,      0.960784f, 0.933333f, 1.0f}; // #FFF5EE (255, 245, 238)
inline constexpr Color Color::SIENNA                    {0.627451f, 0.321569f, 0.176471f, 1.0f}; // #A0522D (160,  82,  45)
inline constexpr Color Color::SILVER                    {0.752941f, 0.752941f, 0.752941f, 1.0f}; // #C0C0C0 (192, 192, 192)
inline constexpr Color Color::SKY_BLUE                  {0.529412f, 0.807843f, 0.921569f, 1.0f}; // #87CEEB (135, 206, 235)
inline constexpr Color Color::SLATE_BLUE                {0.415686f, 0.352941f, 0.803922f, 1.0f}; // #6A5ACD (106,  90, 205)
inline constexpr Color Color::SLATE_GRAY                {0.439216f, 0.501961f, 0.564706f, 1.0f}; // #708090 (112, 128, 144)
inline constexpr Color Color::SLATE_GREY                {0.439216f, 0.501961f, 0.564706f, 1.0f}; // #708090 (112, 128, 144)
inline constexpr Color Color::SNOW                      {1.0f,      0.980392f, 0.980392f, 1.0f}; // #FFFAFA (255, 250, 250)
inline constexpr Color Color::SPRING_GREEN              {0.0f,      1.0f,      0.498039f, 1.0f}; // #00FF7F (  0, 255, 127)
inline constexpr Color Color::STEEL_BLUE                {0.27451f,  0.509804f, 0.705882f, 1.0f}; // #4682B4 ( 70, 130, 180)
inline constexpr Color Color::TAN                       {0.823529f, 0.705882f, 0.54902f,  1.0f}; // #D2B48C (210, 180, 140)
inline constexpr Color Color::TEAL                      {0.0f,      0.501961f, 0.501961f, 1.0f}; // #008080 (  0, 128, 128)
inline constexpr Color Color::THISTLE                   {0.847059f, 0.74902f,  0.847059f, 1.0f}; // #D8BFD8 (216, 191, 216)
inline constexpr Color Color::TOMATO                    {1.0f,      0.388235f, 0.278431f, 1.0f}; // #FF6347 (255,  99,  71)
inline constexpr Color Color::TURQUOISE                 {0.25098f,  0.878431f, 0.815686f, 1.0f}; // #40E0D0 ( 64, 224, 208)
inline constexpr Color Color::VIOLET                    {0.933333f, 0.509804f, 0.933333f, 1.0f}; // #EE82EE (238, 130, 238)
inline constexpr Color Color::WHEAT                     {0.960784f, 0.870588f, 0.701961f, 1.0f}; // #F5DEB3 (245, 222, 179)
inline constexpr Color Color::WHITE                     {1.0f,      1.0f,      1.0f,      1.0f}; // #FFFFFF (255, 255, 255)
inline constexpr Color Color::WHITE_SMOKE               {0.960784f, 0.960784f, 0.960784f, 1.0f}; // #F5F5F5 (245, 245, 245)
inline constexpr Color Color::YELLOW                    {1.0f,      1.0f,      0.0f,      1.0f}; // #FFFF00 (255, 255,   0)
inline constexpr Color Color::YELLOW_GREEN              {0.603922f, 0.803922f, 0.196078f, 1.0f}; // #9ACD32 (154, 205,  50)
// clang-format on

} // namespace donut

#endif
