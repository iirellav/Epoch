#pragma once

class FastNoise;

namespace Epoch
{
	class Noise
	{
	public:
		static void SetSeed(int aSeed);

		static float SimplexNoise(float x, float y);
		static float PerlinNoise(float x, float y);
	};
}
