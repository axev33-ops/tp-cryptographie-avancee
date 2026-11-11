#include <stdio.h>
#include <stdbool.h>
#include <string.h>		// pour memcpy
#include "binary-algebra.h"

BinaryMatrix *matrix_create(size_t rows, size_t columns)
{
	BinaryMatrix *m = malloc(sizeof(BinaryMatrix));
	if (m == NULL) {
		fprintf(stderr,
			"Erreur : allocation mémoire BinaryMatrix struct.\n");
		return NULL;
	}

	m->rows = rows;
	m->columns = columns;

	// calloc initialise à 0
#ifdef USE_NAIVE
	m->coeffs = calloc(rows * columns, sizeof(bool));
#else				// !USE_NAIVE
	// nombre de uint64_t par ligne (arrondi à l'excès de columns / 64)
	m->blocks_per_row = (columns + 63) / 64;
	m->coeffs = calloc(rows * m->blocks_per_row, sizeof(uint64_t));
#endif				// USE_NAIVE
	if (m->coeffs == NULL) {
		fprintf(stderr,
			"Erreur : allocation mémoire BinaryMatrix coeffs.\n");
		free(m);
		return NULL;
	}

	return m;
}

void matrix_destroy(BinaryMatrix *m)
{
	if (m != NULL) {
		if (m->coeffs != NULL)
			free(m->coeffs);
		free(m);
	}
}

BinaryVector *vector_create(size_t length)
{
	BinaryVector *v = malloc(sizeof(BinaryVector));
	if (v == NULL) {
		fprintf(stderr,
			"Erreur : allocation mémoire BinaryVector struct.\n");
		return NULL;
	}

	v->length = length;

#ifdef USE_NAIVE
	v->coeffs = calloc(length, sizeof(bool));
#else				// !USE_NAIVE
	// nombre de uint64_t par ligne (arrondi à l'excès de length / 64)
	v->blocks = (length + 63) / 64;
	v->coeffs = calloc(v->blocks, sizeof(uint64_t));
#endif				// USE_NAIVE

	if (v->coeffs == NULL) {
		fprintf(stderr,
			"Erreur : allocation mémoire BinaryVector coeffs.\n");
		free(v);
		return NULL;
	}

	return v;
}

void vector_destroy(BinaryVector *v)
{
	if (v != NULL) {
		if (v->coeffs != NULL)
			free(v->coeffs);
		free(v);
	}
}

bool matrix_is_square(const BinaryMatrix *a)
{
	return a->rows == a->columns;
}

bool matrix_have_same_dimensions(const BinaryMatrix *a, const BinaryMatrix *b)
{
	return a->rows == b->rows && a->columns == b->columns;
}

bool matrix_get_bit(const BinaryMatrix *m, size_t row, size_t col)
{
#ifdef USE_NAIVE
	return m->coeffs[row * m->columns + col];
#else				// !USE_NAIVE
	size_t block_idx = col / 64;
	size_t bit_idx = col % 64;
	uint64_t block = m->coeffs[row * m->blocks_per_row + block_idx];

	// l'opérateur & signifie ET bit à bit
	return (block & (1ULL << bit_idx)) != 0;
#endif				// USE_NAIVE
}

bool vector_get_bit(const BinaryVector *v, size_t index)
{
#ifdef USE_NAIVE
	return v->coeffs[index];
#else				// !USE_NAIVE
	size_t block_idx = index / 64;
	size_t bit_idx = index % 64;
	uint64_t block = v->coeffs[block_idx];
	return (block & (1ULL << bit_idx)) != 0;
#endif				// USE_NAIVE
}

void matrix_set_bit_1(BinaryMatrix *m, size_t row, size_t col)
{
#ifdef USE_NAIVE
	m->coeffs[row * m->columns + col] = 1;
#else				// !USE_NAIVE
	size_t block_idx = col / 64;
	size_t bit_idx = col % 64;

	// l'opérateur | signifie OU bit à bit
	m->coeffs[row * m->blocks_per_row + block_idx] |= (1ULL << bit_idx);
#endif				// USE_NAIVE
}

void matrix_set_bit_0(BinaryMatrix *m, size_t row, size_t col)
{
#ifdef USE_NAIVE
	m->coeffs[row * m->columns + col] = 0;
#else				// !USE_NAIVE
	size_t block_idx = col / 64;
	size_t bit_idx = col % 64;

	// l'opérateur ~ signifie NON bit à bit
	m->coeffs[row * m->blocks_per_row + block_idx] &= ~(1ULL << bit_idx);
#endif				// USE_NAIVE
}

void vector_set_bit_1(BinaryVector *v, size_t index)
{
#ifdef USE_NAIVE
	v->coeffs[index] = 1;
#else				// !USE_NAIVE
	size_t block_idx = index / 64;
	size_t bit_idx = index % 64;

	v->coeffs[block_idx] |= (1ULL << bit_idx);
#endif				// USE_NAIVE
}

void vector_set_bit_0(BinaryVector *v, size_t index)
{
#ifdef USE_NAIVE
	v->coeffs[index] = 0;
#else				// !USE_NAIVE
	size_t block_idx = index / 64;
	size_t bit_idx = index % 64;

	v->coeffs[block_idx] &= ~(1ULL << bit_idx);
#endif				// USE_NAIVE
}

MatrixStatus matrix_copy_coeffs(const BinaryMatrix *m_src, BinaryMatrix *m_dest)
{
	if (m_src == NULL || m_dest == NULL) {
		fprintf(stderr, "Erreur : matrice pointeur nul.\n");
		return MATRIX_ERR_MEMORY;
	}
	if (!(matrix_have_same_dimensions(m_src, m_dest))) {
		fprintf(stderr,
			"Erreur : dimensions matrices incompatibles pour copie.\n");
		return MATRIX_ERR_DIMENSION;
	}
#ifdef USE_NAIVE
	size_t n_coeff = m_src->rows * m_src->columns;

	// memcpy plus rapide que des boucles
	memcpy(m_dest->coeffs, m_src->coeffs, n_coeff * sizeof(bool));
#else				// !USE_NAIVE
	size_t n_blocks = m_src->rows * m_src->blocks_per_row;
	memcpy(m_dest->coeffs, m_src->coeffs, n_blocks * sizeof(uint64_t));
#endif				// USE_NAIVE

	return MATRIX_SUCCESS;
}

MatrixStatus matrix_set_zeros(BinaryMatrix *a)
{
	if (a == NULL)
		return MATRIX_ERR_MEMORY;

#ifdef USE_NAIVE
	size_t n_coeff = a->rows * a->columns;
	memset(a->coeffs, 0, n_coeff * sizeof(bool));
#else				// !USE_NAIVE
	size_t n_blocks = a->rows * a->blocks_per_row;
	memset(a->coeffs, 0, n_blocks * sizeof(uint64_t));
#endif				// USE_NAIVE

	return MATRIX_SUCCESS;
}

MatrixStatus matrix_set_identity(BinaryMatrix *a)
{
	if (a == NULL)
		return MATRIX_ERR_MEMORY;

	if (a->rows != a->columns) {
		fprintf(stderr,
			"Erreur : une matrice identité doit être carrée.\n");
		return MATRIX_ERR_DIMENSION;
	}

	matrix_set_zeros(a);

	size_t n = a->rows;
	for (size_t i = 0; i < n; i++)
		matrix_set_bit_1(a, i, i);

	return MATRIX_SUCCESS;
}

// /!\  La fonction rand() crée des matrices qui ne sont pas de rang plein
//      (expérimenté pour taille 1000 \times 500).
//      Solution : utiliser la fonction ci-après utilisant /dev/urandom.
//MatrixStatus matrix_set_random(BinaryMatrix *m)
//{
//      if (m == NULL)
//              return MATRIX_ERR_MEMORY;
//
//      size_t n_row = m->rows;
//      size_t n_col = m->columns;
//      for (size_t r = 0; r < n_row; r++) {
//              for (size_t c = 0; c < n_col; c++) {
//                      if (rand() % 2 == 1)
//                              matrix_set_bit_1(m, r, c);
//                      else
//                              matrix_set_bit_0(m, r, c);
//              }
//      }
//
//      return MATRIX_SUCCESS;
//}
MatrixStatus matrix_set_random(BinaryMatrix *m)
{
	if (m == NULL)
		return MATRIX_ERR_MEMORY;

	// Ouvre la source d'entropie de Linux
	FILE *urandom = fopen("/dev/urandom", "r");
	if (urandom == NULL) {
		fprintf(stderr, "Erreur : Impossible de lire /dev/urandom\\n");
		return MATRIX_ERR_MEMORY;
	}

#ifdef USE_NAIVE
	size_t n_bytes = m->rows * m->columns * sizeof(bool);
	fread(m->coeffs, 1, n_bytes, urandom);
#else
	size_t n_bytes = m->rows * m->blocks_per_row * sizeof(uint64_t);
	fread(m->coeffs, 1, n_bytes, urandom);
#endif

	fclose(urandom);
	return MATRIX_SUCCESS;
}

// r1 et r2 sont les indices des lignes à échanger
void matrix_swap_rows(BinaryMatrix *a, size_t r1, size_t r2)
{
	if (r1 == r2)
		return;

	if (r1 >= a->rows || r2 >= a->rows) {
		fprintf(stderr,
			"Erreur : indice de ligne de matrice impossible.\n");
		return;
	}

#ifdef USE_NAIVE
	size_t n_col = a->columns;
	for (size_t c = 0; c < n_col; c++) {
		bool tmp = a->coeffs[r1 * n_col + c];
		a->coeffs[r1 * n_col + c] = a->coeffs[r2 * n_col + c];
		a->coeffs[r2 * n_col + c] = tmp;
	}
#else				// !USE_NAIVE
	size_t bpr = a->blocks_per_row;
	for (size_t b = 0; b < bpr; b++) {
		uint64_t tmp = a->coeffs[r1 * bpr + b];
		a->coeffs[r1 * bpr + b] = a->coeffs[r2 * bpr + b];
		a->coeffs[r2 * bpr + b] = tmp;
	}
#endif				// USE_NAIVE
}

// la ligne d'indice r_dest recevra la somme des lignes d'indices r_dest et r_src
void matrix_add_rows(BinaryMatrix *a, size_t r_src, size_t r_dest)
{
	if (r_src >= a->rows || r_dest >= a->rows) {
		fprintf(stderr,
			"Erreur : indice de ligne de matrice impossible.\n");
		return;
	}

	// l'opérateur ^ signifie XOR bit à bit (addition dans GF(2))
#ifdef USE_NAIVE
	size_t n_col = a->columns;
	for (size_t c = 0; c < n_col; c++)
		a->coeffs[r_dest * n_col + c] ^= a->coeffs[r_src * n_col + c];
#else				// !USE_NAIVE
	size_t bpr = a->blocks_per_row;
	for (size_t b = 0; b < bpr; b++)
		a->coeffs[r_dest * bpr + b] ^= a->coeffs[r_src * bpr + b];
#endif				// USE_NAIVE
}

// inversion optimisée en gestion de la mémoire destinée à faire plusieurs
// inversions de matrices de mêmes dimensions
MatrixStatus matrix_inverse(const BinaryMatrix *m, BinaryMatrix *inv,
			    BinaryMatrix *rough_var)
{
	if (m == NULL || inv == NULL || rough_var == NULL) {
		fprintf(stderr, "Erreur : matrice pointeur nul.\n");
		return MATRIX_ERR_MEMORY;
	}
	if (!(matrix_is_square(m) && matrix_have_same_dimensions(m, inv)
	      && matrix_have_same_dimensions(m, rough_var))) {
		fprintf(stderr,
			"Erreur : dimensions incorrectes pour l'inversion.\n");
		return MATRIX_ERR_DIMENSION;
	}
	size_t n = m->rows;

	// simule [m | I]
	matrix_copy_coeffs(m, rough_var);	// pour ne pas changer m
	matrix_set_identity(inv);

	// simule le passage à [I | m^{-1}] (élimination de Gauss-Jordan)
	for (size_t c = 0; c < n; c++) {

		// cherche le pivot
		size_t pivot = c;
		while (pivot < n && matrix_get_bit(rough_var, pivot, c) == 0)
			pivot++;
		// s'il n'y en pas la matrice n'est pas inversible
		if (pivot == n) {
			return MATRIX_ERR_SINGULAR;
		}
		// place le pivot en bonne position
		matrix_swap_rows(rough_var, pivot, c);
		matrix_swap_rows(inv, pivot, c);

		// élimine les 1 de la colonne
		for (size_t r = 0; r < n; r++) {
			if (r != c && matrix_get_bit(rough_var, r, c) != 0) {
				matrix_add_rows(rough_var, c, r);
				matrix_add_rows(inv, c, r);
			}
		}
	}
	return MATRIX_SUCCESS;
}

// les premiers indices de la permutation serviront à extraire les colonnes
MatrixStatus matrix_extract_columns(const BinaryMatrix *m_src,
				    const Permutation *p, BinaryMatrix *m_dest)
{
	if (m_src == NULL || p == NULL || m_dest == NULL) {
		fprintf(stderr,
			"Erreur : pointeur nul extraction de colonnes.\n");
		return MATRIX_ERR_MEMORY;
	}
	size_t n_row = m_src->rows;
	size_t n_col = m_src->columns;
	size_t n_col_to_extract = m_dest->columns;

	if (p->size != n_col) {
		fprintf(stderr,
			"Erreur : taille de Permutation différente du nombre de colonnes de la matrice.\n");
		return MATRIX_ERR_DIMENSION;
	}
	if (n_col_to_extract > n_col || m_dest->rows != n_row) {
		fprintf(stderr,
			"Erreur : mauvaises dimensions de la sous-matrice.\n");
		return MATRIX_ERR_DIMENSION;
	}

	matrix_set_zeros(m_dest);

	for (size_t r = 0; r < n_row; r++) {
		for (size_t c = 0; c < n_col_to_extract; c++) {
			size_t c_perm = p->indices[c];
			if (matrix_get_bit(m_src, r, c_perm) != 0)
				matrix_set_bit_1(m_dest, r, c);
		}
	}
	return MATRIX_SUCCESS;
}

size_t vector_hamming_weight(const BinaryVector *v)
{
	if (v == NULL)
		return 0;

	size_t weight = 0;

#ifdef USE_NAIVE
	for (size_t i = 0; i < v->length; i++)
		if (v->coeffs[i] == 1)
			weight++;
#else				// !USE_NAIVE
	for (size_t b = 0; b < v->blocks; b++)
		// __builtin_popcountll compte le nombre de 1 d'un uint64_t
		// https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html
		weight += __builtin_popcountll(v->coeffs[b]);
#endif				// USE_NAIVE

	return weight;
}

void vector_set_zeros(BinaryVector *v)
{
	if (v == NULL)
		return;

#ifdef USE_NAIVE
	memset(v->coeffs, 0, v->length * sizeof(bool));
#else				// !USE_NAIVE
	memset(v->coeffs, 0, v->blocks * sizeof(uint64_t));
#endif				// USE_NAIVE
}

// le résultat est stocké dans sum
MatrixStatus vector_add(BinaryVector *v1, BinaryVector *v2, BinaryVector *sum)
{
	if (v1 == NULL || v2 == NULL || sum == NULL) {
		fprintf(stderr,
			"Erreur : pointeur nul dans addition de vecteurs.\n");
		return MATRIX_ERR_MEMORY;
	}
	if (v1->length != v2->length || v1->length != sum->length) {
		fprintf(stderr,
			"Erreur : dimensions incompatibles pour addition de vecteurs.\n");
		return MATRIX_ERR_DIMENSION;
	}

#ifdef USE_NAIVE
	size_t n = v1->length;
	for (size_t i = 0; i < n; i++) {
		sum->coeffs[i] = v1->coeffs[i] ^ v2->coeffs[i];
	}
#else				// !USE_NAIVE
	size_t n_block = v1->blocks;
	for (size_t b = 0; b < n_block; b++) {
		sum->coeffs[b] = v1->coeffs[b] ^ v2->coeffs[b];
	}
#endif				// USE_NAIVE
	return MATRIX_SUCCESS;
}

// Mx = y (où x et y sont considérés comme vecteur colonnes)
MatrixStatus vector_multiply_by_matrix_left(const BinaryMatrix *m,
					    const BinaryVector *x,
					    BinaryVector *y)
{
	if (m == NULL || x == NULL || y == NULL) {
		fprintf(stderr,
			"Erreur : pointeur nul dans multiplication matrice-vecteur.\n");
		return MATRIX_ERR_MEMORY;
	}

	if (m->columns != x->length || m->rows != y->length) {
		fprintf(stderr,
			"Erreur : dimensions incompatibles pour y = m * x.\n");
		return MATRIX_ERR_DIMENSION;
	}

	size_t n_row = m->rows;
#ifdef USE_NAIVE
	size_t n_col = m->columns;

	for (size_t r = 0; r < n_row; r++) {
		bool sum = 0;
		for (size_t c = 0; c < n_col; c++) {
			sum ^= (m->coeffs[r * n_col + c] & x->coeffs[c]);
		}
		y->coeffs[r] = sum;
	}
#else				// !USE_NAIVE
	size_t bpr = m->blocks_per_row;

	vector_set_zeros(y);

	for (size_t r = 0; r < n_row; r++) {
		uint64_t sum = 0;
		for (size_t b = 0; b < bpr; b++) {
			sum ^= (m->coeffs[r * bpr + b] & x->coeffs[b]);
		}
		// sum est de poids impair SSI la somme de ses bits est 1
		if (__builtin_popcountll(sum) % 2 == 1)
			vector_set_bit_1(y, r);
	}
#endif				// USE_NAIVE

	return MATRIX_SUCCESS;
}
