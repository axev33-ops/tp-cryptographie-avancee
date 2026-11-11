#ifndef MDPC_H
#define MDPC_H

#include "poly.h"

// Structure pour stocker la paire de clés
typedef struct {
	PolyGF2 *h0;
	PolyGF2 *h1;
} MDPCPrivateKey;

typedef struct {
	PolyGF2 *h;
} MDPCPublicKey;

// Génération des clés (n = longueur, w = poids de h0 et h1)
int mdpc_keygen(size_t n, size_t w, MDPCPrivateKey * sk, MDPCPublicKey * pk);

// Chiffrement (Encapsulation)
void mdpc_encrypt(const MDPCPublicKey * pk, size_t t, PolyGF2 * ciphertext,
		  PolyGF2 * e0_out, PolyGF2 * e1_out);

// Déchiffrement via Bit-Flipping
int mdpc_decrypt(const MDPCPrivateKey * sk, const PolyGF2 * ciphertext,
		 size_t t, size_t threshold, PolyGF2 * e0_out,
		 PolyGF2 * e1_out);

// Cœur de l'algorithme de décodage itératif
int bit_flipping(const PolyGF2 * h0, const PolyGF2 * h1,
		 const PolyGF2 * syndrome, size_t t, size_t threshold,
		 PolyGF2 * e0, PolyGF2 * e1);

#endif				// MDPC_H
