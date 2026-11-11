#include <stdlib.h>
#include <stdio.h>
#include "permutation.h"

Permutation *permutation_create(size_t n)
{
	Permutation *p = malloc(sizeof(Permutation));
	if (p == NULL) {
		fprintf(stderr,
			"Erreur : allocation mémoire Permutation struct.\n");
		return NULL;
	}

	p->size = n;
	p->indices = malloc(n * sizeof(size_t));
	if (p->indices == NULL) {
		fprintf(stderr,
			"Erreur : allocation mémoire Permutation indices.\n");
		free(p);
		return NULL;
	}

	for (size_t i = 0; i < n; i++) {
		p->indices[i] = i;
	}

	permutation_shuffle(p);

	return p;
}

// mélange une permutation avec Fisher-Yates
// voir https://fr.wikipedia.org/wiki/M%C3%A9lange_de_Fisher-Yates
void permutation_shuffle(Permutation *p)
{
	if (p == NULL || p->indices == NULL)
		return;

	size_t n = p->size;
	for (size_t i = n - 1; i > 0; i--) {
		size_t j = rand() % (i + 1);

		size_t tmp = p->indices[i];
		p->indices[i] = p->indices[j];
		p->indices[j] = tmp;
	}
}

void permutation_destroy(Permutation *p)
{
	if (p != NULL) {
		if (p->indices != NULL) {
			free(p->indices);
		}
		free(p);
	}
}
