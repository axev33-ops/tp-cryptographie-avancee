#include <stdlib.h>
#include "isd.h"

void generate_random_error(CodeWord * e, size_t t);
double compute_expected_iterations(size_t n, size_t k, size_t t);
void run_batch_test(FILE * csv_file, size_t n, size_t k, size_t t,
		    size_t n_runs);
