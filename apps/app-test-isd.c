#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "isd.h"
#include "isd-test-lib.h"

int main(int argc, char *argv[])
{
	srand(time(NULL));

	const char *filename = "data/isd-results.csv";
	if (argc > 1)
		filename = argv[1];
	else
		printf
		    ("Pas de nom de fichier csv donné en argument. Défaut utilisé : %s\n",
		     filename);

	FILE *csv_file = fopen(filename, "w");
	if (csv_file == NULL) {
		fprintf(stderr,
			"Erreur : impossible d'écrire le fichier CSV %s.\n",
			filename);
		return 1;
	}
	fprintf(csv_file,
		"n,k,t,expected_iterations,run,iterations,time_sec\n");

	printf("Début des tests, les données seront sauvegardées dans %s.\n",
	       filename);

	run_batch_test(csv_file, 1000, 500, 10, 50);
	run_batch_test(csv_file, 400, 200, 20, 10);

	fclose(csv_file);
	return 0;
}
