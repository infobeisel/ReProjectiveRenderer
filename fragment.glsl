#version 410 core

const float NearClippingPlane = 0.3f;
const float FarClippingPlane = 10000.0f;
const float MY_GL_TEXTURE_MAX_LOD = 1000.0f;
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

uniform int debugMode;

//Stereoscopic related
uniform int eyeIndex;
uniform float eyeSeparation; // in centimeters
uniform vec3 rightCameraWorldPos;
uniform float depthThreshold; //threshold for discriminating occluded fragments from reusable fragments
uniform mat4 V;
uniform mat4 P;
uniform float width;
uniform float height;
layout(location = 1) in vec4 cameraSpacePos; //fragment in camera space coordinates
uniform sampler2D exchangeBufferSampler;
uniform sampler2D exchangeBuffer2Sampler;
uniform sampler2D leftImageSampler;
//---------------------
uniform vec3 cameraWorldPos; // the camera's world position

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float specularExponent;
uniform float transparency;

uniform mat3 NormalM;
uniform mat4 MV;



uniform int lightCount;
const int maxLightCount = 20;
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

            //calculate specular contribution for right eye in left eye pass, write error in exchange buffer
            if(eyeIndex == 0) {
                float RLVRight = max(dot(rl,vRight),0.0);
                vec3 s = (Ks * lights[i].specular);
                s *= pow(RLVRight,specularExponent);
                s *= atten;
                specContrRight += vec4(s,1.0);
            }

        }
    }
    vec4 tColour = specContr * textureColorSpec + ambContr * textureColorAmb +  diffContr * textureColorDif;
    tColour.a = transparency;
    if(textureColorSpec.a < 0.5f && textureColorAmb.a < 0.5f && textureColorDif.a < 0.5f ) discard;

    //store color
    color = tColour;


    //store depth data
    if(eyeIndex == 0) {
        float depth = - cameraSpacePos.z / FarClippingPlane ; // z is in negative in opengl camera space. division by far plane to get normalized value

        vec2 lod = textureQueryLod(diffuseSampler,vec2(interpolatedUV.x  ,interpolatedUV.y));

        //calculate specular contribution for right eye in left eye pass, write error in exchange buffer
        vec3 tSpecError = specContr.rgb - specContrRight.rgb;
        float specError = abs(tSpecError.r)+abs(tSpecError.g)+abs(tSpecError.b);

        exchangeBuffer = vec4(0.0,lod.y,specError,depth);
    }
}






void main() {

   if(transparency == 0.0 && eyeIndex == 1) { //transparent objects get always rerendered
        //perform reprojection
        //right camera space position + translation resulting from eye separation = left camera space position
        vec4 leftCameraSpacePos = cameraSpacePos + vec4(eyeSeparation ,0.0,0.0,1.0);
        //projection * left camera space position = Clip coordinates
        vec4 clipSpacePosLeftEye = P * leftCameraSpacePos; //NOT between -1 and 1 yet. divde by w.
        float uvSpaceLeftImageXCoord =   clipSpacePosLeftEye.x / clipSpacePosLeftEye.w; // between -1 and 1. NDC.
        uvSpaceLeftImageXCoord += 1.0f ; // between 0 and 2
        uvSpaceLeftImageXCoord *= 0.5f; // between 0 and 1 (if in viewport). is now the x coordinate of this fragment on the left camera image

        vec4 exchangeBufferData = texture(exchangeBufferSampler,vec2(uvSpaceLeftImageXCoord ,(gl_FragCoord.y / height))); // sample depth value

        //calculate depth difference
        float leftEyeCameraSpaceDepth = exchangeBufferData.a;
        float rightEyeCameraSpaceDepth = - cameraSpacePos.z / FarClippingPlane ;
        float d = abs(leftEyeCameraSpaceDepth - rightEyeCameraSpaceDepth); //normalized difference

        //specular error
        bvec3 isSpecular;
        isSpecular = equal(Ks, vec3(0.0,0.0,0.0));
        isSpecular.x = (isSpecular.x  == false || isSpecular.y  == false || isSpecular.z  == false);
        float specError = exchangeBufferData.b;

        //calculate if outside the view frustum
        bool outsideViewFrustum = uvSpaceLeftImageXCoord >= 1.0 || uvSpaceLeftImageXCoord <= 0.0f;

        //calculate over/undersampling error
        float lodReprojected = exchangeBufferData.g;
        vec2 lod = textureQueryLod(diffuseSampler,vec2(interpolatedUV.x  ,interpolatedUV.y));
        float lodError = abs(lodReprojected - lod.y);

        if(outsideViewFrustum
         || d  > depthThreshold
         || (isSpecular.x && specError > 0.001f)
         || (lodError > 0.5f)                  ) {

            if(debugMode == 1) fullRenderPass();
            else {
                if        (outsideViewFrustum) { // pink for unavailable pixels
                    color = vec4(1.0,0.0,0.8,1.0);
                } else if (d  > depthThreshold) { //green for fragments that don't pass the depth comparison test
                    color = vec4(0.0,(d - depthThreshold)/ depthThreshold,0.0,1.0); //d-t/t =
                } else if ((isSpecular.x && specError > 0.001f)) { // if specular,blue
                    color = vec4(0.0,0.0,1.0,1.0);
                } else if ((lodError > 0.5f)) {// undersampled areas : yellow
                    color = vec4(1.0,1.0,0.0,1.0);
                }
            }
        } else {
            color = texture(leftImageSampler,vec2(uvSpaceLeftImageXCoord ,(gl_FragCoord.y / height))); // sample the reprojected fragment
        }

    } else {
        fullRenderPass();

    }

}



