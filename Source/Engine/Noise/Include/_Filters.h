#pragma once

//============================================================================================================
// Built-in noise generation methods
//============================================================================================================
// - Simple		--	Generates simple random noise
// - Fractal	--	Generates fractal noise
//					Parameter 0: Number of octaves
//					Parameter 1: Upper mirror threshold during noise combination for ridged noise (default: 1)
//					Parameter 2: Additional octave strength (default: 1.0 [100%])
// - Perlin		--	Generates Perlin noise
//					Parameter 0: Number of octaves
//					Parameter 1: Upper mirror threshold during noise combination for ridged noise (default: 1)
//					Parameter 2: Contribution smoothness between octaves (default: 1.0 [100%])
// Author: Michael Lyashenko
//============================================================================================================
// Built-in essential filters
//============================================================================================================
// - Normalize	--	Normalizes the noise, bringing it into 0 to 1 range
// - Blur		--	Weighted Gaussian blur filter
//					Parameter 0: Number of passes (default: 1)
//					Parameter 1: Lowest boundary (below it blur will be 100% strength) (default: 1)
//					Parameter 2: Highest boundary (above it blur will not happen) (default: 1)
// - Power		--	Power of # filter
//					Parameter 0: Power (default: 2)
// - Sqrt		--	Square root filter
// - Add		--	Addition filter
//					Parameter 0: Value to add
// - Multiply	--	Multiplication filter
//					Parameter 0: Value to multiply by
// - Round		--	Rounds the noise values down to specified precision
//					Parameter 0: Precision (0.1 means values will be 0, 0.1, 0.2, 0.3, etc)
// - Clamp		--	Clamps the values to be within specified range
//					Parameter 0: Lowest boundary
//					Parameter 1: Highest boundary
// - Mirror		--	Mirrors all values that exceed the specified range (bounce effect)
//					Parameter 0: Lowest boundary
//					Parameter 1: Highest boundary
// - Erode		--	Thermal erosion filter
//					Parameter 0: Number of iterations (default: 10)
//					Parameter 1: Strength of erosion (default: 0.5)
//					Parameter 2: Deposition amount (default: 0.0)
//============================================================================================================

#define FILTER(name)	void name(	Random&						r,			\
									Noise::FloatPtr&			data,		\
									Noise::FloatPtr&			aux,		\
									const Vector2i&				size,		\
									const Noise::Parameters&	params,		\
									bool						seamless )

namespace R5
{
	namespace Filter
	{
		FILTER(Simple);
		FILTER(Fractal);
		FILTER(Perlin);
		FILTER(Normalize);
		FILTER(Blur);
		FILTER(Power);
		FILTER(Sqrt);
		FILTER(Add);
		FILTER(Multiply);
		FILTER(Round);
		FILTER(Clamp);
		FILTER(Mirror);
		FILTER(Erode);
	};
};