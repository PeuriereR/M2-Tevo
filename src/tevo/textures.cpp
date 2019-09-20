#include "textures.h"

Texture::Texture(int _type, int _width, int _height, char const * path_to_texture): type(_type), width(_width), height(_height){
    // Le niveau de mipmap pourrait (devrait) être un paramètre
    // N'utilisant ici que le mimap0, ce n'est pas nécessaire

    // Creation & Bind de la texture
    glGenTextures(1, &textureName);
    glActiveTexture(GL_TEXTURE0 + 0);

    if (type == CUBEMAP_TEXTURE) glBindTexture(GL_TEXTURE_CUBE_MAP, textureName);
    else glBindTexture(GL_TEXTURE_2D, textureName);

    // Paramétrisation de la texture
    if( type == DEPTH_TEXTURE ){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }else if (type == COLOR_TEXTURE ){

        // Lecture de l'image en entrée
        ImageData image= read_image_data(path_to_texture);

        GLenum data_format;
        if(image.channels == 4){
            data_format= GL_RGBA;
        }
        if(image.channels == 3){
            data_format= GL_RGB;
        }
        GLenum data_type= GL_UNSIGNED_BYTE;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, data_format, data_type, image.buffer());

    }else if (type == FB_COLOR_TEXTURE){

        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    }else if (type == CUBEMAP_TEXTURE){

        // Lecture de la cubemap
        ImageData cubemapImage= read_image_data("data/cubemapCORRECTE.jpg");

        int size= cubemapImage.width / 4;
        int height = cubemapImage.height / 3;

        GLenum data_format;
        GLenum data_type= GL_UNSIGNED_BYTE;
        if(cubemapImage.channels == 3) data_format= GL_RGB;
        else data_format= GL_RGBA;

        glPixelStorei(GL_UNPACK_ROW_LENGTH, cubemapImage.width);

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 2*size +   1*height*cubemapImage.width);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0,GL_RGBA, size, size, 0,data_format, data_type, cubemapImage.buffer());

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0*size + 1*height*cubemapImage.width);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0,GL_RGBA, size, size, 0,data_format, data_type, cubemapImage.buffer());

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 1*size + 0*height*cubemapImage.width);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0,GL_RGBA, size, size, 0,data_format, data_type, cubemapImage.buffer());

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 1*size + 2*height*cubemapImage.width);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0,GL_RGBA, size, size, 0,data_format, data_type, cubemapImage.buffer());

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 1*size + 1*height*cubemapImage.width);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0,GL_RGBA, size, size, 0,data_format, data_type, cubemapImage.buffer());

        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 3*size + 1*height*cubemapImage.width);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0,GL_RGBA, size, size, 0,data_format, data_type, cubemapImage.buffer());
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        // filtrage "correct" sur les bords du cube...
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        // nettoyage
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    }else{
        fprintf(stderr,"Erreur de generation de texture\n");
        exit(-1);
    }

    if (type != CUBEMAP_TEXTURE){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glBindTexture(GL_TEXTURE_2D,0);
    }
}

void Texture::bind(int unit){

    glActiveTexture(GL_TEXTURE0 + unit);

    if (type == CUBEMAP_TEXTURE) glBindTexture(GL_TEXTURE_CUBE_MAP, textureName);
    else glBindTexture(GL_TEXTURE_2D, textureName);

}

GLuint Texture::getTextureReference(){
    return textureName;
}

void Texture::release(){
    glDeleteTextures(1, &textureName);
}
