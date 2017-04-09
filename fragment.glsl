#version 410 core


//forward declaration & constants
float GetUnocclusionFactor(vec3 worldPosition);

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
extern uniform float FarClippingPlane;


//reprojective parameters
extern uniform float SpecularError;
extern uniform float LodError;
extern uniform float depthThreshold; //threshold for discriminating occluded fragments from reusable fragments

//Stereoscopic related
extern uniform int eyeIndex;
extern uniform vec3 rightCameraWorldPos;
extern uniform mat4 P;
extern uniform mat4 R; //the reprojection matrix
extern uniform float height;extern uniform sampler2DArray bumpSampler;

extern layout(location = 1) in vec4 clipSpacePos; //fragment in camera space coordinates
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


extern uniform int lightCount;
extern uniform LightSource lights[];

extern layout(location = 2) in vec2 interpolatedUV;
extern layout(location = 3) in vec3 interpolatedNormal;
extern layout(location = 0) in vec3 interpolatedPos;
//texture information: in which layer this fragment has to lookup the texture?
extern uniform float diffuseTextureArrayIndex;
extern uniform sampler2DArray diffuseSampler;
extern uniform float ambientTextureArrayIndex;
extern uniform sampler2DArray ambientSampler;
extern uniform float specularTextureArrayIndex;
extern uniform sampler2DArray specularSampler;
extern uniform float bumpTextureArrayIndex;



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
    if(debugMode == 1 && transparency != 0.0f) //debug mode and transparent render pass
        color = vec4(0.0,0.0,1.0,1.0);


    //store depth data
    if(eyeIndex == 0) {
        //calculate specular contribution for right eye in left eye pass, write error in exchange buffer
        //specular error
        bvec3 isSpecular;
        isSpecular = equal(Ks, vec3(0.0,0.0,0.0));
        isSpecular.x = (isSpecular.x  == false || isSpecular.y  == false || isSpecular.z  == false);
        vec3 tSpecError = specContr.rgb - specContrRight.rgb;
        float specError = abs(tSpecError.r)+abs(tSpecError.g)+abs(tSpecError.b);
        if((isSpecular.x && specError > SpecularError))
            exchangeBuffer = 0; //not reprojectable
        else
           exchangeBuffer = 1; // reprojectable
    }
}






void main()
{
   if(transparency == 0.0 && eyeIndex == 1) { //transparent objects get always rerendered
        //perform reprojection
        vec4 clipSpacePosLeftEye = R * clipSpacePos; //NOT between -1 and 1 yet. divde by w.
        float uvSpaceLeftImageXCoord =   clipSpacePosLeftEye.x / clipSpacePosLeftEye.w; // between -1 and 1. NDC.
        uvSpaceLeftImageXCoord += 1.0f ; // between 0 and 2
        uvSpaceLeftImageXCoord *= 0.5f; // between 0 and 1 (if in viewport). is now the x coordinate of this fragment on the left camera image

        vec4 exchangeBufferData = texture(exchangeBufferSampler,vec2(uvSpaceLeftImageXCoord ,(gl_FragCoord.y / height))); // sample depth value
        float reprojectableSpecular = exchangeBufferData.r;

        //calculate depth difference
        float leftEyeCameraSpaceDepth = texture(exchangeBuffer2Sampler, vec2(uvSpaceLeftImageXCoord ,(gl_FragCoord.y / height))).r;

        //linearize depth value
        float p33 = P[2][2];
        float p43 = P[3][2];
        leftEyeCameraSpaceDepth         = ( - p43 / (leftEyeCameraSpaceDepth + p33));
        float rightEyeCameraSpaceDepth  = ( - p43 / (gl_FragCoord.z + p33));


        //normalize
        leftEyeCameraSpaceDepth /= FarClippingPlane;
        rightEyeCameraSpaceDepth /= FarClippingPlane;

        float d = abs( leftEyeCameraSpaceDepth - rightEyeCameraSpaceDepth); //normalized difference. leftEyeCameraSpaceDepth could be negative, absolute

        //calculate over/undersampling error
        float lodErr = max(dFdx(uvSpaceLeftImageXCoord),dFdy(uvSpaceLeftImageXCoord));
        //calculate if outside the view frustum
        bool outsideViewFrustum = uvSpaceLeftImageXCoord >= 1.0 || uvSpaceLeftImageXCoord <= 0.0f;

        bool dontReproject =  (outsideViewFrustum ||
                          d  > depthThreshold
                          || reprojectableSpecular  == 0.0 //spec error too big, see in l. ~186
                          || (lodErr < LodError )                  );
        if(dontReproject) {
            if(debugMode == 0)  color = vec4(0.0,0.0,0.0,0.0);
            else {
                color = vec4(0.0,0.0,1.0,1.0);
            }
        } else {
            color = texture(leftImageSampler,vec2(uvSpaceLeftImageXCoord ,(gl_FragCoord.y / height))); // sample the reprojected fragment
        }

    } else {
        fullRenderPass();

    }



}



