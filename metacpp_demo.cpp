#include "metacpp.hpp"
// #include "lisp.hpp"

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

namespace test_lang_string {

constexpr char str[] = "abc abc";
constexpr char match_str1[] = "abc";
constexpr char match_str2[] = "a";
constexpr char match_str3[] = "d";

/* using str_type = metacpp::to_string_t <str>;
using match_str_type1 = metacpp::to_string_t <match_str1>;
using match_str_type2 = metacpp::to_string_t <match_str2>;
using match_str_type3 = metacpp::to_string_t <match_str3>; */

constexpr metacpp::data::constexpr_string constexpr_str(str, sizeof(str) - 1);
constexpr metacpp::data::constexpr_string constexpr_match_str1(match_str1, sizeof(match_str1) - 1);
constexpr metacpp::data::constexpr_string constexpr_match_str2(match_str2, sizeof(match_str2) - 1);
constexpr metacpp::data::constexpr_string constexpr_match_str3(match_str3, sizeof(match_str3) - 1);

static_assert(metacpp::lang::match_char(constexpr_str, 'a').success);
static_assert(metacpp::lang::match_char(constexpr_str, 'a').next == 1);

static_assert(metacpp::lang::match_char(constexpr_str, 'b', 1).success);
static_assert(metacpp::lang::match_char(constexpr_str, 'b', 1).next == 2);

static_assert(metacpp::lang::match_string(constexpr_str, constexpr_match_str1).success);
static_assert(metacpp::lang::match_string(constexpr_str, constexpr_match_str1).next == 3);

static_assert(metacpp::lang::match_string(constexpr_str, constexpr_match_str2).success);
static_assert(metacpp::lang::match_string(constexpr_str, constexpr_match_str2).next == 1);

static_assert(!metacpp::lang::match_string(constexpr_str, constexpr_match_str3).success);
static_assert(metacpp::lang::match_string(constexpr_str, constexpr_match_str3).next == 0);

}

namespace test_lang_whitespace {

constexpr char str[] = R"(
	abc
	abc
)";

constexpr metacpp::data::constexpr_string constexpr_str(str, sizeof(str) - 1);

static_assert(metacpp::lang::match_whitespace(constexpr_str).removed == 2);
static_assert(metacpp::lang::match_whitespace(constexpr_str).next == 2);

static_assert(metacpp::lang::match_whitespace(constexpr_str, 4).removed == 0);
static_assert(metacpp::lang::match_whitespace(constexpr_str, 5).removed == 2);

static_assert(metacpp::lang::match_whitespace(constexpr_str, 9).removed == 0);
static_assert(metacpp::lang::match_whitespace(constexpr_str, 10).removed == 1);

}

namespace test_lang_numeric {

constexpr char int_str[] = "123";
constexpr metacpp::data::constexpr_string constexpr_int_str(int_str, sizeof(int_str) - 1);

// Integer parsing
static_assert(metacpp::lang::match_int <int> (constexpr_int_str).success);
static_assert(metacpp::lang::match_int <int> (constexpr_int_str).value == 123);

// Making sure that the next index is correct
static_assert(metacpp::lang::match_int <int> (constexpr_int_str).next == 3);

// Negative integer parsing
constexpr char negative_int_str[] = "-123";
constexpr metacpp::data::constexpr_string constexpr_negative_int_str(negative_int_str, sizeof(negative_int_str) - 1);
static_assert(metacpp::lang::match_int <int> (constexpr_negative_int_str).success);
static_assert(metacpp::lang::match_int <int> (constexpr_negative_int_str).value == -123);

// Float parsing
constexpr char float_str[] = "123.456";
constexpr metacpp::data::constexpr_string constexpr_float_str(float_str, sizeof(float_str) - 1);
static_assert(metacpp::lang::match_float <float> (constexpr_float_str).success);
static_assert(metacpp::lang::match_float <float> (constexpr_float_str).dot);
static_assert(metacpp::lang::match_float <float> (constexpr_float_str).value == 123.456f);

// Negative float parsing
constexpr char negative_float_str[] = "-123.456";
constexpr metacpp::data::constexpr_string constexpr_negative_float_str(negative_float_str, sizeof(negative_float_str) - 1);
static_assert(metacpp::lang::match_float <float> (constexpr_negative_float_str).success);
static_assert(metacpp::lang::match_float <float> (constexpr_negative_float_str).value == -123.456f);

// Edge cases
constexpr char minus_sign[] = "-";
constexpr metacpp::data::constexpr_string constexpr_minus_sign(minus_sign, sizeof(minus_sign) - 1);
static_assert(!metacpp::lang::match_int <int> (constexpr_minus_sign).success);
static_assert(!metacpp::lang::match_float <float> (constexpr_minus_sign).success);

constexpr char double_minus_sign[] = "--5";
constexpr metacpp::data::constexpr_string constexpr_double_minus_sign(double_minus_sign, sizeof(double_minus_sign) - 1);
static_assert(!metacpp::lang::match_int <int> (constexpr_double_minus_sign).success);
static_assert(!metacpp::lang::match_float <float> (constexpr_double_minus_sign).success);

}

namespace test_lang_list {

constexpr char int_list_str[] = "1,23,456,7890";
constexpr metacpp::data::constexpr_string constexpr_int_list_str(int_list_str, sizeof(int_list_str) - 1);
using int_list_expected = metacpp::data::list <int, 1, 23, 456, 7890>;
static_assert(metacpp::lang::match_list <constexpr_int_list_str, int, 4> ::success);
static_assert(metacpp::lang::match_list <constexpr_int_list_str, int, 4> ::next == constexpr_int_list_str.size);
static_assert(std::is_same <
	metacpp::lang::match_list <constexpr_int_list_str, int, 4> ::type,
	int_list_expected
> ::value);

constexpr char float_list_str[] = "1,23,456,7890";
constexpr metacpp::data::constexpr_string constexpr_float_list_str(float_list_str, sizeof(float_list_str) - 1);
using float_list_expected = metacpp::data::list <float, 1.0f, 23.0f, 456.0f, 7890.0f>;
static_assert(metacpp::lang::match_list <constexpr_float_list_str, float, 4> ::success);
static_assert(metacpp::lang::match_list <constexpr_float_list_str, float, 4> ::next == constexpr_float_list_str.size);
static_assert(std::is_same <
	metacpp::lang::match_list <constexpr_float_list_str, float, 4> ::type,
	float_list_expected
> ::value);

constexpr char single_list_str[] = "1";
constexpr metacpp::data::constexpr_string constexpr_single_list_str(single_list_str, sizeof(single_list_str) - 1);
using single_list_expected = metacpp::data::list <int, 1> ;
static_assert(metacpp::lang::match_list <constexpr_single_list_str, int, 1> ::success);
static_assert(metacpp::lang::match_list <constexpr_single_list_str, int, 1> ::next == 1);
static_assert(std::is_same <
	metacpp::lang::match_list <constexpr_single_list_str, int, 1> ::type,
	single_list_expected
> ::value);

constexpr char forever_list_str[] = "1,2,3,4,5,6,7,8,9";
// TODO: pass delimiter string...
constexpr metacpp::data::constexpr_string constexpr_forever_list_str(forever_list_str, sizeof(forever_list_str) - 1);
using forever_list_expected = metacpp::data::list <int, 1, 2, 3, 4, 5, 6, 7, 8, 9> ;
static_assert(metacpp::lang::match_list <constexpr_forever_list_str, int, -1> ::success);
static_assert(metacpp::lang::match_list <constexpr_forever_list_str, int, -1> ::next == sizeof(forever_list_str) - 1);
static_assert(std::is_same <
	metacpp::lang::match_list <constexpr_forever_list_str, int, -1> ::type,
	forever_list_expected
> ::value);

void rt_main()
{
	std::string int_list_s;
	std::string float_list_s;
	std::string single_list_s;
	std::string forever_list_s = metacpp::io::to_string <
		metacpp::lang::match_list <constexpr_forever_list_str, int, -1> ::type
	> ();

	printf("int_list_s: %s\n", int_list_s.c_str());
	printf("float_list_s: %s\n", float_list_s.c_str());
	printf("single_list_s: %s\n", single_list_s.c_str());
	printf("forever_list_s: %s\n", forever_list_s.c_str());
}

}

/*
constexpr char lisp_source[] = R"(
(list 1.05 2.77 (list 3.14 2.71) (+ 1 2) (- 3.5 (* 3 1.5)))
)";

using lisp_source_type = metacpp::to_string_t <lisp_source>;
using results = typename lisp::eval_t <lisp_source_type>;

// TODO: branching

struct constexpr_string {
	const char *str;
	size_t size;

	explicit constexpr constexpr_string(const char *str, size_t size)
		: str(str), size(size) {}
};

template <constexpr_string s, int Index>
struct constexpr_string_at {
	static constexpr char value = s.str[Index];
}; */

int main()
{
	/* test_lang_list::rt_main();
	printf("RESULTS: %s\n", metacpp::io::to_string <results> ().data());
	constexpr constexpr_string s(lisp_source, sizeof(lisp_source) - 1);
	printf("s: %s, size: %d, string size= %d, char=%c\n",
		s.str, s.size, std::string(lisp_source).size(),
		constexpr_string_at <s, 10> ::value
	);
	constexpr_string_at <s, 10> x;
	printf("x: %c\n", x.value); */

	return 0;
}
