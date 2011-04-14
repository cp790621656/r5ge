#pragma once

//============================================================================================================
//					R5 Game Engine, Copyright (c) 2007-2011 Tasharen Entertainment
//									http://r5ge.googlecode.com/
//============================================================================================================
// Button that smoothly fades from one state to the next
// Author: Michael Lyashenko
//============================================================================================================

class UIAnimatedButton : public UIButton
{
protected:

	float	mCurrentAlpha[2];	// Current alpha (0 = highlight, 1 = pushed)
	float	mStartAlpha  [2];	// Starting alpha for interpolation
	float	mTargetAlpha [2];	// Target alpha for interpolation
	float	mStartTime	 [2];	// Timestamp of the start of the animation
	float	mAnimTime;			// Time to animate from start to target value in seconds

public:

	UIAnimatedButton();

	float GetAnimationTime() const		{ return mAnimTime; }
	void  SetAnimationTime(float val)	{ mAnimTime = val;  }

	// Changes the state of the button
	virtual bool SetState(uint state, bool val);

public:

	// Area creation
	static UIWidget* _CreateNew() { return new UIAnimatedButton(); }

	// Area functions
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};