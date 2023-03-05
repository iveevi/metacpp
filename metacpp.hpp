#pragma once

// Standard headers
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

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

template <typename T, int Index, T x, T ... Values>
struct impl_index <list <T, x, Values...>, Index> {
	static constexpr T value = impl_index <list <T, Values...>, Index - 1> ::value;
};

template <typename T, T x, T ... Values>
struct impl_index <list <T, x, Values...>, 0> {
	static constexpr T value = x;
};

// Insertion/pushing
template <typename T, typename, T>
struct impl_insert_back {};

template <typename T, T x, T ... Values>
struct impl_insert_back <T, list <T, Values...>, x> {
	using type = list <T, Values..., x>;
};

template <typename T, typename, T>
struct impl_insert_front {};

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

// TODO: parsing namespace...
// Matching characters and substrings
template <char E, typename S>
requires is_string <S> ::value
struct match_char {
	static constexpr bool success = false;
	using next = S;
};

template <char E, char ... Chars>
struct match_char <E, const data::string <E, Chars...>> {
	static constexpr bool success = true;
	using next = const data::string <Chars...>;
};

template <typename E, typename S>
requires is_string <E> ::value && is_string <S> ::value
struct match_string {
	static constexpr bool success = false;
	using next = S;
};

template <typename S>
struct match_string <const data::string <>, S> {
	static constexpr bool success = true;
	using next = S;
};

template <char E, char ... Es, char ... Chars>
struct match_string <const data::string <E, Es...>, const data::string <E, Chars...>> {
	static constexpr bool success = match_string <
		const data::string <Es...>,
		const data::string <Chars...>
	> ::success;

	using next = std::conditional_t <
		success,
		typename match_string <
			const data::string <Es...>,
			const data::string <Chars...>
		> ::next,
		const data::string <E, Chars...>
	>;
};

// Skipping whitespace
template <typename T>
requires data::impl_is_string <T> ::value
struct match_whitespace {
	static constexpr int removed = 0;
	using next = T;
};

template <char C, char... Chars>
requires (C == ' ' || C == '\t' || C == '\n')
struct match_whitespace <const data::string <C, Chars...>> {
	static constexpr int removed = 1 + match_whitespace <
		const data::string <Chars...>
	> ::removed;

	using next = typename match_whitespace <
		const data::string <Chars...>
	> ::next;
};

// TODO: indicating failure?

// Reading integers from compile-time strings
template <typename I, typename>
struct match_int {
	static constexpr bool success = false;
	using next = const data::string <>;
};

// NOTE: Unconstrained specialization is used to terminate the recursion
template <typename I, char C, char ... Chars>
struct match_int <I, const data::string <C, Chars...>> {
	static constexpr bool success = false;
	using next = const data::string <C, Chars...>;

	static constexpr I value(I accumulated = 0) {
		return accumulated;
	}
};

template <typename I, char C, char ... Chars>
requires (C >= '0' && C <= '9')
struct match_int <I, const data::string <C, Chars...>> {
	using impl_rest = const data::string <Chars...>;

	static constexpr bool success = true;
	using next = typename match_int <I, impl_rest> ::next;

	static constexpr I value(I accumulated = 0) {
		I a = accumulated * 10 + (C - '0');
		if constexpr (sizeof...(Chars) > 0)
			return match_int <I, impl_rest> ::value(a);
		else
			return a;
	}
};

// WARNING: Need to fix syntax like --1
template <typename I, char C, char ... Chars>
requires (C == '-') && match_int <I, const data::string <Chars...>> ::success
struct match_int <I, const data::string <C, Chars...>> {
	using impl_rest = const data::string <Chars...>;

	static constexpr bool success = match_int <I, impl_rest> ::success;
	using next = typename match_int <I, impl_rest> ::next;

	static constexpr I value(I accumulated = 0) {
		return -match_int <I, impl_rest> ::value(accumulated);
	}
};

// Reading floats from compile-time strings
template <typename F, typename>
struct match_float {
	static constexpr bool success = false;
};

template <typename F, typename, typename>
requires std::is_floating_point_v <F>
struct impl_match_float {
	using next = const data::string <>;
};

template <typename F, typename B, char C, char ... Chars>
struct impl_match_float <F, B, const data::string <C, Chars...>> {
	using next = const data::string <C, Chars...>;

	static constexpr bool dot() {
		return false;
	}

	static constexpr F value(F accumulated) {
		return accumulated;
	}

	static constexpr F value() {
		return 0;
	}
};

template <typename F, char C, char ... Chars>
requires (C >= '0' && C <= '9' || C == '.')
struct impl_match_float <F, std::false_type, const data::string <C, Chars...>> {
	// NOTE: second parameter is a std::false_type, so we are not
	// reading the decimal part of the float
	using impl_rest = const data::string <Chars...>;
	using next = typename impl_match_float <F, std::false_type, impl_rest> ::next;

	static constexpr bool dot() {
		if constexpr (sizeof...(Chars) > 0) {
			if constexpr (C == '.') {
				return true;
			} else {
				return impl_match_float <F, std::false_type, impl_rest> ::dot();
			}
		} else {
			return false;
		}
	}

	static constexpr F value(F accumulated = 0) {
		// TODO: simplify this nested...
		if constexpr ((C < '0' || C > '9') && C != '.') {
			return accumulated;
		} else {
			if constexpr (sizeof...(Chars) > 0) {
				if constexpr (C == '.') {
					return accumulated + impl_match_float <F, std::true_type, impl_rest> ::value();
				} else {
					F a = accumulated * 10 + (C - '0');
					return impl_match_float <F, std::false_type, impl_rest> ::value(a);
				}
			} else {
				F a = accumulated * 10 + (C - '0');
				return a;
			}
		}
	}
};

template <typename F, char C, char ... Chars>
requires (C >= '0' && C <= '9')
struct impl_match_float <F, std::true_type, const data::string <C, Chars...>> {
	// NOTE: second parameter is a std::false_type, so we are not
	// reading the decimal part of the float

	using impl_rest = const data::string <Chars...>;
	using next = typename impl_match_float <F, std::true_type, impl_rest> ::next;

	static constexpr F value() {
		if constexpr (C < '0' || C > '9') {
			return 0.0f;
		} else {
			if constexpr (sizeof...(Chars) > 0)
				return ((C - '0') + impl_match_float <F, std::true_type, impl_rest> ::value())/10.0f;
			else
				return (C - '0')/10.0f;
		}
	}
};

template <typename F, char C, char ... Chars>
requires (C >= '0' && C <= '9' || C == '.')
struct match_float <F, const data::string <C, Chars...>> {
	static constexpr bool success = true;
	using impl_parser = impl_match_float <F, std::false_type, const data::string <C, Chars...>>;
	using next = typename impl_parser::next;

	static constexpr bool dot = impl_parser::dot();
	static constexpr F value() {
		return impl_parser::value();
	}
};

// WARNING: This allows for composed negative numbers, like --1
template <typename F, char C, char ... Chars>
requires (C == '-') && match_float <F, const data::string <Chars...>> ::success
struct match_float <F, const data::string <C, Chars...>> {
	static constexpr bool success = match_float <F, const data::string <Chars...>> ::success;
	using next = typename match_float <F, const data::string <Chars...>> ::next;

	static constexpr bool dot = match_float <F, const data::string <Chars...>> ::dot();
	static constexpr F value() {
		return -match_float <F, const data::string <Chars...>> ::value();
	}
};

template <typename T, char ... Chars>
using impl_arithmetic_parser = typename std::conditional <
	std::is_floating_point_v <T>,
	match_float <T, const data::string <Chars...>>,
	match_int <T, const data::string <Chars...>>
> ::type;

// NOTE: Readlist <Type, Count, data::string>
template <typename, int, typename>
struct match_list {};

template <typename T, int, int, typename S>
struct impl_match_list {
	static constexpr bool success = false;

	using type = data::list <T>;
	using next = S;
};

template <typename T, int Count, int Index>
struct impl_match_list <T, Count, Index, const data::string <>> {
	static constexpr bool success = true;

	using type = data::list <T>;
	using next = const data::string <>;
};

template <typename T, int Count, int Index, char ... Chars>
requires std::is_arithmetic_v <T> && (Index < Count)
struct impl_match_list <T, Count, Index, const data::string <Chars...>> {
	using impl_parser = impl_arithmetic_parser <T, Chars...>;
	using impl_comma = match_char <',', typename impl_parser::next>;

	// NOTE: Looks complex, but is conceptually simple:
	// 1. Parse the first element
	// 2. If we're at the end of the list, then we're done
	// 3. Otherwise, parse a comma and then recurse
	static constexpr bool success = impl_parser::success
		&& ((Index + 1 == Count)
		|| (impl_comma::success
			&& impl_match_list <
				T, Count, Index + 1, typename impl_comma::next
			> ::success)
		);

	using type = insert_front_t <
		T,
		typename impl_match_list <
			T, Count, Index + 1, typename impl_comma::next
		> ::type,
		impl_parser::value()
	>;

	using next = std::conditional_t <
		success,
		typename impl_match_list <
			T, Count, Index + 1, typename impl_comma::next
		> ::next,
		const data::string <Chars...>
	>;
};

// TODO: delimiter?
template <typename T, int Count, char ... Chars>
requires std::is_arithmetic_v <T>
struct match_list <T, Count, const data::string <Chars...>> {
	static constexpr bool success = impl_match_list <
		T, Count, 0, const data::string <Chars...>
	> ::success;

	using impl_list_parser = impl_match_list <
		T, Count, 0, const data::string <Chars...>
	>;

	using type = typename impl_list_parser::type;
	using next = typename impl_list_parser::next;
};

}

namespace io {

// Printer
template <typename...>
struct impl_printf {
	static std::string value() {
		return "?";
	}
};

// Printing generic lists
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
}; 

// Printing wrapper
template <typename T>
std::string to_string() {
	return impl_printf <T> ::value();
}

}

}
