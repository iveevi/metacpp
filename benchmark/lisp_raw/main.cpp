#include "metacpp.hpp"
#include "lisp.hpp"

#ifndef LISP_SOURCE
#define LISP_SOURCE "(+ 1 2)"
#endif

// Benchmarking initial version of the lisp compile-time interpreter
constexpr char lisp_source[] = LISP_SOURCE;

using lisp_source_type = metacpp::to_string_t <lisp_source>;
using results = typename lisp::eval_t <lisp_source_type>;

int main()
{
	printf("results: %s\n", metacpp::io::to_string <results> ().data());
	return 0;
}
