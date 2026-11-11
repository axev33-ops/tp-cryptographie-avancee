#include <stdio.h>
#include <stdlib.h>
#include "isd-test-lib.h"

int main()
{
	size_t k = 4813;
	size_t n = 2 * k;
	size_t t = 78;
	double expected_iter = compute_expected_iterations(n, k, t);
	double correction_factor = 3.4627;

	printf("Pour les paramètres (n, k, t) = (%zu, %zu, %zu) on estime le nombre d'itération à :\n", n, k, t);
	printf("    %.5e en supposant toute matrice H' inversible\n", expected_iter);
	printf("    %.5e en essayant de corriger ce biais\n", expected_iter * correction_factor);
	return 0;
}
