#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Changes the alpha value that the glow animates to
//============================================================================================================

void Glare::_SetTargetAlpha (float target)
{
	if (target != mAlpha.z)
	{
		mAlpha.x = mAlpha.y;
		mAlpha.z = target;
		mTimeStamp = Time::GetTime();
	}
}

//============================================================================================================
// If the alpha is changing, interpolate the value
//============================================================================================================

void Glare::OnUpdate()
{
	// Update the alpha
	if ( Float::IsNotEqual(mAlpha.y, mAlpha.z) )
	{
		float current = Time::GetTime();
		float factor = (current - mTimeStamp) * 8.0f;
		factor = Float::Clamp(factor, 0.0f, 1.0f);
		mAlpha.y = mAlpha.x * (1.0f - factor) + mAlpha.z * factor;
	}

	// Automatically start moving the alpha toward 0, unless overwritten by the Glare::OnDraw later.
	_SetTargetAlpha(0.0f);

	// If coordinates have changed, adjust the absolute bounds
	if (mIsDirty)
	{
		mAbsoluteBounds.Reset();
		mAbsoluteBounds.Include(mAbsolutePos);
	}
}

//============================================================================================================
// Draw the billboard
//============================================================================================================

uint Glare::OnDraw (const Deferred::Storage& storage, uint group, const ITechnique* tech)
{
	// IGraphics::IsVisible() check is affected by the ModelView matrix
	IGraphics* graphics = mCore->GetGraphics();

	// If the point is visible, alpha should be moving toward '1'
	if (graphics->IsPointVisible(mAbsolutePos)) _SetTargetAlpha(1.0f);

	// Current scale
	float scale = mAbsoluteScale * mAlpha.y;

	// Only draw the billboard if it's large enough to be drawn
	if (scale > 0.0f)
	{
		graphics->ResetModelViewMatrix();
		IGraphics* graphics = mCore->GetGraphics();
		Matrix43 mat (graphics->GetViewMatrix());
		mat.PreTranslate(mAbsolutePos);
		mat.ReplaceScaling(scale);
		graphics->SetModelViewMatrix(mat);
		DrawBillboard();
		graphics->ResetModelViewMatrix();
	}
	return 1;
}