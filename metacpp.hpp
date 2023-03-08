#pragma once

// Standard headers
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include <typeinfo>

namespace metacpp {

namespace data {

// TODO: make lower case...

// Generic list
template <typename ... Values>
struct generic_list {};

template <typename A, typename ... Values>
struct generic_list <A, Values...> {
	using value = A;
	using next = generic_list <Values...>;
};

// Statically typed list
template <typename T, T ... Values>
struct list {};

template <typename T, T x, T ... Values>
struct list <T, x, Values...> {
	static constexpr T value = x;
	using next = list <T, Values...>;
};

// Efficient and simple constexpr string
struct constexpr_string {
	const char *str;
	size_t size;

	constexpr_string() = delete;
	explicit constexpr constexpr_string(const char *str, size_t size)
		: str(str), size(size) {}
};

// Indexing
template <typename, int>
struct impl_index {};

template <int Index, typename A, typename ... Values>
struct impl_index <generic_list <A, Values...>, Index> {
	using type = typename impl_index <generic_list <Values...>, Index - 1> ::type;
};

template <typename A, typename ... Values>
struct impl_index <generic_list <A, Values...>, 0> {
	using type = A;
};

// Faster specialization for lists
template <typename T, int Index, T ... Values>
requires (Index >= 0 && Index < sizeof...(Values))
struct impl_index <list <T, Values...>, Index> {
	static constexpr T value = std::array <T, sizeof...(Values)> {Values...} [Index];
};

template <typename T, int Index, T ... Values>
requires (Index >= 0 && Index < sizeof...(Values))
struct impl_index <const list <T, Values...>, Index> {
	static constexpr T value = std::array <T, sizeof...(Values)> {Values...} [Index];
};

// Insertion/pushing
template <typename T, typename, T>
struct impl_insert_back {
	static_assert(
		!std::is_same <T, T> ::value,
		"Invalid overload for impl_insert_back:"
		" expected <T, list <T, Values...>, T x>"
	);

	using type = void;
};

template <typename T, T x, T ... Values>
struct impl_insert_back <T, list <T, Values...>, x> {
	using type = list <T, Values..., x>;
};

template <typename T, typename, T>
struct impl_insert_front {
	static_assert(
		!std::is_same <T, T> ::value,
		"Invalid overload for impl_insert_front:"
		" expected <T, list <T, Values...>, T x>"
	);

	using type = void;
};

template <typename T, T x, T ... Values>
struct impl_insert_front <T, list <T, Values...>, x> {
	using type = list <T, x, Values...>;
};

// Erasing/popping
template <typename T, typename>
struct impl_erase_front {};

template <typename T, T x, T ... Values>
struct impl_erase_front <T, list <T, x, Values...>> {
	static constexpr T value = x;
	using type = list <T, Values...>;
};

template <typename T, typename>
struct impl_erase_back {};

template <typename T, T x>
struct impl_erase_back <T, list <T, x>> {
	using type = list <T>;
	static constexpr T value = x;
};

template <typename T, T x, T ... Values>
struct impl_erase_back <T, list <T, x, Values...>> {
	using type = typename impl_insert_front <T,
		typename impl_erase_back <T, list <T, Values...>> ::type, x
	> ::type;

	static constexpr T value = impl_erase_back <T, list <T, Values...>> ::value;
};

// Concatenation
template <typename, typename>
struct impl_concat {};

template <typename ... Values1, typename ... Values2>
struct impl_concat <generic_list <Values1...>, generic_list <Values2...>> {
	using type = generic_list <Values1..., Values2...>;
};

template <typename T, T ... Values1, T ... Values2>
struct impl_concat <list <T, Values1...>, list <T, Values2...>> {
	using type = list <T, Values1..., Values2...>;
};

// Compile time type checks
template <typename>
struct impl_is_list {
	static constexpr bool value = false;
};

template <typename ... Values>
struct impl_is_list <generic_list <Values...>> {
	static constexpr bool value = true;
};

template <typename ... Values>
struct impl_is_list <const generic_list <Values...>> {
	static constexpr bool value = true;
};

template <typename T, T ... Values>
struct impl_is_list <list <T, Values...>> {
	static constexpr bool value = true;
};

template <typename T, T ... Values>
struct impl_is_list <const list <T, Values...>> {
	static constexpr bool value = true;
};

// Size utility
template <typename T>
requires impl_is_list <T> ::value
struct impl_size {
	static constexpr size_t value = 0;
};

template <typename ... Values>
struct impl_size <generic_list <Values...>> {
	static constexpr size_t value = sizeof...(Values);
};

template <typename T, T ... Values>
struct impl_size <list <T, Values...>> {
	static constexpr size_t value = sizeof...(Values);
};

// Other properties
template <typename T>
struct impl_is_empty {
	static constexpr bool value = (impl_size <T> ::value == 0);
};

}

// Type checks
template <typename T>
using is_list = data::impl_is_list <T>;

// WARNING: Use the constexpr versions with caution: they will haunt the
// resulting binary with a lot of template instantiations. If binary size is a
// concern, use the non-constexpr versions instead.
template <typename T>
static constexpr bool is_list_v = data::impl_is_list <T> ::value;

// TODO: add for other data structures

// Global namespace operations (methods)
template <typename T>
using size = data::impl_size <T>;

template <typename T>
static constexpr auto size_v = data::impl_size <T> ::value;

// Empty check
template <typename T>
using is_empty = data::impl_is_empty <T>;

template <typename T>
constexpr bool is_empty_v = data::impl_is_empty <T> ::value;

// Indexing
template <typename T, int Index>
using index = data::impl_index <T, Index>;

template <typename T, int Index>
static constexpr auto index_v = data::impl_index <T, Index> ::value;

template <typename T, int Index>
using index_t = typename data::impl_index <T, Index> ::type;

// Indexing variadics
template <typename T, int Index, T ... Values>
struct index_variadic {
	static constexpr T value = std::array <T, sizeof...(Values)> {Values...} [Index];
};

// Methods
template <typename T, typename U, T x>
using insert_back_t = typename data::impl_insert_back <T, U, x> ::type;

template <typename T, typename U, T x>
using insert_front_t = typename data::impl_insert_front <T, U, x> ::type;

template <typename T, typename U>
using erase_front = typename data::impl_erase_front <T, U>;

template <typename T, typename U>
using erase_front_t = typename data::impl_erase_front <T, U> ::type;

template <typename T, typename U>
constexpr T erase_front_v = erase_front <T, U> ::value;

template <typename T, typename U>
using erase_back = typename data::impl_erase_back <T, U>;

template <typename T, typename U>
using erase_back_t = typename data::impl_erase_back <T, U> ::type;

template <typename T, typename U>
constexpr T erase_back_v = erase_back <T, U> ::value;

template <typename U, typename V>
using concat_t = typename data::impl_concat <U, V> ::type;

// Language utilities
namespace lang {

// This namespace contains *Turing Machines* which parse compile-time strings
// (data::string). The specification is as follows:
//
// * The input alphabet is the set of ASCII characters
// * To check whether a string is accepted by a machine, use the `success`
//  static constexpr member of the machine
//
// All machines are defined as templates; at least one field specifies the
// input string, and another field specifies the starting index (state) which
// is 0 (start) by default.

struct result {
	bool success;
	size_t next;
};

constexpr result match_char(const data::constexpr_string &str, char c, size_t index = 0)
{
	bool success = (str.str[index] == c);
	if (success)
		index++;

	return {success, index};
}

// Matching strings
constexpr result match_string(const data::constexpr_string &str, const data::constexpr_string &match, size_t index = 0)
{
	if (index >= str.size)
		return {false, index};

	if (index + match.size > str.size)
		return {false, index};

	for (int i = 0; i < match.size; i++) {
		if (str.str[index + i] != match.str[i])
			return {false, index};
	}

	return {true, index + match.size};
}

// Skipping whitespace
struct whitespace_result {
	bool success;
	size_t removed;
	size_t next;
};

constexpr whitespace_result match_whitespace(const data::constexpr_string &str, size_t index = 0)
{
	if (index >= str.size)
		return {false, index};

	size_t removed = 0;
	while (index < str.size) {
		char c = str.str[index];
		if (c == ' ' || c == '\t' || c == '\n') {
			removed++;
			index++;
		} else {
			break;
		}
	}

	return {true, removed, index};
}

// Reading integers from compile-time strings
template <typename I>
struct int_result {
	bool success;
	size_t next;
	I value;
};

template <typename I>
requires (std::is_arithmetic <I> ::value)
constexpr int_result <I> match_int(const data::constexpr_string &str, size_t index = 0)
{
	size_t original_index = index;
	if (index >= str.size)
		return {false, index, 0};

	bool negative = false;
	if (str.str[index] == '-') {
		negative = true;
		index++;
	}

	// TODO: ensure that if negative there is at least one digit
	I value = 0;

	int digit_count = 0;
	while (index < str.size) {
		char c = str.str[index];
		if (c >= '0' && c <= '9') {
			value = 10 * value + (c - '0');
			digit_count++;
			index++;
		} else {
			break;
		}
	}

	if (digit_count == 0)
		return {false, original_index, 0};

	if (negative)
		value = -value;

	return {true, index, value};
}

// Reading floats from compile-time strings
template <typename F>
struct float_result {
	bool success;
	size_t next;
	bool dot;
	F value;
};

template <typename F>
requires (std::is_floating_point <F> ::value)
constexpr float_result <F> match_float(const data::constexpr_string &str, size_t index = 0)
{
	size_t original_index = index;
	if (index >= str.size)
		return {false, index, false, 0};

	bool negative = false;
	if (str.str[index] == '-') {
		negative = true;
		index++;
	}

	F value_before = 0;
	F value_after = 0;
	F inv_power = 1/F(10);
	bool dot = false;

	int digit_count = 0;
	while (index < str.size) {
		char c = str.str[index];
		if (c >= '0' && c <= '9') {
			if (dot) {
				value_after = value_after + inv_power * (c - '0');
				inv_power /= F(10);
			} else {
				value_before = 10 * value_before + (c - '0');
			}

			digit_count++;
			index++;
		} else if (c == '.' && !dot) {
			dot = true;
			index++;
		} else {
			break;
		}
	}

	if (digit_count == 0)
		return {false, original_index, false, 0};

	F value = value_before + value_after;
	if (negative)
		value = -value;

	return {true, index, dot, value};
}

// NOTE: match_list <data::string, type, count, start index>
template <data::constexpr_string, typename T, int Count = -1, int Index = 0>
struct match_list {
	static constexpr bool success = (Count == 0);
	static constexpr size_t next = Index;
	using type = data::list <T>;
};

// Specialization for int lists
template <data::constexpr_string Str, typename T, int Count, int Index>
requires (std::is_arithmetic <T> ::value && Count != 0 && Index >= 0 && Index < Str.size)
class match_list <Str, T, Count, Index> {
	// Choose the correct match function
	static constexpr auto impl_match() {
		if constexpr (std::is_floating_point <T> ::value)
			return match_float <T> (Str, Index);
		else
			return match_int <T> (Str, Index);
	}

	static constexpr auto impl_match_result = impl_match();
	static constexpr bool impl_next_in_bounds = (impl_match_result.next < Str.size);
	static constexpr bool impl_comma = (Str.str[impl_match_result.next] == ',');
	static constexpr size_t impl_after_next = impl_match_result.next + impl_comma;

	using impl_next_t = match_list <Str, T, Count - 1, impl_after_next>;

	static constexpr bool impl_success() {
		if constexpr (Count < 0) {
			return true;
		} else {
			// Deal with single element case explicitly
			if constexpr (Count == 1) {
				return impl_match_result.success;
			} else {
				if constexpr (impl_next_in_bounds && impl_comma)
					return impl_next_t::success;
				else
					return false;
			}
		}
	}

	static constexpr size_t impl_next() {
		if constexpr (success)
			return impl_next_t::next; // Always valid...
		else
			return Index;

	}
public:
	static constexpr bool success = impl_success();
	static constexpr size_t next = impl_next();

	template <bool, typename>
	struct select {
		using type = data::list <T>;
	};

	template <typename U, U ... Us>
	struct select <true, data::list <U, Us...>> {
		using type = data::list <U, impl_match_result.value, Us...>;
	};

	using type = typename select <success, typename impl_next_t::type> ::type;
};

}

namespace io {

// Printer to primitives
template <typename T>
std::string primitive_to_string(const T &value)
{
	return std::to_string(value);
};

template <>
inline std::string primitive_to_string <char> (const char &value)
{
	return std::string { "'" } + value + "'";
};

// Printer
template <typename T, typename ... Ts>
struct impl_printf {
	static std::string value() {
		return typeid(T).name();
	}
};

// Printing generic lists
template <typename T, typename ... Ts>
struct impl_printf <data::generic_list <T, Ts...>> {
	static std::string impl_value() {
		std::string result = impl_printf <T> ::value();
		if constexpr (sizeof ... (Ts) == 0)
			return result;
		else
			return result + ", " + impl_printf <data::generic_list <Ts...>> ::impl_value();
	}

	static std::string value() {
		return "(" + impl_value() + ")";
	}
};

template <>
struct impl_printf <data::generic_list <>> {
	static std::string value() {
		return "()";
	}
};

// Printing type restricted lists
template <typename T, T x, T ... Ts>
struct impl_printf <data::list <T, x, Ts...>> {
	static std::string impl_value() {
		std::string result = primitive_to_string(x);
		if constexpr (sizeof ... (Ts) == 0)
			return result;
		else
			return result + ", " + impl_printf <data::list <T, Ts...>> ::impl_value();
	}

	static std::string value() {
		return "(" + impl_value() + ")";
	}
};

template <typename T>
struct impl_printf <data::list <T>> {
	static std::string value() {
		return "()";
	}
};

template <typename T, T x, T ... Ts>
struct impl_printf <const data::list <T, x, Ts...>> {
	static std::string value() {
		return "const " + impl_printf <data::list <T, x, Ts...>> ::value();
	}
};

// Printing wrapper
template <typename T>
std::string to_string() {
	return impl_printf <T> ::value();
}

}

}
