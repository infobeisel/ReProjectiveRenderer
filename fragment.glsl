#version 330 core

out vec4 colour;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;

in vec2 interpolatedUV;
in vec3 interpolatedNormal;

//texture information: in which layer this fragment has to lookup the texture?
uniform float diffuseTextureArrayIndex;
uniform sampler2DArray diffuseSampler;
uniform float ambientTextureArrayIndex;
uniform sampler2DArray ambientSampler;
uniform float specularTextureArrayIndex;
uniform sampler2DArray specularSampler;
uniform float bumpTextureArrayIndex;
uniform sampler2DArray bumpSampler;
//uniform vec2 uvScaleFactor;

void main() {
    vec4 textureColor = vec4(0.0);

    if(diffuseTextureArrayIndex >= 0.0f)
        textureColor +=  texture(diffuseSampler,vec3(interpolatedUV.x  ,interpolatedUV.y ,diffuseTextureArrayIndex)) * vec4(Kd,1.0f);
    if(ambientTextureArrayIndex >= 0.0f)
        textureColor +=  texture(ambientSampler,vec3(interpolatedUV.x  ,interpolatedUV.y ,ambientTextureArrayIndex))*  vec4(Ka,1.0f);
    if(specularTextureArrayIndex >= 0.0f)
        textureColor +=  texture(specularSampler,vec3(interpolatedUV.x  ,interpolatedUV.y ,specularTextureArrayIndex))*  vec4(Ks,1.0f);
    //if(bumpTextureArrayIndex >= 0.0f)
     //   textureColor += texture(bumpSampler,vec3(interpolatedUV.x  ,interpolatedUV.y ,bumpTextureArrayIndex));
    colour = textureColor;
}
