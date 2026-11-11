#ifndef BINARY_ALGEBRA_H
#define BINARY_ALGEBRA_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "permutation.h"

#ifdef USE_NAIVE

// Première approche naïve : un coeff est représenté par un bool, le tableau de
// coefficients est un tableau de bool, un élément du tableau est un
// coefficient.
typedef struct {
	size_t rows;
	size_t columns;
	bool *coeffs;
} BinaryMatrix;

// idem
typedef struct {
	size_t length;
	bool *coeffs;
} BinaryVector;

#else				// !USE_NAIVE

// Deuxième approche plus optimisée : chaque bit des coeffs sera utilisé pour
// représenter un coefficient au détail près qu'une ligne commencera sur un
// nouveau uint64_t (entier non signé sur 64 bits) ; un uint64_t du tableau de
// coefficients sera dit un bloc.
typedef struct {
	size_t rows;		// nombre de bits par ligne
	size_t columns;		//nombre de bits par colonne
	size_t blocks_per_row;	// nombre de uint64_t par ligne
	uint64_t *coeffs;
} BinaryMatrix;

// idem
typedef struct {
	size_t length;
	size_t blocks;		// nombre de uint64_t
	uint64_t *coeffs;
} BinaryVector;

#endif				// USE_NAIVE

// singular, en parlant d'une matrice, signifie carrée et non inversible
typedef enum {
	MATRIX_SUCCESS = 0,
	MATRIX_ERR_MEMORY,
	MATRIX_ERR_DIMENSION,
	MATRIX_ERR_SINGULAR,
	MATRIX_ERR_MAX_ITER
} MatrixStatus;

BinaryMatrix *matrix_create(size_t rows, size_t columns);
void matrix_destroy(BinaryMatrix * m);
bool matrix_get_bit(const BinaryMatrix * m, size_t row, size_t col);
void matrix_set_bit_1(BinaryMatrix * m, size_t row, size_t col);
void matrix_set_bit_0(BinaryMatrix * m, size_t row, size_t col);
MatrixStatus matrix_set_random(BinaryMatrix * m);

BinaryVector *vector_create(size_t length);
void vector_destroy(BinaryVector * v);
void vector_set_zeros(BinaryVector * v);
size_t vector_hamming_weight(const BinaryVector * v);
bool vector_get_bit(const BinaryVector * v, size_t index);
void vector_set_bit_1(BinaryVector * v, size_t index);
void vector_set_bit_0(BinaryVector * v, size_t index);
MatrixStatus vector_add(BinaryVector * v1, BinaryVector * v2,
			BinaryVector * sum);

MatrixStatus matrix_inverse(const BinaryMatrix * m, BinaryMatrix * inv,
			    BinaryMatrix * rough_var);
MatrixStatus matrix_extract_columns(const BinaryMatrix * m_src,
				    const Permutation * p,
				    BinaryMatrix * m_dest);

MatrixStatus vector_multiply_by_matrix_left(const BinaryMatrix * m,
					    const BinaryVector * x,
					    BinaryVector * y);

#endif				// BINARY_ALGEBRA_H
