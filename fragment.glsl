#version 330 core

out vec4 colour;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

in vec2 interpolatedUV;
//in vec3 interpolatedNormal;

//texture information: in which layer this fragment has to lookup the texture?
uniform float textureArrayIndex;
uniform sampler2DArray tex;
//uniform vec2 uvScaleFactor;

void main() {
    vec4 textureColor = texture(tex,vec3(interpolatedUV.x  ,interpolatedUV.y ,textureArrayIndex));
    colour = textureColor;
}
