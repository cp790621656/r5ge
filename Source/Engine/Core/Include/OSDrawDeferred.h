#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Draws the scene using deferred rendering
//============================================================================================================

class OSDrawDeferred : public OSDraw
{
protected:

	Camera*		mCam;
	float		mBloom;
	Vector3f	mFocalRange;
	uint		mSSAO;

	OSDrawDeferred() : mCam(0), mBloom(1.0f), mSSAO(0) {}

public:

	R5_DECLARE_INHERITED_CLASS("OSDrawDeferred", OSDrawDeferred, OSDraw, Script);

	// Bloom makes areas above the specified threshold appear to glow
	float GetBloom() const { return mBloom; }
	void SetBloom (float val) { mBloom = val; }

	// Focal range for depth-of-field (X = 100% focus distance, Y = range from X that's still 100%, Z = 0%)
	const Vector3f& GetFocalRange() const { return mFocalRange; }
	void SetFocalRange (const Vector3f& range) { mFocalRange = range; }

	// Screen-space ambient occlusion level (0 = off, 1 = low, 2 = high)
	byte GetSSAO() const { return (byte)mSSAO; }
	void SetSSAO (byte val) { mSSAO = val; }

	// Initialize the scene
	virtual void OnInit();

	// Draw callback
	virtual void OnDraw();

	// Serialization
	virtual void OnSerializeTo	 (TreeNode& root) const;
	virtual void OnSerializeFrom (const TreeNode& node);
};