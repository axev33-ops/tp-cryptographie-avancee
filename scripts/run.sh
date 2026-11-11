#!/usr/bin/env bash

if [ ! -d "./bin" ]; then
	echo "Dossier ./bin introuvable. Lancez 'make' d'abord." 1>&2
	exit 1
fi

if ! command -v fzf >/dev/null 2>&1; then
	echo "Installer 'fzf' pour utiliser ce script." 1>&2
	echo "Sinon lancer les exécutable manuellement (voir répertoire bin/)." 1>&2
	exit 1
fi

liste_execs=$(find ./bin -maxdepth 1 -type f -executable)

if [ -z "$liste_execs" ]; then
	echo "Aucun exécutable trouvé dans ./bin/."
	echo "Lancer 'make'."
	exit 1
fi

champs_fzf=$(sed 's:./bin/\(.*\):\0\t\1:' <<<"$liste_execs")

champ_selectionne=$(fzf --with-nth 2.. --prompt "Exécutable> " \
	--header "Sélectionner un programme à lancer." \
	<<<"$champs_fzf")

chemin_exec="${champ_selectionne%%$'\t'*}"

if [ -n "$chemin_exec" ]; then
	echo "Lancement de $chemin_exec..."
	echo "-----------------------------------"
	exec "$chemin_exec"
fi
