#version 410 core
layout(location = 0 ) out vec4 color;
uniform float width;
uniform float height;
uniform sampler2D rightImageColor;
/**
  * copies the color buffer to the stencil buffer:
  * fills the stencil buffer by discarding or keeping fragments. reads values from color buffer
  *
  */
void main()
{
    vec4 c =  texture(rightImageColor,vec2(gl_FragCoord.x / width  ,gl_FragCoord.y / height));
    bool t = c.r == 0.0f && c.g == 0.0f && c.b == 0.0f && c.a == 0.0f;
    if(t) discard;
}



