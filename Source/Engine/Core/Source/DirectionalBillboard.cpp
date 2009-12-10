#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Draw the billboard
//============================================================================================================

uint DirectionalBillboard::OnDraw (const ITechnique* tech, bool insideOut)
{
	IGraphics* graphics = mCore->GetGraphics();

	Matrix43 mat (graphics->GetViewMatrix());

	const Vector3f& camPos = graphics->GetCameraPosition();
	const Vector2f& camRange = graphics->GetCameraRange();
	mat.PreTranslate(camPos - mAbsoluteRot.GetForward() * camRange.y);
	mat.ReplaceScaling(mAbsoluteScale * camRange.y * 0.1f);

	graphics->SetModelViewMatrix(mat);
	DrawBillboard();
	return 1;
}