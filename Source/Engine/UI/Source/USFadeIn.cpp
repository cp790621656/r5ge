#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Set the game input field
//============================================================================================================

void USFadeIn::OnInit()
{
	mWidget->RemoveScript<USFadeOut>();
	mEvents = mWidget->GetEventHandling();
	mWidget->SetEventHandling(UIWidget::EventHandling::None);
}

//============================================================================================================
// Change the color of the slider base on value
//============================================================================================================

void USFadeIn::OnUpdate (bool areaChanged)
{
	float alpha = mWidget->GetRegion().GetAlpha();

	if (alpha < 1.0f)
	{
		alpha += (Time::GetDelta() * (1.0f / mDuration));
		mWidget->GetRegion().SetAlpha(alpha);
	}
	else
	{
		mWidget->GetRegion().SetAlpha(1.0f);
		DestroySelf();
	}
}

//============================================================================================================
// Serialization -- Save
//============================================================================================================

void USFadeIn::OnSerializeTo (TreeNode& root) const
{
	root.AddChild("Duration", mDuration);
}

//============================================================================================================
// Serialization -- Load
//============================================================================================================

void USFadeIn::OnSerializeFrom(const TreeNode& node)
{
	if (node.mTag == "Duration") node.mValue >> mDuration;
}