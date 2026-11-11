#!/usr/bin/env python3

import pandas as pd
import argparse
import sys

def summarize_benchmarks(input_csv, output_csv):
    try:
        data_file = pd.read_csv(input_csv)
    except FileNotFoundError:
        print(f"Erreur : fichier non trouvé '{input_csv}'")
        sys.exit(1)

    summary = data_file.groupby(['n', 'k', 't']).agg({
        'expected_iterations': 'first',
        'iterations': 'mean',
        'time_sec': 'mean',
        'run': 'count'
    }).reset_index()

    summary.rename(columns={
        'iterations': 'avg_iterations',
        'time_sec': 'avg_time_sec',
        'run': 'successful_runs'
    }, inplace=True)

    summary['time_per_iter_us'] = (summary['avg_time_sec'] / summary['avg_iterations']) * 1_000_000

    print(f"\nRésultats ({input_csv})")
    print(summary.to_string(index=False))

    summary.to_csv(output_csv, index=False)
    print(f"\nDonnées traitées sauvegardées dans : {output_csv}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Interprète les CSV des tests d'ISD.")
    parser.add_argument("-i", "--input", help="Chemin vers le CSV d'entrée")
    parser.add_argument("-o", "--output", help="Chemin vers le CSV d'entrée de sortie")

    args = parser.parse_args()
    summarize_benchmarks(args.input, args.output)
