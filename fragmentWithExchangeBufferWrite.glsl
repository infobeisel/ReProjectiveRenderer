#version 410 core

//forward declaration & constants
float GetUnocclusionFactor(vec3 worldPosition);
const float NearClippingPlane = 0.3f;
const float FarClippingPlane = 1000.0f;
const float MY_GL_TEXTURE_MAX_LOD = 1000.0f;

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



extern layout(location = 0 ) out vec4 color;
//layout(location = 1 ) out vec4 exchangeBuffer;
extern layout(location = 1 ) out float exchangeBuffer;




extern uniform int debugMode;

//Stereoscopic related
extern uniform int eyeIndex;
extern uniform float eyeSeparation; // in centimeters
extern uniform vec3 rightCameraWorldPos;
extern uniform float depthThreshold; //threshold for discriminating occluded fragments from reusable fragments
extern uniform mat4 V;
extern uniform mat4 P;
extern uniform float width;
extern uniform float height;
extern layout(location = 1) in vec4 cameraSpacePos; //fragment in camera space coordinates
extern uniform sampler2D exchangeBufferSampler;
extern uniform sampler2D exchangeBuffer2Sampler;
extern uniform sampler2D leftImageSampler;
//---------------------
extern uniform vec3 cameraWorldPos; // the camera's world position

extern uniform vec3 Ka;
extern uniform vec3 Kd;
extern uniform vec3 Ks;
extern uniform float specularExponent;
extern uniform float transparency;

extern uniform mat3 NormalM;
extern uniform mat4 MV;



extern uniform int lightCount;
extern uniform LightSource lights[];

extern layout(location = 2) in vec2 interpolatedUV;
extern layout(location = 3) in vec3 interpolatedNormal;
extern layout(location = 0) in vec3 interpolatedPos;
extern layout(location = 4) in vec3 interpolatedViewDir;
//texture information: in which layer this fragment has to lookup the texture?
extern uniform float diffuseTextureArrayIndex;
extern uniform sampler2DArray diffuseSampler;
extern uniform float ambientTextureArrayIndex;
extern uniform sampler2DArray ambientSampler;
extern uniform float specularTextureArrayIndex;
extern uniform sampler2DArray specularSampler;
extern uniform float bumpTextureArrayIndex;
extern uniform sampler2DArray bumpSampler;







void fullRenderPass() {

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

    //calculate specular contribution for right eye in left eye pass, write error in exchange buffer
    vec3 vRight = normalize(rightCameraWorldPos - interpolatedPos); //world coordinate direction
    vec4 specContrRight = vec4(0.0);



    //treat them as point lights
    for(int i = 0; i < lightCount; i++) {
        //lights
        float dist = distance(interpolatedPos,lights[i].position);


        float atten = 1.0f;
        vec3 l = -lights[i].direction; // world coordinate direction, from surface poitn to light source
        float unocclusion = 1.0f;

        //point light
        if(lights[i].direction.x == 0.0f && lights[i].direction.y == 0.0f && lights[i].direction.z == 0.0f) {
            //light direction (point light!)
            l = normalize(lights[i].position - interpolatedPos); // world coordinate direction
            atten = 1.0f / ( lights[i].attenuationConstant + lights[i].attenuationLinear * dist + lights[i].attenuationQuadratic * dist*dist);
        } else {//directional light:
            //shadow map sample
            unocclusion = GetUnocclusionFactor(interpolatedPos);
        }

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
            d *= unocclusion;
            a *= atten;
            a *= unocclusion;
            s *= pow(RLV,specularExponent);
            s *= atten;
            s *= unocclusion;

            diffContr += vec4(d,1.0);
            ambContr += vec4(a,1.0);
            specContr += vec4(s,1.0);

            //calculate specular contribution for right eye in left eye pass, write error in exchange buffer
            if(eyeIndex == 0) {
                float RLVRight = max(dot(rl,vRight),0.0);
                vec3 s = (Ks * lights[i].specular);
                s *= pow(RLVRight,specularExponent);
                s *= atten;
                s *= unocclusion;
                specContrRight += vec4(s,1.0);
            }

        }
    }

    vec4 tColour = vec4(Ka,1.0) * textureColorDif +  diffContr * textureColorDif + specContr * textureColorSpec;

    tColour.a = transparency;
    //if(textureColorSpec.a < 0.5f && textureColorAmb.a < 0.5f && textureColorDif.a < 0.5f ) discard;

    //store color
    color = tColour;


    //store depth data
    if(eyeIndex == 0) {
        float depth = - cameraSpacePos.z / FarClippingPlane ; // z is in negative in opengl camera space. division by far plane to get normalized value
        //calculate specular contribution for right eye in left eye pass, write error in exchange buffer
        //specular error
        bvec3 isSpecular;
        isSpecular = equal(Ks, vec3(0.0,0.0,0.0));
        isSpecular.x = (isSpecular.x  == false || isSpecular.y  == false || isSpecular.z  == false);
        vec3 tSpecError = specContr.rgb - specContrRight.rgb;
        float specError = abs(tSpecError.r)+abs(tSpecError.g)+abs(tSpecError.b);
        if((isSpecular.x && specError > 0.003f))
            exchangeBuffer = 0; //not reprojectable
        else
           exchangeBuffer = 1; // reprojectable
    }
}






void main()
{
    fullRenderPass();
}



