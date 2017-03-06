#version 410 core



layout(location = 0 ) out vec4 color;
//layout(location = 1 ) out vec4 exchangeBuffer;
layout(location = 1 ) out float exchangeBuffer;
layout(location = 2 ) out float exchangeBuffer2;



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
uniform float NearClippingPlane;
uniform float FarClippingPlane;

//reprojective parameters
uniform float SpecularError;
uniform float LodError;
uniform float depthThreshold; //threshold for discriminating occluded fragments from reusable fragments



//Stereoscopic related
uniform int eyeIndex;
uniform float eyeSeparation; // in centimeters
uniform vec3 rightCameraWorldPos;
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








