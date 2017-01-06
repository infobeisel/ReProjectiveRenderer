#version 330 core

out vec4 colour;

//uniform vec3 Ka;
//uniform vec3 Kd;
//uniform vec3 Ks;

in vec2 interpolatedUV;
//in vec3 interpolatedNormal;

//texture information: in which layer this fragment has to lookup the texture?
//uniform float textureArrayIndex;
//uniform sampler2DArray tex;
//uniform float uvFactorx;
//uniform float uvFactory;

void main() {
    colour = vec4(interpolatedUV.x,interpolatedUV.y,0.0,1.0);
}
