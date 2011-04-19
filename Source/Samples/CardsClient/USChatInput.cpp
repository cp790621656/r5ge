//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================

#include "CardsClient.h"
using namespace R5;

extern Core* g_core;
extern String g_message;

//============================================================================================================
// Ensure the script is attached to an input and register a key listener
//============================================================================================================

void USChatInput::OnInit()
{
	mInput = R5_CAST(UIInput, mWidget);
	if (mInput == 0) DestroySelf();
	else g_core->AddOnKey( bind(&USChatInput::OnKey, this), 1000000 );
}

//============================================================================================================
// Remove bound callbacks
//============================================================================================================

void USChatInput::OnDestroy()
{
	if (g_core != 0) g_core->RemoveOnKey(bind(&USChatInput::OnKey, this));
}

//============================================================================================================
// Key event callback registered with the core
//============================================================================================================

uint USChatInput::OnKey (const Vector2i& pos, byte key, bool isDown)
{
	if (key == Key::Return || key == Key::NumpadEnter)
	{
		if (!isDown)
		{
			if (mWidget->GetUI()->GetFocusArea() != mWidget)
			{
				mInput->SetFocus(true);
				mInput->SelectAll();
			}
		}
		return EventDispatcher::EventResponse::Handled;
	}
	return EventDispatcher::EventResponse::NotHandled;
}

//============================================================================================================
// Submit the text
//============================================================================================================

void USChatInput::OnValueChange()
{
	g_message = mInput->GetText();
	mInput->SetFocus(false);
	mInput->SetText("");
}