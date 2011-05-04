#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Fade the widget out and optionally destory it when at full alpha
// Author: Michael Lyashenko
//============================================================================================================

class USFadeOut : public USFadeIn
{
	bool mDestroyWhenDone;

	USFadeOut() : mDestroyWhenDone(false) {}

public:

	R5_DECLARE_INHERITED_CLASS(USFadeOut, USFadeIn, UIScript);

	void SetDestroyWhenDone (bool val) { mDestroyWhenDone = val; }

	virtual void OnInit();
	virtual void OnUpdate(bool areaChanged);
};
