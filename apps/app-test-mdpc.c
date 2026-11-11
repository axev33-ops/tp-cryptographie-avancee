#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "utils.h"
#include "poly.h"
#include "mdpc.h"

/* * Fonction pour afficher les positions des bits à 1.
*/
void print_positions(const char *label, const uint64_t *data, size_t bit_length)
{
	printf("%s (indices des bits a 1) :\n[ ", label);
	int count = 0;
	for (size_t i = 0; i < bit_length; i++) {
		if ((data[i / 64] >> (i % 64)) & 1) {
			printf("%zu ", i);
			count++;
		}
	}
	printf("]\n(Poids mesure : %d)\n\n", count);
}

/*
 * Fonction de comparaison stricte entre deux vecteurs
 */
int compare_errors(const PolyGF2 *a, const PolyGF2 *b)
{
	for (size_t i = 0; i < a->num_words; i++) {
		if (a->coeffs[i] != b->coeffs[i])
			return 0;
	}
	return 1;
}

int main()
{
	// Initialisation du générateur aléatoire
	srand(time(NULL));

	// PARAMÈTRES DE SÉCURITÉ 
	size_t n = 4813;
	size_t w = 39;		// Poids de h0 et h1
	size_t t = 78;		// Poids total de l'erreur (e0, e1)
	size_t T = 26;		// Seuil  pour le Bit-Flipping

	printf("====================================================\n");
	printf("   SYSTEME CRYPTOGRAPHIQUE MDPC (BIKE-2)\n");
	printf("   Paramètres : n=%zu, w=%zu, t=%zu\n", n, w, t);
	printf("====================================================\n\n");

	// Allocation des structures
	MDPCPrivateKey sk;
	MDPCPublicKey pk;
	PolyGF2 *ciphertext = poly_create(n);
	PolyGF2 *e0_orig = poly_create(n);
	PolyGF2 *e1_orig = poly_create(n);
	PolyGF2 *e0_found = poly_create(n);
	PolyGF2 *e1_found = poly_create(n);

	// --- 1. GÉNÉRATION DES CLÉS ---
	printf("[1] Génération des clés en cours...\n");
	struct timespec start_kg = timer_start();
	mdpc_keygen(n, w, &sk, &pk);
	double time_kg = timer_end(start_kg);

	printf("    -> Clé publique 'h' générée en %.4f s.\n", time_kg);
	print_positions("    -> h0 (secret)", sk.h0->coeffs, n);

	// --- 2. CHIFFREMENT ---
	printf("[2] Chiffrement ...\n");
	struct timespec start_enc = timer_start();
	mdpc_encrypt(&pk, t, ciphertext, e0_orig, e1_orig);
	double time_enc = timer_end(start_enc);

	print_positions("    -> e0 original", e0_orig->coeffs, n);
	print_positions("    -> e1 original", e1_orig->coeffs, n);
	printf("    -> Syndrome 'c' prêt, généré en %.4f s.\n\n", time_enc);

	// --- 3. DÉCHIFFREMENT (BIT-FLIPPING) ---
	printf("[3] Déchiffrement (clé privée + Bit-Flipping)...\n");
	struct timespec start_dec = timer_start();
	int success = mdpc_decrypt(&sk, ciphertext, t, T, e0_found, e1_found);
	double time_dec = timer_end(start_dec);

	if (success) {
		printf("    -> Convergence atteinte en %.4f s.\n", time_dec);

		// --- VÉRIFICATION D'ÉGALITÉ ---
		if (compare_errors(e0_orig, e0_found)
		    && compare_errors(e1_orig, e1_found)) {
			printf
			    ("    -> VÉRIFICATION : [OK] Les clés sont identiques !\n");
		} else {
			printf
			    ("    -> VÉRIFICATION : [ECHEC] Le décodeur a trouvé une autre solution.\n");
		}
	} else {
		printf
		    ("    -> ÉCHEC : Le décodeur n'a pas pu corriger les erreurs.\n");
	}
	// --- Résumé
	printf("RÉSUMÉ DES DURÉES :\n");
	printf("\tGénération des clés : %.4f s\n", time_kg);
	printf("\tChiffrement : %.4f s\n", time_enc);
	printf("\tDéchiffrement : %.4f s\n", time_dec);

	// Nettoyage de la mémoire
	poly_free(sk.h0);
	poly_free(sk.h1);
	poly_free(pk.h);
	poly_free(ciphertext);
	poly_free(e0_orig);
	poly_free(e1_orig);
	poly_free(e0_found);
	poly_free(e1_found);

	printf("\n====================================================\n");
	printf("   FIN DU PROGRAMME\n");
	printf("====================================================\n");

	return 0;
}
