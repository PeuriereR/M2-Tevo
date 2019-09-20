#include "framebuffer.h"

FrameBuffer::FrameBuffer(GLuint _attachedTexture, int _type, int _width, int _height): attachedTexture(_attachedTexture), type(_type), width(_width), height(_height){

    // Cree un FrameBuffer de type TYPE, de taille WIDTH / HEIGHT avec la texture ATTACHEDTEXTURE

    if (type == DEPTH_FRAMEBUFFER){
        // N'utilise qu'une texture de profondeur
        glGenFramebuffers(1, &frameBufferName);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attachedTexture, 0);
        // 0 is the mipmap level used
        glReadBuffer(GL_NONE);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            fprintf(stderr,"Erreur de creation du FBO\n");
            exit(-1);
        }

        // unbind
    }else if (type == COLOR_FRAMEBUFFER){
        // Cree une texture de profondeur, ici non récupérée (pas de fonction associée)
        glGenTextures(1, &attachedDepth);
        glBindTexture(GL_TEXTURE_2D, attachedDepth);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        glGenFramebuffers(1, &frameBufferName);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);

        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, attachedTexture, 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER,  /* attachment */ GL_DEPTH_ATTACHMENT, /* texture */ attachedDepth, /* mipmap level */ 0);

        GLenum buffers[]= { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, buffers);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            fprintf(stderr,"Erreur de creation du FBO\n");
            exit(-1);
        }

    }else {
        // Error
        fprintf(stderr," Erreur de generation de FrameBuffer\n");
        exit(-1);
    }

    // Clean
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void FrameBuffer::bind(){
    // Bind le FBO
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
    glViewport(0,0,width,height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

GLuint FrameBuffer::getAttachedTexture(){
    return attachedTexture;
}

void FrameBuffer::release(){
    // Detruit le FBO
    glDeleteTextures(1, &attachedTexture);
    glDeleteTextures(1, &attachedDepth);
    glDeleteFramebuffers(1, &frameBufferName);
}

void unbindCurrentFrameBuffer(int _width, int _height){
    // Unbind & Clean
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glViewport(0,0,_width,_height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Provoque l'erreur Texture 0 is base level inconsistent a la 2e passe ?
}
