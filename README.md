# Questionnaire

## Mécanisme d'appel système

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
