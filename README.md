# Questionnaire

## Mécanisme d'appel système

Les fichiers mise en jeu sont:

* syscall.h
* libnachos.h
* sys.s

pour les fonctions/méthodes:

* syscall
* ensemble des appels système (cf. syscall.h)

Voici l'ordre d'exécution d'un appel système de ce type:

1. On appel une fonction défini dans la librairie standard de nachos
2. Cette dernière va faire un appel système (défini dans syscall)
3. On initialise les registres r2, r4, r5, r6 et r7 
4. On déclenche l'exception adéquate via l'instruction syscall (fichier sys.s) et on passe en mode noyau

## Gestion de threads et de processus

1. Ce qui est sauvegardé lors d'un changement de contexte sont les registres du processeurs MIPS (Le contexte d'execution du processeur).
2. La variable qui gère la liste des processus prêt est readyList. Cependant le thread qui est actif sur le processeur n'est pas de cette liste (mais il est accessible via le pointeur *g_current_thread*):

> Le thread élu (qui s’exécute sur le processeur) ne fait pas partie de la file des prêts.

3.
4. Les routines de gestion de listes ne soucient pas de l'allocation et libération de la mémoire mais il gère l'allocation et la désallocation des objets chainés (Car...)

> Les objets
instances de la classe List sont les chaînons (classe ListElement), assurant la mise en liste des
éléments sans se soucier de l’allocation et libération de la mémoire qu’ils occupent (un objet
de la classe ListElement contient un pointeur sur l’élément mis en liste)

5. Un objet thread est placé dans l’ensemble d’attente associée de la condition
cond.

> int CondWait(CondId cond) : met dans l’ensemble d’attente associée de la condition
cond le thread courant. Renvoie 0 si la condition est valide.

6.
7.
8. La variable membre *type* sert à identifier un objet lors d'un appel systeme.

>Les différents objets manipulés par Nachos sont identifiées par un type, codé par un
entier (voir fichier kernel/system.h, type ObjetTypeId). Ce type est utilisé dans chaque appel
système pour vérifier qu’il est bien appliqué sur le bon type d’objet (les paramètres des appels
systèmes sont transmis dans des registres, donc ne peuvent pas être typés).

## Environnement de développement

1.
2. On peut utiliser un outil tel que gdb pour mettre au point un noyau comme Nachos.

>  les outils de mise au point classiques (par exemple gdb, cf annexe A) peuvent être utilisés.

3. Oui. Cela pour la simple raison que Nachos est un noyau émulé. ayant accès à gdb et aux "librairies" mises en jeu par les programmes utilisateurs, nous avont donc aucune restriction. + manipulation possible des fichiers *.elf* par gdb
