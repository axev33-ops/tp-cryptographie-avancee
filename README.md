# Projet/TP cryptograhie avancée codes

Travaux pratiques réalisés dans le cadre de l'unité d'enseignement *Cryptologie
avancée* du M1 CRYPTIS de l'Université de Limoges. Le sujet est la cryptographie
basée sur les codes correcteurs.

## Auteurs

Yann Denichou et Younes Kantas.

## Énoncé et avancement

### 1. Algorithme *Information Set Decoding* (ISD)

1. Expliquer rapidement le principe de l'algorithme vu en cours (version de
   base).
2. Programmer une inversion de matrice binaire par la méthode du pivot de Gauss.
3. Programmer l'algorithme ISD et le tester pour les paramètres suivants :
   - $n = 400$, $k = 200$, poids de l'erreur $t = 20$ (paramètres faciles pour
     un McEliece).
   - $n = 1000$, $k = 500$ et $t = 10$.

   Donner vos temps de calcul.

### 2. Système de chiffrement MDPC

**Remarque :** vous pouvez vous référer au document `cbc_rev2.pdf` sur le
discord, paragraphe 1.4.3 (ce schéma correspond au schéma BIKE-2 soumis au
NIST).

1. Rappeler le principe de l'algorithme BitFlip vu en TP (ne pas oublier le
   seuil $T$ dans l'algo).
2. Décrire le système de chiffrement MDPC vu en TP (section 1.4.3 du document
   `cbc_rev2.pdf`).
3. Programmer le système de chiffrement MDPC pour des paramètres (quasi-)réels :
   - poids de $x$ et $y$ : $w = 39$ ;
   - longueur de $x$ et $y$ : $n = 4813$;
   - poids total de l'erreur $e$ : $w_e = 78$ ;
   - seuil pour l'algorithme BitFlip : $T = 26$.

   Donner vos temps de calcul pour :
   - chiffrement ;
   - déchiffrement ;
   - création des clés.
4. Que se passe-t-il au niveau du décodage lorsqu'on fait varier le seuil $T$
   autour de $26$ ou que l'on augmente $w$ (de $10$ par exemple) ?

### 3. Travail à rendre

Un fichier PDF décrivant les divers algorithmes demandés et les résultats
obtenus avec les temps, ainsi que les fichiers source (langage au choix : C ou
Magma), considérer une approche polynomiale pour les calculs matriciels.
