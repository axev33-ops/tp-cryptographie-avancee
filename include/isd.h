#ifndef ISD_H
#define ISD_H

#include <stdlib.h>
#include "binary-algebra.h"

// alias
typedef BinaryMatrix GeneratorMatrix;	// supposons taille k \times n
typedef BinaryMatrix ParityCheckMatrix;	// taille (n-k) \times n
typedef BinaryVector CodeWord;	// taille n
typedef BinaryVector InfoWord;	// taille k
typedef BinaryVector Syndrome;	// taille n - k
typedef BinaryVector ErrorWord;	// taille n

typedef struct {
	BinaryMatrix *submatrix;
	BinaryMatrix *inverse;
	BinaryMatrix *rough_var;
	Permutation *permutation;
	Syndrome *syndrome;
	Syndrome *error_sub;
	ErrorWord *error_full;
} PrangeContext;

PrangeContext *prange_context_create(size_t n, size_t n_minus_k);
void prange_context_destroy(PrangeContext * ctx);

MatrixStatus decode_prange(ParityCheckMatrix * h, CodeWord * y,
			   size_t target_weight, CodeWord * y_corrected,
			   size_t *iterations, size_t *max_iterations);

#endif				// ISD_H
