#include "metacpp.hpp"
#include "lisp.hpp"

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
	std::is_same_v <metacpp::concat_t <list3, list2>, list4>,
	"concat_t failed"
);

int rt_main()
{
	printf("Results:\n");
	printf("list1 erase back: %s\n", typeid(metacpp::erase_back_t <int, list1>).name());
	printf("\tvalue: %d\n", metacpp::erase_back_t <int, list1> ::value);
	printf("list3: %s\n", typeid(list3).name());
	printf("concat: %s\n", typeid(metacpp::concat_t <list2, list3>).name());
	printf("list4: %s\n", typeid(list4).name());
	return 0;
}

}

namespace test_generic_list {

using generic_list1 = metacpp::data::generic_list <int, float, char>;

using i0 = metacpp::index_t <generic_list1, 0>;
using i1 = metacpp::index_t <generic_list1, 1>;
using i2 = metacpp::index_t <generic_list1, 2>;

static_assert(
	std::is_same_v <i0, int>
	&& std::is_same_v <i1, float>
	&& std::is_same_v <i2, char>
);

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
static_assert(metacpp::lang::match_float <float, float_str_type> ::dot);
static_assert(metacpp::lang::match_float <float, float_str_type> ::value() == 123.456f);

// Negative float parsing
constexpr char negative_float_str[] = "-123.456";
using negative_float_str_type = metacpp::to_string_t <negative_float_str>;
static_assert(metacpp::lang::match_float <float, negative_float_str_type> ::success);
static_assert(metacpp::lang::match_float <float, negative_float_str_type> ::value() == -123.456f);

// Edge cases
constexpr char minus_sign[] = "-";
using minus_sign_type = metacpp::to_string_t <minus_sign>;
static_assert(!metacpp::lang::match_int <int, minus_sign_type> ::success);
static_assert(!metacpp::lang::match_float <float, minus_sign_type> ::success);

}

// TODO: negative number parsing...
// TODO: reduce compile time by avoiding concepts/requirements?
// TODO: benchmark such cases...
constexpr char lisp_source[] = R"(
(list 1.05 2.77 (list 3.14 2.71) (+ 1 2) (- 3.5 (* 3 1.5)))
)";

using lisp_source_type = metacpp::to_string_t <lisp_source>;
using results = typename lisp::eval_t <lisp_source_type>;

// TODO: branching

#include <array>

template <size_t Na, size_t Nb>
constexpr int strcmp(const std::array <char, Na> &a, const std::array <char, Nb> &b)
{
	for (size_t i = 0; i < Na && i < Nb; i++) {
		if (a[i] != b[i])
			return a[i] - b[i];
	}

	return Na - Nb;
}

int main()
{
	// test_lists::rt_main();
	printf("RESULTS: %s\n", metacpp::io::to_string <results> ().data());
	return 0;
}
