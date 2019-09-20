#include "wavefront.h"
#include "texture.h"
#include "orbiter.h"
#include "draw.h"
#include "app.h"
#include "app_time.h"
#include "program.h"
#include "uniforms.h"
#include "color.h"

#include "terrain.h"
#include "framebuffer.h"
#include "textures.h"
#include "camera.h"


class TP : public AppTime{ // Ou App
public:
    TP( ) : AppTime(1024, 640) {
        NB_REGIONS = 5;
        TAILLE_REGION = 50;

        nbMat = 6;

        mapTEX = new Texture[nbMat];
        mapTEX_laterale = new Texture[nbMat];
    }
    int init( )
    {

        // Chargement des shaders
        cubemapShader = read_program("data/shaders/cubemap.glsl");                // Cubemap shader
        program_print_errors(cubemapShader);
        i2DShader= read_program("data/shaders/2d.glsl");                    // 2D Display shader (debug)
        program_print_errors(i2DShader);
        waterShader = read_program("data/shaders/reflectshader.glsl");      // Reflection shader
        program_print_errors(waterShader);
        mainShader = read_program("data/shaders/mainShader.glsl");         // Main shader
        program_print_errors(mainShader);
        shadowShader = read_program("data/shaders/shadowmapshader.glsl");  // Shadowmap shader
        program_print_errors(shadowShader);

        std::cout << "****************************************************   Lecture des shaders achevée." <<std::endl<<std::endl;

        // Chargement des images & textures

        dudvMAP = Texture(COLOR_TEXTURE,0,0,"data/dudvmap.png");            // Image dudv pour distortion de l'eau
        normalMAP = Texture(COLOR_TEXTURE,0,0,"data/normalmap.png");        // Normal map pour l'eau
        cubemapTexture = Texture(CUBEMAP_TEXTURE,0,0,"data/cubemapCORRECTE.jpg");  // Cubemap

        // Textures du terrain
        mapTEX[0] = Texture(COLOR_TEXTURE,0,0,"data/textures/ice_packed.png");
        mapTEX[1] = Texture(COLOR_TEXTURE,0,0,"data/textures/sable_2.png");
        mapTEX[2] = Texture(COLOR_TEXTURE,0,0,"data/textures/concrete_green.png");
        mapTEX[3] = Texture(COLOR_TEXTURE,0,0,"data/textures/concrete_brown.png");
        mapTEX[4] = Texture(COLOR_TEXTURE,0,0,"data/textures/stone.png");
        mapTEX[5] = Texture(COLOR_TEXTURE,0,0,"data/textures/snow.png");
        // Textures latérales des cubes
        mapTEX_laterale[0] = Texture(COLOR_TEXTURE,0,0,"data/textures/coarse_dirt.png");
        mapTEX_laterale[1] = Texture(COLOR_TEXTURE,0,0,"data/textures/sable.png");
        mapTEX_laterale[2] = Texture(COLOR_TEXTURE,0,0,"data/textures/concrete_green.png");
        mapTEX_laterale[3] = Texture(COLOR_TEXTURE,0,0,"data/textures/dirt.png");
        mapTEX_laterale[4] = Texture(COLOR_TEXTURE,0,0,"data/textures/stone_andesite.png");
        mapTEX_laterale[5] = Texture(COLOR_TEXTURE,0,0,"data/textures/stone_diorite.png");

        std::cout << "****************************************************   Lecture des images achevée." <<std::endl<<std::endl;

        // Création des FBO et de leurs textures

        // ShadowMap
        shadowTEX = Texture(DEPTH_TEXTURE, window_width(), window_height() );
        shadowFBO = FrameBuffer(shadowTEX.getTextureReference() ,DEPTH_FRAMEBUFFER,window_width(),window_height());
        // Reflection
        reflexionTexture = Texture(FB_COLOR_TEXTURE,window_width(),window_height());
        reflectionFBO = FrameBuffer(reflexionTexture.getTextureReference() , COLOR_FRAMEBUFFER, window_width(), window_height());
        // Refraction
        refractionTexture = Texture(FB_COLOR_TEXTURE,window_width(),window_height());
        refractionFBO = FrameBuffer(refractionTexture.getTextureReference() , COLOR_FRAMEBUFFER, window_width(), window_height());

        // Créations des maillages

        debug_rect1 = Mesh(GL_TRIANGLES);  // Pour debug
        debug_rect1.texcoord(vec2(0,0)).vertex(-1,-0.5,0);
        debug_rect1.texcoord(vec2(1,1)).vertex(0,0.5,0);
        debug_rect1.texcoord(vec2(0,1)).vertex(-1,0.5,0);

        debug_rect1.texcoord(vec2(0,0)).vertex(-1,-0.5,0);
        debug_rect1.texcoord(vec2(1,0)).vertex(0,-0.5,0);
        debug_rect1.texcoord(vec2(1,1)).vertex(0,0.5,0);


        debug_rect2 = Mesh(GL_TRIANGLES); // Pour debug
        debug_rect2.texcoord(vec2(0,0)).vertex(0,-0.5,0);
        debug_rect2.texcoord(vec2(1,1)).vertex(1,0.5,0);
        debug_rect2.texcoord(vec2(0,1)).vertex(0,0.5,0);

        debug_rect2.texcoord(vec2(0,0)).vertex(0,-0.5,0);
        debug_rect2.texcoord(vec2(1,0)).vertex(1,-0.5,0);
        debug_rect2.texcoord(vec2(1,1)).vertex(1,0.5,0);

        cube_mesh= read_mesh("data/obj/cube.obj");
        if(!cube_mesh.vertex_count()) return -1;


        // Initialisation des variables pour l'affichage
        showBorders = false;
        moveFactor = 0; // Mouvement de l'eau (dudv)
        int taille = TAILLE_REGION * NB_REGIONS;
        h_water = ( 2.5 * taille / 250 );
        waterModelMatrix = Scale(taille,1,-taille) * Translation(0.5,h_water,0.5);

        // Creating du terrain
        t = Terrain("data/terrain/Clipboard02.png", NB_REGIONS, TAILLE_REGION, nbMat);

        // Binding vao
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Completion des buffers
        fillBuffer(mainShader, "position", cube_mesh, VERTEX_BUFFER);
        fillBuffer(mainShader, "normal", cube_mesh, NORMAL_BUFFER);
        fillBuffer(mainShader, "texcoord", cube_mesh, TEXCOORD_BUFFER);

        t.fillBuffers(mainShader);
        std::cout << "****************************************************   Création du terrain achevée." <<std::endl<<std::endl;

        // Vao de la cubemap
        glGenVertexArrays(1, &vao_null);
        glBindVertexArray(vao_null);

        // Clean
        glUseProgram(0);
        glBindVertexArray(0);

        // Parametrage
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        glClearDepth(1.f);                          // profondeur par defaut

        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);                    // activer le ztest
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);

        std::cout << "****************************************************   Initialisation achevée." <<std::endl<<std::endl;
        return 0;   // ras, pas d'erreur
    }

    // destruction des objets de l'application
    int quit( )
    {

        //Textures & samplers
        dudvMAP.release();
        normalMAP.release();
        shadowTEX.release();
        reflexionTexture.release();
        refractionTexture.release();
        cubemapTexture.release();
        for (int i = 0; i < nbMat; i++) {
            mapTEX[i].release();
            mapTEX_laterale[i].release();
        }

        //Shaders
        release_program(waterShader);
        release_program(mainShader);
        release_program(shadowShader);
        release_program(i2DShader);
        release_program(cubemapShader);

        // FrameBuffer
        shadowFBO.release();
        reflectionFBO.release();
        refractionFBO.release();

        // VAO
        glDeleteVertexArrays(1, &vao);
        glDeleteVertexArrays(1, &vao_null);

        //Mesh
        debug_rect1.release();
        debug_rect2.release();
        cube_mesh.release();

        return 0;
    }

    int render( )
    {

        /********************************************************************/
        // Gestion de la camera

        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);

        if(mb & SDL_BUTTON(1)){
            cam.move(-2*my,0,0);
            cam.move(0,0,-2*mx);
        }
        SDL_MouseWheelEvent wheel= wheel_event();
        if(wheel.y != 0){
            clear_wheel_event();
            cam.move(0,wheel.y*5,0);
        }

        if (key_state('q')) cam.moveSide(-1);
        else if (key_state('d')) cam.moveSide(1);
        if (key_state('z')) cam.moveFront(-1);
        else if (key_state('s')) cam.moveFront(1);

        if(key_state('l')){
            clear_key_state('l');
            showBorders = !showBorders;
        }

        /********************************************************************/
        // Passe de la Shadow Map

        shadowFBO.bind(); // bind du FBO
        glBindVertexArray(vao); // bind du VAO
        glUseProgram(shadowShader); // bind du Shader

        Point lightInvDir = /* RotationY(global_time()/20) */ (Point(150,100,200));

        Transform lightProjection = Ortho(-200,200,-200,200,1,1000);
        Transform lightView = Lookat(lightInvDir, Point(150,0,-150), Vector(0,1,0));
        Transform LSDepth = lightProjection * lightView;

        program_uniform(shadowShader,"lightSpaceMatrix",LSDepth);
        glDrawArraysInstanced(GL_TRIANGLES, 0, cube_mesh.vertex_count(), t.getCubesNumber() );

        // Clean
        unbindCurrentFrameBuffer(window_width(),window_height());

        /********************************************************************/
        // Passe de la texture de réflexion

        Transform projection =  Perspective(45, window_width()/window_height(), 0.1,1000.0);

        // On inverse la caméra en fonction de la hauteur de l'eau
        float dist_to_water = (cam.position.y-h_water);
        float cam_to_water = std::abs(cam.center.y - h_water);
        cam.position.y -=  2 * dist_to_water;
        cam.invertPitch();
        cam.center.y += 2 * cam_to_water;

        Transform view = Lookat(cam.getPosition(),cam.center,cam.getUpVector() );

        cam.center.y -= 2 * cam_to_water;
        cam.invertPitch();
        cam.position.y +=  2* dist_to_water;

        // Bind du FBO
        reflectionFBO.bind();

        // Affichage de la cubemap
        glBindVertexArray(vao_null);
        glUseProgram(cubemapShader);
        cubemapTexture.bind(0);
        glUniform1i( glGetUniformLocation(cubemapShader, "texture0"), 0);
        program_uniform(cubemapShader, "vpInvMatrix", Inverse(projection * view));
        program_uniform(cubemapShader, "camera_position", cam.center);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(vao);// On rebind l'ancien vao & shader
        glUseProgram(mainShader);

        shadowTEX.bind(1); // On associe la texture d'ombre
        glUniform1i( glGetUniformLocation(mainShader, "shadowMap"), 1);

        Transform mvp;
        mvp = projection * view;
        // Uniforms
        program_uniform(mainShader,"mvpMatrix",mvp);
        glUniformMatrix4fv(glGetUniformLocation(mainShader, "viewInvMatrix"), 1, GL_FALSE, &(Inverse(view)).m[0][0]);
        program_uniform(mainShader,"mvpDepthMatrix",LSDepth);
        program_uniform(mainShader,"pos_so",vec3(lightInvDir));
        program_uniform(mainShader,"showBorders",showBorders);
        program_uniform(mainShader,"tailleRegion",TAILLE_REGION);

        // Calcul de visiblité > Le fait d'avoir le pitch inversé ne change rien !! Notre terrain est plat
        t.performVisibilityTest( mvp );

        // Affichage par matière
        for(int i=1; i<nbMat; i++){
            // Bind
            mapTEX[i].bind(0);
            glUniform1i( glGetUniformLocation(mainShader, "texture0") , 0);
            // Presque inutile de bind les textures non laterales avec Fresnel, impossible de voir le dessus
            mapTEX_laterale[i].bind(2);
            glUniform1i( glGetUniformLocation(mainShader, "texture_laterale") , 2);
            // Draw
            t.draw_by_mat(i, cube_mesh.vertex_count() );
        }
        // Clean
        glActiveTexture(GL_TEXTURE0+0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0+2);
        glBindTexture(GL_TEXTURE_2D, 0);
        unbindCurrentFrameBuffer(window_width(),window_height());

        /********************************************************************/
        // Passe de la texture de refraction

        refractionFBO.bind(); // Vao & program & shadowMap(1) deja bind, on ne change pas

        view = Lookat(cam.getPosition(),cam.center,cam.getUpVector() ); // On a changé de point de vue
        mvp = projection * view;

        // Uniform des variables modifiées
        program_uniform(mainShader,"mvpMatrix",mvp);
        glUniformMatrix4fv(glGetUniformLocation(mainShader, "viewInvMatrix"), 1, GL_FALSE, &(Inverse(view)).m[0][0]);

        for(int i=0; i<1; i++){
            // Une seule matière : sous h_water : l'eau
            mapTEX[i].bind(0);
            glUniform1i( glGetUniformLocation(mainShader, "texture0") , 0);
            mapTEX_laterale[i].bind(2); // inutile si petite map
            glUniform1i( glGetUniformLocation(mainShader, "texture_laterale") , 2);
            t.draw_by_mat(i, cube_mesh.vertex_count() );
        }

        // Clean
        glActiveTexture(GL_TEXTURE0+0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, 0);
        unbindCurrentFrameBuffer(window_width(),window_height());

        /***********************************************************************/
        // Passe principale

        // Affichage de la cubemap
        glBindVertexArray(vao_null);
        glUseProgram(cubemapShader);
        cubemapTexture.bind(0);
        glUniform1i(glGetUniformLocation(cubemapShader, "texture0"), 0);
        program_uniform(cubemapShader, "vpInvMatrix", Inverse(projection * view));
        program_uniform(cubemapShader, "camera_position", cam.center);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(vao); // On remet le bon vao & shader
        glUseProgram(mainShader);

        // Uniforms
        program_uniform(mainShader,"mvpMatrix",mvp);
        glUniformMatrix4fv(glGetUniformLocation(mainShader, "viewInvMatrix"), 1, GL_FALSE, &(Inverse(view)).m[0][0]);
        program_uniform(mainShader,"mvpDepthMatrix",LSDepth);
        program_uniform(mainShader,"pos_so",vec3(lightInvDir));
        program_uniform(mainShader,"showBorders",showBorders);
        program_uniform(mainShader,"tailleRegion",TAILLE_REGION);
        shadowTEX.bind(1); // On associe la texture d'ombre
        glUniform1i( glGetUniformLocation(mainShader, "shadowMap"), 1);

        // Affichage par matière
        // Nous n'avons pas besoin d'afficher sous l'eau
        for(int i=1; i<nbMat; i++){
            mapTEX[i].bind(0);
            glUniform1i( glGetUniformLocation(mainShader, "texture0") , 0);
            mapTEX_laterale[i].bind(2);
            glUniform1i( glGetUniformLocation(mainShader, "texture_laterale") , 2);
            t.draw_by_mat(i, cube_mesh.vertex_count() );
        }


        // Nettoyage
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glActiveTexture(GL_TEXTURE0+2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0+0);
        glBindTexture(GL_TEXTURE_2D, 0);

        /***********************************************************************/
        // Passe de l'eau

        glUseProgram(waterShader);

        reflexionTexture.bind(0);
        refractionTexture.bind(1);
        dudvMAP.bind(2);
        normalMAP.bind(3);

        // Uniforms

        // textures
        glUniform1i( glGetUniformLocation(waterShader, "reflectTexture") , 0);
        glUniform1i( glGetUniformLocation(waterShader, "refractedTexture") , 1);
        glUniform1i( glGetUniformLocation(waterShader, "dudv") , 2);
        glUniform1i( glGetUniformLocation(waterShader, "normalMap") , 3);

        // autres
        mvp = projection * view * waterModelMatrix;
        program_uniform(waterShader,"moveFactor",moveFactor);
        program_uniform(waterShader,"lightPosition",lightInvDir);
        program_uniform(waterShader,"mvpMatrix",mvp);
        program_uniform(waterShader,"cameraPosition",cam.position);
        program_uniform(waterShader,"modelMatrix",waterModelMatrix);

        // Draw
        glDrawArrays(
            GL_TRIANGLES,
            30,
            6 // 6 (2*3sommets par face > 2 triangles)
        );


        glActiveTexture(GL_TEXTURE0+0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0+2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0+3);
        glBindTexture(GL_TEXTURE_2D, 0);

        if(key_state('k')){
            // For debug
            glUseProgram(i2DShader);

            reflexionTexture.bind(0);
            glUniform1i( glGetUniformLocation(i2DShader, "textu") , 0);
            draw(debug_rect1,i2DShader);

            refractionTexture.bind(0);
            glUniform1i( glGetUniformLocation(i2DShader, "textu") , 0);
            draw(debug_rect2,i2DShader);
        }

        // Clean
        glActiveTexture(GL_TEXTURE0+0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        // Water movement update
        moveFactor +=0.0003 ;
        if (moveFactor>=1.0)
        moveFactor=0.0 ;

        return 1;
    }

private:

    // Variables d'affichage
    int NB_REGIONS;
    int TAILLE_REGION;

    int nbMat;
    int h_water;
    float moveFactor;
    bool showBorders;
    Transform waterModelMatrix;

    // Camera
    Camera cam;

    // Maillages
    Mesh debug_rect1 ,debug_rect2;
    Mesh cube_mesh;

    // Textures
    Texture dudvMAP;
    Texture normalMAP;
    Texture reflexionTexture;
    Texture refractionTexture;
    Texture shadowTEX;
    Texture* mapTEX;
    Texture* mapTEX_laterale;
    Texture cubemapTexture;

    // FBO
    FrameBuffer reflectionFBO;
    FrameBuffer refractionFBO;
    FrameBuffer shadowFBO;

    // Shaders
    GLuint waterShader;
    GLuint mainShader;
    GLuint shadowShader;
    GLuint cubemapShader;
    GLuint i2DShader;

    // VAO
    GLuint vao;
    GLuint vao_null;

    // Terrain
    Terrain t;
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();

    return 0;
}
