#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Update the relative coordinates based on the current time factor
//============================================================================================================

void Animatable::SetTime (float val)
{
	if (!mSplineV.IsEmpty()) SetRelativePosition( mSplineV.Sample(val) );
	if (!mSplineQ.IsEmpty()) SetRelativeRotation( mSplineQ.Sample(val) );
	if (!mSplineF.IsEmpty()) SetRelativeScale	( mSplineF.Sample(val) );
}