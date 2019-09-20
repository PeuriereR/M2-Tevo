#include "glcore.h"
#include "image.h"
#include "image_io.h"
#include <iostream>

#define DEPTH_TEXTURE 0
#define COLOR_TEXTURE 1
#define FB_COLOR_TEXTURE 2
#define CUBEMAP_TEXTURE 3

class Texture{
    GLuint textureName;
    GLuint attachedDepth;

    int type;
    int width;
    int height;

public:
    Texture(){};
    Texture(int _type, int _width, int _height, char const * path_to_texture="");

    void bind(int unit);
    GLuint getTextureReference();
    void release();

};
