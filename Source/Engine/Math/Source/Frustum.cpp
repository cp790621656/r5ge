#include "../Include/_All.h"
using namespace R5;

//#define FULL_TRANSFORM

//============================================================================================================

Frustum::Frustum()
{
	for (uint plane = 0; plane < 6; ++plane)
		for (uint i = 0; i < 4; ++i)
			mF[plane][i] = 0.0f;
}

//============================================================================================================
// Updates the frustum
//============================================================================================================

void Frustum::Update (const Matrix44& vp)
{
	uint add = 0;

	for (uint plane = 0; plane < 6; ++plane)
	{
		float* f (mF[plane]);

		if (plane % 2 == 0)
		{
			// Even sides: Left (0), bottom (2), front (4)
			f[0] = vp[ 3] + vp[add     ];
			f[1] = vp[ 7] + vp[add +  4];
			f[2] = vp[11] + vp[add +  8];
			f[3] = vp[15] + vp[add + 12];
		}
		else
		{
			// Odd sides: Right (1), top (3), back (5)
			f[0] = vp[ 3] - vp[add     ];
			f[1] = vp[ 7] - vp[add +  4];
			f[2] = vp[11] - vp[add +  8];
			f[3] = vp[15] - vp[add + 12];

			++add;
		}

		// Normalize the plane
		float factor = 1.0f / Float::Sqrt( f[0] * f[0] + f[1] * f[1] + f[2] * f[2] );
		f[0] *= factor;
		f[1] *= factor;
		f[2] *= factor;
		f[3] *= factor;
	}
}

//============================================================================================================
// Returns whether the sphere is visible
// Up to 42 floating-point operations
//============================================================================================================

bool Frustum::IsVisible (const Vector3f& v, float radius) const
{
	for (uint plane = 0; plane < 6; ++plane)
	{
		const float* f (mF[plane]);
		if (v.x * f[0] +
			v.y * f[1] +
			v.z * f[2] + f[3] < -radius)
			return false;
	}
	return true;
}

//============================================================================================================
// Returns whether the box formed by these two points is visible
// Up to 180 floating-point operations
//============================================================================================================

bool Frustum::IsVisible (const Vector3f& v0, const Vector3f& v1) const
{
	float d, x0, x1, y0, y1, z0, z1;

	for (uint plane = 0; plane < 6; ++plane )
	{
		const float* f (mF[plane]);

		d  = -f[3];
		x0 = v0.x * f[0];
		x1 = v1.x * f[0];
		y0 = v0.y * f[1];
		y1 = v1.y * f[1];
		z0 = v0.z * f[2];
		z1 = v1.z * f[2];

		if (x0 + y0 + z0 < d &&
			x1 + y0 + z0 < d &&
			x0 + y1 + z0 < d &&
			x1 + y1 + z0 < d &&
			x0 + y0 + z1 < d &&
			x1 + y0 + z1 < d &&
			x0 + y1 + z1 < d &&
			x1 + y1 + z1 < d )
		{
			return false;
		}
	}
	return true;
}

//============================================================================================================
// Returns whether the transformed bounding volume is visible
//============================================================================================================

bool Frustum::IsVisible (const Bounds& bounds, const Vector3f& pos, const Quaternion& rot, float scale) const
{
	if ( bounds.IsValid() )
	{
		bool identity = rot.IsIdentity();
		Vector3f center ( pos + (identity ? bounds.GetCenter() : bounds.GetCenter() * rot) * scale );
		float radius ( bounds.GetRadius() * scale );

		if (IsVisible(center, radius))
		{
			if (identity)
			{
				Vector3f min ( pos + bounds.GetMin() * scale );
				Vector3f max ( pos + bounds.GetMax() * scale );

				return IsVisible(min, max);
			}
			else
			{
#ifdef FULL_TRANSFORM
				Bounds temp (bounds);
				temp.Transform(pos, rot, scale);
				return IsVisible(temp.GetMin(), temp.GetMax());
#else
				return true;
#endif
			}
		}
	}
	return false;
}

//============================================================================================================
// Returns whether the transformed bounding volume is visible,
// and includes the transformed volume in the provided final bounds
//============================================================================================================

bool Frustum::IncludeIfVisible (const Bounds&		bounds,
								const Vector3f&		pos,
								const Quaternion&	rot,
								float				scale,
								Bounds&				final) const
{
	if ( bounds.IsValid() )
	{
		bool identity = rot.IsIdentity();
		Vector3f center ( pos + (identity ? bounds.GetCenter() : bounds.GetCenter() * rot) * scale );
		float radius ( bounds.GetRadius() * scale );

		if (IsVisible(center, radius))
		{
			if (identity)
			{
				Vector3f min ( pos + bounds.GetMin() * scale );
				Vector3f max ( pos + bounds.GetMax() * scale );

				if ( IsVisible(min, max) )
				{
					final.Include(min);
					final.Include(max);
					return true;
				}
			}
			else
			{
#ifdef FULL_TRANSFORM
				Bounds temp (bounds);
				temp.Transform(pos, rot, scale);

				if ( IsVisible(temp.GetMin(), temp.GetMax()) )
				{
					final.Include(temp);
					return true;
				}
#else
				final.Include(center, radius);
				return true;
#endif
			}
		}
	}
	return false;
}