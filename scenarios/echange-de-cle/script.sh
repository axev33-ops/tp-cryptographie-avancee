#!/usr/bin/env bash

# pour que bob/* se développe en une chaîne vide
shopt -s nullglob

CLI="../../bin/mdpc-cli"

# Couleurs pour un affichage propre dans le terminal
BLUE='\033[34m'
YELLOW='\033[33m'
GRAS='\033[1m'
RESET='\033[0m' # No Color

run_cmd() {
	echo -e "${BLUE}Commande : $RESET$*"
	"$@"
}

run_eval() {
	echo -e "${BLUE}Commande : $RESET$1"
	eval "$1"
}

pause() {
	echo ""
	read -n 1 -srp "Presser une touche pour continuer..."
	printf "\r\033[K"
}

((i = 0))
step() {
	pause
	echo -e "${BLUE}--------------------------------------------------------------------------------"
	echo -e "$GRAS${BLUE}[$((i++))] $*$RESET\n"
}

consequence() {
	echo -e "${BLUE}-> $*$RESET"
}

end_step() {
	echo -e "\n${YELLOW}Fichiers d'Alice : " alice/* "$RESET"
	echo -e "${YELLOW}Fichiers de Bob : " bob/* "$RESET"
}

# Nettoyage des anciens fichiers
rm -rf alice/ bob/
mkdir alice bob

echo -e "${BLUE}Démonstration d'un échange de clé avec MDPC$RESET"

step "Alice génère sa paire de clés asymétriques."
run_cmd $CLI -G -s alice/sk.bin -p alice/pk.bin
consequence "Alice publie 'alice/pk.bin' et garde 'alice/sk.bin' secret."
end_step

step "Bob récupère la clé publique d'Alice."
run_cmd cp alice/pk.bin bob/alice-pk.bin
end_step

step "Bob crée un secret et l'encapsule avec la clé publique d'Alice."
run_cmd $CLI -E -p bob/alice-pk.bin -o bob/cipher.bin -S bob/secret.bin
consequence "Bob possède son propre secret : 'bob/secret.bin'."
end_step

step "Bob envoie le chiffré à Alice."
run_cmd cp bob/cipher.bin alice/cipher.bin
end_step

step "Alice reçoit le chiffré et décapsule le secret à l'aide de sa clé secrète."
run_cmd $CLI -D -s alice/sk.bin -i alice/cipher.bin -S alice/secret.bin
consequence "Alice a retrouvé l'erreur et l'a sauvegardée dans 'alice/secret.bin'."
end_step

step "Alice et Bob partagent maintenant un secret, le script le vérifie."
run_eval "if cmp -s alice/secret.bin bob/secret.bin; then echo 'SUCCÈS'; else echo 'ÉCHEC'; fi"

step "Alice et Bob peuvent se servir du hashé de ce secret comme clé pour un chiffrement symétrique."
run_cmd sha256sum alice/secret.bin
run_cmd sha256sum bob/secret.bin
