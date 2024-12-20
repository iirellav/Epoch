#include "epch.h"
#include "Noise.h"
#include "FastNoise/FastNoise.h"

namespace Epoch
{
	static FastNoise mySimplexFastNoise;
	static FastNoise myPerlinFastNoise;

	void Noise::SetSeed(int aSeed)
	{
		mySimplexFastNoise.SetSeed(aSeed);
		myPerlinFastNoise.SetSeed(aSeed);
	}

	float Noise::SimplexNoise(float x, float y)
	{
		return mySimplexFastNoise.GetNoise(x, y);
	}

	float Noise::PerlinNoise(float x, float y)
	{
		return myPerlinFastNoise.GetNoise(x, y);
	}
}