#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Set the game input field
//============================================================================================================

void USFadeOut::OnInit()
{
	mWidget->RemoveScript<USFadeIn>();
	mEvents = mWidget->GetEventHandling();
	mWidget->SetEventHandling(UIWidget::EventHandling::None);
}

//============================================================================================================
// Change the color of the slider base on value
//============================================================================================================

void USFadeOut::OnUpdate (bool areaChanged)
{
	float alpha = mWidget->GetRegion().GetAlpha();

	if (alpha > 0.0f)
	{
		mWidget->GetRegion().SetAlpha(alpha - (Time::GetDelta() * (1.0f / mDuration)));
	}
	else
	{
		mWidget->GetRegion().SetAlpha(0.0f);

		if (mDestroyWhenDone)
		{
			mWidget->DestroySelf();
		}
		else
		{
			DestroySelf();
		}
	}
}
