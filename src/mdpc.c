#include "mdpc.h"
#include "poly.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

/* GÉNÉRATION DES CLÉS
  */

int mdpc_keygen(size_t n, size_t w, MDPCPrivateKey *sk, MDPCPublicKey *pk)
{
	sk->h0 = poly_create(n);
	sk->h1 = poly_create(n);
	pk->h = poly_create(n);

	int invertible = 0;
	PolyGF2 *h0_inv = poly_create(n);

	while (!invertible) {
		// 1. Générer h0 et h1 aléatoires de poids w
		poly_set_zero(sk->h0);
		poly_set_zero(sk->h1);
		generate_random_vector_fixed_weight(sk->h0->coeffs, n, w);
		generate_random_vector_fixed_weight(sk->h1->coeffs, n, w);

		// 2. Inverser h0 pour calculer h = h1 * h0^-1
		invertible = poly_invert(sk->h0, h0_inv);
	}

	// Calcul de la clé publique pk->h = sk->h1 * (sk->h0)^-1
	poly_mul(sk->h1, h0_inv, pk->h);

	poly_free(h0_inv);
	return 1;
}

/* CHIFFREMENT (ENCAPSULATION)
 */

void mdpc_encrypt(const MDPCPublicKey *pk, size_t t, PolyGF2 *ciphertext,
		  PolyGF2 *e0_out, PolyGF2 *e1_out)
{
	size_t n = pk->h->n;
	PolyGF2 *temp = poly_create(n);

	// si ei_out est nul, la mémoire est allouée juste pour le calcul
	PolyGF2 *e0 = e0_out;
	PolyGF2 *e1 = e1_out;
	if (e0 == NULL)
		e0 = poly_create(n);
	if (e1 == NULL)
		e1 = poly_create(n);

	// 1. Générer e0 et e1 de poids t/2 (si on suppose que le poids total est t)
	size_t weight_each = t / 2;
	generate_random_vector_fixed_weight(e0->coeffs, n, weight_each);
	generate_random_vector_fixed_weight(e1->coeffs, n, weight_each);

	// 2. c = e0 + h * e1
	poly_mul(pk->h, e1, temp);	// temp = h * e1
	poly_add(e0, temp, ciphertext);	// ciphertext = e0 + temp

	poly_free(temp);

	// si ei_out est nul, la mémoire est allouée juste pour le calcul
	if (e0_out == NULL)
		poly_free(e0);
	if (e1_out == NULL)
		poly_free(e1);
}

/* DÉCHIFFREMENT ET BIT-FLIPPING
 */
static size_t count_overlaps(const PolyGF2 *a, const PolyGF2 *b)
{
	size_t count = 0;
	for (size_t i = 0; i < a->num_words; i++) {
		count += __builtin_popcountll(a->coeffs[i] & b->coeffs[i]);
	}
	return count;
}

int bit_flipping(const PolyGF2 *h0, const PolyGF2 *h1, const PolyGF2 *syndrome,
		 size_t t, size_t threshold, PolyGF2 *e0, PolyGF2 *e1)
{
	size_t n = h0->n;
	int max_iterations = 10;
	int converged = 0;

	// Vecteurs de travail pour faire "tourner" h0 et h1
	PolyGF2 *rot_h0 = poly_create(n);
	PolyGF2 *rot_h1 = poly_create(n);

	// Initialisation : on commence avec une erreur estimée à 0
	poly_set_zero(e0);
	poly_set_zero(e1);

	// Le syndrome courant commence comme le syndrome reçu
	PolyGF2 *current_synd = poly_create(n);
	poly_copy(syndrome, current_synd);

	for (int iter = 0; iter < max_iterations; iter++) {
		// Si le syndrome est nul, on a fini !
		if (hamming_weight
		    (current_synd->coeffs, current_synd->num_words) == 0) {
			// (Optionnel) on peut aussi vérifier que ||e0|| + ||e1|| == t
			converged = 1;
			break;
		}
		// On repart de la position initiale de h0 et h1
		poly_copy(h0, rot_h0);
		poly_copy(h1, rot_h1);

		// a) et b) Calcul des compteurs d'équations insatisfaites et Flip
		for (size_t i = 0; i < n; i++) {
			// Combien d'équations insatisfaites impliquent le bit i de e0 ?
			size_t sum0 = count_overlaps(current_synd, rot_h0);
			if (sum0 >= threshold) {
				// Flipper le bit i de e0
				e0->coeffs[i / 64] ^= (1ULL << (i % 64));
				// Mettre à jour le syndrome : on XOR le syndrome avec la colonne i de H0
				poly_add(current_synd, rot_h0, current_synd);
			}
			// Combien d'équations insatisfaites impliquent le bit i de e1 ?
			size_t sum1 = count_overlaps(current_synd, rot_h1);
			if (sum1 >= threshold) {
				// Flipper le bit i de e1
				e1->coeffs[i / 64] ^= (1ULL << (i % 64));
				// Mettre à jour le syndrome : on XOR le syndrome avec la colonne i de H1
				poly_add(current_synd, rot_h1, current_synd);
			}
			// Faire tourner rot_h0 et rot_h1 pour vérifier la position (i+1) à la boucle suivante
			poly_rotate_right(rot_h0);
			poly_rotate_right(rot_h1);
			// À l'intérieur de la boucle for (int iter = 0; ...) dans mdpc.c
			size_t current_w = hamming_weight(current_synd->coeffs,
							  current_synd->num_words);
#ifdef VERBOSE
			printf("   Iteration %d : Poids du syndrome = %zu\n",
			       iter, current_w);
#endif				// VERBOSE
		}
	}

	poly_free(rot_h0);
	poly_free(rot_h1);
	poly_free(current_synd);

	return converged;
}

int mdpc_decrypt(const MDPCPrivateKey *sk, const PolyGF2 *ciphertext,
		 size_t t, size_t threshold, PolyGF2 *e0_out, PolyGF2 *e1_out)
{
	// 1. Calcul du syndrome étendu : s = h0 * ciphertext
	// Car ciphertext = e0 + h*e1, donc s = h0*e0 + h0*(h1*h0^-1)*e1 = h0*e0 + h1*e1
	PolyGF2 *syndrome = poly_create(sk->h0->n);
	poly_mul(sk->h0, ciphertext, syndrome);

	// 2. Appel au décodeur Bit-Flipping
	int success =
	    bit_flipping(sk->h0, sk->h1, syndrome, t, threshold, e0_out,
			 e1_out);

	poly_free(syndrome);
	return success;		// 1 si succès, 0 si DFR (Decryption Failure Rate)
}
