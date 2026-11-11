#!/usr/bin/env bash

# CONFIGURATION

FORMAT_CMD='indent'
FORMAT_FLAGS='-linux'
OUTPUT_SUFFIX=".formatted"

declare -a FILES
mapfile -t FILES < <(find . -type f \( -name "*.c" -o -name "*.h" \))

format_file() {
	local input_file output_file
	input_file="$1"
	output_file="$2"
	$FORMAT_CMD $FORMAT_FLAGS "$input_file" -o "$output_file"
}

ask_accept_format() {
	while true; do
		echo -n "Accepter le formattage [Y/n] ? "
		read -r response
		response=${response,,} # minuscules
		if [[ "$response" =~ ^(y|yes|)$ ]]; then
			return 0
		elif [[ "$response" =~ ^(n|no)$ ]]; then
			return 1
		else
			echo "Réponse invalide. Répondre 'y' ou 'n'."
		fi
	done
}

process_files() {
	local confirm_flag
	confirm_flag="$1"
	for file in "${FILES[@]}"; do
		formatted_file="${file}${OUTPUT_SUFFIX}"
		format_file "$file" "$formatted_file"
		if [[ -f "$formatted_file" ]]; then
			if cmp -s "$file" "$formatted_file"; then
				echo "$file est déjà bien formatté !"
				rm "$formatted_file"
			else
				echo "Changements du formattage proposé pour $file :"
				diff --color=auto "$file" "$formatted_file"
				if { ! $confirm_flag; } || ask_accept_format; then
					rm "$file" && mv "$formatted_file" "$file"
				else
					rm "$formatted_file"
				fi
			fi
		fi
	done
}

main() {
	local confirm_flag
	confirm_flag=true
	while (($# > 0)); do
		case "$1" in
		--no-confirm) confirm_flag=false ;;
		*)
			echo "Argument '$1' non accepté." >&2
			exit 1
			;;
		esac
		shift
	done
	process_files "$confirm_flag"
}

main "$@"
