#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <stdint.h>
#include <stddef.h>

/* ====================================================================
 * CHRONOMÉTRAGE
 * ==================================================================== */

// Démarre le chronomètre (utilise CLOCK_MONOTONIC pour une haute précision)
struct timespec timer_start(void);

// Arrête le chronomètre et retourne le temps écoulé en secondes
double timer_end(struct timespec start);

/* ====================================================================
 * MATHÉMATIQUES ET MANIPULATION DE BITS
 * ==================================================================== */

// Poids de Hamming : compte le nombre total de bits à '1' dans un tableau de mots 64 bits
size_t hamming_weight(const uint64_t * data, size_t num_words);

/* ====================================================================
 * GÉNÉRATION ALÉATOIRE
 * ==================================================================== */

// Remplit un vecteur binaire avec exactement 'weight' bits à 1, placés aléatoirement
void generate_random_vector_fixed_weight(uint64_t * vec, size_t length,
					 size_t weight);

// Génère une permutation aléatoire parfaite (algorithme de Fisher-Yates) de taille n
void generate_random_permutation(size_t *p, size_t n);

/* ====================================================================
 * DÉBOGAGE ET AFFICHAGE
 * ==================================================================== */

// Affiche le contenu d'un tableau binaire sous forme de 0 et de 1 (utile pour débugger)
void print_binary(const uint64_t * data, size_t bit_length);

#endif				// UTILS_H
