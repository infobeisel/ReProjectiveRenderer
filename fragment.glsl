#version 330 core

out vec4 colour;


struct LightSource {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position; //in camera space coordinates
    vec3 direction;
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
};

uniform vec3 cameraWorldPos;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float specularExponent;
uniform float transparency;

uniform mat3 NormalM;
uniform mat4 MV;


uniform int lightCount;
const int maxLightCount = 10;
uniform LightSource lights[maxLightCount];


in vec2 interpolatedUV;
in vec3 interpolatedNormal;
in vec3 interpolatedPos; // in camera space coordinates
in vec3 interpolatedViewDir;
//texture information: in which layer this fragment has to lookup the texture?
uniform float diffuseTextureArrayIndex;
uniform sampler2DArray diffuseSampler;
uniform float ambientTextureArrayIndex;
uniform sampler2DArray ambientSampler;
uniform float specularTextureArrayIndex;
uniform sampler2DArray specularSampler;
uniform float bumpTextureArrayIndex;
uniform sampler2DArray bumpSampler;

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
        float dist = distance(interpolatedPos,lights[i].position);

        float atten = 1.0f / ( lights[i].attenuationConstant + lights[i].attenuationLinear * dist + lights[i].attenuationQuadratic * dist*dist);

        //light direction (point light!)
        vec3 l = normalize(lights[i].position - interpolatedPos); // world coordinate direction
        //vec3 l = vec3(normalize(interpolatedPos -lights[i].position)); // world coordinate direction
        vec3 v = normalize(cameraWorldPos - interpolatedPos); //world coordinate direction
        vec3 n = normalize(interpolatedNormal);

        float NL = dot(n , l);
        //NL is negative -> do not contribute negative values
        if(NL > 0.0) {
            //vec3 rl = reflect(l, n);
            //d−2(d⋅n)n
            vec3 rl = normalize ( -l - 2 * dot(-l , n) * n );
            float RLV = max(dot(rl,v),0.0);
            vec3 d = (Kd * lights[i].diffuse);
            vec3 a = (Ka * lights[i].ambient);
            vec3 s = (Ks * lights[i].specular);
            d *= NL;
            d *= atten;
            a *= atten;
            s *= pow(RLV,specularExponent);
            s *= atten;
            diffContr += vec4(d,1.0);
            ambContr += vec4(a,1.0);
            specContr += vec4(s,1.0);
        }
    }
    vec4 tColour = specContr * textureColorSpec + ambContr * textureColorAmb +  diffContr * textureColorDif;
    tColour.a = transparency;
    if(textureColorSpec.a < 0.5f && textureColorAmb.a < 0.5f && textureColorDif.a < 0.5f ) discard;
    colour = tColour;

}
