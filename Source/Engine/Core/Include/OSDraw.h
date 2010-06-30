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

	Core*				mCore;
	IGraphics*			mGraphics;
	OSSceneRoot*		mRoot;
	Scene				mScene;
	DirectionalShadow	mShadow;
	bool				mGrid;

	OSDraw() : mCore(0), mGraphics(0), mRoot(0), mGrid(false) {}

public:

	R5_DECLARE_ABSTRACT_CLASS("OSDraw", Script);

	Scene& GetScene() { return mScene; }

	// Convenience functionality
	void SetRenderTarget (IRenderTarget* target) { mScene.SetFinalTarget(target); }
	IRenderTarget* GetRenderTarget() { return mScene.GetFinalTarget(); }

	// For debugging purposes it should be possible to display a simple grid at the origin
	void SetShowGrid (bool val) { mGrid = val; }
	bool IsShowinGrid() const { return mGrid; }

	// Virtual functionality
	virtual void OnInit();
	virtual void OnDestroy();
	virtual void OnDraw()=0;

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual void OnSerializeFrom (const TreeNode& node);
};