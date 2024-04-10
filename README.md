[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/pM2OQhWF)
TP4: FAT 32 
=============

Introduction 
------------

Le système de fichier FAT aurait été dévelopé par Bill Gates lui-même
sur une napkin dans un hôtel. C'est un système de fichier simple,
robuste et relativement compact. La première implémentation visait
l'utilisation sur des disquettes, mais son adoption générale en fait un
système de fichier encore compatible avec beaucoup de matériel moderne.
Bien que les besoins de performances associées à des utilisations
commerciales des systèmes de fichiers rendent FAT32 obsolète (il ne
permet pas un accès très rapide aux fichiers), il reste important de nos
jours et son étude est encore très pertinente.

Instructions générales 
----------------------

Afin de tester votre implémentation, un fichier **floppy.img** est
fourni. L'arborescence de ce fichier est donnée en fin de l'énoncé. Ce
fichier peut être accédé par les mécanismes standards de lecture de
fichier en C. Cependant, la structure de ce fichier correspond
**exactement** à ce que vous retrouveriez sur un disque dur (ou une
_floppy_).

Si vous avez besoin de ressources supplémentaires,
voici quelques liens qui pourraient vous être utiles:

1.  [La documentation officielle de
    FAT32](https://www.cs.fsu.edu/~cop4610t/assignments/project3/spec/fatspec.pdf)

2.  <https://github.com/IFT2245/FAT32>

Afin de simplifier ce travail, vous n'avez pas à vous soucier du
partitionnement du disque représenté par l'image. En effet, il y aura
toujours une partition qui occupe l'entièreté du disque. De plus,
l'image donnée ainsi que celles utilisées pour les tests correspondront
toujours à de vraies images FAT32 correctes. Cependant, vous ne pouvez
pas assumer que la géométrie de l'image sera toujours la même. Par
exemple, un *cluster* pourrait contenir plus de secteurs pour un disque
qu'un autre. De plus, les secteurs ne seront **pas** toujours 512 bytes.

Vous pouvez aussi assumer que le système de fichier n'est pas corrompu
et donc que la première table FAT est valide.

Avant de commencer à regarder les fonctions à implémenter, prenez
connaissance du squelette de code donné ainsi que des déclarations qui
sont faites.

L'ordre des fonctions à coder peut sembler arbitraire. Cependant, il est
conçu pour vous forcer à avoir des questions en premier et ainsi éviter
que vous devez refaire des sections au complet parce que vous avez mal
compris quelque chose. Cependant, vous pouvez faire les questions dans
l'ordre que vous le voulez. La question 5 pourrait être faite en premier
afin de tester les autres fonctions. Cependant, comme dans la vraie vie,
il faut parfois faire une quantitée de code non-triviale afin de pouvoir
tester des choses plus petites. Cela fait partie de la difficulté de ce
TP.

Objectifs d'apprentissage 
-------------------------

Ce TP vise à vous faire découvrire les "joies" du dévelopement de support
pour un système de fichier. En effet, vous aurez à implémenter une très
petite partie de ce qui est nécéssaire pour accéder à des fichier sur un
système de fichier FAT32. Comme vous verrez, il s'agit d'une tâche non
triviale, même pour un système de fichier aussi simple que FAT32.

Voici une liste non-exhaustive des éléments pédagogiques que vous
retrouverez dans ce TP

-   Lecture et compréhension de documents techniques

-   Principes généraux des systèmes de fichiers

-   Implémentation de code résistant aux erreurs

Gestion des erreurs 
-------------------

Plusieurs erreurs peuvent se produire lors de la lecture d'un disque.
Des outils sont mis à votre disposition dans l'entête du fichier pour
traiter les erreurs. Vous devez respecter le fait qu'une erreur possède
un code d'erreur négatif mais vous pouvez ajouter des codes d'erreurs et
redéfinir les codes à votre guise. Parfois, le code d'erreur est utilisé
pour retourner une donnée sur l'exécution de la fonction. Portez
attention à la description des fonctions dans le code ainsi qu'à ce
document pour faire la bonne chose. Portez attention aux arguments des
fonctions. Des arguments nuls sont une condition suffisante pour
provoquer une erreur lors de l'appel d'une fonctions.

Fonctions à implémenter 
-----------------------

### 1- Cluster vers LBA 

La première fonction que vous aurez à implémenter est la suivante:

    uint32 cluster_to_lba(BPB *block,
                          uint32 cluster,
                          uint32 first_data_sector);

Cette fonction prend un numéro de cluster et retourne le LBA (logical
block address) qui correspond au secteur à lire. La conversion doit
s'effectuer selon la spécification pour que la lecture de fichier soit
correcte. De plus, le BPB doit être utilisé pour tenir compte de la
géométrie du disque. La formule de conversion est donné verbatim dans
notre document.

### 2- Aller chercher le prochain maillon de la chaîne FAT 

    error_code get_cluster_chain_value(BPB *block,
                                       uint32 cluster,
                                       uint32 *value,
                                       FILE *archive);

Cette fonction prend en paramètre (entre autre) un numéro de cluster.
Cette fonction doit placer dans le pointeur value le cluster qui suit le
cluster courant. Cette information se trouve dans la table FAT du
disque. Portez attention à bien calculer l'adresse en byte du cluster
courant. Si vous faites une erreur, le mauvais cluster sera lu et la
lecture de fichier sera impossible. Il n'est pas nécéssaire de vérifier
l'information avec la deuxième table FAT, seule la première suffira dans
ce TP.

### 3- Vérifier le nom d'une entrée 

    bool file_has_name(FAT_entry *entry, char *name)

Le but de cette fonction est de comparer un nom de fichier ou de dossier
FAT avec un nom de fichier entré par un utilisateur. Attention! Il ne
s'agit **pas** d'une simple comparaison de chaînes de caractères.
Veuillez vous fier à la spécification officielle (qui est **très**
claire à ce sujet) pour comparer les noms. Vous n'avez pas
nécéssairement besoin de gérer tous les cas spéciaux, mais cela pourrait
être utile dans d'autres fonctions.

### 4- Analyser un chemin 

    error_code break_up_path(char *path, uint8 level, char **output);

Cette fonction va vous servir à prendre un chemin dans l'archive et le
séparer en morceau. La spécification exacte de la fonction est donnée en
docstring au dessus de celle-ci. Cette fonction ne requiert pas de
connaissances particulières du système de fichier FAT32. Il faut bien
sûr allouer une chaîne de caractère pour y placer le morceau du chemin
voulu. De plus, un indicateur de chemin relatif (ici \".\", \"..\")
constitue un morceau de chemin valide, puisque ces marqueurs sont
construits comme des fichiers dans FAT32 (on laisse donc ces morceaux
dans la chaîne). Le erreur code, en cas ou il n'y a pas d'erreur,
retourne la longueur de ce tableau.

### 5- Aller lire le *boot block* 

    error_code read_boot_block(FILE *archive, BPB **block);

Cette fonction va lire le boot block dans l'archive et va allouer la
mémoire nécéssaire pour le placer. Cette fonction est essentielle pour
le bon fonctionnement du reste du TP, puisque le boot block contient
toute l'information nécéssaire pour la lecture de l'archive. En
particulier, le BPB donne des informations quant à la géométrie du
disque.

### 6- Trouver un descripteur de fichier 

    error_code find_file_descriptor(FILE *archive,
                                    BPB *block,
                                    char *path,
                                    FAT_entry **entry);

Cette fonction reçoit en entrée un path qui correspond au chemin du
fichier à aller chercher. Cette fonction doit donc rechercher le fichier
au travers l'arborescence de dossiers. La fonction doit être capable de
déterminer que le chemin n'est pas valide. Par exemple, un chemin qui
utilise un fichier comme un dossier ne devrait pas fonctionner.

### 7- Aller lire le contenu d'un fichier 

    error_code read_file(FILE *archive,
                         BPB *block,
                         FAT_entry *entry,
                         void *buff,
                         size_t max_len);

Cette fonction prend en paramètre une entrée FAT ainsi qu'un buffer et
sa longueur maximale. Si le descripteur décrit bien un fichier, le
contenu du fichier est lu au complet (à moins de dépasser la longueur
maximale, dans ce cas, il est partiellement lu) et placé dans le buffer.
Le code d'erreur doit retourner le nombre de bytes lu s'il n'y a pas
d'erreur. Si le fichier n'est pas dans l'arborescence, vous devez
retourner un code d'erreur.

Le chemin pourrait contenir des éléments de chemin relatif (par exemple,
des \"..\"). Afin de minimiser le travaille à faire, je vous conseille
**fortemment** de lire la spécification FAT32 au sujet de la structure
des dossiers, qui explique comment les relations entre ceux-ci sont
modélisées dans le système de fichier FAT32.

Contenu de l'image 
------------------

Si vous utilisez linux, vous pouvez *monter* l'image pour modifier son
contenu. Cela vous permet d'ajouter des fichiers, et de changer
l'architecture de l'image. La commande s'effectue ainsi:

``` {.sh language="sh"}
mount floppy.img mntpnt 
umount mntpnt
```

La commande *mount* permet de mapper l'archive à un dossier, ce qui
permet d'opérer dessus comme si c'était un disque. La commande *umount*
permet de défaire l'opération et de continuer à utiliser l'archive.

**Attention!**
La commande *mount* ne fonctionne pas sur le WSLV1. Elle ne fonctionne pas 
sur Windows et sur MacOS le fonctionnement est peut-être différent. Être capable
de monter l'archive n'est **pas** nécéssaire, mais peut être un outil utile pour
débugger. Le contenu du disque est décrit dans la prochaine section, ce qui devrait
vous aider à ne pas avoir besoin de le monter l'image.
**Attention!**

Si vous modifier l'archive, portez attention à entrer des noms de
fichiers en majuscules, de taille inférieure ou égale à 11, incluant
l'extention. Sinon, votre système créera des fichiers en utilisant
l'extention *long file name* à FAT32, ce qui rendra les noms de fichiers
différents de ce que vous vous attendez. L'archive donnée tiens compte
de cette particularité.

L'image **floppy.img** contient le système de fichier suivant:

    ----AFOLDER
    |    |---ANOTHER
    |       |-- CANDIDE.TXT
    |---SPANISH
    |   |---LOS.TXT
    |   |---TITAN.TXT
    ----ZOLA.TXT
    ----HELLO.TXT

Cette représentation à été obtenue en appelant la commande *tree* à la
racine du dossier de l'archive. Les fichiers sont des livres du Projet
Gutenberg. Leur longueur dépend des fichiers, vous pouvez les monter
pour aller les lire. Le fichier HELLO.TXT contient le texte \"Bonne
chance pour le TP4\". Celui-ci est plus petit et est contenu dans un
cluster ce qui devrait faciliter la tâche au départ.

Remise 
------

Chaque jour de retard est -15%, mais après le deuxième jour la remise ne
sera pas acceptée.

Indiquez clairement votre nom et votre matricule dans un commentaire en
bloc au début de chaque fichier, dans ce format:

        Prenom1 NomDeFamille1 Matricule1

Vous pouvez valider l'entête de votre fichier en utilisant le projet
situé ici: <https://github.com/IFT2245/NameValidator>. Une entête
invalide se méritera la note de 0.


Évaluation du TP 
----------------

Nous évaluerons votre TP principalement grâce à la fonction read\_file.
Si le contenu d'un fichier est lu correctement, le test passera.

Barème 
------

-   Votre note sera divisé équitablement entre chaque question, sauf les
    questions 6 et 7 qui ont pour valeur l'équivalent de 2 questions.
- Les barèmes standards du [TPX](https://github.com/IFT2245/TPX) s'appliquent (fuites mémoires, accès illégaux, etc).
