# GestionnaireProcessus
Projet de gestionnaire de processus

Pour lancer le programme : 
- tapez 'make all' dans ce répertoire pour générer un exécutable (uniquement sous linux).
- tapez './main' pour lancer le programme de manière normale
- tapez './main -t' pour lancer le programme en modifiant la table d'allocation CPU (les pourcentages d'utilisation du CPU attribués pour chaque priorité de processus).
- tapez './main -f' pour lancer le programme en utilisant le jeu de données stocké dans 'donnees.txt'. Voir ci-dessous pour comprendre la structure de ce fichier.

Structure de 'donnees.txt' : 
Première ligne : pourcentages attribués à chaque priorité, par ordre croissant (priorité 0, puis 1, etc...)
Seconde ligne : piorité attribuée à chaque processus, par ordre de processus (Processus 0, puis Processus 1, etc...)
Troisième ligne : date d'entrée de chaque processus, par ordre de processus (Processus 0, puis Processus 1, etc...)
Quatrième ligne : Temps attribué à chaque processus, par ordre de processus (Processus 0, puis Processus 1, etc...), en quantum.

Ainsi, pour le processus 0, on a : la priorité 6, la date d'entrée 9 et le temps d'exécution 3.

Les lignes suivantes décrivent dans quel ordre ces processus devraient s'exécuter. Choisir l'option '-f' supprime la part d'aléatoire lors de la création des processus et assure ainsi un ordre identique à chaque exécution.