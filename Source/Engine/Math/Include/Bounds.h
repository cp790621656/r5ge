#pragma once

//============================================================================================================
//              R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Bounding sphere + box for quick viewing frustum checks
//============================================================================================================

class Bounds
{
	Vector3f			mMin;		// Min point of the bounding box
	Vector3f			mMax;		// Max point
	bool				mIsValid;	// Whether the bounds are valid
	mutable Vector3f	mCenter;	// Calculated center of the bounds for sphere collision checks
	mutable float		mRadius;	// Calculated radius of the sphere
	mutable bool		mIsDirty;	// Whether the center and radius need to be updated

private:

	void _Update() const
	{
		mIsDirty = false;
		mCenter = (mMax + mMin) * 0.5f;
		mRadius = (mMax - mMin).Magnitude();
	}

public:

	Bounds() : mRadius(0.0f), mIsValid(false), mIsDirty(true) {}

	inline void Set (const Vector3f& min, const Vector3f& max)
	{
		mMin = min;
		mMax = max;
		mIsValid = true;
		mIsDirty = true;
	}

	void Set (const Vector3f& center, float radius)
	{
		radius *= 0.577350258827f;
		mMin = center - radius;
		mMax = center + radius;
		mIsValid = true;
		mIsDirty = true;
	}

	inline void				Reset()				{ mIsValid = false; }
	inline bool				IsValid()	const	{ return mIsValid; }
	inline const Vector3f&	GetMin()	const	{ return mMin; }
	inline const Vector3f&	GetMax()	const	{ return mMax; }
	inline const Vector3f&	GetCenter() const	{ if (mIsDirty) _Update(); return mCenter; }
	inline float			GetRadius() const	{ if (mIsDirty) _Update(); return mRadius; }
	inline bool Contains (const Vector3f& pos)	{ return (mIsValid && (pos > mMin) && (pos < mMax)); }
	inline bool Contains (const Bounds& b)
	{
		return Contains(b.GetMin()) && Contains(b.GetMax());
	}

	inline void Include (const Bounds& b)
	{
		if (b.IsValid())
		{
			if (mIsValid)
			{
				Include(b.GetMin());
				Include(b.GetMax());
			}
			else *this = b;
		}
	}

	inline void Include (const Vector3f& center, float radius)
	{
		radius *= 0.577350258827f;
		Include(center - radius);
		Include(center + radius);
	}

	// Include the specified vertex into the volume
	void Include (const Vector3f& v);

	// Transform the bounding volume by the specified transformation
	void Transform (const Vector3f& pos, const Quaternion& rot, float scale);
};