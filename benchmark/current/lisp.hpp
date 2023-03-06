#pragma once

#include "metacpp.hpp"

// Lisp parser
namespace lisp {								// namespace lisp

#define CHARS const metacpp::data::string <Chars...>
#define CCHARS const metacpp::data::string <C, Chars...>

// Fundamental types
template <long int X>
struct Int {
	static constexpr long int value = X;
};

template <double X>
struct Float {
	static constexpr double value = X;
};

// Default function dispatcher
template <typename T>
requires metacpp::data::impl_is_string <T> ::value
struct impl_ftn_dispatcher {
	static constexpr bool success = false;
};

// Constructor for list types
constexpr char impl_list_cstr[] = "list"; // Integer list
using impl_list_str = metacpp::to_string_t <impl_list_cstr>;

template <typename T>
using impl_list_matcher = metacpp::lang::match_string <impl_list_str, T>;

// Parse list
template <typename>
struct impl_parse_list {};

template <char... Chars>
requires (impl_ftn_dispatcher <CHARS> ::success)
struct impl_parse_list <CHARS> {
	// Skip whitespace
	using impl_next_start = typename metacpp::lang::match_whitespace <
		typename impl_ftn_dispatcher <CHARS> ::next
	> ::next;

	using impl_current_type = typename impl_ftn_dispatcher <CHARS> ::type;

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = metacpp::concat_t <
		impl_current_type,
		typename impl_parse_list <impl_next_start> ::type
	>;
};

// Until we reach the end of the list (e.g. ')')
template <char C, char... Chars>
requires (C == ')')
struct impl_parse_list <CCHARS> {
	using type = metacpp::data::generic_list <>;
	using next = CHARS;
};

// Auto type casting
struct auto_type {
	struct eInt {};
	struct eFloat {};
};

template <typename...>
struct impl_auto_cast {
	using type = auto_type::eInt;
};

template <long int X, typename ... Values>
struct impl_auto_cast <Int <X>, Values...> {
	using type = typename impl_auto_cast <
		metacpp::data::generic_list <Values...>
	> ::type;
};

template <double X, typename ... Values>
struct impl_auto_cast <Float <X>, Values...> {
	using type = auto_type::eFloat;
};

// Arithmetic operations
enum class op_type {
	plus,
	minus,
	multiply,
	divide
};

template <op_type Op, typename>
struct impl_op {};

template <typename T>
struct impl_op <op_type::plus, T> {
	static constexpr int impl_value = 0;
};

template <typename T>
struct impl_op <op_type::multiply, T> {
	static constexpr int impl_value = 1;
};

// ADDITION

// Addition with pure integers
template <long int X, typename ... Ts>
requires std::is_same <typename impl_auto_cast <Ts...> ::type, auto_type::eInt> ::value
struct impl_op <op_type::plus, metacpp::data::generic_list <Int <X>, Ts...>> {
	static constexpr long int impl_value = X + impl_op <
		op_type::plus,
		metacpp::data::generic_list <Ts...>
	> ::impl_value;

	using type = Int <impl_value>;
};

// Otherwise, upcast to double
template <typename T, typename ... Ts>
struct impl_op <op_type::plus, metacpp::data::generic_list <T, Ts...>> {
	static constexpr double impl_value = T::value + impl_op <
		op_type::plus,
		metacpp::data::generic_list <Ts...>
	> ::impl_value;

	using type = Float <impl_value>;
};

// MULTIPLICATION

// Multiplication with pure integers
template <long int X, typename ... Ts>
// TODO: avoid the nesting...						over here!
requires std::is_same <typename impl_auto_cast <Ts...> ::type, auto_type::eInt> ::value
struct impl_op <op_type::multiply, metacpp::data::generic_list <Int <X>, Ts...>> {
	static constexpr long int impl_value = X * impl_op <
		op_type::multiply,
		metacpp::data::generic_list <Ts...>
	> ::impl_value;

	using type = Int <impl_value>;
};

// Otherwise, upcast to double
template <typename T, typename ... Ts>
struct impl_op <op_type::multiply, metacpp::data::generic_list <T, Ts...>> {
	static constexpr double impl_value = T::value * impl_op <
		op_type::multiply,
		metacpp::data::generic_list <Ts...>
	> ::impl_value;

	static_assert(impl_value != 0);
	using type = Float <impl_value>;
};

// SUBTRACTION

// Subtraction with pure integers
template <long int X, long int Y>
struct impl_op <op_type::minus, metacpp::data::generic_list <Int <X>, Int <Y>>> {
	static constexpr long int impl_value = X - Y;
	using type = Int <impl_value>;
};

// Otherwise, upcast to double
template <typename X, typename Y>
struct impl_op <op_type::minus, metacpp::data::generic_list <X, Y>> {
	static constexpr double impl_value = double(X::value) - double(Y::value);
	using type = Float <impl_value>;
};

// DIVISION

// Integer result only when perfectly divisible
template <long int X, long int Y>
requires (X % Y == 0)
struct impl_op <op_type::divide, metacpp::data::generic_list <Int <X>, Int <Y>>> {
	static constexpr long int impl_value = X / Y;
	using type = Int <impl_value>;
};

// Otherwise, upcast to double
template <typename X, typename Y>
struct impl_op <op_type::divide, metacpp::data::generic_list <X, Y>> {
	static constexpr double impl_value = double(X::value)/double(Y::value);
	using type = Float <impl_value>;
};

// List dispatcher (starts with 'list')
template <char C, char... Chars>
requires impl_list_matcher <CCHARS> ::success
struct impl_ftn_dispatcher <CCHARS> {
	// NOTE: successfuly matched list
	using impl_next_start = typename metacpp::lang::match_whitespace <
		typename impl_list_matcher <CCHARS> ::next
	> ::next;

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = typename impl_parse_list <impl_next_start> ::type;

	static constexpr bool success = true;
};

// Addition dispatcher (starts with '+')
template <char ... Chars>
requires metacpp::lang::match_char <'+', CHARS> ::success
struct impl_ftn_dispatcher <CHARS> {
	// NOTE: successfuly matched plus
	using impl_next_start = typename metacpp::lang::match_whitespace <
		typename metacpp::lang::match_char <'+', CHARS> ::next
	> ::next;

	using impl_current_type = typename impl_parse_list <impl_next_start> ::type;
	static_assert(metacpp::size <impl_current_type> ::value > 0,
		"Expected at least one argument to '+'"
	);

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = typename impl_op <op_type::plus, impl_current_type> ::type;

	static constexpr bool success = true;
};

// Multiplication dispatcher (starts with '*')
template <char... Chars>
requires metacpp::lang::match_char <'*', CHARS> ::success
struct impl_ftn_dispatcher <CHARS> {
	// NOTE: successfuly matched plus
	using impl_next_start = typename metacpp::lang::match_whitespace <
		typename metacpp::lang::match_char <'*', CHARS> ::next
	> ::next;

	using impl_current_type = typename impl_parse_list <impl_next_start> ::type;
	static_assert(metacpp::size <impl_current_type> ::value > 0,
		"Expected at least one argument to '*'"
	);

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = typename impl_op <op_type::multiply, impl_current_type> ::type;

	static constexpr bool success = true;
};

// Subtraction dispatcher (starts with '-')
template <char... Chars>
requires metacpp::lang::match_char <'-', CHARS> ::success
struct impl_ftn_dispatcher <CHARS> {
	// NOTE: successfuly matched plus
	using impl_next_start = typename metacpp::lang::match_whitespace <
		typename metacpp::lang::match_char <'-', CHARS> ::next
	> ::next;

	using impl_current_type = typename impl_parse_list <impl_next_start> ::type;
	static_assert(metacpp::size <impl_current_type> ::value == 2,
		"Expected two arguments to '-'"
	);

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = typename impl_op <op_type::minus, impl_current_type> ::type;

	static constexpr bool success = true;
};

// Division dispatcher (starts with '/')
template <char... Chars>
requires metacpp::lang::match_char <'/', CHARS> ::success
struct impl_ftn_dispatcher <CHARS> {
	// NOTE: successfuly matched plus
	using impl_next_start = typename metacpp::lang::match_whitespace <
		typename metacpp::lang::match_char <'/', CHARS> ::next
	> ::next;

	using impl_current_type = typename impl_parse_list <impl_next_start> ::type;
	static_assert(metacpp::size <impl_current_type> ::value == 2,
		"Expected two arguments to '/'"
	);

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = typename impl_op <op_type::divide, impl_current_type> ::type;

	static constexpr bool success = true;
};

// Expression dispatcher (starts with '(')
template <char C, char... Chars>
requires (C == '(')
struct impl_ftn_dispatcher <CCHARS> {
	using impl_next_start = typename metacpp::lang::match_whitespace <
		CHARS
	> ::next;

	using next = typename impl_ftn_dispatcher <impl_next_start> ::next;
	using type = metacpp::data::generic_list <
		typename impl_ftn_dispatcher <impl_next_start> ::type
	>;

	static constexpr bool success = true;
};

// Integer dispatcher
template <char... Chars>
requires (metacpp::lang::match_float <double, CHARS> ::success
	&& !metacpp::lang::match_float <double, CHARS> ::dot)
struct impl_ftn_dispatcher <CHARS> {
	// Skip whitespace
	using next = typename metacpp::lang::match_whitespace <
		typename metacpp::lang::match_int <int, CHARS> ::next
	> ::next;

	using type = metacpp::data::generic_list <
		Int <metacpp::lang::match_int <int, CHARS> ::value()>
	>;

	static constexpr bool success = true;
};

// Read float when there is a dot
template <char... Chars>
requires (metacpp::lang::match_float <double, CHARS> ::success
	&& metacpp::lang::match_float <double, CHARS> ::dot)
struct impl_ftn_dispatcher <CHARS> {
	// Skip whitespace
	using next = typename metacpp::lang::match_whitespace <
		typename metacpp::lang::match_float <double, CHARS> ::next
	> ::next;

	using type = metacpp::data::generic_list <
		Float <metacpp::lang::match_float <double, CHARS> ::value()>
	>;

	static constexpr bool success = true;
};

// Check if a string is non-whitespace
template <typename T>
requires metacpp::data::impl_is_string <T> ::value
struct impl_is_non_whitespace {
	static constexpr bool value = false;
};

template <char C, char... Chars>
requires (C != ' ' && C != '\t' && C != '\n')
struct impl_is_non_whitespace <CCHARS> {
	static constexpr bool value = true;
};

template <char C, char... Chars>
requires (C == ' ' || C == '\t' || C == '\n')
struct impl_is_non_whitespace <CCHARS> {
	static constexpr bool value = impl_is_non_whitespace <CHARS> ::value;
};

// Final dispatcher (starts at beginning of string)
template <typename>
struct eval {
	using type = metacpp::data::generic_list <>;
};

template <char... Chars>
requires impl_is_non_whitespace <CHARS> ::value
struct eval <CHARS> {
	using impl_next_start = typename metacpp::lang::match_whitespace <CHARS> ::next;
	using impl_current_type = typename impl_ftn_dispatcher <impl_next_start> ::type;
	using impl_after_current = typename impl_ftn_dispatcher <impl_next_start> ::next;

	using type = metacpp::concat_t <
		impl_current_type,
		typename eval <impl_after_current> ::type
	>;

	// NOTE: no next, since we expect to parse the entire string
};

template <typename T>
using eval_t = typename eval <T> ::type;

}										// namespace lisp

// + Meta overrrides
namespace metacpp::io {

// Printing integers
// TODO: generate at compile time
template <long int X>
struct impl_printf <lisp::Int <X>> {
	static std::string value() {
		return std::to_string(X);
	}
};

// Printing floats
template <double X>
struct impl_printf <lisp::Float <X>> {
	static std::string value() {
		return std::to_string(X);
	}
};

}
