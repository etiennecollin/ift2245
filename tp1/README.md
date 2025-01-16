[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/iLuIOQUp)

# TP1: Shell

## Table des matières
* [!!ATTENTION!!](#attention)
* [Build, Exécution et Tests](#build-exécution-et-tests)
  * [CLion](#clion)
  * [En ligne de commande](#en-ligne-de-commande)
* [Introduction](#introduction)
* [Implémentation](#implémentation)
  * [Exit](#exit)
  * [Tokenizer](#tokenizer)
  * [1. Parser](#1-parser)
  * [2. Shell](#2-shell)
* [Barème et remise](#barème-et-remise)

## !!ATTENTION!!

- **Avant de commencer quoi que ce soit, ajoutez vos noms et matricules dans 
`names.txt`! Vérifiez avec `name_validator.py` que le format est ok!**

- Ne modifiez rien dans le dossier `test`! Si nous détectons une modification, vous aurez la note de 0.
 **FAITES BIEN ATTENTION QUE LE AUTOGRADER DE GITHUB ROULE**

- **N'AJOUTEZ PAS DE INCLUDES DANS VOTRE CODE, SAUF AVEC PERMISSION!**

- **N'UTILISEZ PAS DE FONCTIONS DE LA BIBLIOTHÈQUE STANDARD QUI FERAIENT 
LE TRAVAIL POUR VOUS. VOUS AUREZ LA NOTE DE 0.**

- **SI VOTRE SHELL NE SUPPORTE PAS LA COMMANDE `exit`, VOUS AUREZ AUTOMATIQUEMENT 0.**

- **Si vous utilisez CLion, ouvrer le projet au top level CMakeLists.txt, c'est-à-dire
le répertoire qui contient ce README.md**

## Build, Exécution et Tests

Ce TP est un projet **CMake**. Nous vous recommandons d'utiliser CLion pour le faire, mais vous pouvez
aussi le faire en ligne de commande.

### CLion

#### Ouverture du projet (Détection de CMake automatique)
- Ouvrez le projet au top level CMakeLists.txt, c'est-à-dire le répertoire qui contient ce README.md
- Cmake devrait être détecté automatiquement, si ce n'est pas le cas, vous avez probablement ouvert le mauvais dossier.

#### Build & Exécution
- Assurez-vous que `shell` est sélectionné dans le menu en haut à droite (Configuration CMake).
- Cliquez sur l'icône vert *run* à côté de `shell` dans le menu en haut à droite.

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
./src/shell
```

#### Tests (Autograding)

```sh
cd votre-dossier-de-build
ctest --verbose
```


## Introduction

Le shell est une composante essentielle de tout système Unix. Il est un point
de contact important entre l'utilisateur et le système d'exploitation.

Dans le shell ligne de commande, l'utilisateur peut interagir avec le système
d'exploitation en utilisant des commandes textuelles. Ces commandes sont
interprétées par un programme appelé "interpréteur de commandes" (ou shell).

Ce TP vise à vous familiariser avec la programmation système dans un
système d’exploitation de style POSIX.

Vous devrez implémenter un terminal/shell typique Linux. Il devra pouvoir 
appeler des commandes linux typiques (comme `ls` ou `cat`). Il devra aussi 
implémenter un certain nombre d'opérateurs de contrôle de flux (comme `;`, `&&`, `||`, `|`).

Le programme ce divise en 3 parties, dont vous avec besoin d'implémenter 2:

- [**tokenizer**](https://en.wikipedia.org/wiki/Lexical_analysis): Cette partie est déjà implémentée pour vous. Elle permet de
transformer une chaîne de caractères en une liste de tokens. Vous n'avez pas besoin de la modifier.
- [**parser**](https://en.wikipedia.org/wiki/Parsing): Cette partie n'est pas implémenté pour vous. Elle permet de
transformer une liste de tokens en une liste de commandes. Vous devez compléter cette partie.
- [**shell**](https://www.gnu.org/software/bash/manual/bash.html): Cette partie n'est pas implémentée pour vous.
Elle permet d'exécuter les commandes. Vous devez implémenter cette partie.

Voici les fonctionnalités que votre shell doit supporter:

| **Symbole** | **Nom**      | **Description**                                                                     |
|-------------|--------------|-------------------------------------------------------------------------------------|
| `;`         | *séparateur* | Exécute la commande suivante après la commande précédente.                          |
| `\|`        | *pipe*       | Redirige la sortie de la commande précédente vers l'entrée de la commande suivante. |
| `&&`        | *and / et*   | Exécute la commande suivante si la commande précédente a réussi.                    |
| `\|\|`      | *or / ou*    | Exécute la commande suivante si la commande précédente a échoué.                    |

A la fin de ce TP, vous devriez être capable de lancer des commandes comme
celles-ci (Essayez-les dans votre terminal pour voir ce qu'elles font !) :

```sh
echo "Hello, world!" ; ls -l | wc -c && invalid_command || echo "Goodbye, world!"
```

## Implémentation

### Exit

Avant de faire quoi que ce soit, prenez note de la fonctionnalité "exit" de votre shell.
Si l'utilisateur entre `exit` dans votre shell, votre shell DOIT sortir, avec le code de sortie `0`.

Nous avons inclus une implémentation de `exit` dans votre code. Vous devez vous assurer que
cette fonctionnalité ne brise pas avec le code que vous allez ajouter.

### Tokenizer

La partie tokenizer est déjà implémentée pour vous dans `tokenizer.h` et `tokenizer.c`.
Vous n'avez pas besoin de la modifier. Cependant, il peut être utile de comprendre comment 
elle fonctionne pour implémenter la partie parser.

Un token est une unité lexicale, c'est-à-dire un élément de base d'un langage. Le tokenizer 
permet de transformer une chaîne de caractères en une liste de tokens.

Les tokens sont définis dans `tokenizer.h`. Voici une description de chaque token :

| **Token**            | **Description**                                                     |
|----------------------|---------------------------------------------------------------------|
| `TOK_INVALID`        | Token invalide                                                      |
| `TOK_SYMBOL`         | Symbole normal (comme `echo`, `ls`, `a`, `1`)                       |
| `TOK_STRING_LITERAL` | Chaîne de caractères (comme `"Hello, world!"` ou `'Hello, world!'`) |
| `TOK_SEMICOLON`      | `;` (séparateur)                                                    |
| `TOK_PIPE`           | `\|` (pipe)                                                         |
| `TOK_AND`            | `&&` (et)                                                           |
| `TOK_OR`             | `\|\|` (ou)                                                         |

**Exemple**

La commande `echo hello ", world!" | cat` est représentée par les tokens suivants 
(vous pouvez utiliser `tok_debug_print` pour imprimer les tokens) :

```txt
TOK_SYMBOL("echo")
TOK_SYMBOL("hello")
TOK_STRING_LITERAL(", world!")
TOK_PIPE
TOK_SYMBOL("cat")
```

### 1. Parser

Un parser transforme une liste de tokens en un arbre de syntaxe abstraite (AST) qui 
représente la structure du programme. Dans notre cas, l'arbre de syntaxe abstraite 
est tout simplement une liste de commandes.

Vous devez implémenter la fonction `cmd_parse` et `cmd_free` dans `parser.c`. La fonction 
`cmd_parse` prend en entrée une liste de tokens et retourne une liste de commandes. La
fonction `cmd_free` libère la mémoire allouée par `cmd_parse`.

Un commande, défini dans `parser.h`, est tous simplement une liste chaînée qui
contient la liste d'arguments (chaînes de caractères) de chaque commande et l'opérateur
qui la suit.

Les listes d'arguments sont des listes chaînées de chaînes de caractères qui sont
null-terminées, c'est-à-dire que le dernier élément de la liste est `NULL`.

| **Opérateur**   | **Description**              |
|-----------------|------------------------------|
| `OP_TERMINATOR` | Fin de la liste de commandes |
| `OP_SEPARATOR`  | `;` (séparateur)             |
| `OP_AND`        | `&&` (et)                    |
| `OP_OR`         | `\|\|` (ou)                  |
| `OP_PIPE`       | `\|` (pipe)                  |

**Exemple**

La commande `echo hello ", world!" | cat` est représentée par les commandes suivantes 
(vous pouvez utiliser `cmd_debug_print` pour imprimer les commandes) :

```txt
echo hello , world! OP_PIPE
cat OP_TERMINATOR
```

Voici un exemple de code qui permet de construire cette liste de commandes :

```c
command *cmd = malloc(sizeof(command));
cmd->args = malloc(sizeof(char *) * 4);
cmd->args[0] = "echo";
cmd->args[1] = "hello";
cmd->args[2] = ", world!";
cmd->args[3] = NULL;
cmd->op = OP_PIPE;
cmd->next = malloc(sizeof(command));
cmd->next->args = malloc(sizeof(char *) * 2);
cmd->next->args[0] = "cat";
cmd->next->args[1] = NULL;
cmd->next->next = NULL;
cmd->next->op = OP_TERMINATOR;
```

### 2. Shell

Vous devez implémenter la fonction `sh_run` dans `shell.c`. Cette
fonction prend en entrée une liste de commandes et exécute les commandes.

Vous devez absolument utiliser les fonctions `fork`, `execvp`, `waitpid`, `pipe`
pour exécuter les commandes. Il sera probablement nécessaire d'utiliser `dup2` pour
rediriger les entrées et sorties des commandes (pipe).

Cette fonction est compliquée, alors nous vous recommandons de la diviser en plusieurs
étapes. Il est important de bien comprendre les fonctionnalités si dessous avant de commencer.

#### 2.1. Exécution de commandes



Pour exécuter une commande, vous devez créer un nouveau processus et en suite exécuter le
programme correspondant à la commande. 

Si la commanda à échouer. Vous devez alors imprimer le message d'erreur
suivant :

```c
printf("%s: command not found\n", cmd->args[0]);
```

#### 2.2 Opérateur `;`

L'opérateur `;` permet d'exécuter une commande après une autre. Par exemple, la commande
`echo "Hello, world!" ; ls -l` exécute `echo "Hello, world!"` puis `ls -l`.

Pour implémenter cet opérateur, vous devez exécuter la première commande, puis la deuxième
commande. Vous devez attendre que la première commande termine avant d'exécuter la deuxième
commande.

#### 2.3 Opérateurs `&&` et `||`

Les opérateurs `&&` et `||` permettent d'exécuter une commande conditionnellement. Par exemple,
la commande `echo "Hello, world!" && ls -l` exécute `echo "Hello, world!"` puis `ls -l` si
`echo "Hello, world!"` a réussi. La commande `bloop || ls -l` exécute `bloop`
puis `ls -l` si `bloop` a échoué.

#### 2.4 Opérateur `|`

L'implémentation de l'opérateur `|` est plus compliquée. Cet opérateur permet de rediriger la
sortie d'une commande vers l'entrée d'une autre commande. C'est une forme de communication
entre les processus (IPC). Par exemple, la commande `echo "Hello, world!" | cat` exécute 
`echo "Hello, world!"` puis `cat` avec la sortie de `echo "Hello, world!"` comme entrée.

Pour implémenter cet opérateur, vous devez créer un pipe avec la fonction `pipe`. Vous devez
ensuite exécuter la première commande et rediriger sa sortie vers le pipe. Vous devez ensuite
exécuter la deuxième commande et rediriger son entrée vers le pipe.

Voici un exemple de code qui crée un pipe :

```c
int pipefd[2]; // pipefd[0] est l'entrée du pipe, pipefd[1] est la sortie du pipe
pipe(pipefd);
```

Voici un exemple de code qui redirige la sortie d'un processus vers un pipe :

```c
dup2(pipefd[1], STDOUT_FILENO); // Redirige la sortie vers le pipe
```

Voici un exemple de code qui redirige l'entrée d'un processus vers un pipe :

```c
dup2(pipefd[0], STDIN_FILENO); // Redirige l'entrée vers le pipe
```

## Barème et remise

- Votre dernière version doit être sur GitHub à la date de remise. Chaque jour de retard enlève 15%,
après deux jours, la remise ne sera pas acceptée.
- Votre note sera divisé équitablement entre chaque opérateur (séparateur, and, or, pipe).
- Ce TP sera corrigé par des tests automatisés. **Une partie des tests est fournie**, cependant,
vous devez vous assurer que votre code généralise bien à d'autres cas vu que nous allons
ajouter des **tests cachés**.
- Si vous ne respectez pas les consignes, vous aurez la **note de 0**.
- Vous pouvez valider votre code avec les tests fournis en regardant le GitHub Actions de votre
projet. Vous pouvez aussi les exécuter manuellement avec CMake et CTest.
- Vous serez **pénalisé pour chaque bug : fuite mémoire, accès illégal, etc**. Nous allons utiliser
**Valgrind** pour vérifier votre code. Nous recommandons fortement d'utiliser Valgrind, et 
possiblement d'autres outils, pour vérifier votre code avant de le soumettre.
