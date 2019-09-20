***********************************************************************
*                                                                     *
*               README : Projet de Synthèse d'Image 3D                *
*                                                                     *
***********************************************************************
Monôme - Peurière Romain 11707721



Compilation:

> make (all/tevo/"")

Aide:       > make help
Clean:      > make clean
Exécution:  > ./bin/tevo



Commandes:

Mouvement de la caméra:
Clic gauche

Mouvement du centre de la caméra:
"Z/Q/S/D"

Activer/Désactiver l'affichage des régions:
Touche "l"

[Debug] Afficher les textures de réflexion/refraction de l'eau en live:
Touche "k"



Paramètres:

Il est possible de modifier le nombre de régions et leur taille ligne 24 & 25 (main.cpp)
Pour les petites configurations: il est possible de réduire la taille des textures lignes 76 79 et 82 (main.cpp)


***********************************************************************
Fichiers:

Sources:
Gkit: dossier src/gKit/
Projet: dossier src/tevo/
    - Camera: Classe de gestion de camera
    - FrameBuffer: Classe de gestion des FBO
    - Textures: Classe de gestion des textures
    - Region: Classe représentant une région du terrain
    - Terrain: Contient toutes les informations et traitements liées au terrain (heightmap)
    - Utility: Diverses fonctions génériques
    - Main: Application openGL, init & render

Données:
dossier data/
    - Images diverses (cubemap, dudvMap, normalMap, Font...)
    - obj/: .obj du cube
    - shaders/: différents shaders utilisés (dont ceux fournis: cubemap text)
    - textures/: textures de nos blocs
