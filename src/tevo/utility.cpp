#include "utility.h"

void fillBuffer( GLuint program, char const * programLocation, Mesh& mesh, int type){

    // Rempli le buffer TYPE du mesh
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    GLint index;
    switch(type){
        case(VERTEX_BUFFER):
        glBufferData(GL_ARRAY_BUFFER, mesh.vertex_buffer_size(), mesh.vertex_buffer(), GL_STATIC_DRAW);
        index= glGetAttribLocation(program, programLocation);
        glVertexAttribPointer(index,3,GL_FLOAT,GL_FALSE,0,0);
        break;

        case(NORMAL_BUFFER):
        glBufferData(GL_ARRAY_BUFFER, mesh.normal_buffer_size(), mesh.normal_buffer(), GL_STATIC_DRAW);
        index= glGetAttribLocation(program, programLocation);
        glVertexAttribPointer(index,3,GL_FLOAT,GL_FALSE,0,0);
        break;

        case(TEXCOORD_BUFFER):
        glBufferData(GL_ARRAY_BUFFER, mesh.texcoord_buffer_size(), mesh.texcoord_buffer(), GL_STATIC_DRAW);
        index= glGetAttribLocation(program, programLocation);
        glVertexAttribPointer(index,2,GL_FLOAT,GL_FALSE,0,0);
        break;

        default:
        fprintf(stderr,"Erreur fillBuffer %d\n",type);
        exit(-1);
    }
    glEnableVertexAttribArray(index);

}

bool visible(const Transform& mvp, const Point& pmin, const Point& pmax ){
    // Renvoie vrai si la boite englobante est au moins partiellement visible
    // Test effectué dans les 2 repères

    bool sensUN = true, sensDEUX = true;
    int planes[6] = { };
    int planes2[6] = { };

    // Pour chaque sommet
    for(unsigned int i= 0; i < 8; i++){
        Point p= pmin;
        if(i & 1) p.x= pmax.x;
        if(i & 2) p.y= pmax.y;
        if(i & 4) p.z= pmax.z;

        // transformation du point homogene (x, y, z, w= 1) dans le repère projectif
        vec4 h= mvp(vec4(p));
        // teste la position du point homogene par rapport aux 6 faces de la region visible
        if(h.x < -h.w) planes[0]++;     // trop a gauche
        if(h.x > h.w) planes[1]++;      // trop a droite

        if(h.y < -h.w) planes[2]++;     // trop bas
        if(h.y > h.w) planes[3]++;      // trop haut

        if(h.z < -h.w) planes[4]++;     // trop pres
        if(h.z > h.w) planes[5]++;      // trop loin
    }

    // Verifie si tous les sommets sont du "mauvais cote" d'une seule face, planes[i] == 8
    for(unsigned int i= 0; i < 6; i++){
        if(planes[i] == 8){
            sensUN = false;       // la boite englobante n'est pas visible
        }
    }

    // On teste dans l'autre sens avec les sommets du frustrum

    Point pminF(-1,-1,1);
    Point pmaxF(1,1,-1);

    for(unsigned int i= 0; i < 8; i++){
        Point p= pminF;
        if(i & 1) p.x= pmaxF.x;
        if(i & 2) p.y= pmaxF.y;
        if(i & 4) p.z= pmaxF.z;

        // transformation du point dans le repère réel
        vec4 h= Inverse(mvp)(vec4(p));
        h.x = h.x/h.w;
        h.y = h.y/h.w;
        h.z = h.z/h.w;
        // h homogène

        if(h.x < pmin.x) planes2[0]++;     // trop a gauche
        if(h.x > pmax.x) planes2[1]++;      // trop a droite

        if(h.y < pmin.y) planes2[2]++;     // trop bas
        if(h.y > pmax.y) planes2[3]++;      // trop haut

        // On inverse les z car on affiche en -z avec openGL
        if(h.z > pmin.z) planes2[4]++;     // trop pres
        if(h.z < pmax.z) planes2[5]++;      // trop loin
    }

    for(unsigned int i= 0; i < 6; i++){
        if(planes2[i] == 8){
            sensDEUX = false;       // la boite englobante n'est pas visible
        }
    }

    return sensUN & sensDEUX;
}
