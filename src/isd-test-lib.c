#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "isd.h"
#include "isd-test-lib.h"

// CodeWord et pas ErrorWord car on suppose que le mot de code est 0
void generate_random_error(CodeWord *e, size_t t)
{
	vector_set_zeros(e);

	size_t weight = 0;
	while (weight < t) {
		size_t random_index = rand() % e->length;
		if (vector_get_bit(e, random_index) == 0) {
			vector_set_bit_1(e, random_index);
			weight++;
		}
	}
}

// (t parmi n) / (t parmi n - k) = \prod_{i = 0}^{t - 1} (n - i) / (n - k - i)
double compute_expected_iterations(size_t n, size_t k, size_t t)
{
	size_t n_minus_k = n - k;
	double expected = 1.0;

	for (size_t i = 0; i < t; i++) {
		expected *= (double)(n - i) / (double)(n_minus_k - i);
	}

	return expected;
}

void run_batch_test(FILE *csv_file, size_t n, size_t k, size_t t, size_t n_runs)
{
	printf("\nTest de décodage : n = %zu, k = %zu, t = %zu\n", n, k, t);

	double expected_iterations = compute_expected_iterations(n, k, t);
	printf("Nombre moyen d'itérations (théorique) : %.2f\n",
	       expected_iterations);

	size_t max_iterations = 100 * expected_iterations;

	ParityCheckMatrix *h = matrix_create(n - k, n);
	CodeWord *y = vector_create(n);
	CodeWord *y_corrected = vector_create(n);

	if (h == NULL || y == NULL || y_corrected == NULL) {
		fprintf(stderr, "Erreur allocation mémoire pour le test.\n");
		return;
	}

	for (size_t run = 1; run <= n_runs; run++) {
		matrix_set_random(h);

		// on suppose le mot de code 0
		generate_random_error(y, t);

		clock_t start_time = clock();
		size_t iterations = 0;
		MatrixStatus status =
		    decode_prange(h, y, t, y_corrected, &iterations,
				  &max_iterations);
		clock_t end_time = clock();

		double time_spent =
		    (double)(end_time - start_time) / CLOCKS_PER_SEC;

		if (status == MATRIX_SUCCESS) {
			size_t final_weight =
			    vector_hamming_weight(y_corrected);

			if (final_weight == 0) {
				printf("\rSUCCÈS tentative %zu/%zu\n", run,
				       n_runs);
				fflush(stdout);
				fprintf(csv_file,
					"%zu,%zu,%zu,%f,%zu,%zu,%f\n", n, k,
					t, expected_iterations, run, iterations,
					time_spent);
				fflush(csv_file);
			} else {
				printf
				    ("ÉCHEC tentative %zu : renvoi MATRIX_SUCCESS mais mauvaise correction.\n",
				     run);
			}
		} else {
			printf("ÉCHEC tentative %zu : renvoi %d\n", run,
			       status);
		}
		//printf("Durée de calcul : %.4f secondes\n", time_spent);
	}
	printf("\n");

	matrix_destroy(h);
	vector_destroy(y);
	vector_destroy(y_corrected);
}
