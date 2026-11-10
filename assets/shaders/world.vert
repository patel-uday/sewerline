#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;
uniform mat4 uMVP;
uniform mat4 uModel;
out vec2 vUV;out vec3 vNormal;out float vDepth;
void main(){vec4 world=uModel*vec4(aPos,1.0);gl_Position=uMVP*world;vUV=aUV;vNormal=mat3(uModel)*aNormal;vDepth=gl_Position.w;}
