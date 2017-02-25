#version 410 core
// Texture.glsl


#define DeclareTexture2D(textureName) uniform sampler2D textureName
#define DeclareTexture2DInteger(textureName) uniform isampler2D textureName
#define DeclareTexture2DUnsigned(textureName) uniform usampler2D textureName
#define DeclareTexture2DShadow(textureName) uniform sampler2DShadow textureName
#define DeclareTextureCube(textureName) uniform samplerCube textureName
#define SampleTexture2D(textureName, coordinate) texture(textureName, coordinate)
#define SampleTexture2DLOD(textureName, coordinate, lod) textureOffset(textureName, coordinate, ivec2(0.0, 0.0), lod)
#define SampleTexture2DInteger(textureName, coordinate) texture(textureName, coordinate)
#define SampleTexture2DUnsigned(textureName, coordinate) texture(textureName, coordinate)
#define SampleTexture2DShadow(textureName, coordinate) texture(textureName, coordinate)
#define SampleTextureCube(textureName, coordinate) texture(textureName, coordinate)
