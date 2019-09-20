#include <iostream>
#include "glcore.h"
#include "mesh.h"
#include "mat.h"

#define VERTEX_BUFFER 0
#define NORMAL_BUFFER 1
#define TEXCOORD_BUFFER 2

// Diverses fonctions génériques

void fillBuffer( GLuint program, char  const * programLocation, Mesh& mesh, int type);

bool visible(const Transform& mvp, const Point& pmin, const Point& pmax );
