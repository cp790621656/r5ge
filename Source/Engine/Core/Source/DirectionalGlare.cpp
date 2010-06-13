#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// If the alpha is changing, interpolate the value
//============================================================================================================

void DirectionalGlare::OnUpdate()
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
}

//============================================================================================================
// Draw the glare
//============================================================================================================

uint DirectionalGlare::OnDraw (const Deferred::Storage& storage, uint group, const ITechnique* tech, bool insideOut)
{
	IGraphics* graphics = mCore->GetGraphics();

	const Vector3f& camPos = graphics->GetCameraPosition();
	const Vector3f& camRange = graphics->GetCameraRange();

	// Horizon offset
	Vector3f offset (mAbsoluteRot.GetForward() * camRange.y);

	// Distant point on the horizon
	Vector3f distant (camPos - offset * 0.99f);

	// If the distant point is visible, our glare should be as well
	if (graphics->IsPointVisible(distant)) _SetTargetAlpha(1.0f);

	// Current scale
	float scale = mAbsoluteScale * mAlpha.y;

	// Only draw the billboard if it's large enough to be drawn
	if (scale > 0.0f)
	{
		IGraphics* graphics = mCore->GetGraphics();
		Matrix43 mat (graphics->GetViewMatrix());
		mat.PreTranslate(camPos - offset);
		mat.ReplaceScaling(scale * camRange.y * 0.1f);
		graphics->SetModelViewMatrix(mat);
		DrawBillboard();
		graphics->ResetModelViewMatrix();
	}
	return 1;
}