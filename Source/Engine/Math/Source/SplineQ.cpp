#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Adds an element to the spline
//============================================================================================================

void SplineQ::AddKey (float time, const Quaternion& rot)
{
	bool sort = (mCp.IsValid() && mCp.Back().mTime > time);
	uint index = 0;

	CtrlPoint ctrl;
	ctrl.mVal	= rot;
	ctrl.mTime	= time;
	ctrl.mTan	= 0.0f;
	ctrl.mCp	= rot;
	mIsSmooth	= false;

	if (sort)
	{
		index = mCp.AddSorted(ctrl);
	}
	else
	{
		index = mCp.GetSize();
		mCp.Expand() = ctrl;
	}

	// Start at the previous entry
	if (index > 0) --index;

	// If we have more than one point to work with
	if (mCp.GetSize() > 1)
	{
		// Run through all of the following points
		for (uint i = index, imax = mCp.GetSize() - 1; i < imax; ++i)
		{
			CtrlPoint& p0 = mCp[i];
			CtrlPoint& p1 = mCp[i+1];

			// Take the shortest path
			if (p0.mVal.Dot(p1.mVal) < 0.0f)
			{
				p1.mVal.Flip();
				p1.mCp.Flip();
			}
			else break;
		}
	}
}

//============================================================================================================
// Calculates all tangents and rotational control points
//============================================================================================================

void SplineQ::Smoothen()
{
	mIsSmooth = true;

	if (mCp.GetSize() > 2)
	{
		CtrlPoint *past, *current (&mCp.Front()), *future(&mCp.Back()), *end;
		float duration0, duration1, dot0, dot1;

		// Set the tangents and control points
		for ( current = &mCp.Front() + 1, end = &mCp.Back(); current < end; ++current )
		{
			past	= current - 1;
			future	= current + 1;

			duration0 = current->mTime - past->mTime;
			duration1 = future->mTime  - current->mTime;

			dot0 = current->mVal.Dot(past->mVal);
			dot1 = current->mVal.Dot(future->mVal);

			current->mTan = Interpolation::GetHermiteTangent( dot0, dot1, duration0, duration1 );
			current->mCp  = Interpolation::GetSquadControlRotation( past->mVal, current->mVal, future->mVal );
		}

		// For splines that should be seamless, wrap the first and last values around
		// so that there are no seams when the sampling is looped. When this feature
		// is requested, first and last points should be identical.

		if (mSeamless)
		{
			CtrlPoint& p0 = mCp[0];
			CtrlPoint& p1 = mCp[1];
			CtrlPoint& p2 = mCp[mCp.GetSize() - 2];
			CtrlPoint& p3 = mCp[mCp.GetSize() - 1];

			if (p0.mVal != p3.mVal) return;

			duration0 = p3.mTime - p2.mTime;
			duration1 = p1.mTime - p0.mTime;

			dot0 = p0.mVal.Dot(p2.mVal);
			dot1 = p0.mVal.Dot(p1.mVal);

			p0.mTan = Interpolation::GetHermiteTangent( dot0, dot1, duration0, duration1 );
			p0.mCp  = Interpolation::GetSquadControlRotation( p2.mVal, p0.mVal, p1.mVal );
			p3.mTan = p0.mTan;
			p3.mCp	= p0.mCp;
		}
	}
}

//============================================================================================================
// Sample the spline at the given time
//============================================================================================================

Quaternion SplineQ::Sample (float time, bool smooth) const
{
	// No point in proceeding if there is nothing available
	if ( mCp.IsValid() )
	{
		// If the time is past the first frame
		if ( time > mCp.Front().mTime )
		{
			// If the requested time is before the last entry
			if ( time < mCp.Back().mTime )
			{
				// Smoothen the spline if it hasn't been done yet
				if (!mIsSmooth) ((SplineQ*)this)->Smoothen();

				// Run through all control points until the proper time is encountered
				for ( const CtrlPoint *key = &mCp.Front(), *end = &mCp.Back(); key != end; ++key )
				{
					const CtrlPoint *nextKey (key + 1);

					if ( time < nextKey->mTime )
					{
						float duration (nextKey->mTime - key->mTime);
						float factor   ((time - key->mTime) / duration);

						if (smooth)
						{
							factor = Interpolation::Hermite(key->mTan, nextKey->mTan, factor, duration);

							return Interpolation::Squad(key->mVal,	nextKey->mVal,
														key->mCp,	nextKey->mCp,
														factor );
						}
						return Interpolation::Slerp(key->mVal, nextKey->mVal, factor);
					}
				}
			}
			return mCp.Back().mVal;
		}
		return mCp.Front().mVal;
	}
	return Quaternion();
}