[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/gPYRZTPa)

# TP2: Ordonnancement 

## Table des matières
* [!!ATTENTION!!](#attention)
* [Build, Exécution et Tests](#build-exécution-et-tests)
  * [CLion](#clion)
  * [En ligne de commande](#en-ligne-de-commande)
* [Introduction](#introduction)
* [Structure d'un processus](#structure-dun-processus)
* [API OS](#api-os)
* [Implémentation](#implémentation)
  * [Ready Queue](#ready-queue)
  * [Worker](#worker)
* [Barème](#barème)
  * [Test Scheduler](#test-scheduler)
  * [Score Baselines](#score-baselines)
  * [Bugs Valgrind](#bugs-valgrind)
* [Remise](#remise)

## !!ATTENTION!!

- **Avant de commencer quoi que ce soit, ajoutez vos noms et matricules dans 
`names.txt`! Vérifiez avec `name_validator.py` que le format est ok!**

- Ne modifiez rien dans le dossier `test`! Si nous détectons une modification, vous aurez la note de 0.
 **FAITES BIEN ATTENTION QUE LE AUTOGRADER DE GITHUB ROULE**

- **N'AJOUTEZ PAS DE INCLUDES DANS VOTRE CODE, SAUF AVEC PERMISSION!**

- **NE MODIFIER RIEN DANS `os.h`, `os.c` et `main.c`, VOUS AUREZ LA NOTE 0!**

## Build, Exécution et Tests

Ce TP est un projet **CMake**. Nous vous recommandons d'utiliser CLion pour le faire, mais vous pouvez
aussi le faire en ligne de commande.

### CLion

#### Ouverture du projet (Détection de CMake automatique)
- Ouvrez le projet au top level CMakeLists.txt, c'est-à-dire le répertoire qui contient ce README.md
- Cmake devrait être détecté automatiquement, si ce n'est pas le cas, vous avez probablement ouvert le mauvais dossier.

#### Build & Exécution
- Allez dans edit configuration du `scheduler` (à gauche de l'icône play) et ajoutez `../../test/test_50_p.csv` dans les arguments de la commande.
- Assurez-vous que `scheduler` est sélectionné dans le menu en haut à droite (Configuration CMake).
- Cliquez sur l'icône vert *run* à côté de `scheduler` dans le menu en haut à droite.

### Tests (Autograding)

- Assurez-vous que `All CTest` est sélectionné dans le menu en haut à droite (Configuration CMake).
- Cliquez sur l'icône vert *run* à côté de `All CTest` dans le menu en haut à droite.

### En ligne de commande

#### Build

```sh
mkdir votre-dossier-de-build
cd votre-dossier-de-build
cmake ..
make
```

#### Exécution

```sh
cd votre-dossier-de-build
./src/scheduler test/test_50_p.csv
```

#### Tests (Autograding)

```sh
cd votre-dossier-de-build
ctest --verbose
```

## Introduction

Nous avons vu dans le cours que des algorithmes d'ordonnancement comme **Shortest Job First (SJF)**,
sont très efficaces pour minimiser le **turnaround time**. Cependant, ces algorithmes ne sont pas
idéaux pour les applications en temps réel, car ils ne garantissent pas un temps de réponse rapide.
Un algorithme comme **Round Robin (RR)** est plus approprié pour les applications en temps réel,
parce qu'il garantit un temps de réponse rapide, néanmoins, il n'est pas optimal **turnaround time**.
En fait, il n'existe pas d'algorithme d'ordonnancement qui est optimal pour les deux.

**Notre mission est claire** : créer un ordonnanceur de système d'exploitation qui privilégie à 
la fois le `turnaround time` et le `response time`, garantissant que les processus soient réactifs
dans des scénarios informatiques en temps réel.

Pour ce faire, nous allons essayer de développer un ordonnanceur qui minimise à la fois le `turnaround time`
et le `response time`. Nous allons utiliser une formule simple pour calculer le score d'un ordonnanceur :

```c
score = (wait_time + response_time * 2) / burst_time
```

Avec cette formule, nous pouvons mesurer la performance de notre ordonnanceur pour nos besoins. Si le
`score` est plus bas, c'est mieux, le meilleur score possible est 0, ce qui signifie que tous les
processus ont été exécutés immédiatement et sans interruption.

L'implémentation de votre ordonnanceur est libre à vous, mais il est recommandé de commencer
avec des baselines solides vu en classe comme :

- **First Come First Serve (FCFS)**: Il est simple et facile à implémenter.
- **Shortest Job First (SJF)**: Pour minimiser le `turnaround time`. (Attention, burst time inconnu)
- **Round Robin (RR)**: Pour garantir un temps de réponse rapide.

Vous pouvez ensuite essayer d'implémenter des algorithmes plus complexes comme :
- **Ticket-based scheduling**
- **Multilevel Feedback Queue (MLFQ)**
- **Highest Response Ratio Next (HRRN)**
- **Completely Fair Scheduler (CFS)**
- **Earliest Eligible Virtual Deadline First (EEVDF)**
- **Votre propre algorithme**

Le programme se divise en 3 parties, dont vous avez besoin d'implémenter/d'utiliser:

- **os**: Cette partie est déjà implémentée pour vous. Elle contient un API que vous devez utiliser
pour implémenter votre ordonnanceur. Vous ne devez pas modifier cette partie, elle est utilisée pour
tester votre code.
- **process**: Contient la structure pour votre processus, elle est déjà implémentée pour vous, mais
vous devez la modifier pour ajouter des informations nécessaires pour votre ordonnanceur.
- **ready_queue**: Cette partie est à implémenter. Elle contient la structure du ready queue de
votre ordonnanceur. 
- **worker**: Cette partie est à implémenter. C'est le coeur de votre ordonnanceur.

## Structure d'un processus

Chaque processus a un nombre de bursts, un `burst_time` et un `io_time`, tous les bursts du
même processus ont le même `burst_time` et `io_time`. Le `burst_time` est le temps nécessaire
pour exécuter le processus, et le `io_time` est le temps nécessaire pour faire une opération
d'entrée/sortie. Ces valeurs sont générées aléatoirement pour chaque processus, et vous devez
les trouver pour chaque processus si vous voulez les utiliser dans votre ordonnanceur.

Les csvs dans le dossier `test` contiennent une liste de processus avec leurs `burst_time` et
`io_time`. Vous pouvez utiliser ces csvs pour tester votre ordonnanceur.

```csv
PID;ARRIVAL;BURSTS;BURST_TIME;IO_TIME
0;0;2;10;10
```

## API OS

L'API OS est déjà implémentée pour vous. Vous ne devez pas la modifier. Elle contient les fonctions
que vous devez utiliser pour implémenter votre ordonnanceur.

```c
int os_run_process(process_t *process, uint32_t core, uint64_t time_slice);
```

Cette fonction exécute un processus sur un coeur et bloque le processus jusqu'à ce qu'il termine. 
Vous pouvez spécifier un `time_slice` pour que le processus soit préempté après un certain temps si
le processus n'a pas terminé. Pour executé le processes sans interruption, jusqu'à ce qu'il termine,
vous pouvez spécifier `0` pour `time_slice`.

La fonction retourne `OS_RUN_DONE` si le processus a terminé, donc ce processus ne sera plus dans la
ready queue.

La fonction retourne `OS_RUN_PREEMPTED` si le processus a été préempté, donc ce processus doit soit
continuer à être exécuté, soit être replacé dans la ready queue.

La fonction retourne `OS_RUN_BLOCKED` si le processus a été bloqué par une opération d'entrée/sortie,
vous devez exécuter l'opération d'entrée/sortie et replacer le processus dans la ready queue. Voir
la prochaine fonction pour plus de détails.

```c
void os_start_io(process_t *process);
```

Cette fonction est appelée lorsqu'un processus qui a été bloqué par une opération d'entrée/sortie veut
effectuer l'opération d'entrée/sortie. Cette fonction **ne bloque pas**, vu que les opérations d'entrée/sortie
sont asynchrones, et n'utilisent pas de temps de CPU. Une fois que l'opération d'entrée/sortie est terminée,
le système d'exploitation remettra le processus dans la ready queue **automatiquement** pour vous.

```c
uint64_t os_time(void);
```

Cette fonction retourne le temps actuel du système d'exploitation. Vous pouvez l'utiliser pour mesurer
le temps d'exécution.

## Implémentation

### Ready Queue

La ready queue est une structure de données qui contient les processus qui sont prêts à être exécutés.
Vous devez implémenter cette structure de données dans `ready_queue.c`. 

Cette structure sera utilisée par votre ordonnanceur pour stocker les processus qui sont prêts à être
exécutés, et pour choisir le prochain processus à exécuter.

Les opérations `push` et `pop` vont être utilisées par plusieurs cœurs en même temps, donc
vous devez vous assurer que ces opérations sont **thread safe**. Et utiliser des primitives de
synchronisation comme **mutex** ainsi que **conditions variables** pour synchroniser les accès
à la ready queue.

Commencez par implémenter une ready queue simple FIFO (First In First Out), vous pouvez
ensuite essayer d'implémenter une ready queue plus complexe qui prend des décisions plus
intelligentes sur le prochain processus à exécuter.

Vous pouvez utiliser n'importe quelle structure de données que vous voulez, mais vous devez 
implémenter les fonctions suivantes :

```c
void ready_queue_init(ready_queue_t *queue);
```

Cette fonction initialise la ready queue.

```c
void ready_queue_destroy(ready_queue_t *queue);
```

Cette fonction libère les ressources allouées par la ready queue.

```c
void ready_queue_push(ready_queue_t *queue, process_t *process);
```

Cette fonction ajoute un processus à la ready queue.

Vous devez utiliser un **mutex** pour synchroniser l'accès à la ready queue, ansi qu'une
**condition variable** pour débloquer les cœurs qui attendent un processus dans la ready queue.

Vous devez aussi supporter les processus `NULL`, ceux-ci sont envoyés quand votre queue est
vide à la fin de l'exécution. Ils sont utilisés comme `poison pill` pour débloquer les cœurs
qui attendent un processus dans la ready queue et terminer l'exécution.

```c
process_t *ready_queue_pop(ready_queue_t *queue);
```

Cette fonction retire un processus de la ready queue. 

Vous devez utiliser un **mutex** pour synchroniser l'accès à la ready queue, ansi qu'une 
**condition variable** pour bloquer l'opération `pop` si la ready queue est vide.

Si le processus retourné est `NULL`, cela signifie que l'exécution est terminée, et vous devez
terminer l'exécution du cœur (`poison pill`).

```c
bool ready_queue_size(ready_queue_t *queue);
```

Cette fonction retourne la taille de la ready queue.

Vous devez utiliser un **mutex** pour synchroniser l'accès à la ready queue.

### Worker

Le worker est le cœur de votre ordonnanceur. Vous devez implémenter cette structure dans `worker.c`.

Le worker est responsable de l'exécution des processus. Il doit faire appel à l'API OS et le ready queue
pour contrôler le flux d'exécution des processus.

Chaque worker est responsable de l'exécution des processus sur un seul cœur. Un worker est une boucle
infinie, qui execute sur son propre thread, et qui termine seulement lorsque qu'un `poison pill` est
envoyé à la ready queue.

Vous devez implémenter les fonctions suivantes :

```c
worker_t* worker_create(int core, ready_queue_t *ready_queue);
```

Cette fonction crée un worker pour un cœur donné. Vous devez initialiser les ressources nécessaires
pour le worker, et lancer un thread qui exécute la fonction `worker_run`.

```c
void worker_destroy(worker_t *worker);
```

Cette fonction libère les ressources allouées par le worker.

```c
void worker_join(worker_t *worker);
```

Cette fonction attend que le thread du worker termine. Vous devez utiliser un thread `join` pour
attendre que le thread du worker termine.

```c
void *worker_run(void *arg);
```

Cette fonction est la fonction principale du worker. Elle est appelée lorsque le thread du worker est
créé. Vous devez implémenter une boucle infinie qui exécute les processus de la ready queue.

## Barème

| Section                 | Points |
|-------------------------|--------|
| Unit Tests Ready Queue  | 30     |
| Test Scheduler          | 20     |
| Simple Score Baselines  | 25     |
| Complex Score Baselines | 25     |
| Bugs Valgrind           | -10    |

### Test Scheduler

Nous allons tester que votre ordonnanceur fonctionne correctement. Il doit compléter les processus,
sans erreurs :

- Tous les processus doivent être exécutés avant un temps donné.
- L'ordre d'exécution des processus doit être correct, par exemple, si un processus doit faire du
  I/O avant d'être replacé dans la ready queue.

Si votre ordonnanceur passe ce test, vous aurez tous les points, sinon, aucun point.

### Score Baselines

Nous allons mesurer la performance de votre ordonnanceur contre nos baselines. Vous aurez une note 
basée sur la performance de votre ordonnanceur.

Nous utiliserons un nouveau généré test avec **50 processus** pour mesurer la performance de votre ordonnanceur.

Pour les baselines simples, nous allons utiliser les algorithmes d'ordonnancement vu en classe :
- **First Come First Serve (FCFS)**
- **Shortest Job First (SJF)**
- **Round Robin (RR)**

Les baselines complexes sont des algorithmes inconnus que vous devez battre pour obtenir des points :

### Bugs Valgrind

Si valgrind détecte un bug, vous perdrez des points (Correction negative), pour un maximum de -10 points.

## Remise
- Votre dernière version doit être sur GitHub à la date de remise. Chaque jour de retard enlève 15%,
  après deux jours, la remise ne sera pas acceptée.
- Si vous ne respectez pas les consignes, vous aurez la **note de 0**.
