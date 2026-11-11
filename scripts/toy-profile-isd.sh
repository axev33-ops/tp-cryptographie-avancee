#!/usr/bin/env bash

# finir immédiatement le script si une commande échoue
set -e

OUTPUT_DIR=profile
mkdir -p "$OUTPUT_DIR"

EXECUTABLE="./bin/test-isd"

run_profile() {
	local extra_cflags extra_ldflags csv_file output_file profile_file
	extra_cflags="$1"
	extra_ldflags="$2"
	profile_file="$OUTPUT_DIR/$3-profile.txt"

	# profilage
	make clean >/dev/null
	make EXTRA_CFLAGS="$extra_cflags -pg -g" EXTRA_LDFLAGS="$extra_ldflags -pg" >/dev/null
	$EXECUTABLE
	gprof $EXECUTABLE gmon.out >"$profile_file"
	echo "Profil sauvegardé dans le fichier $profile_file"
}

echo "PROFILAGE ISD"
echo "============="
printf "\nIMPLÉMENTATION NAÏVE\n"
echo "--------------------"
run_profile "-DUSE_NAIVE -DVERBOSE" "" "isd-naive"

printf "\nIMPLÉMENTATION OPTIMISÉE\n"
echo "------------------------"
run_profile "-DVERBOSE" "" "isd-opt"
