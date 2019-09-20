class Region{

    int tailleBuffer; // Taille de la partie du buffer associée à cette région
    int startBuffer; // Indice à partir duquel cette région est présente dans le buffer global
    Point a,b; // Bounding box de la région
    bool visible; // Visibilité de la région

public:

    // Matières associées à la région
    // Est de taille NB_MATIERES-1, chaque entier représente l'indice dans le buffer à partir
    // duquel on passe à une autre matière

    // ex:  mat[3] tailleBuffer = 20
    //      mat[0] = 5  >   0  à 5  matiere 1
    //      mat[1] = 7  >   5  à 7  matiere 2
    //      mat[2] = 12 >   7  à 12 matiere 3
    //               /  >   12 à 20 matiere 4
    int* mat;

    Region(int nbMat, Point pmi, Point pma):a(pmi),b(pma),visible(true){
        mat = new int[nbMat-1];
    };

    void setStartR(int start){ startBuffer = start; }
    void setTailleR(int taille){ tailleBuffer = taille; }
    void setVisible(bool vis){ visible = vis; }

    int getStartR(){ return startBuffer; }
    int getTailleR(){ return tailleBuffer; }

    Point getPmin(){ return a; }
    Point getPmax(){ return b; }

    bool isVisible(){ return visible; }
};
