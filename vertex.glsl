#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;


uniform mat4 MVP;
uniform mat4 M;
uniform mat4 MV;
uniform mat4 VP;
uniform mat3 NormalM;

out vec3 interpolatedPos;
out vec2 interpolatedUV;
out vec3 interpolatedNormal;
out vec3 interpolatedViewDir;
void main() {
    gl_Position = MVP * vec4(position,1.0);
    interpolatedPos = vec3(M * vec4(position,1.0)); //camera space coordinates

    interpolatedUV = uv;
    interpolatedNormal = normalize(NormalM * normal);
    //interpolatedNormal = normalize( normal);
    //interpolatedNormal = normalize(NormalM * vec3(0.0,1.0,0.0));
    interpolatedViewDir =  normalize(  vec3(-interpolatedPos));


}
