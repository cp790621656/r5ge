#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Whether the specified bounds fit into current ones
//============================================================================================================

bool Bounds::Contains (const Bounds& b) const
{
	Vector3f min (b.mMin - mMin);
	Vector3f max (mMax - b.mMax);

	return	min.x > -0.00001f &&
			min.y > -0.00001f &&
			min.z > -0.00001f &&
			max.x > -0.00001f &&
			max.y > -0.00001f &&
			max.z > -0.00001f;
}

//============================================================================================================
// Whether the specified bounds intersect with the current ones
//============================================================================================================

bool Bounds::Intersects (const Bounds& b) const
{
	// Containment test
	if (b.mMin.x >= mMin.x &&
		b.mMax.x <= mMax.x &&
		b.mMin.y >= mMin.y &&
		b.mMax.y <= mMax.y &&
		b.mMin.z >= mMin.z &&
		b.mMax.z <= mMax.z) return true;

	// Intersection test
	return !(mMax.x < b.mMin.x  ||
			 mMin.x > b.mMax.x  ||
			 mMax.y < b.mMin.y  ||
			 mMin.y > b.mMax.y  ||
			 mMax.z < b.mMin.z  ||
			 mMin.z > b.mMax.z);
}

//============================================================================================================
// Include the specified vertex into the volume
//============================================================================================================
// 6 floating-point comparisons
//============================================================================================================

void Bounds::Include (const Vector3f& v)
{
	if (!mIsValid)
	{
		mMin = v;
		mMax = v;

		mIsDirty = true;
		mIsValid = true;
	}
	else
	{
		if (mMin.x > v.x) { mMin.x = v.x; mIsDirty = true; }
		if (mMin.y > v.y) { mMin.y = v.y; mIsDirty = true; }
		if (mMin.z > v.z) { mMin.z = v.z; mIsDirty = true; }

		if (mMax.x < v.x) { mMax.x = v.x; mIsDirty = true; }
		if (mMax.y < v.y) { mMax.y = v.y; mIsDirty = true; }
		if (mMax.z < v.z) { mMax.z = v.z; mIsDirty = true; }
	}
}

//============================================================================================================
// Transform the bounding volume by the specified transformation
//============================================================================================================

void Bounds::Transform (const Vector3f& pos, const Quaternion& rot, float scale)
{
	if (mIsValid)
	{
		if (rot.IsIdentity())
		{
			mMin = pos + mMin * scale;
			mMax = pos + mMax * scale;
			mIsDirty = true;
		}
		else
		{
			scale *= 0.5f;

			Vector3f dir0 ( (mMax - mMin) * scale);		// Top-right
			Vector3f dir1 ( dir0.x, dir0.y, -dir0.z);	// Bottom-right
			Vector3f dir2 (-dir0.x, dir0.y,  dir0.z);	// Top-left
			Vector3f dir3 (-dir0.x, dir0.y, -dir0.z);	// Bottom-left

			dir0 *= rot;
			dir1 *= rot;
			dir2 *= rot;
			dir3 *= rot;

			Clear();

			mCenter  = (mMax + mMin) * scale;
			mCenter *= rot;
			mCenter += pos;

			Include(mCenter + dir0);
			Include(mCenter + dir1);
			Include(mCenter + dir2);
			Include(mCenter + dir3);

			Include(mCenter - dir0);
			Include(mCenter - dir1);
			Include(mCenter - dir2);
			Include(mCenter - dir3);
		}
	}
}

//============================================================================================================
// Transform the bounding volume by the specified rotation
//============================================================================================================

void Bounds::Transform (const Quaternion& rot)
{
	if (mIsValid)
	{
		Vector3f dir0 ( (mMax - mMin) * 0.5f);		// Top-right
		Vector3f dir1 ( dir0.x, dir0.y, -dir0.z);	// Bottom-right
		Vector3f dir2 (-dir0.x, dir0.y,  dir0.z);	// Top-left
		Vector3f dir3 (-dir0.x, dir0.y, -dir0.z);	// Bottom-left

		dir0 *= rot;
		dir1 *= rot;
		dir2 *= rot;
		dir3 *= rot;

		Clear();

		mCenter = (mMax + mMin) * 0.5f;
		mCenter *= rot;

		Include(mCenter + dir0);
		Include(mCenter + dir1);
		Include(mCenter + dir2);
		Include(mCenter + dir3);

		Include(mCenter - dir0);
		Include(mCenter - dir1);
		Include(mCenter - dir2);
		Include(mCenter - dir3);
	}
}