#include "../Include/_All.h"

namespace R5
{
//============================================================================================================
// Find rotation angle and arbitrary axis that would rotate two vectors to match another set of vectors
//============================================================================================================

void GetAxisAngle(const Vector3f& vA1, const Vector3f& vA2, const Vector3f& vB1, const Vector3f& vB2, Vector3f& axis, float &radAngle)
{
	Vector3f v1(vA2 - vA1);
	Vector3f v2(vB2 - vB1);
	
	// If v1 is too close to 0, we need to eliminate at least one axis
	if (!v1)
	{
		// If both are too close to zero, we don't need to rotate anything
		if (!v2)
		{
			axis.Set(0, 0, 1);
			radAngle = 0;
			return;
		}
		
		// Otherwise the axis of rotation is obvious
		axis = vA1;
		
		// If the resulted axis is zero, a new one is needed
		if (!axis) axis = Normalize(vA1 - vB1);

		// Can't use A1/A2 because there's no angle between them
		v1 = vB2 - axis * vB2.Dot(axis);
		v2 = vB1 - axis * vB1.Dot(axis);
	}
	// Same as above, but with the other axis
	else if (!v2)
	{
		axis = vB1;
		if (!axis) axis = Normalize(vA1 - vB1);
		v1 = vA2 - axis * vA2.Dot(axis);
		v2 = vA1 - axis * vA1.Dot(axis);
	}
	// If both vectors are significant, calculate a new axis of rotation
	else
	{
		axis = Normalize(Cross(v1, v2));
		if (!axis) axis = Normalize(vA1 - vB1);
		v1 = vA2 - axis * vA2.Dot(axis);
		v2 = vA1 - axis * vA1.Dot(axis);
	}

	// The third vector is calculated
	Vector3f vCross(Cross(v1, v2));
	
	// Find the actual angle
	radAngle = atan2f(Magnitude(vCross), v1.Dot(v2));
	
	// Flip it if necessary
	if (vCross.Dot(axis) > 0) radAngle = -radAngle;
}
}; // namespace R5