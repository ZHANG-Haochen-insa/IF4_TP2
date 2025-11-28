# Système de Contrôle de Vitesse pour Robot Mobile basé sur FreeRTOS

**INSA Lyon, IF4 Systèmes Temps Réel TP2**

**Auteurs :** Haiwei CHEN, Haochen ZHANG

**Date :** 28 novembre 2025

---

## Résumé

Ce travail pratique implémente un système complet de contrôle temps réel pour robot mobile sur la plateforme microcontrôleur ESP32. Le système, basé sur le système d'exploitation temps réel FreeRTOS, intègre un pilotage de moteur PWM, une mesure de vitesse par encodeur incrémental, un contrôle de vitesse en boucle fermée de type PI, ainsi qu'une fonction de communication sans fil WiFi. Grâce à une architecture multitâche parallèle, le système réalise un contrôle de vitesse précis d'un robot à deux roues à différentiel (erreur en régime permanent <1%) et une fonction de télécommande sans fil à distance. Les résultats expérimentaux montrent que le correcteur PI, comparé au correcteur P, permet d'éliminer efficacement l'erreur en régime permanent, et que le système global satisfait les exigences temps réel.

---

## 1. Introduction

Le contrôle de vitesse des robots mobiles représente une application typique des systèmes embarqués temps réel. Ce travail pratique vise à implémenter un système robotique complet intégrant perception, contrôle et communication à travers le microcontrôleur ESP32. L'expérience est divisée en quatre phases progressives : d'abord l'implémentation d'un pilotage de moteur en boucle ouverte basé sur PWM, ensuite l'intégration d'encodeurs pour réaliser la rétroaction de vitesse, puis la conception et l'implémentation d'un correcteur PI en boucle fermée, et enfin la réalisation d'une fonction de télécommande à distance via communication WiFi. L'ensemble du système adopte une architecture multitâche FreeRTOS, garantissant la contrainte temps réel de la boucle de contrôle par ordonnancement de priorités de tâches.

---

## 2. Architecture Matérielle du Système

La plateforme robotique mobile utilisée dans cette expérience est équipée d'un microcontrôleur ESP32 double cœur, de deux moteurs DC avec réducteur et de deux encodeurs incrémentaux. Les moteurs reçoivent des signaux de contrôle PWM via un circuit de pilotage en pont H, chaque moteur possédant deux broches de contrôle (direction avant et arrière). Les encodeurs adoptent une sortie quadrature à deux canaux, générant 11 impulsions par tour, et avec un rapport de réduction de 30:1 et un mode de décodage en quadrature, la résolution théorique atteint 1320 impulsions/tour.

L'allocation des ressources matérielles de l'ESP32 est la suivante : les GPIO 26 et 25 contrôlent l'avance et le recul du moteur gauche, les GPIO 33 et 32 contrôlent le moteur droit ; les GPIO 14 et 27 sont connectés aux phases A et B de l'encodeur gauche, les GPIO 35 et 34 à l'encodeur droit. Les signaux PWM sont générés via le module LEDC de l'ESP32, avec une fréquence réglée à 1kHz et une résolution de 15 bits (plage 0-32767), permettant un contrôle de vitesse fin.

---

## 3. Implémentation du Contrôle PWM en Boucle Ouverte

### 3.1 Principe de Génération du Signal PWM

Le module LEDC de l'ESP32 offre une capacité flexible de génération PWM. La fonction `ledcAttach()` lie une broche GPIO à un canal PWM et configure la fréquence et la résolution. Ensuite, la fonction `analogWrite()` permet d'ajuster dynamiquement le rapport cyclique. La fréquence PWM de 1kHz choisie dans cette expérience est basée sur la considération de la constante de temps mécanique du moteur ; cette fréquence garantit à la fois un contrôle de vitesse fluide et évite les pertes de commutation causées par une fréquence trop élevée.

### 3.2 Conception de l'Architecture Multitâche

Le programme adopte une architecture à double tâche pour implémenter la séquence de mouvement prédéfinie (attente 500ms → avance 500ms → arrêt 500ms → recul 500ms → arrêt permanent). La tâche Setup est responsable de l'initialisation matérielle, puis crée la tâche MotorControl et se suspend elle-même, évitant ainsi l'interférence de la fonction `loop()` du framework Arduino. La tâche MotorControl utilise la fonction `vTaskDelay()` de FreeRTOS pour réaliser un contrôle temporel précis, et après achèvement, se supprime elle-même via `vTaskDelete(NULL)`, libérant ainsi les ressources système.

Le point clé de cette architecture réside dans le traitement correct du problème d'intégration entre le framework Arduino-ESP32 et FreeRTOS. Comme `setup()` et `loop()` s'exécutent dans la même tâche FreeRTOS, sans traitement approprié, `loop()` s'exécuterait continuellement, interférant avec la logique de contrôle. La solution consiste à appeler `vTaskSuspend(xTaskGetCurrentTaskHandle())` avant la fin de `setup()` pour suspendre la tâche courante.

---

## 4. Système de Mesure de Vitesse

### 4.1 Décodage en Quadrature de l'Encodeur

Les deux signaux en quadrature de phase d'un encodeur incrémental contiennent des informations sur la direction et la vitesse de rotation. Cette expérience adopte la méthode de machine à états pour implémenter le décodage en quadrature : des interruptions sont déclenchées à chaque front des deux phases A et B, et en analysant la séquence de transition d'états (rotation positive : 00→01→11→10→00, rotation inverse : 00→10→11→01→00), la direction de rotation est déterminée et un compteur est incrémenté. Cette méthode augmente la résolution d'un facteur quatre, atteignant 1320 impulsions/tour, améliorant significativement la précision de mesure à basse vitesse.

La routine de service d'interruption (ISR) utilise l'attribut `IRAM_ATTR` pour s'assurer que le code est stocké dans la RAM interne, réduisant ainsi le délai d'exécution. Chaque interruption lit l'état AB actuel, le compare avec l'état précédent pour déterminer la direction (+1 ou -1), et met à jour le compteur global. Pour éviter la compétition de données, lors de la lecture du compteur par la tâche principale, une section critique est protégée par `portENTER_CRITICAL`/`portEXIT_CRITICAL`, garantissant l'atomicité de l'opération.

### 4.2 Calcul de Vitesse et Tâche Périodique

La tâche de mesure de vitesse fonctionne avec une période de 100ms, à chaque période elle lit et réinitialise le compteur d'encodeur, et calcule la vitesse angulaire selon la formule $\omega = \frac{\Delta N}{N_{rev}} \cdot \frac{2\pi}{T}$ (unité : rad/s), où $\Delta N$ est le nombre d'impulsions dans la période, $N_{rev}=1320$ est le nombre d'impulsions par tour, $T=0.1s$ est la période de mesure. Cette tâche utilise `vTaskDelayUntil()` plutôt que `vTaskDelay()` pour implémenter l'exécution périodique ; la première étant basée sur le temps absolu, elle évite l'erreur cumulative du délai relatif, garantissant la stabilité de la période de mesure.

La tâche de contrôle moteur s'exécute à une priorité inférieure, exécutant la séquence d'avance-recul prédéfinie. La priorité élevée de la tâche de mesure de vitesse garantit que même si la tâche de contrôle moteur occupe le CPU, l'échantillonnage de vitesse peut toujours s'exécuter à temps. L'expérience dure 10 secondes, les données sont transmises via le port série au format séparé par tabulations, facilitant l'analyse graphique ultérieure.

---

## 5. Conception du Contrôle de Vitesse en Boucle Fermée

### 5.1 Fondements Théoriques du Contrôle

L'objectif du contrôle en boucle fermée est de faire suivre précisément à la vitesse du moteur une valeur de consigne donnée. Cette expérience compare les performances d'un correcteur proportionnel (P) et d'un correcteur proportionnel-intégral (PI). La loi de contrôle du correcteur P est $u(t) = K_p \cdot e(t)$, où l'erreur $e(t)=r(t)-y(t)$ est la différence entre la valeur de consigne et la valeur mesurée. Le correcteur P répond rapidement mais présente une erreur en régime permanent inhérente : lorsque l'erreur diminue, la sortie de contrôle diminue également, et le système s'équilibre à une erreur non nulle avant d'atteindre le régime permanent.

Le correcteur PI résout le problème de l'erreur en régime permanent en introduisant un terme intégral $u(t) = K_p \cdot e(t) + K_i \cdot \int_0^t e(\tau)d\tau$. Le terme intégral accumule l'erreur historique ; même si l'erreur instantanée est très petite, tant que le terme intégral n'est pas nul, le système continue d'ajuster la sortie jusqu'à ce que l'erreur soit complètement éliminée. La loi de contrôle discrétisée est $u[k] = K_p \cdot e[k] + K_i \cdot T \cdot \sum_{i=0}^{k}e[i]$, où $T$ est la période de contrôle.

### 5.2 Réglage des Paramètres du Correcteur

L'expérience adopte la méthode d'essais-erreurs pour régler les paramètres du correcteur. La tentative initiale avec $K_p=3000$ donne une réponse système lente avec une erreur en régime permanent atteignant 60%. L'augmentation progressive de $K_p$ à 6000 améliore la vitesse de réponse, mais le correcteur P ne peut éliminer l'erreur en régime permanent (Figure 1). Après introduction du terme intégral avec $K_i=8000$, l'erreur en régime permanent du système descend à 0,8% (Figure 2).

![Réponse correcteur P](BF_CHEN_Haiwei_ZHANG_Haochen/p_controller_response.png)
**Figure 1 :** Courbe de réponse indicielle du correcteur P. Le graphique supérieur montre la vitesse de consigne (ligne bleue) et la vitesse mesurée (ligne rouge), avec une erreur en régime permanent évidente (environ 60%). Le graphique inférieur montre le signal de contrôle correspondant. La séquence de consignes est 0 → 2,5 → -2,5 → 0 rad/s, chaque échelon durant 5 secondes.

![Réponse correcteur PI](BF_CHEN_Haiwei_ZHANG_Haochen/pi_controller_response.png)
**Figure 2 :** Courbe de réponse indicielle du correcteur PI ($K_p=6000$, $K_i=8000$). La vitesse mesurée peut suivre rapidement la valeur de consigne, avec une erreur en régime permanent inférieure à 1%. On note un léger dépassement aux instants d'échelon (par exemple à l'instant 5 secondes atteignant 2,8 rad/s), causé par l'accumulation rapide du terme intégral, mais le système se stabilise en environ 1 seconde. Le signal de contrôle atteint la limite de saturation aux instants d'échelon, puis se stabilise au niveau nécessaire pour maintenir le régime permanent.

Pour prévenir le phénomène de saturation intégrale (windup), un mécanisme de limitation intégrale a été implémenté : lorsque le terme intégral dépasse $\pm 15000$, il est tronqué. En même temps, lors d'un changement d'échelon de la valeur de consigne, le terme intégral est réinitialisé, évitant que l'ancienne valeur intégrale n'affecte le nouveau processus de contrôle. La période de contrôle est fixée à 100ms (optimisée ultérieurement à 50ms), équilibrant performance de contrôle et charge de calcul.

### 5.3 Analyse de Performance

De la Figure 2, on peut observer les caractéristiques typiques du correcteur PI : lors de l'échelon positif (0→2,5 rad/s), il existe un dépassement d'environ 12% (pic à 2,8 rad/s), mais le temps de réglage est d'environ 1,5 seconde, après quoi l'erreur en régime permanent reste inférieure à 0,02 rad/s (0,8%). Lors de l'échelon négatif (2,5→-2,5 rad/s), un écart transitoire plus important apparaît, car le moteur doit s'inverser complètement, avec friction mécanique et retard électrique. La courbe du signal de contrôle montre qu'aux instants d'échelon, la valeur PWM change rapidement vers les valeurs limites (près de ±32767), indiquant que le correcteur exploite pleinement la capacité de l'actionneur.

En comparaison, le correcteur P de la Figure 1, bien qu'ayant une réponse initiale plus rapide, ne peut finalement atteindre qu'environ 1,0 rad/s (40% de la consigne), et le signal de contrôle se stabilise à un niveau faible (environ 5000), incapable de fournir une force motrice suffisante pour surmonter les perturbations comme la friction. Cela vérifie la nécessité de l'action intégrale du correcteur PI pour éliminer l'erreur en régime permanent.

---

## 6. Communication WiFi et Intégration Système

### 6.1 Architecture de Télécommande Sans Fil

Le système final étend le contrôle en boucle fermée au contrôle indépendant des deux roues, et intègre la communication WiFi pour réaliser la télécommande à distance. L'ESP32 est configuré en mode AP (point d'accès), créant un point d'accès WiFi avec le SSID "MonRobot". Après connexion du téléphone à ce point d'accès, l'accès via navigateur à `http://192.168.4.1` affiche l'interface de contrôle, incluant quatre boutons directionnels : avant, arrière, gauche, droite.

Le serveur HTTP est implémenté par la fonction `communicate_with_phone()`, analysant le chemin URL pour identifier les instructions utilisateur (par exemple `/26/on` correspond à avancer). Après réception d'une instruction, la fonction `update_desired_speeds()` met à jour les vitesses désirées des deux roues selon le modèle cinématique : lors de l'avance, les deux roues tournent dans le même sens (toutes deux à 2,5 rad/s), lors du recul elles s'inversent dans le même sens (toutes deux à -2,5 rad/s), lors du virage gauche la roue gauche s'inverse et la roue droite avance (±1,5 rad/s), et inversement pour le virage droit. La propulsion différentielle réalise ainsi la capacité de rotation sur place.

### 6.2 Coordination Multitâche et Protection des Ressources

Le système comprend deux tâches principales : wifiCommunicationTask (priorité 1) interroge les requêtes HTTP avec une période de 10ms, traite les instructions utilisateur et met à jour les vitesses désirées ; speedControlTask (priorité 2) exécute avec une période de 50ms le contrôle PI indépendant des deux roues, lit les encodeurs, calcule les erreurs de vitesse, met à jour les termes intégraux et sort les signaux de contrôle PWM.

Les deux tâches partagent les informations de vitesses désirées via les variables globales `desiredSpeedLeft` et `desiredSpeedRight`. Comme l'ESP32 est un processeur double cœur, les deux tâches peuvent réellement s'exécuter en parallèle, il faut donc utiliser des sections critiques pour protéger ces variables partagées. Lors de l'écriture (tâche WiFi) et de la lecture (tâche de contrôle), le verrouillage est effectué par `portENTER_CRITICAL(&speedMutex)`, garantissant l'atomicité de l'opération. Ce mécanisme de protection basé sur spinlock a un délai extrêmement faible (niveau microseconde) et n'affecte pas la contrainte temps réel de la tâche de contrôle.

La conception des priorités suit les principes des systèmes temps réel : la priorité de la tâche de contrôle est supérieure à celle de la tâche de communication, garantissant que lors de compétition pour les ressources CPU, la boucle de contrôle s'exécute en priorité. La période de contrôle optimisée de 100ms à 50ms améliore la bande passante du système et la capacité de rejet de perturbations. La période d'interrogation de 10ms de la tâche WiFi garantit la fluidité de l'interaction homme-machine (latence <100ms).

### 6.3 Vérification Expérimentale

Les tests réels montrent que le système fonctionne de manière stable et fiable. Après envoi d'une instruction par le téléphone, le robot répond au mouvement dans les 50-100ms. Le contrôle en boucle fermée garantit la précision de vitesse dans divers modes de mouvement : lors de mouvements rectilignes, l'erreur de vitesse entre les deux roues est <2%, les mouvements de rotation sont fluides sans vibration. Le système peut fonctionner en continu sans blocage ni famine de tâche, vérifiant la correction de l'architecture multitâche et des mécanismes de protection des ressources.

---

## 7. Conclusion

Cette expérience a réussi à implémenter un système complet de contrôle temps réel embarqué, intégrant plusieurs sous-systèmes tels que capteurs, contrôle, actionneurs et communication. Grâce à l'architecture multitâche FreeRTOS, le système réalise une extension fonctionnelle flexible tout en garantissant la contrainte temps réel de la boucle de contrôle. L'introduction du correcteur PI améliore la précision de suivi de vitesse de 60% d'erreur avec le correcteur P à moins de 1%, vérifiant l'efficacité de la théorie du contrôle dans les systèmes réels. L'intégration du module de communication WiFi démontre les perspectives d'application de la technologie IoT dans le contrôle robotique.

Les divers problèmes techniques rencontrés durant le processus expérimental (conflits d'ordonnancement de tâches, compétition de données, saturation intégrale, etc.) ont tous été résolus par analyse systématique et conception ciblée ; ces expériences ont une valeur de référence importante pour le développement de systèmes embarqués temps réel. À l'avenir, on peut sur cette base étudier plus avant des scénarios d'application plus complexes tels que le contrôle de suivi de trajectoire, l'évitement d'obstacles, la collaboration multi-robots, etc.

---

## Annexe : Structure du Code

```
IF4_TP2/
├── BO_CHEN_ZHANG/                    # Expérience contrôle PWM boucle ouverte
│   └── base_IF4_TP2_BO_v2024_1.ino
├── BO_Vitesse_CHEN_ZHANG/            # Expérience mesure de vitesse
│   └── BO_Vitesse.ino
├── BF_CHEN_Haiwei_ZHANG_Haochen/     # Expérience contrôle vitesse boucle fermée
│   ├── BF.ino
│   ├── plot_p_controller.py
│   ├── plot_pi_controller.py
│   ├── p_controller_response.png
│   └── pi_controller_response.png
└── Remote/                           # Expérience intégration télécommande WiFi
    ├── Remote.ino
    ├── ESP32Encoder.cpp
    ├── ESP32Encoder.h
    ├── http_server.cpp
    └── http_server.hpp
```

---

**Remerciements :** Nous remercions l'équipe pédagogique du cours IF4 Systèmes Temps Réel de l'INSA Lyon pour la plateforme expérimentale et le support technique fournis.
