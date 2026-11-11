#ifndef POLY_H
#define POLY_H

#include <stdint.h>
#include <stddef.h>

/*
 * Un polynôme dans F2[X]/(X^n - 1) est stocké comme un tableau de mots de 64 bits.
 */
typedef struct {
	size_t n;		// Degré maximal (longueur n)
	size_t num_words;	// Nombre de uint64_t nécessaires
	uint64_t *coeffs;
} PolyGF2;

// Allocation et libération
PolyGF2 *poly_create(size_t n);
void poly_free(PolyGF2 * p);

// Copie
void poly_copy(const PolyGF2 * src, PolyGF2 * dest);

// Initialisation au polynôme nul
void poly_set_zero(PolyGF2 * p);

// Addition polynomiale (XOR bit à bit)
void poly_add(const PolyGF2 * a, const PolyGF2 * b, PolyGF2 * out);

// Multiplication polynomiale modulo X^n - 1
void poly_mul(const PolyGF2 * a, const PolyGF2 * b, PolyGF2 * out);

// Inversion dans l'anneau (Algorithme d'Euclide étendu)
// Retourne 1 si inversible, 0 sinon
int poly_invert(const PolyGF2 * a, PolyGF2 * out);

// Rotation circulaire (équivalent à la multiplication par X)
void poly_rotate_right(PolyGF2 * p);

#endif				// POLY_H
