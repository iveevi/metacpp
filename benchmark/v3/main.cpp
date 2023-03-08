#include "metacpp.hpp"
#include "lisp.hpp"

#ifndef LISP_SOURCE
#define LISP_SOURCE "(+ 1 2)"
#endif

constexpr char lisp_source[] = LISP_SOURCE;
constexpr metacpp::data::constexpr_string lisp_source_str(lisp_source, sizeof(lisp_source) - 1);
using results = typename lisp::eval_t <lisp_source_str>;

int main()
{
	printf("RESULTS: %s\n", metacpp::io::to_string <results> ().data());
}
