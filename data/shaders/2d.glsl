#version 330
#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;

uniform mat4 mvpMatrix;
out vec2 vertex_texcoord;

void main(void){

    gl_Position= vec4(position.xy,-1,1);
    vertex_texcoord= texcoord;
}

#endif

#ifdef FRAGMENT_SHADER

uniform sampler2D textu;
in vec2 vertex_texcoord;

out vec4 out_Color;

void main(void){
    out_Color = texture(textu, vertex_texcoord);
}

#endif
