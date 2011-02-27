#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Random generator constructor -- very simple method of ensuring that every instance of the class will
// generate its own set of random numbers, unless the seed is specified directly.
//============================================================================================================

uint g_counter = 0;

Random::Random() { SetSeed(g_counter++); }

//============================================================================================================
// Resets the random seed to the specified value
//============================================================================================================

void Random::SetSeed (uint val)
{
	a = val;
	b = val ^ B;
	c = (val >> 5) ^ C;
	d = (val >> 7) ^ D;

	// Fully seed the generator
	for (uint i = 0; i < 4; ++i) a = GenerateUint();
}