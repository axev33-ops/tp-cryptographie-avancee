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

echo -e "${BLUE}Démonstration d'un échange de message avec MDPC$RESET"

step "Contexte : Bob veut envoyer un message à Alice."
echo "Salut Alice, c'est top secret." >bob/plain-message.txt
run_cmd cat bob/plain-message.txt
end_step

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

step "Bob hache le secret pour obtenir un masque jetable."
run_eval "sha256sum bob/secret.bin | awk '{print \$1}' | xxd -r -p > bob/mask.bin"
end_step

step "Bob chiffre (XOR) son message avec ce masque jetable."
run_cmd $CLI -X -s bob/mask.bin -i bob/plain-message.txt -o bob/cipher-message.bin
end_step

step "Bob envoie le chiffré du secret et le chiffré du message à Alice."
run_cmd cp bob/cipher.bin bob/cipher-message.bin alice/
end_step

step "Alice reçoit le chiffré et décapsule le secret à l'aide de sa clé secrète."
run_cmd $CLI -D -s alice/sk.bin -i alice/cipher.bin -S alice/secret.bin
consequence "Alice obtient le même secret que Bob."
end_step

step "Alice hache le secret pour retrouver le même masque jetable."
run_eval "sha256sum alice/secret.bin | awk '{print \$1}' | xxd -r -p > alice/mask.bin"
end_step

step "Alice déchiffre le message chiffré en appliquant le masque."
run_cmd $CLI --xor --sk alice/mask.bin --in alice/cipher-message.bin --out alice/plain-message.txt
consequence "Alice peut lire le message de Bob."
run_cmd cat alice/plain-message.txt
