#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================

namespace Intersect
{
	bool RaySphere (const Vector3f& origin, const Vector3f& dir, const Vector3f& center, float radius);
	bool RayBounds (const Vector3f& origin, const Vector3f& dir, const Bounds& bounds);
	bool BoundsSphere (const Bounds& bounds, const Vector3f& pos, float radius);

	inline bool BoundsPos (const Bounds& bounds, const Vector3f& pos) { return bounds.Contains(pos); }
};