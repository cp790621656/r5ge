#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Viewing frustum
//============================================================================================================

class Frustum
{
	float mF[6][4];

public:

	Frustum();

	// Updates the frustum
	void Update (const Matrix44& vp);

	// Returns whether the sphere is visible
	bool IsVisible (const Vector3f& center, float radius = 0.0f) const;

	// Returns whether the box formed by these two points is visible
	bool IsVisible (const Vector3f& v0, const Vector3f& v1) const;

	// Returns whether the specified bounding volume is visible
	inline bool IsVisible (const Bounds& bounds) const
	{
		return (bounds.IsValid() &&
				IsVisible(bounds.GetCenter(), bounds.GetRadius()) &&
				IsVisible(bounds.GetMin(), bounds.GetMax()));
	}

	// Returns whether the transformed bounding volume is visible
	bool IsVisible (const Bounds& bounds, const Vector3f& pos, const Quaternion& rot, float scale) const;

	// Returns whether the transformed bounding volume is visible,
	// and includes the transformed volume in the provided final bounds
	/*bool IncludeIfVisible ( const Bounds&		bounds,
							const Vector3f&		pos,
							const Quaternion&	rot,
							float				scale,
							Bounds&				final ) const;*/
};