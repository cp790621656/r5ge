#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Class containing common functionality that can be used by draw scripts
// Author: Michael Lyashenko
//============================================================================================================

class OSDraw : public Script
{
protected:

	Core*				mCore;
	IGraphics*			mGraphics;
	OSSceneRoot*		mRoot;
	Camera*				mCam;
	Scene				mScene;
	Color4f				mBackground;
	Vector2f			mFogRange;
	DirectionalShadow	mShadow;
	bool				mGrid;
	ITexture*			mFinalDepth;
	Vector3f			mMousePos;
	mutable bool		mSaveMousePos;

	OSDraw() : mCore(0), mGraphics(0), mRoot(0), mCam(0), mGrid(false), mFinalDepth(0), mSaveMousePos(false) {}

public:

	R5_DECLARE_ABSTRACT_CLASS("OSDraw", Script);

	Scene& GetScene() { return mScene; }

	// Convenience functionality
	void SetRenderTarget (IRenderTarget* target) { mScene.SetFinalTarget(target); }
	IRenderTarget* GetRenderTarget() { return mScene.GetFinalTarget(); }

	// For debugging purposes it should be possible to display a simple grid at the origin
	void SetShowGrid (bool val) { mGrid = val; }
	bool IsShowinGrid() const { return mGrid; }

	// Fog range is 0-1 based, with 0 being the near clipping plane and 1 being the far clipping plane
	const Vector2f& GetFogRange() const { return mFogRange; }
	void SetFogRange (const Vector3f& range) { mFogRange = range; }

	// Background color, acts as a fog color as well
	const Color4f& GetBackgroundColor() const { return mBackground; }
	void SetBackgroundColor (const Color4f& c) { mBackground = c; }

	// World-space mouse position (acquired by sampling the depth)
	const Vector3f& GetMousePos() const { mSaveMousePos = true; return mMousePos; }

	// Whether the mouse's 3D position will be recorded each frame
	bool IsSavingMousePos() const { return mSaveMousePos; }
	void SetSavingMousePos (bool val) { mSaveMousePos = val; }

	// Virtual functionality
	virtual void OnInit();
	virtual void OnDestroy();
	virtual void OnDraw()=0;

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual void OnSerializeFrom (const TreeNode& node);
};