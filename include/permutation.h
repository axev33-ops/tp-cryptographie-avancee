#ifndef PERMUTATION_H
#define PERMUTATION_H

#include <stdlib.h>

typedef struct {
	size_t size;
	size_t *indices;
} Permutation;

Permutation *permutation_create(size_t n);
void permutation_shuffle(Permutation * p);
void permutation_destroy(Permutation * p);

#endif				// PERMUTATION_H
