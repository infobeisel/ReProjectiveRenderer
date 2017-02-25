#version 410 core
// Shadow_Use_PCF.glsl

//
//

//forward declarations

extern uniform vec2 Poisson25[25];
extern uniform vec2 Poisson64[64];
extern uniform vec2 Poisson128[128];
uint InitializeRandomSeed(uint idx);
vec2 RotateVec2(vec2 baseVec, float amt);
uint Random_Xorshift(inout uint seed);
float Random(inout uint seed);



uniform sampler2DShadow ShadowMap;

uniform vec3 ShadowCameraPosition;
uniform mat4 ShadowCameraViewProjectionMatrix;

uniform uvec2 ShadowMapSize;

const float SeedMultiplier = 9999;

const float ShadowBias = 5e-4f;

const int CountShadowSamples = 25;

float GetUnocclusionFactor(vec3 worldPosition)
{
	vec3 projectedPos = (ShadowCameraViewProjectionMatrix * vec4(worldPosition, 1.0)).xyz;
	vec3 nProjectedPos = (projectedPos + 1.0) * 0.5;

	vec2 texCoord = nProjectedPos.xy;
	float depth = nProjectedPos.z - ShadowBias;

	if (texCoord.x < 0.0 || texCoord.x > 1.0 || texCoord.y < 0.0 || texCoord.y > 1.0)
	{
		return 1.0;
	}
	uint seed = InitializeRandomSeed(uint(gl_FragCoord.y * SeedMultiplier + gl_FragCoord.x));
	float rotationFrame = Random(seed);
	vec2 pcfRadius = 2.0 / vec2(ShadowMapSize);
	float unocclusion = 0.0;
	for (int i = 0; i < CountShadowSamples; ++i)
	{
		vec2 offset = Poisson25[i] * pcfRadius;
		offset = RotateVec2(offset, rotationFrame);
                float tmp = texture(ShadowMap, vec3(texCoord + offset, depth)); // texture with shadow maps returns float
                unocclusion += tmp;
	}
	unocclusion /= CountShadowSamples;

	return unocclusion;
}

