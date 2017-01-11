#version 330 core

out vec4 colour;


struct LightSource {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec4 position;
    vec3 direction;
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
};


uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;


uniform int lightCount;
const int maxLightCount = 10;
uniform LightSource lights[maxLightCount];


in vec2 interpolatedUV;
in vec3 interpolatedNormal;
in vec4 interpolatedPos;

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
    vec4 textureColorAmb = vec4(0.0);
    vec4 textureColorDif = vec4(0.0);
    vec4 textureColorSpec = vec4(0.0);
    vec4 diffContr = vec4(0.0);
    vec4 ambContr = vec4(0.0);
    vec4 specContr = vec4(0.0);

    if(diffuseTextureArrayIndex >= 0.0f)
        textureColorDif =  texture(diffuseSampler,vec3(interpolatedUV.x  ,interpolatedUV.y ,diffuseTextureArrayIndex)) * vec4(Kd,1.0f);
    if(ambientTextureArrayIndex >= 0.0f)
        textureColorAmb =  texture(ambientSampler,vec3(interpolatedUV.x  ,interpolatedUV.y ,ambientTextureArrayIndex))*  vec4(Ka,1.0f);
    if(specularTextureArrayIndex >= 0.0f)
        textureColorSpec =  texture(specularSampler,vec3(interpolatedUV.x  ,interpolatedUV.y ,specularTextureArrayIndex))*  vec4(Ks,1.0f);
    //treat them as point lights
    for(int i = 0; i < lightCount; i++) {
        //lights
        float d = distance(interpolatedPos,lights[i].position);
        float atten = 1.0f / ( lights[i].attenuationConstant + lights[i].attenuationLinear * d + lights[i].attenuationQuadratic * d*d);
        //float NL = dot(interpolatedNormal , lights[i].direction);
        vec3 t = (Kd * lights[i].diffuse);
        t *= atten;
        diffContr += vec4(t,1.0);
    }
    colour =  diffContr * textureColorDif;
}
