#version 330
#define M_PI 3.1415926535897932384626433832795

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;

uniform mat4 mvpMatrix;
uniform vec3 cameraPosition;
uniform mat4 modelMatrix;
uniform vec3 lightPosition;

out vec3 toCameraVector;
out vec4 clipSpace;
out vec2 texCoordinates;
out vec3 fromLightVector;

const float tiling = 10.0;

void main( )
{
    vec3 world_pos = (modelMatrix * vec4(position,1)).xyz;
    clipSpace = mvpMatrix * vec4(position,1);
    gl_Position= clipSpace;

    // Donnees à transmettre
    texCoordinates = texcoord * tiling;
    toCameraVector = cameraPosition - world_pos;
    fromLightVector = world_pos - lightPosition;
}

#endif


#ifdef FRAGMENT_SHADER

out vec4 fragment_color;
in vec4 clipSpace;
in vec2 texCoordinates;
in vec3 toCameraVector;
in vec3 fromLightVector;

uniform sampler2D reflectTexture;
uniform sampler2D refractedTexture;
uniform sampler2D dudv;
uniform sampler2D normalMap;

uniform float moveFactor;

const float waveStrength=0.02; //0.001
const vec3 lightColor= vec3(1,1,1);
const float shineDamper = 25.0;
const float reflectivity = 0.6;

void main( )
{
    // Projection
    vec2 coords = (clipSpace.xy / clipSpace.w)/2.0 + 0.5;

    vec2 reflectedcoords = vec2(coords.x,-coords.y);
    vec2 refractedcoords = vec2(coords.x,coords.y);
    // On combine
    // Distortion dudv avec le moveFactor (rapidité du mouvement de l'eau)
    vec2 distortedTexCoords = texture(dudv, vec2(texCoordinates.x + moveFactor, texCoordinates.y)).rg*0.1;
    distortedTexCoords = texCoordinates + vec2(distortedTexCoords.x, distortedTexCoords.y+moveFactor);

    vec2 totalDistortion = (texture(dudv, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength;

    reflectedcoords += totalDistortion;

    // Eviter les effets du aux bords
    reflectedcoords.x = clamp(reflectedcoords.x, 0.001,0.999);
    reflectedcoords.y = clamp(reflectedcoords.y, -0.999,-0.001);

    refractedcoords += totalDistortion;
    refractedcoords = clamp(refractedcoords, 0.001,0.999);

    vec4 tex = texture(reflectTexture,reflectedcoords);
    vec4 tex2 = texture(refractedTexture,refractedcoords);

    vec3 viewVector = normalize(toCameraVector);
    // Fresnel effect
    float refractiveFactor = dot(viewVector,vec3(0,1,0));


    vec4 normalMapColor = texture(normalMap,distortedTexCoords);
    // Normale extraite de la normal map
    vec3 normal = vec3( 2 * normalMapColor.r - 1.0, normalMapColor.b , normalMapColor.g * 2 -1.0);
    normal = normalize(normal);

    // On ne cherche pas ici à faire une reflexion realiste : surtout speculaire
    vec3 reflectedLight = reflect(normalize(fromLightVector), normal);
    float cos_theta = max(dot(reflectedLight, viewVector), 0.0);
    vec3 h = normalize(toCameraVector + -fromLightVector);
    float cos_theta_hb = dot(normalize(normal),h);

    float kb = 0.1; // peu de diffus, surtout speculaire sur l'eau
    float nb = 15; // concentration reflet

    float fctb = kb / M_PI + (1- (1 - kb) * (nb + 1) / (2 * M_PI) * pow(cos_theta_hb,nb));
    float specular = fctb * max(cos_theta, 0.0);

    vec3 reflectViewVector = normalize(-fromLightVector);

    // On ajoute quelques reflets meme si on est pas en face du soleil
    float fakeSpecular = max(dot(reflectedLight, reflectViewVector),0.0);
    specular = pow(specular, shineDamper);

    vec3 specularHighlights = max( lightColor * specular * reflectivity,      lightColor * fakeSpecular * reflectivity);

    // On mixe les textures reflexion refraction selon Fresnel
    fragment_color = mix(tex,tex2,refractiveFactor);
    // On ajoute les effets speculaires
    fragment_color = fragment_color + vec4(specularHighlights,1);

}

#endif
