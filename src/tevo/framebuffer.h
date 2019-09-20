#include <iostream>
#include "glcore.h"

#define COLOR_FRAMEBUFFER 0
#define DEPTH_FRAMEBUFFER 1

class FrameBuffer{

    GLuint frameBufferName;
    GLuint attachedTexture;
    GLuint attachedDepth;

    int type; // Type de FBO
    int width; // Taille des textures du FBO
    int height;

public:
    FrameBuffer(){};
    FrameBuffer(GLuint attachedTexture, int type, int width, int height);
    void bind();
    GLuint getAttachedTexture();
    void release();
};

void unbindCurrentFrameBuffer(int _width, int _height);
