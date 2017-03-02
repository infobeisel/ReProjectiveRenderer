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







void main()
{
        //perform reprojection
        //right camera space position + translation resulting from eye separation = left camera space position
        vec4 leftCameraSpacePos = cameraSpacePos + vec4(eyeSeparation ,0.0,0.0,1.0);
        //projection * left camera space position = Clip coordinates
        vec4 clipSpacePosLeftEye = P * leftCameraSpacePos; //NOT between -1 and 1 yet. divde by w.
        float uvSpaceLeftImageXCoord =   clipSpacePosLeftEye.x / clipSpacePosLeftEye.w; // between -1 and 1. NDC.
        uvSpaceLeftImageXCoord += 1.0f ; // between 0 and 2
        uvSpaceLeftImageXCoord *= 0.5f; // between 0 and 1 (if in viewport). is now the x coordinate of this fragment on the left camera image

        vec4 exchangeBufferData = texture(exchangeBufferSampler,vec2(uvSpaceLeftImageXCoord ,(gl_FragCoord.y / height))); // sample depth value
        float reprojectableSpecular = exchangeBufferData.r;

        //calculate depth difference
        float leftEyeCameraSpaceDepth = texture(exchangeBuffer2Sampler, vec2(uvSpaceLeftImageXCoord ,(gl_FragCoord.y / height))).r;
        //linearize depth values
        float p12 = P[2][3];
        float p11 = - P[2][2];
        leftEyeCameraSpaceDepth         = - (p12 / ( leftEyeCameraSpaceDepth - p11));
        float rightEyeCameraSpaceDepth  = - (p12 / ( gl_FragCoord.z - p11));
        //normalize
        leftEyeCameraSpaceDepth /= -FarClippingPlane;
        rightEyeCameraSpaceDepth /= -FarClippingPlane;

        float d = abs( leftEyeCameraSpaceDepth - rightEyeCameraSpaceDepth); //normalized difference. leftEyeCameraSpaceDepth could be negative, absolute

        //calculate if outside the view frustum
        bool outsideViewFrustum = uvSpaceLeftImageXCoord >= 1.0 || uvSpaceLeftImageXCoord <= 0.0f;

        //calculate over/undersampling error
        float lodError = max(dFdx(uvSpaceLeftImageXCoord),dFdy(uvSpaceLeftImageXCoord));


        bool dontReproject =  (outsideViewFrustum
                          || d  > depthThreshold
                          || reprojectableSpecular  == 0.0 //spec error too big, see in l. ~186
                          || (lodError < 0.00025f)                  );
        if(dontReproject) {
            if(debugMode == 1) discard;// color = vec4(0.0,1.0,0.0,1.0);//fullRenderPass();
            else {
               /*if        (outsideViewFrustum) { // pink for unavailable pixels
                    color = vec4(1.0,0.0,0.8,1.0);
                } else if (leftEyeCameraSpaceDepth < 0.0) { // if specular,blue
                    color = vec4(0.0,0.0,1.0,1.0);
                } else if (d  > depthThreshold) { //green for fragments that don't pass the depth comparison test
                    color = vec4(0.0,1.0,0.0,1.0); //d-t/t =
                } else if ((lodError <  0.00025f)) {// undersampled areas : yellow
                    color = vec4(1.0,1.0,0.0,1.0);
                }*/
                color = vec4(0.0,1.0,0.0,1.0);
            }
        } else {
            color = texture(leftImageSampler,vec2(uvSpaceLeftImageXCoord ,(gl_FragCoord.y / height))); // sample the reprojected fragment
        }




}



