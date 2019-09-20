#version 330

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 position;
layout(location= 3) in vec3 instance_position;

uniform mat4 lightSpaceMatrix;

void main(){
    gl_Position =  lightSpaceMatrix * vec4(position + instance_position,1);
}


#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) out float fragmentdepth;

void main(){
    fragmentdepth = gl_FragCoord.z;
}

#endif
