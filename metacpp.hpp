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

// Custom compile-time string derived from list
template <char... Chars>
using string = list <char, Chars...>;

// TODO: alias from list below...

// String constructor
template <size_t N, typename F, size_t... indices>
constexpr auto impl_make_seq_helper(F f, std::index_sequence <indices...> is)
{
	return string <(f()[indices])...> {};
}

template <typename F>
constexpr auto impl_make_seq(F f)
{
	constexpr size_t N = f().size();
	using indices = std::make_index_sequence <N>;
	return impl_make_seq_helper <N> (f, indices {});
};

template <const char *str>
struct to_string {
	constexpr static auto value = data::impl_make_seq(
		[]() {
			return std::string_view {str};
		}
	);

	using type = decltype(value);
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

template <typename>
struct impl_is_string {
	static constexpr bool value = false;
};

template <char ... Chars>
struct impl_is_string <const data::string <Chars...>> {
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

// String constructors
template <const char *str>
using to_string_t = typename data::to_string <str> ::type;

// Type checks
template <typename T>
using is_list = data::impl_is_list <T>;

// WARNING: Use the constexpr versions with caution: they will haunt the
// resulting binary with a lot of template instantiations. If binary size is a
// concern, use the non-constexpr versions instead.
template <typename T>
static constexpr bool is_list_v = data::impl_is_list <T> ::value;

template <typename T>
using is_string = data::impl_is_string <T>;

template <typename T>
static constexpr bool is_string_v = data::impl_is_string <T> ::value;

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

// Matching individual characters
template <typename S, char E, int Index = 0>
requires is_string <S> ::value
struct match_char {
	static constexpr bool success = false;
	static constexpr int next = Index;
};

template <char ... Chars, char E, int Index>
struct match_char <const data::string <Chars...>, E, Index> {
	static constexpr bool success = (index <const data::string <Chars...>, Index> ::value == E);
	static constexpr int next = (success ? Index + 1 : Index);
};

// Matching strings
// TODO: use faster constexpr algorithm; retrieve arrays and strcmp them...
template <typename S, typename E, int Si = 0, int Ei = 0>
requires is_string <S> ::value && is_string <E> ::value
struct match_string {
	static constexpr bool success = false;
	static constexpr int next = Si;
};

template <char ... Chars, char ... Es, int Si, int Ei>
requires (index <const data::string <Chars...>, Si> ::value == index <const data::string <Es...>, Ei> ::value)
class match_string <const data::string <Chars...>, const data::string <Es...>, Si, Ei> {
	using impl_next_t = match_string <
		const data::string <Chars...>,
		const data::string <Es...>,
		Si + 1, Ei + 1
	>;

	static constexpr bool impl_success() {
		// If end of E is reached, success
		if constexpr (Ei == sizeof...(Es) - 1)
			return true;
		else
			return impl_next_t::success;
	}

	static constexpr int impl_next() {
		if constexpr (success)
			return impl_next_t::next;
		else
			return Si;
	}
public:
	static constexpr bool success = impl_success();
	static constexpr int next = impl_next();
};

// Skipping whitespace
template <typename T, int Index = 0>
requires data::impl_is_string <T> ::value
struct match_whitespace {
	static constexpr int removed = 0;
	static constexpr int next = Index;
};

template <char... Chars, int Index>
requires (Index >= 0 && Index < sizeof...(Chars))
class match_whitespace <const data::string <Chars...>, Index> {
	static constexpr char impl_char = index_variadic <char, Index, Chars...> ::value;
	static constexpr bool impl_space = (impl_char == ' ' || impl_char == '\t' || impl_char == '\n');

	static constexpr int impl_removed() {
		if constexpr (impl_space) {
			return 1 + match_whitespace <
				const data::string <Chars...>,
				Index + 1
			> ::removed;
		} else{
			return 0;
		}
	}

	static constexpr int impl_next() {
		if constexpr (impl_space) {
			return match_whitespace <
				const data::string <Chars...>,
				Index + 1
			> ::next;
		} else {
			return Index;
		}
	}
public:
	static constexpr int removed = impl_removed();
	static constexpr int next = impl_next();
};

// TODO: indicating failure?

// Reading integers from compile-time strings
template <typename, typename I, int Index = 0, bool _Start = true>
requires std::is_arithmetic <I> ::value
struct match_int {
	static constexpr bool success = false;
	static constexpr int next = Index;

	static constexpr I value(I accumulated = 0) {
		return accumulated;
	}
};

template <char ... Chars, typename I, int Index, bool _Start>
requires (Index >= 0 && Index < sizeof...(Chars))
class match_int <const data::string <Chars...>, I, Index, _Start> {
	static constexpr char impl_char = index_variadic <char, Index, Chars...> ::value;

	using impl_next_t = match_int <
		const data::string <Chars...>,
		I, Index + 1, false
	>;

	static constexpr bool impl_success() {
		if constexpr (sizeof...(Chars) == 0) {
			return false;
		} else {
			if constexpr (impl_char >= '0' && impl_char <= '9') {
				return true;
			} else if constexpr (impl_char == '-' && _Start) {
				return impl_next_t::success;
			} else {
				return false;
			}
		}
	}

	static constexpr int impl_next() {
		if constexpr (success)
			return impl_next_t::next;
		else
			return Index;
	}
public:
	static constexpr bool success = impl_success();
	static constexpr int next = impl_next();

	static constexpr I value(I accumulated = 0) {
		if constexpr (success) {
			if constexpr (impl_char == '-') {
				return -impl_next_t::value();
			} else {
				I value = impl_char - '0';
				I a = 10 * accumulated + value;
				if constexpr (impl_next_t::success)
					return impl_next_t::value(a);
				else
					return a;
			}
		} else {
			return 0;
		}
	}
};

// Reading floats from compile-time strings
template <typename, typename F, int Index = 0, bool _Start = true, bool _Dot = false>
requires std::is_arithmetic <F> ::value
struct match_float {
	static constexpr bool success = false;
	static constexpr int next = Index;
	static constexpr bool dot = false;

	static constexpr F value(F accumulated = 0) {
		return accumulated;
	}
};

template <char ... Chars, typename F, int Index, bool _Start, bool _Dot>
requires (Index >= 0 && Index < sizeof...(Chars))
struct match_float <const data::string <Chars...>, F, Index, _Start, _Dot> {
	static constexpr char impl_char = index_variadic <char, Index, Chars...> ::value;

	using impl_next_t = match_float <
		const data::string <Chars...>,
		F, Index + 1, false,
		_Dot || (impl_char == '.')
	>;

	static constexpr bool impl_success() {
		if constexpr (sizeof...(Chars) == 0) {
			return false;
		} else {
			if constexpr (impl_char >= '0' && impl_char <= '9') {
				return true;
			} else if constexpr (impl_char == '-' && _Start) {
				return impl_next_t::success;
			} else if constexpr (impl_char == '.' && !_Dot) {
				return impl_next_t::success;
			} else {
				return false;
			}
		}
	}

	static constexpr int impl_next() {
		if constexpr (success)
			return impl_next_t::next;
		else
			return Index;
	}

	static constexpr bool impl_dot() {
		if constexpr (success) {
			if constexpr (_Dot || impl_char == '.') {
				return true;
			} else {
				return impl_next_t::dot;
			}
		} else {
			return false;
		}
	}
public:
	static constexpr bool success = impl_success();
	static constexpr int next = impl_next();
	static constexpr bool dot = impl_dot();

	static constexpr F value(F accumulated = 0) {
		// TODO: split into smaller functions...
		if constexpr (success) {
			if constexpr (impl_char == '-') {
				return -impl_next_t::value();
			} else {
				if constexpr (_Dot) {
					F value = impl_char - '0';
					return (value + impl_next_t::value()) / 10;
				} else {
					if constexpr (impl_char == '.') {
						return accumulated + impl_next_t::value();
					} else {
						F value = impl_char - '0';
						F a = 10 * accumulated + value;
						if constexpr (impl_next_t::success)
							return impl_next_t::value(a);
						else
							return a;
					}
				}
			}
		} else {
			return 0;
		}
	}
};

template <typename T, int Index, char ... Chars>
using impl_arithmetic_parser = std::conditional_t <
	std::is_floating_point_v <T>,
	match_float <const data::string <Chars...>, T, Index>,
	match_int <const data::string <Chars...>, T, Index>
>;

// NOTE: match_list <data::string, type, count, start index>
template <typename, typename T, int Count = -1, int Index = 0>
struct match_list {
	static constexpr bool success = (Count == 0);
	static constexpr int next = Index;

	using type = data::list <T>;
};

template <typename T, char ... Chars, int Count, int Index>
requires (Index >= 0 && Index < sizeof...(Chars) && Count > 0)
class match_list <const data::string <Chars...>, T, Count, Index> {
	static constexpr bool impl_success() {
		if constexpr (impl_parser::success) {
			if constexpr (Count == 1) {
				return true;
			} else {
				if constexpr (impl_next_in_bounds && impl_comma::success) {
					return impl_next_t::success;
				} else {
					return false;
				}
			}
		} else {
			return false;
		}
	}

	static constexpr int impl_next() {
		if constexpr (success) {
			if constexpr (Count == 1)
				return impl_parser::next;
			else
				return impl_next_t::next;
		} else {
			return Index;
		}
	}
public:
	using impl_parser = impl_arithmetic_parser <T, Index, Chars...>;

	static constexpr bool impl_next_in_bounds = (impl_parser::next < sizeof...(Chars));
	static constexpr int impl_next_index = impl_next_in_bounds ? impl_parser::next : Index;

	using impl_comma = match_char <const data::string <Chars...>, ',', impl_next_index>;
	using impl_next_t = match_list <
		const data::string <Chars...>,
		T, Count - 1,
		impl_comma::next
	>;

	static constexpr bool success = impl_success();
	static constexpr int next = impl_next();

	template <bool, typename>
	struct select {
		using type = data::list <T>;
	};

	template <typename U, U ... Us>
	struct select <true, data::list <U, Us...>> {
		using type = data::list <U,
			impl_parser::value(),
			Us...
		>;
	};

	using type = typename select <success, typename impl_next_t::type> ::type;
	using next_type = typename impl_next_t::type;
};

// Unrestricted length
template <typename T, char ... Chars, int Index>
requires (Index >= 0 && Index < sizeof...(Chars))
class match_list <const data::string <Chars...>, T, -1, Index> {
	static constexpr int impl_next() {
		if constexpr (impl_parser::success) {
			if constexpr (impl_next_in_bounds && impl_comma::success) {
				return impl_next_t::next;
			} else {
				return impl_parser::next;
			}
		} else {
			return Index;
		}
	}
public:
	using impl_parser = impl_arithmetic_parser <T, Index, Chars...>;

	static constexpr bool impl_next_in_bounds = (impl_parser::next < sizeof...(Chars));
	static constexpr int impl_next_index = impl_next_in_bounds ? impl_parser::next : Index;

	using impl_comma = match_char <const data::string <Chars...>, ',', impl_next_index>;
	using impl_next_t = match_list <
		const data::string <Chars...>,
		T, -1 - !impl_next_in_bounds,
		impl_comma::next
	>;

	// Always successful
	static constexpr bool success = true;
	static constexpr int next = impl_next();

	// Building the resulting type
	template <bool, typename>
	struct select {
		using type = data::list <T>;
	};

	template <typename U, U ... Us>
	struct select <true, data::list <U, Us...>> {
		using type = data::list <U,
			impl_parser::value(),
			Us...
		>;
	};

	using type = typename select <
		impl_parser::success,
		typename impl_next_t::type
	> ::type;

	using next_type = typename impl_next_t::type;
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

/* Printing generic lists
template <typename T, typename ... Ts>
struct impl_printf <T, Ts...> {
	static std::string value() {
		if constexpr (sizeof ... (Ts) == 0) {
			return impl_printf <T> ::value();
		} else {
			return impl_printf <T> ::value() + ", "
				+ impl_printf <Ts...> ::value();
		}
	}
};

template <typename T, typename ... Ts>
struct impl_printf <data::generic_list <T, Ts...>> {
	static std::string value() {
		return "(" + impl_printf <T, Ts...> ::value() + ")";
	}
}; */

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
