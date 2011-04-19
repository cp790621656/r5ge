#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Bounding sphere + box for quick viewing frustum checks
// Author: Michael Lyashenko
//============================================================================================================

class Bounds
{
	Vector3f			mMin;		// Min point of the bounding box
	Vector3f			mMax;		// Max point
	mutable Vector3f	mCenter;	// Calculated center of the bounds for sphere collision checks
	mutable float		mRadius;	// Calculated radius of the sphere
	mutable bool		mIsDirty;	// Whether the center and radius need to be updated
	bool				mIsValid;	// Whether the bounds are valid

private:

	void _Update() const
	{
		mIsDirty = false;
		mCenter = (mMax + mMin) * 0.5f;
		mRadius = (mMax - mMin).Magnitude();
	}

public:

	Bounds() : mRadius(0.0f), mIsDirty(true), mIsValid(false) {}

	void Set (const Vector3f& min, const Vector3f& max)
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

	void			Clear()				{ mIsValid = false; }
	bool			IsValid()	const	{ return mIsValid; }
	const Vector3f&	GetMin()	const	{ return mMin; }
	const Vector3f&	GetMax()	const	{ return mMax; }
	const Vector3f&	GetCenter() const	{ if (mIsDirty) _Update(); return mCenter; }
	float			GetRadius() const	{ if (mIsDirty) _Update(); return mRadius; }

	bool Contains (const Vector3f& pos) const	{ return (mIsValid && (pos > mMin) && (pos < mMax)); }
	bool Contains (const Bounds& b) const;
	bool Intersects (const Bounds& b) const;

	bool Matches (const Bounds& b) const
	{
		if (!IsValid()) return !b.IsValid();
		float dot0 = (mMin - b.GetMin()).Dot();
		float dot1 = (mMax - b.GetMax()).Dot();
		return (dot0 < 0.0001f) && (dot1 < 0.0001f);
	}

	void Include (const Bounds& b)
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

	void Include (const Vector3f& center, float radius)
	{
		radius *= 0.577350258827f;
		Include(center - radius);
		Include(center + radius);
	}

	// Include the specified vertex into the volume
	void Include (const Vector3f& v);

	// Transform the bounding volume by the specified transformation
	void Transform (const Vector3f& pos, const Quaternion& rot, const Vector3f& scale);

	// Transform the bounding volume by the specified rotation
	void Transform (const Quaternion& rot);
};