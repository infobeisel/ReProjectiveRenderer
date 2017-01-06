#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;


uniform mat4 MVP;

out vec2 interpolatedUV;
//out vec3 interpolatedNormal;
void main() {
    gl_Position = MVP * vec4(position,1.0);
    interpolatedUV = uv;
}
