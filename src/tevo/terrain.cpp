#include "terrain.h"
#include <algorithm>

// Donnees statiques pour std::sort
// Nous ne disposons que d'un terrain dans tous les cas
std::vector<double> Terrain::heights;
Vector Terrain::scale;

Terrain::Terrain(const char * image, int nb_regions, int taille_region, int matieres): nbRegions(nb_regions), tailleRegion(taille_region), nbMat(matieres){

    // Initialisation de la taille du terrain
    // Nous ne créons plus que des terrains carrés x = z
    scale.x = nbRegions * tailleRegion;
    scale.y = (20 * nbRegions * tailleRegion) / 250.f; // Rapport proportionnel correct, modifiable
    scale.z = nbRegions * tailleRegion;

    // Creations des regions
    for(int i=0; i<nbRegions*nbRegions; i++){

        int pos_x = (i%nbRegions)*tailleRegion;
        int pos_z = (i/nbRegions)*tailleRegion;
        regions.push_back(
            Region(
                nbMat,
                Point(pos_x,0,-pos_z),
                Point(pos_x+tailleRegion,scale.y,-(pos_z+tailleRegion))
            )
        );
    }

    // Lecture de l'image grayscale en entree
    Image data= read_image(image);
    int width = data.width();
    int height = data.height();

    for(double y= 0; y < (scale.z); y++){
        for(double x= 0; x < (scale.x); x++){
            // Chargement des hauteurs du terrain
            heights.push_back(data.sample(x / scale.x * width,y / scale.z * height).r * scale.y);
        }
    }

}

// Fonctions explicites

double Terrain::getHeight(int i, int j){
    return heights[index(i,j)];
}

Vector Terrain::getNormal(int i, int j){
    vec2 g = getGradient(i,j);
    return normalize(Vector(-g.x,1.,g.y));
}

vec2 Terrain::getGradient(int i, int j){
    int neighbors[8];
    getNeighbors(i,j,neighbors);

    return vec2(
        (getHeight(neighbors[2],neighbors[3]) - getHeight(neighbors[6],neighbors[7]))/2,
        (getHeight(neighbors[4],neighbors[5]) - getHeight(neighbors[0],neighbors[1]))/2
    );
}

double Terrain::getSlope(int i,int j){
    vec2 gradient = getGradient(i,j);
    return sqrt(gradient.x*gradient.x + gradient.y*gradient.y);
}

Point Terrain::getPoint(int i, int j){
    return Point(i,getHeight(i,j),-j);
}

void Terrain::fillBuffers(GLuint program){

    // Rempli les différents buffers dépendant du terrain
    int startBufferRegion = 0 , tailleBufferRegion;
    int neighbors[8];
    int nbCubeH; // Nombre de cubes en hauteur pour combler les eventuels trous

    // Nous effectuons en premier temps les traitements (tris) uniquement sur les indices des cubes dans le tableau
    // de hauteur afin de limiter le temps d'execution de la boucle
    std::vector<int> indexes_vector;

    // Pour chaque region, on rempli indexes_vector et renseigne la partie du buffer associé à chaque région
    for (int i = 0; i < nbRegions*nbRegions; i++) {

        tailleBufferRegion = tailleRegion*tailleRegion;
        // de base (minimum), modifiable avec les ajouts de nbCubeH

        for(int y= 0; y < tailleRegion; y++){
            for(int x= 0; x < tailleRegion; x++){

                // Calcul de la position x-z de chaque point
                int pos_x = (i%nbRegions)*tailleRegion;
                int pos_z = (i/nbRegions)*tailleRegion;

                // Ajout de son indice (il existe)
                indexes_vector.push_back(index(pos_x+x,pos_z+y));

                // Nous verifions les voisins
                getNeighbors(pos_x+x,pos_z+y,neighbors);
                nbCubeH =
                std::max(
                    abs(getHeight(pos_x+x,pos_z+y)-getHeight(neighbors[0],neighbors[1])),
                    std::max(
                        abs(getHeight(pos_x+x,pos_z+y)-getHeight(neighbors[2],neighbors[3])),
                        std::max(
                            abs(getHeight(pos_x+x,pos_z+y)-getHeight(neighbors[4],neighbors[5])),
                            abs(getHeight(pos_x+x,pos_z+y)-getHeight(neighbors[6],neighbors[7]))
                        )
                    )
                );

                // Si la difference est supérieure à 1, il existe un trou avec au moins un voisin
                // On ajoute donc des cubes en hauteur
                for(int cubeH=0; cubeH<nbCubeH; cubeH++){
                    indexes_vector.push_back(index(pos_x+x,pos_z+y));
                    tailleBufferRegion++; // On augmente consequemment la taille de la partie du buffer associée à la région
                }
            }
        }

        // Premier tri effectué sur les hauteurs
        std::sort(indexes_vector.begin() + startBufferRegion, indexes_vector.end(), [](int a, int b) {
            return heights[a] < heights[b];
        });

        // Par sécurité si certaines matières n'existent pas dans certaines régions
        regions[i].mat[nbMat-2] = tailleBufferRegion;
        for (int nummat = nbMat-3; nummat >= 0 ; nummat--) {
            regions[i].mat[nummat] = 0;
        }

        // Boucle d'assignation aux différentes hauteurs
        // proportionnel à la taille de la map
        for(size_t k=startBufferRegion; k<indexes_vector.size(); k++){
            // manuel et parametrable
            if(heights[indexes_vector[k]]<= (2 * nbRegions * tailleRegion) / 250.f ){ // eau
                regions[i].mat[0] = k-startBufferRegion;
            }
            if(heights[indexes_vector[k]]<= (3 * nbRegions * tailleRegion) / 250.f ){ // sable
                regions[i].mat[1] = k-startBufferRegion;
            }
            if(heights[indexes_vector[k]]<= (7 * nbRegions * tailleRegion) / 250.f ){ // herbe
                regions[i].mat[2] = k-startBufferRegion;
            }
            if(heights[indexes_vector[k]]<= (12 * nbRegions * tailleRegion) / 250.f ){ // terre
                regions[i].mat[3] = k-startBufferRegion;
            }
            if(heights[indexes_vector[k]]<= (14.5 * nbRegions * tailleRegion) / 250 ){ // roche
                regions[i].mat[4] = k-startBufferRegion;
            }
            // au-dela neige-roche
        }

        // Deuxieme tri sur les pentes du terrain entre l'herbe et la roche
        std::sort(indexes_vector.begin() + startBufferRegion + regions[i].mat[1], indexes_vector.begin() + startBufferRegion + regions[i].mat[4], [](int a, int b) {
            int j_a = a / scale.x;
            int i_a = a - j_a * scale.x;

            int j_b = b / scale.x;
            int i_b = b - j_b * scale.x;
            return ((float)getSlope(i_a,j_a) < (float)getSlope(i_b,j_b));
        });

        int stopmat2 = regions[i].mat[2];
        int stopmat3 = regions[i].mat[3];

        // On retrouve les bons indices
        for(int k=startBufferRegion+regions[i].mat[1]; k<startBufferRegion+regions[i].mat[3]; k++){

            int j_index = indexes_vector[k] / scale.x;
            int i_index = indexes_vector[k] - j_index * scale.x;

            if(getSlope(i_index,j_index) < 1.3){
                stopmat3= k - startBufferRegion;
            }

            if(getSlope(i_index,j_index)< 1.0){
                stopmat2= k - startBufferRegion;
            }
        }

        // On reaffecte les bons indices
        regions[i].mat[2] = stopmat2;
        regions[i].mat[3] = stopmat3;

        // On renseigne les données d'affichages a chaque région
        regions[i].setTailleR( tailleBufferRegion );
        regions[i].setStartR( startBufferRegion );

        // Ne disposant que d'un seul buffer, on incrémente le 'start'
        startBufferRegion += tailleBufferRegion;

    }

    // Disposant de tous les indices triés, on ajoute aux vecteurs les autres données
    std::vector<Vector> normals;
    std::vector<float> slopes;
    for (size_t k = 0; k < indexes_vector.size(); k++) {

        // On veut retrouver la position x-z à partir de l'indice

        int j = indexes_vector[k] / scale.x;
        int i = indexes_vector[k] - j * scale.x;

        // On ajoute la position
        positions.push_back( getPoint(i,j) );

        // Si plusieurs indices consécutifs sont les mêmes: on a des cubes empilés, on réduit la hauteur
        if (k > 0){ // Par sécurité
            if( indexes_vector[k] == indexes_vector[k-1] )
            positions[ k ].y = positions[ k-1 ].y - 1;
        }

        // On ajoute le reste des informations
        normals.push_back(getNormal(i,j));
        slopes.push_back((float)getSlope(i,j));
    }

    // On rempli maintenant les buffers

    // Positions des cubes
    GLuint m_instance_buffer;
    glGenBuffers(1, &m_instance_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_instance_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * positions.size(), &positions.front().x, GL_STATIC_DRAW);
    GLint index= glGetAttribLocation(program, "instance_position");
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE,0,0);
    glEnableVertexAttribArray(index);
    glVertexAttribDivisor(index, 1);

    // Normales des positions dans la heightmap
    GLuint normalmap;
    glGenBuffers(1, &normalmap);
    glBindBuffer(GL_ARRAY_BUFFER, normalmap);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector) * normals.size(), &normals.front(), GL_STATIC_DRAW);
    GLint texcoord_attribute3= glGetAttribLocation(program, "normalsmap");
    glVertexAttribPointer(texcoord_attribute3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(texcoord_attribute3);
    glVertexAttribDivisor(texcoord_attribute3, 1);

    // Pente des positions dans la heightmap
    GLuint slopemap;
    glGenBuffers(1, &slopemap);
    glBindBuffer(GL_ARRAY_BUFFER, slopemap);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * slopes.size(), &slopes.front(), GL_STATIC_DRAW);
    GLint texcoord_attribute4= glGetAttribLocation(program, "slopemap");
    glVertexAttribPointer(texcoord_attribute4, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(texcoord_attribute4);
    glVertexAttribDivisor(texcoord_attribute4, 1);

}

void Terrain::draw_by_mat(int mat, int vertex_count){
    // Fonction d'affichage du terrain par matiere

    // Pour une matiere mat
    for(size_t j=0; j<regions.size(); j++){
        // Pour chaque region, si elle est visible
        if(!regions[j].isVisible() ) continue;

        // On recupere le nombres de cubes à afficher
        // Et l'indice du début du buffer
        int nbCubesMat,startCubesMat;
        if(mat==0){
            nbCubesMat = regions[j].mat[0];
            startCubesMat = regions[j].getStartR() + 0;
        }else if(mat==nbMat-1){ // derniere > pas de borne mat[2] == tailleBufferRegion
            nbCubesMat = regions[j].getTailleR() - regions[j].mat[mat-1];
            startCubesMat = regions[j].getStartR() + regions[j].mat[mat-1];
        }else{
            nbCubesMat = regions[j].mat[mat] - regions[j].mat[mat-1];
            startCubesMat = regions[j].getStartR() + regions[j].mat[mat-1];
        }

        // DRaw
        glDrawArraysInstancedBaseInstance(
            GL_TRIANGLES,
            0,
            vertex_count, // 6 (2*3sommets par face > 2 triangles) * 6faces
            nbCubesMat,
            startCubesMat
        );
    }
}


void Terrain::performVisibilityTest(Transform mvpMatrix){
    for(size_t j=0; j<regions.size(); j++){
        // A chaque frame on cherche quelle region est visible
        // Algo des boites englobantes > utility.cpp
        regions[j].setVisible( visible(mvpMatrix,regions[j].getPmin(), regions[j].getPmax()) );
    }
}

void Terrain::getNeighbors(int i, int j, int neighbors[8]){
    // Recherche les voisins d'un point
    // Resultats dans neighbors dans le sens de la montre, celui du haut en premier
    if(i==0){
        if(j==0){ // coin haut gauche
            neighbors[0] = i;
            neighbors[1] = j;
            neighbors[2] = i+1;
            neighbors[3] = j;
            neighbors[4] = i;
            neighbors[5] = j+1;
            neighbors[6] = i;
            neighbors[7] = j;
        }else if(j<scale.z-1){ // barre gauche
            neighbors[0] = i;
            neighbors[1] = j-1;
            neighbors[2] = i+1;
            neighbors[3] = j;
            neighbors[4] = i;
            neighbors[5] = j+1;
            neighbors[6] = i;
            neighbors[7] = j;
        }else{ // coin bas gauche
            neighbors[0] = i;
            neighbors[1] = j-1;
            neighbors[2] = i+1;
            neighbors[3] = j;
            neighbors[4] = i;
            neighbors[5] = j;
            neighbors[6] = i;
            neighbors[7] = j;
        }
    }else if(i<scale.x-1){
        if(j==0){ // barre haut
            neighbors[0] = i;
            neighbors[1] = j;
            neighbors[2] = i+1;
            neighbors[3] = j;
            neighbors[4] = i;
            neighbors[5] = j+1;
            neighbors[6] = i-1;
            neighbors[7] = j;
        }else if(j<scale.z-1){ // milieu
            neighbors[0] = i;
            neighbors[1] = j-1;
            neighbors[2] = i+1;
            neighbors[3] = j;
            neighbors[4] = i;
            neighbors[5] = j+1;
            neighbors[6] = i-1;
            neighbors[7] = j;
        }else{ // bare bas
            neighbors[0] = i;
            neighbors[1] = j-1;
            neighbors[2] = i+1;
            neighbors[3] = j;
            neighbors[4] = i;
            neighbors[5] = j;
            neighbors[6] = i-1;
            neighbors[7] = j;
        }
    }else{
        if(j==0){ // coin haut droite
            neighbors[0] = i;
            neighbors[1] = j;
            neighbors[2] = i;
            neighbors[3] = j;
            neighbors[4] = i;
            neighbors[5] = j+1;
            neighbors[6] = i-1;
            neighbors[7] = j;
        }else if(j<scale.z-1){ // barre droite
            neighbors[0] = i;
            neighbors[1] = j-1;
            neighbors[2] = i;
            neighbors[3] = j;
            neighbors[4] = i;
            neighbors[5] = j+1;
            neighbors[6] = i-1;
            neighbors[7] = j;
        }else{ // coin bas droit
            neighbors[0] = i-1;
            neighbors[1] = j;
            neighbors[2] = i;
            neighbors[3] = j;
            neighbors[4] = i;
            neighbors[5] = j;
            neighbors[6] = i-1;
            neighbors[7] = j;
        }
    }
}
