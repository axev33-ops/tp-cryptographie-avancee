#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

/* ====================================================================
 * CHRONOMÉTRAGE
 * ==================================================================== */

struct timespec timer_start(void)
{
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);
	return start;
}

double timer_end(struct timespec start)
{
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	return (end.tv_sec - start.tv_sec) + (end.tv_nsec -
					      start.tv_nsec) / 1e9;
}

/* ====================================================================
 * MATHÉMATIQUES ET MANIPULATION DE BITS
 * ==================================================================== */

size_t hamming_weight(const uint64_t *data, size_t num_words)
{
	size_t weight = 0;
	for (size_t i = 0; i < num_words; i++) {
		// __builtin_popcountll est une instruction processeur très rapide 
		// pour compter les bits à 1 dans un entier 64 bits non signé.
		weight += __builtin_popcountll(data[i]);
	}
	return weight;
}

/* ====================================================================
 * GÉNÉRATION ALÉATOIRE
 * ==================================================================== */

void generate_random_vector_fixed_weight(uint64_t *vec, size_t length,
					 size_t weight)
{
	// 1. S'assurer que le vecteur est complètement vide (rempli de zéros)
	size_t num_words = (length + 63) / 64;
	for (size_t i = 0; i < num_words; i++) {
		vec[i] = 0;
	}

	// 2. Placer exactement 'weight' bits à 1
	size_t current_weight = 0;
	while (current_weight < weight) {
		// Tirer une position aléatoire entre 0 et length - 1
		size_t pos = rand() % length;

		// Déterminer dans quel mot de 64 bits et à quel index se trouve cette position
		size_t word_idx = pos / 64;
		size_t bit_idx = pos % 64;

		// Si le bit est à 0, on le met à 1
		if (!((vec[word_idx] >> bit_idx) & 1)) {
			vec[word_idx] |= (1ULL << bit_idx);
			current_weight++;
		}
	}
}

void generate_random_permutation(size_t *p, size_t n)
{
	// 1. Initialiser le tableau avec l'identité (0, 1, 2, ..., n-1)
	for (size_t i = 0; i < n; i++) {
		p[i] = i;
	}

	// 2. Mélange de Fisher-Yates (algorithme standard et optimal pour brasser un tableau)
	for (size_t i = n - 1; i > 0; i--) {
		size_t j = rand() % (i + 1);

		// Échange entre p[i] et p[j]
		size_t temp = p[i];
		p[i] = p[j];
		p[j] = temp;
	}
}

/* ====================================================================
 * DÉBOGAGE ET AFFICHAGE
 * ==================================================================== */

void print_binary(const uint64_t *data, size_t bit_length)
{
	for (size_t i = 0; i < bit_length; i++) {
		size_t word_idx = i / 64;
		size_t bit_idx = i % 64;

		// Extraire le bit courant
		int bit = (data[word_idx] >> bit_idx) & 1;
		printf("%d", bit);

		// Ajouter un espace tous les 8 bits pour rendre la lecture plus facile
		if ((i + 1) % 8 == 0) {
			printf(" ");
		}
	}
	printf("\n");
}
