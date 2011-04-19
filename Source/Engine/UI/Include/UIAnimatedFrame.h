#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Frame that smoothly fades in and out when alpha changes
// Author: Michael Lyashenko
//============================================================================================================

class UIAnimatedFrame : public UIFrame
{
protected:

	float	mStartAlpha;	// Starting alpha, for animation
	float	mCurrentAlpha;	// Current relative alpha. Stored here, because Region's alpha is the target alpha
	float	mStartTime;		// Time at which the animation started
	float	mAnimTime;		// Time it takes to animate from starting alpha to target alpha

public:

	UIAnimatedFrame();

	virtual float GetAlpha() const { return mRegion.GetParentAlpha() * mRegion.GetAlpha(); }
	virtual void  SetAlpha (float val, float animTime = 0.0f);

public:

	// Area creation
	static UIWidget* _CreateNew() { return new UIAnimatedFrame(); }

	// Called before OnUpdate(); can be used to override the widget's alpha or dimensions
	virtual bool OnPreUpdate (bool dimensionsChanged);
};