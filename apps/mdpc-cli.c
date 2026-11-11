#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>
#include "mdpc.h"

// Format de sauvergarde d'un polynôme (en binaire) : taille coeffs

void save_poly_to_file(const char *filename, const PolyGF2 *poly)
{
	FILE *f = fopen(filename, "wb");
	if (f == NULL) {
		perror("Erreur d'ouverture du fichier en écriture");
		exit(1);
	}
	fwrite(&poly->n, sizeof(size_t), 1, f);
	fwrite(poly->coeffs, sizeof(uint64_t), poly->num_words, f);
	fclose(f);
}

PolyGF2 *load_poly_from_file(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		perror("Erreur d'ouverture du fichier en lecture");
		exit(1);
	}
	size_t n;
	if (fread(&n, sizeof(size_t), 1, f) != 1) {
		fprintf(stderr,
			"Erreur : lecture de la clé privée impossible.\n");
		exit(1);
	}
	PolyGF2 *poly = poly_create(n);
	fread(poly->coeffs, sizeof(uint64_t), poly->num_words, f);
	fclose(f);
	return poly;
}

void save_public_key(const char *filename, const MDPCPublicKey *pk)
{
	save_poly_to_file(filename, pk->h);
}

void load_public_key(const char *filename, MDPCPublicKey *pk)
{
	pk->h = load_poly_from_file(filename);
}

void save_private_key(const char *filename, const MDPCPrivateKey *sk)
{
	FILE *f = fopen(filename, "wb");
	if (f == NULL) {
		perror("Erreur d'écriture de la clé privée");
		exit(1);
	}

	fwrite(&sk->h0->n, sizeof(size_t), 1, f);
	fwrite(sk->h0->coeffs, sizeof(uint64_t), sk->h0->num_words, f);
	fwrite(sk->h1->coeffs, sizeof(uint64_t), sk->h1->num_words, f);
	fclose(f);
}

void load_private_key(const char *filename, MDPCPrivateKey *sk)
{
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		perror("Erreur de lecture de la clé privée");
		exit(1);
	}

	size_t n;
	if (fread(&n, sizeof(size_t), 1, f) != 1) {
		fprintf(stderr,
			"Erreur : lecture de la clé privée impossible.\n");
		exit(1);
	}

	sk->h0 = poly_create(n);
	sk->h1 = poly_create(n);

	// dans le même ordre que pour save_private_key
	fread(sk->h0->coeffs, sizeof(uint64_t), sk->h0->num_words, f);
	fread(sk->h1->coeffs, sizeof(uint64_t), sk->h1->num_words, f);

	fclose(f);
}

// Pour n = 4813 on ne peut pas écrire des blocs de 64 bits, on minimise
// l'écriture de bits « inutiles » en prenant le minimum d'octets (briques
// minimum d'écriture) pour chacun des polynômes.
// (On pourrait « améliorer » légèrement en regroupant les bits esseulés des
// deux en un seul lorsque cela est possible. Pour 4813 cela ne changerait
// rien : 2 * (4813 % 8) = 10.)
void save_shared_secret(const char *filename, const PolyGF2 *e0,
			const PolyGF2 *e1)
{
	FILE *f = fopen(filename, "wb");
	if (f == NULL) {
		perror("Erreur d'écriture du secret");
		exit(1);
	}
	size_t exact_bytes = (e0->n + 7) / 8;
	fwrite((uint8_t *) e0->coeffs, 1, exact_bytes, f);
	fwrite((uint8_t *) e1->coeffs, 1, exact_bytes, f);
	fclose(f);
}

// Applique un masque XOR (masque jetable) entre un fichier d'entrée et un fichier clé
void apply_xor_mask(const char *in_filename, const char *out_filename,
		    const char *key_filename)
{
	FILE *f_in = fopen(in_filename, "rb");
	FILE *f_key = fopen(key_filename, "rb");

	if (f_in == NULL || f_key == NULL) {
		fprintf(stderr,
			"Erreur : impossible d'ouvrir le fichier d'entrée ou la clé.\n");
		if (f_in)
			fclose(f_in);
		if (f_key)
			fclose(f_key);
		exit(1);
	}
	// détermine la taille de la clé
	fseek(f_key, 0, SEEK_END);
	long key_size = ftell(f_key);
	rewind(f_key);

	if (key_size == 0) {
		fprintf(stderr, "Erreur : Le fichier clé est vide.\n");
		exit(1);
	}
	// récupère la clé
	uint8_t *key = (uint8_t *) malloc(key_size);
	if (fread(key, 1, key_size, f_key) != (size_t)key_size) {
		fprintf(stderr, "Erreur lors de la lecture de la clé.\n");
		exit(1);
	}
	fclose(f_key);

	FILE *f_out = fopen(out_filename, "wb");
	if (f_out == NULL) {
		fprintf(stderr,
			"Erreur : impossible de créer le fichier de sortie.\n");
		free(key);
		fclose(f_in);
		exit(1);
	}
	// applique le masque octet par octet
// dans le cas où le masque est trop petit on l'applique plusieurs fois,
// évidemment ce pas recommandable
	int byte;
	size_t i = 0;
	while ((byte = fgetc(f_in)) != EOF) {
		fputc(byte ^ key[i % key_size], f_out);
		i++;
	}

	free(key);
	fclose(f_in);
	fclose(f_out);
}

void print_usage(const char *prog_name)
{
	printf("Usage: %s [ACTION] [OPTIONS]\n", prog_name);
	printf("Actions:\n");
	printf
	    ("  -G, --genkey       Génère une paire de clés (sauvegarde dans --sk et --pk)\n");
	printf
	    ("  -E, --encrypt      Encapsule un secret. Génère un chiffré (--out) et un secret (--secret)\n");
	printf
	    ("  -D, --decrypt      Décapsule un secret. Lit un chiffré (--in) et retrouve le secret (--secret)\n");
	printf
	    ("  -X, --xor          Fait un XOR de --in et --sk et écrit --out.\n");
	printf("\nFichiers:\n");
	printf("  -s, --sk FILE      Chemin vers la clé secrète\n");
	printf("  -p, --pk FILE      Chemin vers la clé publique\n");
	printf
	    ("  -i, --in FILE      Fichier d'entrée contenant le chiffré (pour -D)\n");
	printf
	    ("  -o, --out FILE     Fichier de sortie du chiffré (pour -E)\n");
	printf
	    ("  -S, --secret FILE  Fichier de sortie du secret partagé (e0, e1)\n");
	printf("\nParamètres MDPC :\n");
	printf("  -n, --length N     Longueur du polynôme (défaut: 4813)\n");
	printf("  -w, --weight W     Poids de h0 et h1 (défaut: 39)\n");
	printf
	    ("  -t, --errors T     Poids total de l'erreur e (défaut: 78)\n");
	printf("  -T, --threshold X  Seuil pour BitFlip (défaut: 26)\n");
	printf("\nExemples d'utilisation :\n");
	printf("  Génération de clés :\n");
	printf("    %s -G -s sk.bin -p pk\n", prog_name);
	printf("  Chiffrement :\n");
	printf("    %s -E -p pk.bin -o cipher.bin -S secret.bin\n", prog_name);
	printf("  Déchiffrement :\n");
	printf("    %s -D -s sk.bin -i cipher.bin -S secret.bin\n", prog_name);
	printf("  Appliquer un masque jetable :\n");
	printf("    %s -X -s mask.bin -i plain.txt -o cipher.bin\n", prog_name);
}

int main(int argc, char **argv)
{
	// Initialisation du générateur aléatoire
	// On utilise le PID car si l'on génère les clés et les erreurs (resp.
	// -G et -E) durant la même seconde, on aura e0 = h0 et e1 = h1 ce qui
	// empêche BitFlip de converger.
	srand(time(NULL) ^ getpid());

	// paramètres par défaut
	size_t n = 4813;
	size_t w = 39;		// Poids de h0 et h1
	size_t t = 78;		// Poids total de l'erreur (e0, e1)
	size_t threshold = 26;	// Seuil  pour le Bit-Flipping

	bool action_genkey = false;
	bool action_encrypt = false;
	bool action_decrypt = false;
	bool action_xor = false;
	char *sk_file = "sk.bin";
	char *pk_file = "pk.bin";
	char *in_file = "cipher.bin";
	char *out_file = "cipher.bin";
	char *secret_file = "secret.bin";

	// configuration de getopt_long
	struct option long_options[] = {
		{"genkey", no_argument, 0, 'G'},
		{"encrypt", no_argument, 0, 'E'},
		{"decrypt", no_argument, 0, 'D'},
		{"xor", no_argument, 0, 'X'},
		{"sk", required_argument, 0, 's'},
		{"pk", required_argument, 0, 'p'},
		{"in", required_argument, 0, 'i'},
		{"out", required_argument, 0, 'o'},
		{"secret", required_argument, 0, 'S'},
		{"length", required_argument, 0, 'n'},
		{"weight", required_argument, 0, 'w'},
		{"errors", required_argument, 0, 't'},
		{"threshold", required_argument, 0, 'T'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	// lecture des options et arguments
	int opt;
	while ((opt =
		getopt_long(argc, argv, "GEDXs:p:i:o:S:n:w:t:T:h", long_options,
			    NULL)) != -1) {
		switch (opt) {
		case 'G':
			action_genkey = true;
			break;
		case 'E':
			action_encrypt = true;
			break;
		case 'D':
			action_decrypt = true;
			break;
		case 'X':
			action_xor = true;
			break;
		case 's':
			sk_file = optarg;
			break;
		case 'p':
			pk_file = optarg;
			break;
		case 'i':
			in_file = optarg;
			break;
		case 'o':
			out_file = optarg;
			break;
		case 'S':
			secret_file = optarg;
			break;
		case 'n':
			n = atoi(optarg);
			break;
		case 'w':
			w = atoi(optarg);
			break;
		case 't':
			t = atoi(optarg);
			break;
		case 'T':
			threshold = atoi(optarg);
			break;
		case 'h':
			print_usage(argv[0]);
			return 0;
		default:
			print_usage(argv[0]);
			return 1;
		}
	}
	//printf("Génération des clés (n=%zu, w=%zu)...\n", n, w);
	//MDPCPrivateKey sk;
	//MDPCPublicKey pk;
	//sk.h0 = poly_create(n);
	//sk.h1 = poly_create(n);
	//pk.h = poly_create(n);

	//if (mdpc_keygen(n, w, &sk, &pk)) {
	//      printf("Clés générées avec succès.\n");
	//} else {
	//      printf("Échec de la génération des clés.\n");
	//}

	//printf("Chiffrement (t=%zu)...\n", t);
	//PolyGF2 *cipher = poly_create(pk.h->n);
	//PolyGF2 *e0 = poly_create(pk.h->n);
	//PolyGF2 *e1 = poly_create(pk.h->n);
	//mdpc_encrypt(&pk, t, cipher, e0, e1);

	//printf("Déchiffrement avec BitFlip (T=%zu)...\n", threshold);
	//PolyGF2 *e0_recover = poly_create(sk.h0->n);
	//PolyGF2 *e1_recover = poly_create(sk.h0->n);

	//int success = mdpc_decrypt(&sk, cipher, t, threshold, e0_recover, e1_recover);

	//if (success) {
	//      printf("Déchiffrement réussi !\n");
	//} else {
	//      printf
	//          ("Échec du déchiffrement. L'algorithme n'a pas convergé.\n");
	//}

	//poly_free(cipher);
	//poly_free(e0);
	//poly_free(e1);
	//poly_free(sk.h0);
	//poly_free(sk.h1);
	//poly_free(pk.h);
	//return 0;

	// exécution des actions
	if (action_genkey) {
		printf("Génération des clés (n=%zu, w=%zu)...\n", n, w);
		MDPCPrivateKey sk;
		MDPCPublicKey pk;
		sk.h0 = poly_create(n);
		sk.h1 = poly_create(n);
		pk.h = poly_create(n);

		if (mdpc_keygen(n, w, &sk, &pk)) {
			printf("Clés générées avec succès.\n");
			save_private_key(sk_file, &sk);
			save_public_key(pk_file, &pk);
			printf("Clés sauvegardées dans %s et %s\n", sk_file,
			       pk_file);
		} else {
			printf("Échec de la génération des clés.\n");
		}

		poly_free(sk.h0);
		poly_free(sk.h1);
		poly_free(pk.h);
	}

	else if (action_encrypt) {
		printf("Chiffrement (t=%zu)...\n", t);
		MDPCPublicKey pk;
		load_public_key(pk_file, &pk);

		PolyGF2 *cipher = poly_create(pk.h->n);
		PolyGF2 *e0 = poly_create(pk.h->n);
		PolyGF2 *e1 = poly_create(pk.h->n);
		mdpc_encrypt(&pk, t, cipher, e0, e1);

		save_poly_to_file(out_file, cipher);
		printf("Chiffré sauvegardé dans %s\n", out_file);

		save_shared_secret(secret_file, e0, e1);
		printf("Secret (e0, e1) sauvegardé dans %s\n", secret_file);

		poly_free(pk.h);
		poly_free(cipher);
		poly_free(e0);
		poly_free(e1);
	}

	else if (action_decrypt) {
		printf("Déchiffrement avec BitFlip (T=%zu)...\n", threshold);
		MDPCPrivateKey sk;
		load_private_key(sk_file, &sk);
		PolyGF2 *cipher = load_poly_from_file(in_file);

		PolyGF2 *e0 = poly_create(sk.h0->n);
		PolyGF2 *e1 = poly_create(sk.h0->n);

		int success = mdpc_decrypt(&sk, cipher, t, threshold, e0, e1);

		if (success) {
			printf("Déchiffrement réussi !\n");
			save_shared_secret(secret_file, e0, e1);
			printf("Secret (e0, e1) sauvegardé dans %s\n",
			       secret_file);
		} else {
			printf
			    ("Échec du déchiffrement. L'algorithme n'a pas convergé.\n");
		}

		poly_free(sk.h0);
		poly_free(sk.h1);
		poly_free(cipher);
		poly_free(e0);
		poly_free(e1);
	}

	else if (action_xor) {
		printf("Application du masque XOR...\n");
		apply_xor_mask(in_file, out_file, sk_file);
		printf
		    ("Opération XOR terminée. Résultat sauvegardé dans %s\n",
		     out_file);
	}

	else {
		printf("Aucune action spécifiée.\n");
		print_usage(argv[0]);
	}

	return 0;
}
