#-------------------------------------------------------------------------------
#
#  Exemples d'utilisation
#--------------------------
#
# Compiler les fichiers exécutables :
#	make
#
# Exécuter un exécutable par un script de sélection (et le compiler avant si
# besoin) :
#	make run
#
# Supprimer les fichiers exécutables et les fichiers objets :
#	make clean
#


#-------------------------------------------------------------------------------
#
#  Variables
#-------------

# dossiers
SRC_DIR := src
INC_DIR := include
APP_DIR := apps
OBJ_DIR := obj
BIN_DIR := bin
TEST_DIR := tests

# compilateur
CC := gcc

# options pour le compilateur
CFLAGS := -I$(INC_DIR) -Wall -Wextra $(EXTRA_CFLAGS)

# -lm est pour l'utilisation de la librairie math
LDFLAGS := -lm $(EXTRA_LDFLAGS)

# fichiers « communs », i.e. implémentations des algo
COMMON_SRCS := $(wildcard $(SRC_DIR)/*.c)
COMMON_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS))

# fichiers d'application, i.e. utilisent les fichiers « communs » en conditions
# « réelles »
APP_SRCS := $(wildcard $(APP_DIR)/*.c)
APP_BINS := $(patsubst $(APP_DIR)/%.c, $(BIN_DIR)/%, $(APP_SRCS))

# fichiers .h
HEADERS := $(wildcard $(INC_DIR)/*.h)

# fichier exécutable du jeu
TARGET := $(BIN_DIR)/jeu-de-role

# fichiers de tests
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/test-%, $(TEST_SRCS))

# script d'indentation
INDENT := ./scripts/format-c-files.sh

# script de lancement d'un exécutable
RUN := ./scripts/run.sh

#-------------------------------------------------------------------------------
#
#  Règles
#----------

# cible par défaut (make sans argument)
all: $(APP_BINS) $(TEST_BINS)

# Cible        fichier exécutable d'« application »
# Dépendances  fichier source (.c) de l'« app », fichiers objets (.o) et
#              dossier bin/
$(BIN_DIR)/%: $(APP_DIR)/%.c $(COMMON_OBJS) | $(BIN_DIR)
	@echo "Liaison des .o pour former l'application $@..."
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Liaison terminée !"

# Cible        fichier exécutable de test
# Dépendances  fichier source (.c) du test, fichiers objets (.o) et dossier
#              bin/
$(BIN_DIR)/test-%: $(TEST_DIR)/%.c $(COMMON_OBJS) | $(BIN_DIR)
	@echo "Liaison des .o pour former le test $@..."
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Liaison terminée !"

# Cible        fichiers objets (.o)
# Dépendances  fichiers sources (.c) et dossier obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compilation de $<..."
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

# Crée le dossier bin/ ou obj/ si besoin
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# supprime les fichiers objets et exécutables
clean:
	@echo "Ménage (suppressions des .o et des exécutables)..."
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run: all
	$(RUN)

tests: $(TEST_BINS)

apps: $(APP_BINS)

indent:
	$(INDENT)

# définit les « actions » que l'on mettra après make, e.g. `make run`
.PHONY: all apps clean indent run tests
