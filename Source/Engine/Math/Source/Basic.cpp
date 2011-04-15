#include "../Include/_All.h"

//============================================================================================================
// Rounds the float down to a specific precision, so 0.39 with precision of 0.25 becomes 0.5
//============================================================================================================

float R5::Float::Round (float val, const float& precision)
{
	bool sign = Float::IsPositive(val);
	val = Float::Abs(val);

	float step = Float::Floor(val);
	while (step < val) step += precision;
	float half = step - (precision * 0.5f);

	if (half > val) step = step - precision;
	return (sign) ? step : -step;
}