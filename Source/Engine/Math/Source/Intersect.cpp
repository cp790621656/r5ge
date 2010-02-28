#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Ray-sphere intersection test
//============================================================================================================

bool Intersect::RaySphere (const Vector3f& origin, const Vector3f& dir, const Vector3f& center, float radius)
{
	Vector3f diff (origin - center);
	float a = diff.Dot(dir);
	float b = (a * a) + (radius * radius) - diff.Dot();
	return Float::IsPositive(b);
	// Distance if 'b' is positive: (-a - Float::Sqrt(b));
}

//============================================================================================================
// Ray-bounds intersection test
//============================================================================================================

bool Intersect::RayBounds (const Vector3f& origin, const Vector3f& dir, const Bounds& bounds)
{
	const Vector3f& max		= bounds.GetMax();
	const Vector3f& center	= bounds.GetCenter();

	Vector3f ext  (max		- center);
	Vector3f diff (origin	- center);

	if (Float::Abs(diff.x) > ext.x && Float::IsProductPositive(diff.x, dir.x)) return false;
	if (Float::Abs(diff.y) > ext.y && Float::IsProductPositive(diff.y, dir.y)) return false;
	if (Float::Abs(diff.z) > ext.z && Float::IsProductPositive(diff.z, dir.z)) return false;

	Vector3f adir (Float::Abs(dir.x), Float::Abs(dir.y), Float::Abs(dir.z));

	float a = dir.y * diff.z - dir.z * diff.y;
	float b = ext.y * adir.z + ext.z * adir.y;

	if (Float::Abs(a) > b) return false;
	
	a = dir.z * diff.x - dir.x * diff.z;
	b = ext.x * adir.z + ext.z * adir.x;

	if (Float::Abs(a) > b) return false;
	
	a = dir.x * diff.y - dir.y * diff.x;
	b = ext.x * adir.y + ext.y * adir.x;

	return !(Float::Abs(a) > b);
}