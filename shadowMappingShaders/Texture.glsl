// Texture.glsl

#ifndef _TEXTURE_GLSL_H_
#define _TEXTURE_GLSL_H_

#if (USE_BINDLESS_TEXTURE != 0)

#define DeclareTexture2D(textureName) uniform uvec2 textureName
#define DeclareTexture2DInteger(textureName) uniform uvec2 textureName
#define DeclareTexture2DUnsigned(textureName) uniform uvec2 textureName
#define DeclareTexture2DShadow(textureName) uniform uvec2 textureName
#define DeclareTextureCube(textureName) uniform uvec2 textureName
#define SampleTexture2D(textureName, coordinate) texture(sampler2D(textureName), coordinate)
#define SampleTexture2DLOD(textureName, coordinate, lod) textureOffset(sampler2D(textureName), coordinate, ivec2(0.0, 0.0), lod)
#define SampleTexture2DInteger(textureName, coordinate) texture(isampler2D(textureName), coordinate)
#define SampleTexture2DUnsigned(textureName, coordinate) texture(usampler2D(textureName), coordinate)
#define SampleTexture2DShadow(textureName, coordinate) texture(sampler2DShadow(textureName), coordinate)
#define SampleTextureCube(textureName, coordinate) texture(samplerCube(textureName), coordinate)

#else

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

#endif

#endif