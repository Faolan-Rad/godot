/**************************************************************************/
/*  test_json.h                                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef TEST_JSON_H
#define TEST_JSON_H

#include "core/io/json.h"

#include "thirdparty/doctest/doctest.h"

namespace TestJSON {

// NOTE: The current JSON parser accepts many non-conformant strings such as
// single-quoted strings, duplicate commas and trailing commas.
// This is intentionally not tested as users shouldn't rely on this behavior.

TEST_CASE("[JSON] Parsing single data types") {
	// Parsing a single data type as JSON is valid per the JSON specification.

	JSON json;

	json.parse("null");
	CHECK_MESSAGE(
			json.get_error_line() == 0,
			"Parsing `null` as JSON should parse successfully.");
	CHECK_MESSAGE(
			json.get_data() == Variant(),
			"Parsing a double quoted string as JSON should return the expected value.");

	json.parse("true");
	CHECK_MESSAGE(
			json.get_error_line() == 0,
			"Parsing boolean `true` as JSON should parse successfully.");
	CHECK_MESSAGE(
			json.get_data(),
			"Parsing boolean `true` as JSON should return the expected value.");

	json.parse("false");
	CHECK_MESSAGE(
			json.get_error_line() == 0,
			"Parsing boolean `false` as JSON should parse successfully.");
	CHECK_MESSAGE(
			!json.get_data(),
			"Parsing boolean `false` as JSON should return the expected value.");

	json.parse("123456");
	CHECK_MESSAGE(
			json.get_error_line() == 0,
			"Parsing an integer number as JSON should parse successfully.");
	CHECK_MESSAGE(
			(int)(json.get_data()) == 123456,
			"Parsing an integer number as JSON should return the expected value.");

	json.parse("0.123456");
	CHECK_MESSAGE(
			json.get_error_line() == 0,
			"Parsing a floating-point number as JSON should parse successfully.");
	CHECK_MESSAGE(
			double(json.get_data()) == doctest::Approx(0.123456),
			"Parsing a floating-point number as JSON should return the expected value.");

	json.parse("\"hello\"");
	CHECK_MESSAGE(
			json.get_error_line() == 0,
			"Parsing a double quoted string as JSON should parse successfully.");
	CHECK_MESSAGE(
			json.get_data() == "hello",
			"Parsing a double quoted string as JSON should return the expected value.");
}

TEST_CASE("[JSON] Parsing arrays") {
	JSON json;

	// JSON parsing fails if it's split over several lines (even if leading indentation is removed).
	json.parse(R"(["Hello", "world.", "This is",["a","json","array.",[]], "Empty arrays ahoy:", [[["Gotcha!"]]]])");

	const Array array = json.get_data();
	CHECK_MESSAGE(
			json.get_error_line() == 0,
			"Parsing a JSON array should parse successfully.");
	CHECK_MESSAGE(
			array[0] == "Hello",
			"The parsed JSON should contain the expected values.");
	const Array sub_array = array[3];
	CHECK_MESSAGE(
			sub_array.size() == 4,
			"The parsed JSON should contain the expected values.");
	CHECK_MESSAGE(
			sub_array[1] == "json",
			"The parsed JSON should contain the expected values.");
	CHECK_MESSAGE(
			sub_array[3].hash() == Array().hash(),
			"The parsed JSON should contain the expected values.");
	const Array deep_array = Array(Array(array[5])[0])[0];
	CHECK_MESSAGE(
			deep_array[0] == "Gotcha!",
			"The parsed JSON should contain the expected values.");
}

TEST_CASE("[JSON] Parsing objects (dictionaries)") {
	JSON json;

	json.parse(R"({"name": "Godot Engine", "is_free": true, "bugs": null, "apples": {"red": 500, "green": 0, "blue": -20}, "empty_object": {}})");

	const Dictionary dictionary = json.get_data();
	CHECK_MESSAGE(
			dictionary["name"] == "Godot Engine",
			"The parsed JSON should contain the expected values.");
	CHECK_MESSAGE(
			dictionary["is_free"],
			"The parsed JSON should contain the expected values.");
	CHECK_MESSAGE(
			dictionary["bugs"] == Variant(),
			"The parsed JSON should contain the expected values.");
	CHECK_MESSAGE(
			(int)Dictionary(dictionary["apples"])["blue"] == -20,
			"The parsed JSON should contain the expected values.");
	CHECK_MESSAGE(
			dictionary["empty_object"].hash() == Dictionary().hash(),
			"The parsed JSON should contain the expected values.");
}

TEST_CASE("[JSON] Parsing escape sequences") {
	// Only certain escape sequences are valid according to the JSON specification.
	// Others must result in a parsing error instead.

	JSON json;

	TypedArray<String> valid_escapes;
	valid_escapes.push_back("\";\"");
	valid_escapes.push_back("\\;\\");
	valid_escapes.push_back("/;/");
	valid_escapes.push_back("b;\b");
	valid_escapes.push_back("f;\f");
	valid_escapes.push_back("n;\n");
	valid_escapes.push_back("r;\r");
	valid_escapes.push_back("t;\t");

	SUBCASE("Basic valid escape sequences") {
		for (int i = 0; i < valid_escapes.size(); i++) {
			String valid_escape = valid_escapes[i];
			String valid_escape_string = valid_escape.get_slice(";", 0);
			String valid_escape_value = valid_escape.get_slice(";", 1);

			String json_string = "\"\\";
			json_string += valid_escape_string;
			json_string += "\"";
			json.parse(json_string);

			CHECK_MESSAGE(
					json.get_error_line() == 0,
					vformat("Parsing valid escape sequence `%s` as JSON should parse successfully.", valid_escape_string));

			String json_value = json.get_data();
			CHECK_MESSAGE(
					json_value == valid_escape_value,
					vformat("Parsing valid escape sequence `%s` as JSON should return the expected value.", valid_escape_string));
		}
	}

	SUBCASE("Valid unicode escape sequences") {
		String json_string = "\"\\u0000\"";
		json.parse(json_string);

		CHECK_MESSAGE(
				json.get_error_line() == 0,
				vformat("Parsing valid unicode escape sequence with value `0000` as JSON should parse successfully."));

		String json_value = json.get_data();
		CHECK_MESSAGE(
				json_value == "\0",
				vformat("Parsing valid unicode escape sequence with value `0000` as JSON should return the expected value."));
	}

	SUBCASE("Invalid escape sequences") {
		for (char32_t i = 0; i < 128; i++) {
			bool skip = false;
			for (int j = 0; j < valid_escapes.size(); j++) {
				String valid_escape = valid_escapes[j];
				String valid_escape_string = valid_escape.get_slice(";", 0);
				if (valid_escape_string[0] == i) {
					skip = true;
					break;
				}
			}

			if (skip) {
				continue;
			}

			String json_string = "\"\\";
			json_string += i;
			json_string += "\"";
			Error err = json.parse(json_string);

			// TODO: Line number is currently kept on 0, despite an error occurring. This should be fixed in the JSON parser.
			// CHECK_MESSAGE(
			// 		json.get_error_line() != 0,
			// 		vformat("Parsing invalid escape sequence with ASCII value `%d` as JSON should fail to parse.", i));
			CHECK_MESSAGE(
					err == ERR_PARSE_ERROR,
					vformat("Parsing invalid escape sequence with ASCII value `%d` as JSON should fail to parse with ERR_PARSE_ERROR.", i));
		}
	}
}
} // namespace TestJSON

#endif // TEST_JSON_H
