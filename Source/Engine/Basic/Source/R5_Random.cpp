#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Random generator constructor -- very simple method of ensuring that every instance of the class will
// generate its own set of random numbers, unless the seed is specified directly.
//============================================================================================================

uint g_counter = 0;

Random::Random() : a(g_counter++), b(B), c(C), d(D) {}