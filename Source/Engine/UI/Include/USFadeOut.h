#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Fade the widget out and optionally destory it when at full alpha
//============================================================================================================

class USFadeOut : public USFadeIn
{
	bool mDestroyWhenDone;

	USFadeOut() : mDestroyWhenDone(false) {}

public:

	R5_DECLARE_INHERITED_CLASS("USFadeOut", USFadeOut, USFadeIn, UIScript);

	void SetDestroyWhenDone (bool val) { mDestroyWhenDone = val; }

	virtual void OnInit();
	virtual void OnUpdate(bool areaChanged);
};
