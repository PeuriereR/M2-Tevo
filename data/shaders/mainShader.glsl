#version 330

#define M_PI 3.1415926535897932384626433832795
#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;
layout(location= 3) in vec3 instance_position;
layout(location= 4) in vec2 texcomap;
layout(location= 5) in vec3 normalsmap;
layout(location= 6) in float slopemap;

uniform mat4 mvpMatrix;
uniform mat4 viewInvMatrix;
uniform mat4 mvpDepthMatrix;
uniform vec3 pos_so;
uniform bool showBorders;
uniform int tailleRegion;

out vec2 vertexCoords;
out float height;
out float orientation;
out float slope;
out vec4 shadowCoords;
out vec3 normal_cube;
out float isborder;

// Pour B.walter non utilisé
out vec3 normal_map;
out vec3 toLightVector;
out vec3 toCameraVector;

void main(){

    vec4 realPos = vec4(position + instance_position, 1);
    gl_Position= mvpMatrix * realPos;

    // Coordonnées dans le repère soleil
    shadowCoords = mvpDepthMatrix * realPos;

    // Données à transmettre
    vertexCoords= texcoord;
    height = instance_position.y;
    slope = slopemap;
    normal_cube = normal;

    vec3 normale =  normalsmap ;
    normal_map = normalsmap;
    // Pour garder un effet cubique:
    // vec3 normale =  ( normalsmap +  normal )/2;

    toLightVector = pos_so - realPos.xyz;

    // Calcul de la luminance
    float cos_theta = dot(normalize(normale), normalize(toLightVector));

    toCameraVector = (viewInvMatrix * vec4(0)).xyz - realPos.xyz;
    vec3 h = normalize(toCameraVector + toLightVector);
    float cos_theta_hb = dot(normalize(normale),h);

    float kb = 1.2;
    float nb = 10; // concentration reflet

    float fctb = kb / M_PI + (1- (1 - kb) * (nb + 1) / (2 * M_PI) * pow(cos_theta_hb,nb));
    orientation = fctb * max(cos_theta, 0.0);

    // Verification pour l'affichage des bordures des regions
    isborder = 0;

    if (showBorders){
        if( int(instance_position.x)%tailleRegion == 0  ){
            isborder = 1;
        }
        if( int(-instance_position.z)%tailleRegion == 0  ){
            isborder = 1;
        }
    }
}


#endif


#ifdef FRAGMENT_SHADER

out vec4 fragment_color;

in float isborder;
in vec2 vertexCoords;
in float height;
in float orientation;
in float slope;
in vec4 shadowCoords;
in vec3 normal_cube;

in vec3 normal_map;
in vec3 toLightVector;
in vec3 toCameraVector;


uniform sampler2D texture0;
uniform sampler2D shadowMap;
uniform sampler2D texture_laterale;

void main(){
    float visibility = 1.0; // facteur de visibilité de l'ombrage  > 1 = pas d'ombre

    vec3 projCoords = shadowCoords.xyz / shadowCoords.w * 0.5 + 0.5;

    // Verification de la shadowmap dans le canal rouge de la texture (depth)
    if ( texture( shadowMap, projCoords.xy ).x   <  projCoords.z-0.005){
        visibility = 0.8;
    }

    vec4 tex;
    // Affectation de la texture
    if( dot(normal_cube, vec3(0,1,0)) == 0){
        tex = texture(texture_laterale,vertexCoords);
    }else{
        tex = texture(texture0,vertexCoords);
    }

    // Ajout des ombres, de la texture, la lumière et influence de la pente
    fragment_color = tex * visibility * max(0.5,orientation) * (0.5+slope);

    if(height < 2) fragment_color = height * fragment_color; // Pour la profondeur de l'eau

    // Affichage éventuel des bordures
    if( isborder > 0 ){
        fragment_color = vec4(0,1,0,1);
    }

        /*

        //Trop bruité & faces opposées totalement noires
        //-> Autre fonction de nombre aléatoire ?

        // B WALTER (Microfacet Models for Refraction through Rough Surfaces)

        // Getting our variables

        vec3 n = normalize(normal_map);
        vec3 i = normalize(toLightVector);
        vec3 o = reflect(-i, n);

        vec3 cam = normalize(toCameraVector);
        vec3 hr = normalize(i + o);

        float reflectionFactor = dot(o, cam); // used to mix refraction and reflection

        // F(i,hr)

        // nt different de ni et ni>nt pour "logique" matériau basique
        float nt = 1; //Index of refraction of media on the transmitted side //largeur lum
        // 1
        float ni = 0.2; //Index of refraction of the media on the incident side
        // 0.2
        float c = abs( dot(i,hr) );
        float g = sqrt( pow(nt,2) / pow(ni,2) - 1 + pow(c,2) );
        float f_i_hr = 1./2 * pow(g-c,2)/pow(g+c,2) * (1 + pow(c*(g+c) - 1,2)/pow(c*(g-c) + 1,2) );

        // G(i,o,hr)

        float ag = 0.2;  //0.2
        float g1_i_hr;

        if ( (dot(i,hr) / dot(i,n)) >0)
        g1_i_hr =2. / (1.0 + sqrt(1.0 + pow(ag,2.0) * pow(tan(acos(dot(-cam,n))),2.0) ) ) ;
        else g1_i_hr = 0;

        float g1_o_hr;
        if ( (dot(o,hr) / dot(o,n)) >0)
        g1_o_hr = 2. / (1.0 + sqrt(1.0 + pow(ag,2.0) * pow(tan(acos(dot(-cam,n))),2.0) ) ) ;
        else g1_o_hr = 0;

        float g_i_o_hr = g1_i_hr * g1_o_hr;

        // D(hr)

        float rand_numb = fract(sin(dot(vertexCoords ,vec2(12.9898,78.233))) * 43758.5453);
        float ap = 100; // 45
        float teta_m = acos( pow(rand_numb,1./(ap+2)));
        float d_hr;
        if( dot(hr,n)>0 )
        d_hr = pow(ag,2) / (M_PI * pow(cos(teta_m),4) * pow(pow(ag,2) + pow(tan(teta_m),2),2) ) ;
        else d_hr = 0;

        float i_n = abs(dot(i,n));
        float o_n = abs(dot(o,n));
        int k = 2; // proportion difus
        float fct = ( (f_i_hr * g_i_o_hr * d_hr) / (4. * i_n * o_n ) ) ;
        float cos_theta = dot(n,i);
        fct = fct * max(cos_theta, 0.0) ;


        fragment_color = vec4(fct);
        */

}

#endif
