#include <stdio.h>
#include <stdlib.h>
#include "isd.h"

PrangeContext *prange_context_create(size_t n, size_t n_minus_k)
{
	PrangeContext *ctx = malloc(sizeof(PrangeContext));
	if (ctx == NULL)
		return NULL;

	ctx->submatrix = matrix_create(n_minus_k, n_minus_k);
	ctx->inverse = matrix_create(n_minus_k, n_minus_k);
	ctx->rough_var = matrix_create(n_minus_k, n_minus_k);
	ctx->permutation = permutation_create(n);
	ctx->syndrome = vector_create(n_minus_k);
	ctx->error_sub = vector_create(n_minus_k);
	ctx->error_full = vector_create(n);

	if (ctx->submatrix == NULL || ctx->inverse == NULL
	    || ctx->rough_var == NULL || ctx->permutation == NULL
	    || ctx->syndrome == NULL || ctx->error_sub == NULL
	    || ctx->error_full == NULL) {
		prange_context_destroy(ctx);
		return NULL;
	}

	return ctx;
}

void prange_context_destroy(PrangeContext *ctx)
{
	if (ctx != NULL) {
		matrix_destroy(ctx->submatrix);
		matrix_destroy(ctx->inverse);
		matrix_destroy(ctx->rough_var);
		permutation_destroy(ctx->permutation);
		vector_destroy(ctx->syndrome);
		vector_destroy(ctx->error_sub);
		vector_destroy(ctx->error_full);
		free(ctx);
	}
}

MatrixStatus compute_syndrome(ParityCheckMatrix *h, CodeWord *y, Syndrome *s)
{
	return vector_multiply_by_matrix_left(h, y, s);
}

MatrixStatus decode_prange(ParityCheckMatrix *h, CodeWord *y,
			   size_t target_weight, CodeWord *y_corrected,
			   size_t *iterations, size_t *max_iterations)
{
	size_t n = h->columns;
	size_t n_minus_k = h->rows;
	PrangeContext *ctx = prange_context_create(n, n_minus_k);
	if (ctx == NULL) {
		fprintf(stderr,
			"Erreur : allocation mémoire PrangeContext.\n");
		return MATRIX_ERR_MEMORY;
	}

	compute_syndrome(h, y, ctx->syndrome);

	size_t curr_iteration = 0;
	size_t min_weight_found = 1000;	// pour déboguer
	while (true) {

		curr_iteration++;
		if (max_iterations != NULL && curr_iteration > *max_iterations) {
#ifdef VERBOSE
			printf
			    ("\nPrange annulé. Poids minimum de e' rencontré : %zu\n",
			     min_weight_found);
#endif				// VERBOSE
			fprintf(stderr,
				"\nPrange annulé : plus de %zu iterations.\n",
				*max_iterations);
			fflush(stderr);
			prange_context_destroy(ctx);
			return MATRIX_ERR_MAX_ITER;
		}
#ifdef VERBOSE
		if (curr_iteration % 500 == 0) {
			printf("\r    Prange : %zu iterations...",
			       curr_iteration);
			fflush(stdout);
		}
#endif				// VERBOSE

		// choix des colonnes random
		permutation_shuffle(ctx->permutation);
		matrix_extract_columns(h, ctx->permutation, ctx->submatrix);

		// calcul de son inverse
		MatrixStatus status =
		    matrix_inverse(ctx->submatrix, ctx->inverse,
				   ctx->rough_var);

		if (status == MATRIX_ERR_SINGULAR)	// non inversible, retour à la case départ
			continue;

		if (status != MATRIX_SUCCESS) {	// si arrive alors erreur à déboguer
			prange_context_destroy(ctx);
			return status;
		}
		// calcul de l'erreur partielle
		vector_multiply_by_matrix_left(ctx->inverse, ctx->syndrome,
					       ctx->error_sub);

		size_t current_weight = vector_hamming_weight(ctx->error_sub);
		if (current_weight < min_weight_found) {
			min_weight_found = current_weight;
		}
		if (vector_hamming_weight(ctx->error_sub) == target_weight)
			break;	// gagné
	}
#ifdef VERBOSE
	printf("\r    Prange : succès après %zu iterations.\n",
	       curr_iteration);
#endif				// VERBOSE
	if (iterations != NULL) {
		*iterations = curr_iteration;
	}

	// reconstruction de l'erreur
	vector_set_zeros(ctx->error_full);
	for (size_t i = 0; i < n_minus_k; i++) {
		size_t original_index = ctx->permutation->indices[i];
		if (vector_get_bit(ctx->error_sub, i) == 1)
			vector_set_bit_1(ctx->error_full, original_index);
	}

	// correction du mot de code
	vector_add(y, ctx->error_full, y_corrected);

	prange_context_destroy(ctx);

	return MATRIX_SUCCESS;
}
