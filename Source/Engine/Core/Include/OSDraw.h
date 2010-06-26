#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Class containing common functionality that can be used by draw scripts
//============================================================================================================

class OSDraw : public Script
{
protected:

	Core*			mCore;
	IGraphics*		mGraphics;
	OSSceneRoot*	mRoot;
	Scene			mScene;

	OSDraw() : mCore(0), mGraphics(0), mRoot(0) {}

public:

	R5_DECLARE_ABSTRACT_CLASS("OSDraw", Script);

	Scene& GetScene() { return mScene; }

	void SetRenderTarget (IRenderTarget* target) { mScene.SetRenderTarget(target); }

	virtual void OnInit();
	virtual void OnDestroy();
	virtual void OnDraw()=0;
};