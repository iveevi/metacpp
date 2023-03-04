#include "metacpp.hpp"

#include <stdio.h>
#include <typeinfo>

// Testing lists
namespace test_lists {

using list1 = metacpp::data::list <int, 1, 2, 3, 4, 5>;
using list2 = metacpp::data::list <int, 2, 3, 4, 5>;
using list3 = metacpp::data::list <int, 1, 2, 3, 4>;
using list4 = metacpp::data::list <int, 1, 2, 3, 4, 2, 3, 4, 5>;

static_assert(
	std::is_same_v <metacpp::insert_front_t <int, list2, 1>, list1>,
	"insert_front_t failed"
);

static_assert(
	// TODO: success field to check if erase_front_t succeeded
	std::is_same_v <metacpp::erase_front_t <int, list1>, list2>
	&& metacpp::erase_front_v <int, list1> == 1,
	"erase_front_t failed"
);

static_assert(
	std::is_same_v <metacpp::erase_back_t <int, list1>, list3>
	&& metacpp::erase_back_v <int, list1> == 5,
	"erase_back_t failed"
);

static_assert(
	std::is_same_v <metacpp::concat_t <int, list3, list2>, list4>,
	"concat_t failed"
);

int rt_main()
{
	printf("Results:\n");
	printf("list1 erase back: %s\n", typeid(metacpp::erase_back_t <int, list1>).name());
	printf("\tvalue: %d\n", metacpp::erase_back_t <int, list1> ::value);
	printf("list3: %s\n", typeid(list3).name());
	printf("concat: %s\n", typeid(metacpp::concat_t <int, list2, list3>).name());
	printf("list4: %s\n", typeid(list4).name());
	return 0;
}

}

namespace test_lang {

constexpr char int_str[] = "123";

// Basic string/list check
using int_str_type = metacpp::to_string_t <int_str>;
static_assert(metacpp::is_string <int_str_type> ::value);
static_assert(metacpp::is_list <int_str_type> ::value);

// Integer parsing
static_assert(metacpp::lang::match_int <int, int_str_type> ::success);
static_assert(metacpp::lang::match_int <int, int_str_type> ::value() == 123);

// Making sure that the next type is empty
using int_str_next_type = metacpp::lang::match_int <int, int_str_type> ::next;
static_assert(metacpp::is_empty <int_str_next_type> ::value);

// Negative integer parsing
constexpr char negative_int_str[] = "-123";
using negative_int_str_type = metacpp::to_string_t <negative_int_str>;
static_assert(metacpp::lang::match_int <int, negative_int_str_type> ::success);
static_assert(metacpp::lang::match_int <int, negative_int_str_type> ::value() == -123);

// Float parsing
constexpr char float_str[] = "123.456";
using float_str_type = metacpp::to_string_t <float_str>;
static_assert(metacpp::lang::match_float <float, float_str_type> ::success);
static_assert(metacpp::lang::match_float <float, float_str_type> ::value() == 123.456f);

// Negative float parsing
constexpr char negative_float_str[] = "-123.456";
using negative_float_str_type = metacpp::to_string_t <negative_float_str>;
static_assert(metacpp::lang::match_float <float, negative_float_str_type> ::success);
static_assert(metacpp::lang::match_float <float, negative_float_str_type> ::value() == -123.456f);

}

// TODO: negative number parsing...
constexpr char str[] = R"(
(list 1 2 3)
(list 4 5 6 7 8 9)
(list 10 11 12 13 14 15 16 17 18 19 20)
(+ 1 2 3 5)
(* 2 3 4 5 6 -7 8 9 10)
(- 10 93)
(/ 10 3)
)";

// TODO: extend to support more types

// Result of Lisp parsing
template <typename...>
struct Results {};

template <typename T, typename ... Ts>
struct append_result {};

template <typename ... Ts, typename ... Us>
struct append_result <Results <Ts...>, Results <Us...>> {
	using type = Results <Ts..., Us...>;
};

using str_type = metacpp::to_string_t <str>;
static_assert(metacpp::is_string <str_type> ::value);

template <typename T>
requires metacpp::data::impl_is_string <T> ::value
struct impl_skip_whitespace {
	static constexpr int removed = 0;
	using next = T;
};

template <char C, char... Chars>
requires (C == ' ' || C == '\t' || C == '\n')
struct impl_skip_whitespace <const metacpp::data::string <C, Chars...>> {
	static constexpr int removed = 1 + impl_skip_whitespace <
		const metacpp::data::string <Chars...>
	> ::removed;

	using next = typename impl_skip_whitespace <
		const metacpp::data::string <Chars...>
	> ::next;
};

// Lisp parser
namespace lisp {								// namespace lisp

#define CHARS const metacpp::data::string <Chars...>
#define CCHARS const metacpp::data::string <C, Chars...>

// Fundamental types
template <long int N>
struct Int {};

// Constructor for list types
constexpr char impl_list_cstr[] = "list"; // Integer list
using impl_list_str = metacpp::to_string_t <impl_list_cstr>;

template <typename T>
using impl_list_matcher = metacpp::lang::match_string <impl_list_str, T>;

// Parse list (TODO: generic type...)
template <typename>
struct impl_parse_list {};

template <char C, char... Chars>
requires metacpp::lang::match_int <int, CCHARS> ::success
struct impl_parse_list <CCHARS> {
	// Skip whitespace
	using impl_next_start = typename impl_skip_whitespace <
		typename metacpp::lang::match_int <int, CCHARS> ::next
	> ::next;

	using impl_current_type = metacpp::data::list <int,
		metacpp::lang::match_int <int, CCHARS> ::value()
	>;

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = metacpp::concat_t <int,
		impl_current_type,
		typename impl_parse_list <impl_next_start> ::type
	>;
};

// Until we reach the end of the list (e.g. ')')
template <char C, char... Chars>
requires (C == ')')
struct impl_parse_list <CCHARS> {
	using type = metacpp::data::list <int>;
	using next = CHARS;
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
	static constexpr int value = 0;
};

template <typename T>
struct impl_op <op_type::multiply, T> {
	static constexpr int value = 1;
};

template <typename T, T x, T ... Ts>
struct impl_op <op_type::plus, metacpp::data::list <T, x, Ts...>> {
	static constexpr T value = x + impl_op <
		op_type::plus,
		metacpp::data::list <T, Ts...>
	> ::value;
};

template <typename T, T x, T ... Ts>
struct impl_op <op_type::multiply, metacpp::data::list <T, x, Ts...>> {
	static constexpr T value = x * impl_op <
		op_type::multiply,
		metacpp::data::list <T, Ts...>
	> ::value;
};

template <typename T, T x, T y>
struct impl_op <op_type::minus, metacpp::data::list <T, x, y>> {
	static constexpr T value = x - y;
};

template <typename T, T x, T y>
struct impl_op <op_type::divide, metacpp::data::list <T, x, y>> {
	static constexpr T value = x / y;
};

// Function dispatcher
template <typename T>
requires metacpp::data::impl_is_string <T> ::value
struct impl_ftn_dispatcher {};

// List dispatcher (starts with 'list')
template <char C, char... Chars>
requires impl_list_matcher <CCHARS> ::success
struct impl_ftn_dispatcher <CCHARS> {
	// NOTE: successfuly matched list
	using impl_next_start = typename impl_skip_whitespace <
		typename impl_list_matcher <CCHARS> ::next
	> ::next;

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = typename impl_parse_list <impl_next_start> ::type;
};

// Addition dispatcher (starts with '+')
template <char ... Chars>
requires metacpp::lang::match_char <'+', CHARS> ::success
struct impl_ftn_dispatcher <CHARS> {
	// NOTE: successfuly matched plus
	using impl_next_start = typename impl_skip_whitespace <
		typename metacpp::lang::match_char <'+', CHARS> ::next
	> ::next;

	using impl_current_type = typename impl_parse_list <impl_next_start> ::type;
	static_assert(metacpp::size <impl_current_type> ::value > 0,
		"Expected at least one argument to '+'"
	);

	static constexpr int value = impl_op <op_type::plus, impl_current_type> ::value;

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = Int <value>;
};

// Multiplication dispatcher (starts with '*')
template <char... Chars>
requires metacpp::lang::match_char <'*', CHARS> ::success
struct impl_ftn_dispatcher <CHARS> {
	// NOTE: successfuly matched plus
	using impl_next_start = typename impl_skip_whitespace <
		typename metacpp::lang::match_char <'*', CHARS> ::next
	> ::next;

	using impl_current_type = typename impl_parse_list <impl_next_start> ::type;
	static_assert(metacpp::size <impl_current_type> ::value > 0,
		"Expected at least one argument to '*'"
	);

	static constexpr int value = impl_op <op_type::multiply, impl_current_type> ::value;

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = Int <value>;
};

// Subtraction dispatcher (starts with '-')
template <char... Chars>
requires metacpp::lang::match_char <'-', CHARS> ::success
struct impl_ftn_dispatcher <CHARS> {
	// NOTE: successfuly matched plus
	using impl_next_start = typename impl_skip_whitespace <
		typename metacpp::lang::match_char <'-', CHARS> ::next
	> ::next;

	using impl_current_type = typename impl_parse_list <impl_next_start> ::type;
	static_assert(metacpp::size <impl_current_type> ::value > 0,
		"Expected two arguments to '-'"
	);

	static constexpr int value = impl_op <op_type::minus, impl_current_type> ::value;

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = Int <value>;
};

// Division dispatcher (starts with '/')
template <char... Chars>
requires metacpp::lang::match_char <'/', CHARS> ::success
struct impl_ftn_dispatcher <CHARS> {
	// NOTE: successfuly matched plus
	using impl_next_start = typename impl_skip_whitespace <
		typename metacpp::lang::match_char <'/', CHARS> ::next
	> ::next;

	using impl_current_type = typename impl_parse_list <impl_next_start> ::type;
	static_assert(metacpp::size <impl_current_type> ::value > 0,
		"Expected two arguments to '/'"
	);

	static constexpr int value = impl_op <op_type::divide, impl_current_type> ::value;

	using next = typename impl_parse_list <impl_next_start> ::next;
	using type = Int <value>;
};

// Expression dispatcher (starts with '(')
template <char C, char... Chars>
requires (C == '(')
struct impl_ftn_dispatcher <CCHARS> {
	using impl_next_start = typename impl_skip_whitespace <
		CHARS
	> ::next;

	using next = typename impl_ftn_dispatcher <impl_next_start> ::next;
	using type = Results <
		typename impl_ftn_dispatcher <impl_next_start> ::type
	>;
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
	using type = Results <>;
};

template <char... Chars>
requires impl_is_non_whitespace <CHARS> ::value
struct eval <CHARS> {
	using impl_next_start = typename impl_skip_whitespace <CHARS> ::next;
	using impl_current_type = typename impl_ftn_dispatcher <impl_next_start> ::type;
	using impl_after_current = typename impl_ftn_dispatcher <impl_next_start> ::next;

	using type = typename append_result <
		impl_current_type,
		typename eval <impl_after_current> ::type
	> ::type;

	// NOTE: no next, since we expect to parse the entire string
};

template <typename T>
using eval_t = typename eval <T> ::type;

}										// namespace lisp

using results = typename lisp::eval_t <str_type>;

int main()
{
	test_lists::rt_main();
	printf("results: %s\n", typeid(results).name());
	return 0;
}
