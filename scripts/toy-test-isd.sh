#!/usr/bin/env bash

# finir immédiatement le script si une commande échoue
set -e

OUTPUT_DIR=test-output
DATA_DIR=data
mkdir -p "$OUTPUT_DIR"

EXECUTABLE="./bin/test-isd"
PROCESS_DATA="python3 scripts/process_isd_test_data.py"

run_test() {
	local extra_cflags extra_ldflags csv_file output_file
	extra_cflags="$1"
	extra_ldflags="$2"
	csv_file="$DATA_DIR/$3.csv"
	csv_processed_file="$DATA_DIR/processed-$3.csv"
	output_file="$OUTPUT_DIR/$3.txt"

	make clean >/dev/null
	make EXTRA_CFLAGS="$extra_cflags" EXTRA_LDFLAGS="$extra_ldflags" >/dev/null
	$EXECUTABLE "$csv_file" | tee "$output_file"
	echo "Sortie sauvegardée dans le fichier $output_file"
	$PROCESS_DATA -i "$csv_file" -o "$csv_processed_file"
}

echo "TESTS SANS OPTIMISATION DE COMPILATION"
echo "======================================"
printf "\nIMPLÉMENTATION NAÏVE\n"
echo "--------------------"
run_test "-DUSE_NAIVE -DVERBOSE" "" "isd-naive"

printf "\nIMPLÉMENTATION OPTIMISÉE\n"
echo "------------------------"
run_test "-DVERBOSE" "" "isd-opt"

printf "\nTESTS AVEC OPTIMISATION DE COMPILATION\n"
echo "======================================"
printf "\nIMPLÉMENTATION NAÏVE\n"
echo "--------------------"
run_test "-DUSE_NAIVE -DVERBOSE -O3" "" "isd-naive-O3"

printf "\nIMPLÉMENTATION OPTIMISÉE\n"
echo "------------------------"
run_test "-DVERBOSE -O3" "" "isd-opt-O3"
