#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Adds an element to the spline
//============================================================================================================

void SplineF::AddKey (float time, float val)
{
	bool sort = (mCp.IsValid() && mCp.Back().mTime > time);

	CtrlPoint& ctrl = mCp.Expand();
	ctrl.mTime		= time;
	ctrl.mVal		= val;
	ctrl.mTan		= 0.0f;
	mIsSmooth		= false;

	if (sort)
	{
		mLastSample	= 0.0f;
		mLastIndex	= 0;
		mCp.Sort();
	}
}

//============================================================================================================
// Calculates all tangents
//============================================================================================================

void SplineF::Smoothen()
{
	if (mCp.GetSize() > 2)
	{
		CtrlPoint *past, *current, *future, *end;

		// Set the tangents and control points
		for ( current = &mCp.Front() + 1, end = &mCp.Back(); current < end; ++current )
		{
			past	= current - 1;
			future	= current + 1;

			current->mTan = Interpolation::GetHermiteTangent(
				current->mVal	- past->mVal,
				future->mVal	- current->mVal,
				current->mTime	- past->mTime,
				future->mTime	- current->mTime);
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

			if (!Float::IsEqual(p0.mVal, p3.mVal)) return;

			p0.mTan = Interpolation::GetHermiteTangent(
				p3.mVal	 - p2.mVal,
				p1.mVal	 - p0.mVal,
				p3.mTime - p2.mTime,
				p1.mTime - p0.mTime);

			p3.mTan = p0.mTan;
		}
	}

	// The spline is now smooth
	mIsSmooth = true;
}

//============================================================================================================
// Sample the spline at the given time
//============================================================================================================

float SplineF::Sample (float time, bool smooth) const
{
	// No point in proceeding if there is nothing available
	if (mCp.IsValid())
	{
		// If the time is past the first frame
		if (time > mCp.Front().mTime)
		{
			// If the requested time is before the last entry
			if (time < mCp.Back().mTime)
			{
				// Smoothen the spline if it hasn't been done yet
				if (!mIsSmooth) ((SplineF*)this)->Smoothen();

				// Start at the last sampled keyframe if we're sampling forward
				uint current = (mLastSample > time) ? 0 : mLastIndex;

				// Cache the current sampling time for the next execution
				mLastSample = time;
				mLastIndex = mCp.GetSize() - 1;

				// Run through all control points until the proper time is encountered
				while (current < mLastIndex)
				{
					const CtrlPoint& key (mCp[current]);
					const CtrlPoint& next (mCp[++current]);

					if (time < next.mTime)
					{
						// Remember the current location
						mLastIndex = current - 1;

						float duration (next.mTime - key.mTime);
						float factor   ((time - key.mTime) / duration);

						if (smooth)
						{
							return Interpolation::Hermite(	key.mVal,	next.mVal,
															key.mTan,	next.mTan,
															factor,		duration );
						}
						return Interpolation::Linear(key.mVal, next.mVal, factor);
					}
				}
			}
			return mCp.Back().mVal;
		}
		return mCp.Front().mVal;
	}
	return 0.0f;
}