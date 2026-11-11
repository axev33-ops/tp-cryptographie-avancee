#include "poly.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*  GESTION DE LA MÉMOIRE ET OUTILS DE BASE
 */

PolyGF2 *poly_create(size_t n)
{
	PolyGF2 *p = (PolyGF2 *) malloc(sizeof(PolyGF2));
	p->n = n;
	p->num_words = (n + 63) / 64;
	p->coeffs = (uint64_t *) calloc(p->num_words, sizeof(uint64_t));
	return p;
}

void poly_free(PolyGF2 *p)
{
	if (!p)
		return;
	free(p->coeffs);
	free(p);
}

void poly_copy(const PolyGF2 *src, PolyGF2 *dest)
{
	dest->n = src->n;
	dest->num_words = src->num_words;
	memcpy(dest->coeffs, src->coeffs, src->num_words * sizeof(uint64_t));
}

void poly_set_zero(PolyGF2 *p)
{
	memset(p->coeffs, 0, p->num_words * sizeof(uint64_t));
}

/*
 ARITHMÉTIQUE DANS L'ANNEAU R = F2[X]/(X^n - 1)
 */

void poly_add(const PolyGF2 *a, const PolyGF2 *b, PolyGF2 *out)
{
	// Dans F2, l'addition est un XOR binaire (^)
	for (size_t i = 0; i < a->num_words; i++) {
		out->coeffs[i] = a->coeffs[i] ^ b->coeffs[i];
	}
}

/* * ASTUCE DU DÉCALAGE (Multiplication par X) :
 * On considère que le coefficient de X^0 est le bit de poids faible (bit 0) du mot 0.
 * Multiplier par X revient à décaler tous les bits d'un cran vers la GAUCHE (<< 1).
 * Le bit qui "sort" par la gauche (poids fort, bit 63) devient le bit 0 du mot suivant.
 * Le coefficient de X^{n-1} "fait le tour" et devient le coefficient de X^0.
 */
void poly_rotate_right(PolyGF2 *p)
{
	// 1. Sauvegarder le coefficient de X^{n-1}
	int bit_n_minus_1 =
	    (p->coeffs[(p->n - 1) / 64] >> ((p->n - 1) % 64)) & 1;

	// 2. L'effacer pour éviter qu'il ne pollue le reste
	p->coeffs[(p->n - 1) / 64] &= ~(1ULL << ((p->n - 1) % 64));

	// 3. Décaler tout vers la gauche, en passant les retenues au mot suivant
	uint64_t carry = 0;
	for (size_t i = 0; i < p->num_words; i++) {
		uint64_t next_carry = p->coeffs[i] >> 63;	// Le bit 63 devient la retenue
		p->coeffs[i] = (p->coeffs[i] << 1) | carry;
		carry = next_carry;
	}

	// 4. Le bit de X^{n-1} réapparaît en X^0 (boucle cyclique)
	if (bit_n_minus_1) {
		p->coeffs[0] |= 1ULL;
	}
}

void poly_mul(const PolyGF2 *a, const PolyGF2 *b, PolyGF2 *out)
{
	// Multiplication  (Décaler et Ajouter)
	poly_set_zero(out);

	// On crée une copie de 'a' qu'on va faire tourner (multiplier par X à chaque tour)
	PolyGF2 *temp = poly_create(a->n);
	poly_copy(a, temp);

	// Pour chaque coefficient du polynôme 'b'
	for (size_t i = 0; i < b->n; i++) {
		// Lire le i-ème bit de 'b'
		int bit_b = (b->coeffs[i / 64] >> (i % 64)) & 1;

		// Si le bit est à 1, on ajoute 'temp' (qui vaut a * X^i) au résultat
		if (bit_b) {
			poly_add(out, temp, out);
		}
		// On multiplie 'temp' par X pour l'itération suivante
		poly_rotate_right(temp);
	}
	poly_free(temp);
}

// Fonction utilitaire interne pour trouver le degré d'un polynôme (bit de poids fort à 1)
static int get_degree(const uint64_t *coeffs, size_t num_words)
{
	for (int i = num_words - 1; i >= 0; i--) {
		if (coeffs[i] != 0) {
			// __builtin_clzll compte les zéros de tête. 63 - zéros = position du bit à 1.
			return (i * 64) + (63 - __builtin_clzll(coeffs[i]));
		}
	}
	return -1;		// Cas du polynôme nul
}

int poly_invert(const PolyGF2 *a, PolyGF2 *out)
{
	// On alloue un peu plus d'espace (ext_words) car R0 commence à X^n + 1 (degré n)
	size_t ext_words = (a->n + 65) / 64;

	uint64_t *R0 = (uint64_t *) calloc(ext_words, sizeof(uint64_t));
	uint64_t *R1 = (uint64_t *) calloc(ext_words, sizeof(uint64_t));
	uint64_t *U0 = (uint64_t *) calloc(ext_words, sizeof(uint64_t));
	uint64_t *U1 = (uint64_t *) calloc(ext_words, sizeof(uint64_t));
	uint64_t *temp = (uint64_t *) calloc(ext_words, sizeof(uint64_t));

	// Initialisation : R0 = X^n + 1
	R0[a->n / 64] |= (1ULL << (a->n % 64));
	R0[0] |= 1ULL;

	// R1 = a
	for (size_t i = 0; i < a->num_words; i++)
		R1[i] = a->coeffs[i];

	// U0 = 0 (déjà fait par calloc)
	// U1 = 1
	U1[0] = 1ULL;

	int d0 = get_degree(R0, ext_words);
	int d1 = get_degree(R1, ext_words);

	while (d1 > 0) {
		int shift = d0 - d1;

		// Si d0 < d1, on échange tout (R0 <-> R1, U0 <-> U1, d0 <-> d1)
		if (shift < 0) {
			uint64_t *t_ptr = R0;
			R0 = R1;
			R1 = t_ptr;
			t_ptr = U0;
			U0 = U1;
			U1 = t_ptr;
			int t_d = d0;
			d0 = d1;
			d1 = t_d;
			shift = -shift;
		}
		// temp = R1 << shift
		memset(temp, 0, ext_words * sizeof(uint64_t));
		size_t w_shift = shift / 64;
		size_t b_shift = shift % 64;
		for (size_t i = 0; i < ext_words; i++) {
			if (R1[i] == 0)
				continue;
			if (i + w_shift < ext_words) {
				temp[i + w_shift] ^= (R1[i] << b_shift);
			}
			if (b_shift > 0 && i + w_shift + 1 < ext_words) {
				temp[i + w_shift + 1] ^=
				    (R1[i] >> (64 - b_shift));
			}
		}

		// R0 = R0 XOR temp
		for (size_t i = 0; i < ext_words; i++)
			R0[i] ^= temp[i];

		// temp = U1 << shift
		memset(temp, 0, ext_words * sizeof(uint64_t));
		for (size_t i = 0; i < ext_words; i++) {
			if (U1[i] == 0)
				continue;
			if (i + w_shift < ext_words) {
				temp[i + w_shift] ^= (U1[i] << b_shift);
			}
			if (b_shift > 0 && i + w_shift + 1 < ext_words) {
				temp[i + w_shift + 1] ^=
				    (U1[i] >> (64 - b_shift));
			}
		}

		// U0 = U0 XOR temp
		for (size_t i = 0; i < ext_words; i++)
			U0[i] ^= temp[i];

		// Mise à jour des degrés
		d0 = get_degree(R0, ext_words);
		d1 = get_degree(R1, ext_words);
	}

	int success = 0;
	// Si d1 == 0, le polynôme R1 est réduit à une constante. 
	// Sur GF(2), si ce n'est pas le polynôme nul, c'est forcément 1 ! L'inversion a réussi.
	if (d1 == 0 && (R1[0] & 1)) {
		success = 1;
		poly_set_zero(out);
		for (size_t i = 0; i < out->num_words; i++) {
			out->coeffs[i] = U1[i];
		}
	}

	free(R0);
	free(R1);
	free(U0);
	free(U1);
	free(temp);
	return success;		// 1 si inversible, 0 sinon
}
