#version 410 core

layout(location = 0 ) out vec4 colour;



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
//Stereoscopic related
uniform bool zPrepass;
uniform int eyeIndex;
uniform float eyeSeparation;
uniform mat4 V;
uniform mat4 P;
uniform float width;
uniform float height;
layout(location = 1) in vec4 cameraSpacePos;
uniform sampler2D reprojectionCoordinateSampler;
uniform sampler2D leftImageSampler;
uniform sampler2D leftImageDepthSampler;
//---------------------
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

layout(location = 2) in vec2 interpolatedUV;
layout(location = 3) in vec3 interpolatedNormal;
layout(location = 0) in vec3 interpolatedPos;
layout(location = 4) in vec3 interpolatedViewDir;
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

    if(zPrepass && eyeIndex == 0) {// left eye z prepass: depth values get written + write reprojected x screen coordinate in buffer
        //reprojection
        //left camera space position + translation resulting from eye separation = right camera space position
        vec4 rightCameraSpacePos = cameraSpacePos + vec4(eyeSeparation,0.0,0.0,1.0);
        //projection * right camera space position = Clip coordinates (where x and y values are screen coordinates)
        vec4 clipSpacePosRightEye = P * rightCameraSpacePos; //NOT between -1 and 1 yet. divde by w.
        float uvSpaceDelta =   clipSpacePosRightEye.x / clipSpacePosRightEye.w; // between -1 and 1
        uvSpaceDelta += 1.0f ; // between 0 and 2
        uvSpaceDelta *= 0.5f; // between 0 and 1
        uvSpaceDelta = (gl_FragCoord.x / width) - uvSpaceDelta; //uvSpaceDelta: the distance which is needed to
        //get from a fragment from the right image to the corresponding fragment on the left image, between -1 and 1

        float error = uvSpaceDelta > -1.0f && uvSpaceDelta < 0.0f ? 0.0f : 1.0f; // uvSpaceDelta is negative, gets clamped to normalized color values (0,1) -> multiple render targets needed.
        colour = vec4(-uvSpaceDelta /*(clipSpacePosRightEye.x + (width/ 2.0f)) / width */,0.0,0.0,1.0);
        //reprojection = uvSpaceDelta;
        //colour = vec4(0.2,0.0,0.0,1.0);
    } else if(zPrepass && eyeIndex == 1) { //right eye z prepass
        //colour = vec4(1.0,1.0,0.0,1.0);
        //watch the deltas
        //colour = texture(reprojectionCoordinateSampler,vec2((gl_FragCoord.x / width),(gl_FragCoord.y / height)));
        colour = texture(reprojectionCoordinateSampler,vec2((gl_FragCoord.x / width),(gl_FragCoord.y / height))); // sample the reprojection distance
        colour = texture(leftImageSampler,vec2((gl_FragCoord.x / width) + colour.r,(gl_FragCoord.y / height))); // sample the reprojected fragment
        //discard;

    } else {

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

}
