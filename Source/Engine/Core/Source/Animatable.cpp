#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Update the relative coordinates based on the current time factor
//============================================================================================================

void Animatable::SetTime (float val)
{
	if (!mPosSpline.IsEmpty())	 SetRelativePosition( mPosSpline.Sample(val) );
	if (!mRotSpline.IsEmpty())	 SetRelativeRotation( mRotSpline.Sample(val) );
	if (!mScaleSpline.IsEmpty()) SetRelativeScale	( mScaleSpline.Sample(val) );
}