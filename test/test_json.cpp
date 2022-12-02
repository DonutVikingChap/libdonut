#include <donut/json.hpp>

#include <cmath>
#include <fmt/format.h>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

namespace json = donut::json;

namespace {

[[nodiscard]] json::Value parseJSONValueFromString(std::u8string_view jsonString) {
	try {
		return json::Value::parse(jsonString);
	} catch (const json::Error& e) {
		throw std::runtime_error{fmt::format("Line {}, Column {}: {}", e.source.lineNumber, e.source.columnNumber, e.what())};
	}
}

[[nodiscard]] json::Value parseJSONValueFromInputStream(std::string jsonString) {
	jsonString.append("Can't-touch-this");
	std::istringstream stream{std::move(jsonString)};
	stream.exceptions(std::istringstream::failbit | std::istringstream::badbit);
	try {
		json::Value result{};
		std::string subsequentString{};
		stream >> result >> subsequentString;
		CHECK(subsequentString == "Can't-touch-this");
		return result;
	} catch (const json::Error& e) {
		throw std::runtime_error{fmt::format("Line {}, Column {}: {}", e.source.lineNumber, e.source.columnNumber, e.what())};
	}
}

} // namespace

TEST_SUITE("JSON") {
	TEST_CASE("Parse UTF-8 string") {
		SUBCASE("Valid input - Kitchen sink") {
			const json::Value value = parseJSONValueFromString(
				u8"\n"
				"// This is a single line comment.\n"
				"\n"
				"/* This is a multi-\n"
				"line comment. */\n"
				"\n"
				"{\n"
				"  // comments\n"
				"  unquoted: 'and you can quote me on that',\n"
				"  singleQuotes: 'I can use \"double quotes\" here',\n"
				"  lineBreaks: \"Look, Mom! \\\n"
				"No \\\\n's!\",\n"
				"  hexadecimal: 0xdecaf,\n"
				"  leadingDecimalPoint: .8675309, andTrailing: 8675309.,\n"
				"  positiveSign: +1,\n"
				"  trailingComma: 'in objects', andIn: ['arrays',],\n"
				"  \"backwardsCompatible\": \"with JSON\",\n"
				"}\n");
			const json::Value expectedValue = json::Object{
				{"unquoted", "and you can quote me on that"},
				{"singleQuotes", "I can use \"double quotes\" here"},
				{"lineBreaks", "Look, Mom! No \\n's!"},
				{"hexadecimal", 0xdecaf},
				{"leadingDecimalPoint", .8675309},
				{"andTrailing", 8675309.},
				{"positiveSign", +1},
				{"trailingComma", "in objects"},
				{"andIn", json::Array{"arrays"}},
				{"backwardsCompatible", "with JSON"},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Empty object") {
			const json::Value value = parseJSONValueFromString(u8"{}");
			const json::Value expectedValue = json::Object{};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Object with two properties and a trailing comma") {
			const json::Value value = parseJSONValueFromString(
				u8"{\n"
				"    width: 1920,\n"
				"    height: 1080,\n"
				"}");
			const json::Value expectedValue = json::Object{
				{"width", 1920},
				{"height", 1080},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Nested objects") {
			const json::Value value = parseJSONValueFromString(
				u8"{\n"
				"    image: {\n"
				"        width: 1920,\n"
				"        height: 1080,\n"
				"        'aspect-ratio': '16:9',\n"
				"    }\n"
				"}");
			const json::Value expectedValue = json::Object{
				{"image", json::Object{{"width", 1920}, {"height", 1080}, {"aspect-ratio", "16:9"}}},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Array of objects") {
			const json::Value value = parseJSONValueFromString(
				u8"[\n"
				"    { name: 'Joe', age: 27 },\n"
				"    { name: 'Jane', age: 32 },\n"
				"]");
			const json::Value expectedValue = json::Array{
				json::Object{{"name", "Joe"}, {"age", 27}},
				json::Object{{"name", "Jane"}, {"age", 32}},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Empty array") {
			const json::Value value = parseJSONValueFromString(u8"[]");
			const json::Value expectedValue = json::Array{};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Array with three elements and a trailing comma") {
			const json::Value value = parseJSONValueFromString(
				u8"[\n"
				"    1,\n"
				"    true,\n"
				"    'three',\n"
				"]");
			const json::Value expectedValue = json::Array{
				1,
				true,
				"three",
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Nested arrays") {
			const json::Value value = parseJSONValueFromString(
				u8"[\n"
				"    [1, true, 'three'],\n"
				"    [4, \"five\", 0x6],\n"
				"]");
			const json::Value expectedValue = json::Array{
				json::Array{1, true, "three"},
				json::Array{4, "five", 0x6},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Line terminator in string") {
			const json::Value value = parseJSONValueFromString(
				u8"'Lorem ipsum dolor sit amet, \\\n"
				"consectetur adipiscing elit.'");
			const json::Value expectedValue = json::String{"Lorem ipsum dolor sit amet, consectetur adipiscing elit."};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Arbitrary escaped characters") {
			const json::Value value = parseJSONValueFromString(u8"'\\A\\C\\/\\D\\C'");
			const json::Value expectedValue = json::String{"AC/DC"};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Numbers") {
			const json::Value value = parseJSONValueFromString(
				u8"{\n"
				"    integer: 123,\n"
				"    withFractionPart: 123.45,\n"
				"    onlyFractionPart: .45,\n"
				"    withExponent: 123e-45,\n"
				"}");
			const json::Value expectedValue = json::Object{
				{"integer", 123},
				{"withFractionPart", 123.45},
				{"onlyFractionPart", .45},
				{"withExponent", 123e-45},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Hexadecimal numbers") {
			const json::Value value = parseJSONValueFromString(
				u8"{\n"
				"    positiveHex: 0xdecaf,\n"
				"    negativeHex: -0xC0FFEE,\n"
				"}");
			const json::Value expectedValue = json::Object{
				{"positiveHex", 0xdecaf},
				{"negativeHex", -0xC0FFEE},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Infinity and NaN") {
			const json::Value value = parseJSONValueFromString(
				u8"{\n"
				"    positiveInfinity: Infinity,\n"
				"    negativeInfinity: -Infinity,\n"
				"    notANumber: NaN,\n"
				"}");
			CHECK(std::isinf(value.get<json::Object>().at("positiveInfinity").get<json::Number>()));
			CHECK(!std::signbit(value.get<json::Object>().at("positiveInfinity").get<json::Number>()));
			CHECK(std::isinf(value.get<json::Object>().at("negativeInfinity").get<json::Number>()));
			CHECK(std::signbit(value.get<json::Object>().at("negativeInfinity").get<json::Number>()));
			CHECK(std::isnan(value.get<json::Object>().at("notANumber").get<json::Number>()));
			CHECK(!std::signbit(value.get<json::Object>().at("notANumber").get<json::Number>()));
		}
	}

	TEST_CASE("Parse ASCII input stream") {
		SUBCASE("Valid input - Kitchen sink") {
			const json::Value value = parseJSONValueFromInputStream(
				"\n"
				"// This is a single line comment.\n"
				"\n"
				"/* This is a multi-\n"
				"line comment. */\n"
				"\n"
				"{\n"
				"  // comments\n"
				"  unquoted: 'and you can quote me on that',\n"
				"  singleQuotes: 'I can use \"double quotes\" here',\n"
				"  lineBreaks: \"Look, Mom! \\\n"
				"No \\\\n's!\",\n"
				"  hexadecimal: 0xdecaf,\n"
				"  leadingDecimalPoint: .8675309, andTrailing: 8675309.,\n"
				"  positiveSign: +1,\n"
				"  trailingComma: 'in objects', andIn: ['arrays',],\n"
				"  \"backwardsCompatible\": \"with JSON\",\n"
				"}\n");
			const json::Value expectedValue = json::Object{
				{"unquoted", "and you can quote me on that"},
				{"singleQuotes", "I can use \"double quotes\" here"},
				{"lineBreaks", "Look, Mom! No \\n's!"},
				{"hexadecimal", 0xdecaf},
				{"leadingDecimalPoint", .8675309},
				{"andTrailing", 8675309.},
				{"positiveSign", +1},
				{"trailingComma", "in objects"},
				{"andIn", json::Array{"arrays"}},
				{"backwardsCompatible", "with JSON"},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Empty object") {
			const json::Value value = parseJSONValueFromInputStream("{}");
			const json::Value expectedValue = json::Object{};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Object with two properties and a trailing comma") {
			const json::Value value = parseJSONValueFromInputStream(
				"{\n"
				"    width: 1920,\n"
				"    height: 1080,\n"
				"}");
			const json::Value expectedValue = json::Object{
				{"width", 1920},
				{"height", 1080},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Nested objects") {
			const json::Value value = parseJSONValueFromInputStream(
				"{\n"
				"    image: {\n"
				"        width: 1920,\n"
				"        height: 1080,\n"
				"        'aspect-ratio': '16:9',\n"
				"    }\n"
				"}");
			const json::Value expectedValue = json::Object{
				{"image", json::Object{{"width", 1920}, {"height", 1080}, {"aspect-ratio", "16:9"}}},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Array of objects") {
			const json::Value value = parseJSONValueFromInputStream(
				"[\n"
				"    { name: 'Joe', age: 27 },\n"
				"    { name: 'Jane', age: 32 },\n"
				"]");
			const json::Value expectedValue = json::Array{
				json::Object{{"name", "Joe"}, {"age", 27}},
				json::Object{{"name", "Jane"}, {"age", 32}},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Empty array") {
			const json::Value value = parseJSONValueFromInputStream("[]");
			const json::Value expectedValue = json::Array{};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Array with three elements and a trailing comma") {
			const json::Value value = parseJSONValueFromInputStream(
				"[\n"
				"    1,\n"
				"    true,\n"
				"    'three',\n"
				"]");
			const json::Value expectedValue = json::Array{
				1,
				true,
				"three",
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Nested arrays") {
			const json::Value value = parseJSONValueFromInputStream(
				"[\n"
				"    [1, true, 'three'],\n"
				"    [4, \"five\", 0x6],\n"
				"]");
			const json::Value expectedValue = json::Array{
				json::Array{1, true, "three"},
				json::Array{4, "five", 0x6},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Line terminator in string") {
			const json::Value value = parseJSONValueFromInputStream(
				"'Lorem ipsum dolor sit amet, \\\n"
				"consectetur adipiscing elit.'");
			const json::Value expectedValue = json::String{"Lorem ipsum dolor sit amet, consectetur adipiscing elit."};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Arbitrary escaped characters") {
			const json::Value value = parseJSONValueFromInputStream("'\\A\\C\\/\\D\\C'");
			const json::Value expectedValue = json::String{"AC/DC"};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Numbers") {
			const json::Value value = parseJSONValueFromInputStream(
				"{\n"
				"    integer: 123,\n"
				"    withFractionPart: 123.45,\n"
				"    onlyFractionPart: .45,\n"
				"    withExponent: 123e-45,\n"
				"}");
			const json::Value expectedValue = json::Object{
				{"integer", 123},
				{"withFractionPart", 123.45},
				{"onlyFractionPart", .45},
				{"withExponent", 123e-45},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Hexadecimal numbers") {
			const json::Value value = parseJSONValueFromInputStream(
				"{\n"
				"    positiveHex: 0xdecaf,\n"
				"    negativeHex: -0xC0FFEE,\n"
				"}");
			const json::Value expectedValue = json::Object{
				{"positiveHex", 0xdecaf},
				{"negativeHex", -0xC0FFEE},
			};
			CHECK(value == expectedValue);
		}

		SUBCASE("Valid input - Infinity and NaN") {
			const json::Value value = parseJSONValueFromInputStream(
				"{\n"
				"    positiveInfinity: Infinity,\n"
				"    negativeInfinity: -Infinity,\n"
				"    notANumber: NaN,\n"
				"}");
			CHECK(std::isinf(value.get<json::Object>().at("positiveInfinity").get<json::Number>()));
			CHECK(!std::signbit(value.get<json::Object>().at("positiveInfinity").get<json::Number>()));
			CHECK(std::isinf(value.get<json::Object>().at("negativeInfinity").get<json::Number>()));
			CHECK(std::signbit(value.get<json::Object>().at("negativeInfinity").get<json::Number>()));
			CHECK(std::isnan(value.get<json::Object>().at("notANumber").get<json::Number>()));
			CHECK(!std::signbit(value.get<json::Object>().at("notANumber").get<json::Number>()));
		}
	}

	TEST_CASE("Output to ASCII string") {
		SUBCASE("Kitchen sink") {
			const std::string string = json::Value{
				json::Object{
					{"unquoted", "and you can quote me on that"},
					{"singleQuotes", "I can use \"double quotes\" here"},
					{"lineBreaks", "Look, Mom! No \\n's!"},
					{"hexadecimal", 0xdecaf},
					{"leadingDecimalPoint", .8675309},
					{"andTrailing", 8675309.},
					{"positiveSign", +1},
					{"trailingComma", "in objects"},
					{"andIn", json::Array{"arrays"}},
					{"backwardsCompatible", "with JSON"},
				}}.toString();
			constexpr std::string_view EXPECTED_STRING =
				"{\n"
				"    \"andIn\": [\"arrays\"],\n"
				"    \"andTrailing\": 8675309,\n"
				"    \"backwardsCompatible\": \"with JSON\",\n"
				"    \"hexadecimal\": 912559,\n"
				"    \"leadingDecimalPoint\": 0.8675309,\n"
				"    \"lineBreaks\": \"Look, Mom! No \\\\n's!\",\n"
				"    \"positiveSign\": 1,\n"
				"    \"singleQuotes\": \"I can use \\\"double quotes\\\" here\",\n"
				"    \"trailingComma\": \"in objects\",\n"
				"    \"unquoted\": \"and you can quote me on that\"\n"
				"}";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Empty object") {
			const std::string string = json::Value{json::Object{}}.toString();
			constexpr std::string_view EXPECTED_STRING = "{}";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Object with two properties and a trailing comma") {
			const std::string string = json::Value{
				json::Object{
					{"width", 1920},
					{"height", 1080},
				}}.toString();
			constexpr std::string_view EXPECTED_STRING = "{ \"height\": 1080, \"width\": 1920 }";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Nested objects") {
			const std::string string = json::Value{
				json::Object{
					{"image", json::Object{{"width", 1920}, {"height", 1080}, {"aspect-ratio", "16:9"}}},
				}}.toString();
			constexpr std::string_view EXPECTED_STRING =
				"{\n"
				"    \"image\": { \"aspect-ratio\": \"16:9\", \"height\": 1080, \"width\": 1920 }\n"
				"}";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Array of objects") {
			const std::string string = json::Value{
				json::Array{
					json::Object{{"name", "Joe"}, {"age", 27}},
					json::Object{{"name", "Jane"}, {"age", 32}},
				}}.toString();
			constexpr std::string_view EXPECTED_STRING =
				"[\n"
				"    { \"age\": 27, \"name\": \"Joe\" },\n"
				"    { \"age\": 32, \"name\": \"Jane\" }\n"
				"]";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Empty array") {
			const std::string string = json::Value{json::Array{}}.toString();
			constexpr std::string_view EXPECTED_STRING = "[]";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Array with three elements and a trailing comma") {
			std::string string = json::Value{json::Array{1, true, "three"}}.toString();
			constexpr std::string_view EXPECTED_STRING = "[1, true, \"three\"]";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Nested arrays") {
			const std::string string = json::Value{
				json::Array{
					json::Array{1, true, "three"},
					json::Array{4, "five", 0x6},
				}}.toString();
			constexpr std::string_view EXPECTED_STRING =
				"[\n"
				"    [1, true, \"three\"],\n"
				"    [4, \"five\", 6]\n"
				"]";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Line terminator in string") {
			const std::string string = json::Value{"Lorem ipsum dolor sit amet, \nconsectetur adipiscing elit."}.toString();
			constexpr std::string_view EXPECTED_STRING = "\"Lorem ipsum dolor sit amet, \\nconsectetur adipiscing elit.\"";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Arbitrary escaped characters") {
			const std::string string = json::Value{"'\\A\\C\\/\\D\\C'"}.toString();
			constexpr std::string_view EXPECTED_STRING = "\"'\\\\A\\\\C\\\\/\\\\D\\\\C'\"";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Numbers") {
			const std::string string = json::Value{
				json::Object{
					{"integer", 123},
					{"withFractionPart", 123.45},
					{"onlyFractionPart", .45},
					{"withExponent", 123e-45},
				}}.toString();
			constexpr std::string_view EXPECTED_STRING =
				"{\n"
				"    \"integer\": 123,\n"
				"    \"onlyFractionPart\": 0.45,\n"
				"    \"withExponent\": 1.23e-43,\n"
				"    \"withFractionPart\": 123.45\n"
				"}";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Hexadecimal numbers") {
			const std::string string = json::Value{
				json::Object{
					{"positiveHex", 0xdecaf},
					{"negativeHex", -0xC0FFEE},
				}}.toString();
			constexpr std::string_view EXPECTED_STRING = "{ \"negativeHex\": -12648430, \"positiveHex\": 912559 }";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Infinity and NaN") {
			const std::string string = json::Value{
				json::Object{
					{"positiveInfinity", std::numeric_limits<json::Number>::infinity()},
					{"negativeInfinity", -std::numeric_limits<json::Number>::infinity()},
					{"notANumber", std::numeric_limits<json::Number>::quiet_NaN()},
				}}.toString();
			constexpr std::string_view EXPECTED_STRING = "{ \"negativeInfinity\": -Infinity, \"notANumber\": NaN, \"positiveInfinity\": Infinity }";
			CHECK(string == EXPECTED_STRING);
		}
	}

	TEST_CASE("Serialize to ASCII stream") {
		SUBCASE("Non-null string") {
			const char* const input = "Some\ntext";
			std::ostringstream stream{};
			json::serialize(stream, input, {});
			const std::string string = std::move(stream).str();
			constexpr std::string_view EXPECTED_STRING = "\"Some\\ntext\"";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Null string") {
			const char* const input = nullptr;
			std::ostringstream stream{};
			json::serialize(stream, input, {});
			const std::string string = std::move(stream).str();
			constexpr std::string_view EXPECTED_STRING = "null";
			CHECK(string == EXPECTED_STRING);
		}

		struct Aggregate {
			int x = 123;
			double y = -5.3;
			std::string z = "abc";
		};

		SUBCASE("Aggregate") {
			const Aggregate input{};
			std::ostringstream stream{};
			json::serialize(stream, input, {});
			const std::string string = std::move(stream).str();
			constexpr std::string_view EXPECTED_STRING = "[123, -5.3, \"abc\"]";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Non-null optional aggregate") {
			const std::optional<Aggregate> input = Aggregate{};
			std::ostringstream stream{};
			json::serialize(stream, input, {});
			const std::string string = std::move(stream).str();
			constexpr std::string_view EXPECTED_STRING = "[123, -5.3, \"abc\"]";
			CHECK(string == EXPECTED_STRING);
		}

		SUBCASE("Null optional aggregate") {
			const std::optional<Aggregate> input = std::nullopt;
			std::ostringstream stream{};
			json::serialize(stream, input, {});
			const std::string string = std::move(stream).str();
			constexpr std::string_view EXPECTED_STRING = "null";
			CHECK(string == EXPECTED_STRING);
		}

		struct SingleFieldAggregate {
			int x = 123;
		};

		SUBCASE("Single-field aggregate") {
			const SingleFieldAggregate input{};
			std::ostringstream stream{};
			json::serialize(stream, input, {});
			const std::string string = std::move(stream).str();
			constexpr std::string_view EXPECTED_STRING = "123";
			CHECK(string == EXPECTED_STRING);
		}
	}

	TEST_CASE("Deserialize from ASCII stream - pretty printed") {
		SUBCASE("Kitchen sink") {
			const json::Value valueA{json::Object{
				{"unquoted", "and you can quote me on that"},
				{"singleQuotes", "I can use \"double quotes\" here"},
				{"lineBreaks", "Look, Mom! No \\n's!"},
				{"hexadecimal", 0xdecaf},
				{"leadingDecimalPoint", .8675309},
				{"andTrailing", 8675309.},
				{"positiveSign", +1},
				{"trailingComma", "in objects"},
				{"andIn", json::Array{"arrays"}},
				{"backwardsCompatible", "with JSON"},
			}};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Value valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Empty object") {
			const json::Object valueA{};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Object with two properties and a trailing comma") {
			const json::Object valueA{
				{"width", 1920},
				{"height", 1080},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Nested objects") {
			const json::Object valueA{
				{"image", json::Object{{"width", 1920}, {"height", 1080}, {"aspect-ratio", "16:9"}}},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Array of objects") {
			const json::Array valueA{
				json::Object{{"name", "Joe"}, {"age", 27}},
				json::Object{{"name", "Jane"}, {"age", 32}},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Array valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Empty array") {
			const json::Array valueA{};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Array valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Array with three elements and a trailing comma") {
			const json::Array valueA{1, true, "three"};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Array valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Nested arrays") {
			const json::Array valueA{
				json::Array{1, true, "three"},
				json::Array{4, "five", 0x6},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Array valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Line terminator in string") {
			const std::string valueA{"Lorem ipsum dolor sit amet, \nconsectetur adipiscing elit."};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			std::string valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Arbitrary escaped characters") {
			const std::string valueA{"'\\A\\C\\/\\D\\C'"};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			std::string valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Numbers") {
			const json::Object valueA{
				{"integer", 123},
				{"withFractionPart", 123.45},
				{"onlyFractionPart", .45},
				{"withExponent", 123e-45},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Hexadecimal numbers") {
			const json::Object valueA{
				{"positiveHex", 0xdecaf},
				{"negativeHex", -0xC0FFEE},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Infinity and NaN") {
			const json::Object valueA{
				{"positiveInfinity", std::numeric_limits<json::Number>::infinity()},
				{"negativeInfinity", -std::numeric_limits<json::Number>::infinity()},
				{"positiveNotANumber", std::numeric_limits<json::Number>::quiet_NaN()},
				{"negativeNotANumber", -std::numeric_limits<json::Number>::quiet_NaN()},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(std::isinf(valueB.at("positiveInfinity").get<json::Number>()));
			CHECK(!std::signbit(valueB.at("positiveInfinity").get<json::Number>()));
			CHECK(std::isinf(valueB.at("negativeInfinity").get<json::Number>()));
			CHECK(std::signbit(valueB.at("negativeInfinity").get<json::Number>()));
			CHECK(std::isnan(valueB.at("positiveNotANumber").get<json::Number>()));
			CHECK(!std::signbit(valueB.at("positiveNotANumber").get<json::Number>()));
			CHECK(std::isnan(valueB.at("negativeNotANumber").get<json::Number>()));
			CHECK(std::signbit(valueB.at("negativeNotANumber").get<json::Number>()));
		}

		SUBCASE("UTF-16 string") {
			constexpr std::string_view INPUT = "Some\ntext";
			std::u16string valueA{INPUT.begin(), INPUT.end()};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			std::u16string valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("UTF-32 string") {
			constexpr std::string_view INPUT = "Some\ntext";
			std::u32string valueA{INPUT.begin(), INPUT.end()};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			std::u32string valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		struct Aggregate {
			int x = 0;
			double y = 0.0;
			std::string z{};

			[[nodiscard]] bool operator==(const Aggregate& other) const = default;
		};

		SUBCASE("Aggregate") {
			const Aggregate valueA{.x = 123, .y = -5.3, .z = "abc"};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			Aggregate valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Non-null optional aggregate") {
			const std::optional<Aggregate> valueA = Aggregate{.x = 123, .y = -5.3, .z = "abc"};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			std::optional<Aggregate> valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Null optional aggregate") {
			const std::optional<Aggregate> valueA = std::nullopt;
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			std::optional<Aggregate> valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		struct SingleFieldAggregate {
			int x = 0;

			[[nodiscard]] bool operator==(const SingleFieldAggregate& other) const = default;
		};

		SUBCASE("Single-field aggregate") {
			const SingleFieldAggregate valueA{.x = 123};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = true});
			std::istringstream streamB{std::move(streamA).str()};
			SingleFieldAggregate valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}
	}

	TEST_CASE("Deserialize from ASCII stream - not pretty printed") {
		SUBCASE("Kitchen sink") {
			const json::Value valueA{json::Object{
				{"unquoted", "and you can quote me on that"},
				{"singleQuotes", "I can use \"double quotes\" here"},
				{"lineBreaks", "Look, Mom! No \\n's!"},
				{"hexadecimal", 0xdecaf},
				{"leadingDecimalPoint", .8675309},
				{"andTrailing", 8675309.},
				{"positiveSign", +1},
				{"trailingComma", "in objects"},
				{"andIn", json::Array{"arrays"}},
				{"backwardsCompatible", "with JSON"},
			}};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Value valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Empty object") {
			const json::Object valueA{};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Object with two properties and a trailing comma") {
			const json::Object valueA{
				{"width", 1920},
				{"height", 1080},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Nested objects") {
			const json::Object valueA{
				{"image", json::Object{{"width", 1920}, {"height", 1080}, {"aspect-ratio", "16:9"}}},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Array of objects") {
			const json::Array valueA{
				json::Object{{"name", "Joe"}, {"age", 27}},
				json::Object{{"name", "Jane"}, {"age", 32}},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Array valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Empty array") {
			const json::Array valueA{};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Array valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Array with three elements and a trailing comma") {
			const json::Array valueA{1, true, "three"};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Array valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Nested arrays") {
			const json::Array valueA{
				json::Array{1, true, "three"},
				json::Array{4, "five", 0x6},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Array valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Line terminator in string") {
			const std::string valueA{"Lorem ipsum dolor sit amet, \nconsectetur adipiscing elit."};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			std::string valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Arbitrary escaped characters") {
			const std::string valueA{"'\\A\\C\\/\\D\\C'"};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			std::string valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Numbers") {
			const json::Object valueA{
				{"integer", 123},
				{"withFractionPart", 123.45},
				{"onlyFractionPart", .45},
				{"withExponent", 123e-45},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Hexadecimal numbers") {
			const json::Object valueA{
				{"positiveHex", 0xdecaf},
				{"negativeHex", -0xC0FFEE},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Infinity and NaN") {
			const json::Object valueA{
				{"positiveInfinity", std::numeric_limits<json::Number>::infinity()},
				{"negativeInfinity", -std::numeric_limits<json::Number>::infinity()},
				{"positiveNotANumber", std::numeric_limits<json::Number>::quiet_NaN()},
				{"negativeNotANumber", -std::numeric_limits<json::Number>::quiet_NaN()},
			};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			json::Object valueB{};
			json::deserialize(streamB, valueB);
			CHECK(std::isinf(valueB.at("positiveInfinity").get<json::Number>()));
			CHECK(!std::signbit(valueB.at("positiveInfinity").get<json::Number>()));
			CHECK(std::isinf(valueB.at("negativeInfinity").get<json::Number>()));
			CHECK(std::signbit(valueB.at("negativeInfinity").get<json::Number>()));
			CHECK(std::isnan(valueB.at("positiveNotANumber").get<json::Number>()));
			CHECK(!std::signbit(valueB.at("positiveNotANumber").get<json::Number>()));
			CHECK(std::isnan(valueB.at("negativeNotANumber").get<json::Number>()));
			CHECK(std::signbit(valueB.at("negativeNotANumber").get<json::Number>()));
		}

		SUBCASE("UTF-16 string") {
			constexpr std::string_view INPUT = "Some\ntext";
			std::u16string valueA{INPUT.begin(), INPUT.end()};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			std::u16string valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("UTF-32 string") {
			constexpr std::string_view INPUT = "Some\ntext";
			std::u32string valueA{INPUT.begin(), INPUT.end()};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			std::u32string valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		struct Aggregate {
			int x = 0;
			double y = 0.0;
			std::string z{};

			[[nodiscard]] bool operator==(const Aggregate& other) const = default;
		};

		SUBCASE("Aggregate") {
			const Aggregate valueA{.x = 123, .y = -5.3, .z = "abc"};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			Aggregate valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Non-null optional aggregate") {
			const std::optional<Aggregate> valueA = Aggregate{.x = 123, .y = -5.3, .z = "abc"};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			std::optional<Aggregate> valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		SUBCASE("Null optional aggregate") {
			const std::optional<Aggregate> valueA = std::nullopt;
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			std::optional<Aggregate> valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}

		struct SingleFieldAggregate {
			int x = 0;

			[[nodiscard]] bool operator==(const SingleFieldAggregate& other) const = default;
		};

		SUBCASE("Single-field aggregate") {
			const SingleFieldAggregate valueA{.x = 123};
			std::ostringstream streamA{};
			json::serialize(streamA, valueA, {.prettyPrint = false});
			std::istringstream streamB{std::move(streamA).str()};
			SingleFieldAggregate valueB{};
			json::deserialize(streamB, valueB);
			CHECK(valueA == valueB);
		}
	}
}

/*
NOTE:
  Some of the test cases above are based on examples from the
  JSON5 specification version 1.0.0 at https://spec.json5.org/
  which was published under the following license:

The MIT License (MIT)

Copyright (c) 2017 Aseem Kishore, Jordan Tucker

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*/
