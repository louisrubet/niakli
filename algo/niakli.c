#include <stdio.h>

/*
Règle 1 Regrouper chaque couleur
Règle 2 Deux blocs séparés par une couleur peuvent se regrouper
Règle 3 Pour déplacer plusieurs blocs, faites glisser au niveau du bloc qui traversera la couleur
Règle 4 Un groupe peut se former tant qu'il ne détruit pas des groupes plus petits
*/

/*
0. Numérotation des formes, 1 numéro par forme connexe [algo récursif]
    - une forme stocke
            - une couleur
            - 4 déplacements absolus max (+x,-x,+y,-y) valables pour toutes les cases

1. Préparer le jeu
    - Déterminer les cases jouables et les directions jouables [algo edge]
    - Pour chaque case
        - déterminer s'il est possible de jouer
            - si réponse = ok alors -> point 2
            - si réponse = ko alors (animation vibration)
2. Jouer
    - Dupliquer le tableau de cases
    - Jouer à la position
    - Détermination des nouvelles formes
    - Si nouvelles formes brisent la règle 4 -> (animation groupes)
    - Calcul du nouveau score
        - Enregistrer
    - Préparer le jeu -> point 1
*/

int main(int argc, char* argv[])
{
    
    return 0;
}
