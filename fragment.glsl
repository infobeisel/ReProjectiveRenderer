#version 410 core

layout(location = 0 ) out vec4 color;
layout(location = 1 ) out vec4 exchangeBuffer;


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
uniform float eyeSeparation; // in centimeters
uniform mat4 V;
uniform mat4 P;
uniform float width;
uniform float height;
layout(location = 1) in vec4 cameraSpacePos;
uniform sampler2D exchangeBufferSampler;
uniform sampler2D leftImageSampler;
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

   if(zPrepass && eyeIndex == 1) { //right eye z prepass
        //perform reprojection
        //right camera space position + translation resulting from eye separation = left camera space position
        vec4 leftCameraSpacePos = cameraSpacePos + vec4(eyeSeparation ,0.0,0.0,1.0);
        //projection * left camera space position = Clip coordinates
        vec4 clipSpacePosLeftEye = P * leftCameraSpacePos; //NOT between -1 and 1 yet. divde by w.
        float uvSpaceLeftImageXCoord =   clipSpacePosLeftEye.x / clipSpacePosLeftEye.w; // between -1 and 1. NDC.
        uvSpaceLeftImageXCoord += 1.0f ; // between 0 and 2
        uvSpaceLeftImageXCoord *= 0.5f; // between 0 and 1 (if in viewport). is now the x coordinate of this fragment on the left camera image

        vec4 r = texture(exchangeBufferSampler,vec2(uvSpaceLeftImageXCoord,(gl_FragCoord.y / height))); // sample depth value
        float leftEyeCameraSpaceDepth = r.g;
        float rightEyeCameraSpaceDepth = - cameraSpacePos.z / 10000.0f ;
        float d = abs(leftEyeCameraSpaceDepth - rightEyeCameraSpaceDepth); //normalized difference
        float treshold = eyeSeparation /  10000.0f; //normalized threshold

        // pink for unavailable pixels
        if( uvSpaceLeftImageXCoord >= 1.0 || uvSpaceLeftImageXCoord <= 0.0f) {
            color = vec4(1.0,0.0,0.8,1.0);
        } else if(d  > treshold) { //green for fragments that don't pass the depth comparison test
            color = vec4(0.0,(d - treshold)/ treshold,0.0,1.0); //d-t/t =
        } else {
            color = texture(leftImageSampler,vec2(uvSpaceLeftImageXCoord ,(gl_FragCoord.y / height))); // sample the reprojected fragment
       }
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

        //store color
        color = tColour;
        //store depth data
        float depth = - cameraSpacePos.z / 10000.0f ; // z is in negative in opengl camera space. division by far plane to get normalized value
        exchangeBuffer = vec4(0.0f,depth,0.0,1.0);

    }

}
