# Documentation du projet `base_IF4_TP2_BO_v2024_1`

Ce document décrit la structure et le fonctionnement du code présent dans ce dossier.

## Objectif du Programme

Le programme principal `base_IF4_TP2_BO_v2024_1.ino` est conçu pour un microcontrôleur ESP32 et sert de base pour un exercice de contrôle en **boucle ouverte (BO)** d'un robot mobile.

Le comportement attendu, décrit en commentaire dans le fichier `.ino`, est le suivant :
1.  Attendre 1 seconde.
2.  Faire avancer le robot pendant 500 ms.
3.  Attendre 500 ms.
4.  Faire reculer le robot pendant 500 ms.
5.  Arrêter définitivement le robot.

## Analyse des Fichiers

### `base_IF4_TP2_BO_v2024_1.ino`

-   **Rôle** : Fichier principal du sketch Arduino.
-   **Logique** :
    -   La fonction `setup()` initialise le port série, configure les broches (pins) des drivers moteurs (`MLF`, `MLB`, `MRF`, `MRB`) comme sorties PWM, et initialise les broches des encodeurs des roues (`SLA`, `SLB`, `SRA`, `SRB`) comme entrées.
    -   Une fois l'initialisation terminée, la tâche `setup()` se met en suspension (`vTaskSuspend`). Cela indique que la logique de contrôle principale n'est pas dans `setup()` ni dans `loop()`.
    -   La fonction `loop()` est vide et n'est pas censée être exécutée.
-   **Conclusion** : Ce fichier est un **template**. La logique de contrôle (la séquence de mouvements décrite plus haut) est **absente** et doit être implémentée par l'étudiant, très probablement en créant une nouvelle tâche FreeRTOS qui pilotera les moteurs via la fonction `analogWrite()`.

### Fichiers `Filter0.*` (`Filter0.c`, `Filter0.h`, etc.)

-   **Origine** : Ces fichiers ont été générés automatiquement par **MATLAB/Simulink** à partir d'un modèle nommé `Filter0`.
-   **Rôle** : Ils implémentent une bibliothèque pour un **filtre numérique** à deux canaux.
    -   La fonction `Filter0_initialize()` prépare le filtre.
    -   La fonction `Filter0_step()` exécute un pas de calcul. Elle lit ses entrées dans une structure globale `Filter0_U` et écrit ses sorties dans `Filter0_Y`.
-   **Utilisation** : Ce filtre **n'est pas utilisé** dans le programme en boucle ouverte (`.ino`). Il est fourni en prévision d'un exercice ultérieur de contrôle en **boucle fermée (BF)**, où la vitesse des roues (mesurée par les encodeurs) serait passée en entrée du filtre pour calculer la commande à appliquer aux moteurs.

## Résumé

Ce projet est une base de départ pour un TP sur le contrôle d'un robot. Le fichier `.ino` ne contient que l'initialisation du matériel. La logique de commande en boucle ouverte est à compléter. Les fichiers du filtre sont une ressource pour une future implémentation en boucle fermée.
