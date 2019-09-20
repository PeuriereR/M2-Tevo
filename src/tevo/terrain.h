#include "wavefront.h"
#include "texture.h"

#include "orbiter.h"
#include "draw.h"
#include "app.h"        // classe Application a deriver
#include "program.h"
#include "uniforms.h"
#include "region.h"
#include <vector>
#include <math.h>
#include "utility.h"

class Terrain{

    std::vector<Region> regions;

    static Vector scale; // Taille de la map
    static std::vector<double> heights; // Hauteurs de la map
    std::vector<Point> positions; // Positions des cubes

    int nbRegions;
    int tailleRegion;
    int nbMat;


public:
    Terrain(){};
    Terrain(const char* image, int nb_regions, int taille_region, int nbMat);

    static inline int index(int i, int j){   return j * scale.x + i ;   }

    static double getHeight(int i, int j);
    Vector getNormal(int i, int j);
    static vec2 getGradient(int i, int j);
    static double getSlope(int i,int j);
    Point getPoint(int i, int j);
    inline int getCubesNumber(){ return positions.size(); };
    static void getNeighbors(int i, int j, int neighbors[8]);

    void draw_by_mat(int mat, int vertex_count);

    // Effectue tout le traitement des données
    void fillBuffers(GLuint program);
    void performVisibilityTest(Transform mvpMatrix);
};
