#!/usr/bin/env bash

# finir immédiatement le script si une commande échoue
set -e

OUTPUT_DIR=test-output
mkdir -p "$OUTPUT_DIR"

printf -v DATETIME '%(%Y-%m-%d-%H-%M-%S)T'
echo $DATETIME
DATA_DIR=data
CSV_FILE="$DATA_DIR/test-isd-$DATETIME.csv"

EXECUTABLE="./bin/app-test-isd"

echo "TEST ISD"
echo "========"
make clean >/dev/null
make EXTRA_CFLAGS="-DVERBOSE -O3"

$EXECUTABLE "$CSV_FILE"
