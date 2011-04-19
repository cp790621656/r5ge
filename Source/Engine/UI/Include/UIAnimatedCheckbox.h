#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Same as the AnimatedButton, but based on the Checkbox
// Author: Michael Lyashenko
//============================================================================================================

class UIAnimatedCheckbox : public UICheckbox
{
protected:

	float	mCurrentAlpha[2];	// Current alpha (0 = highlight, 1 = check)
	float	mStartAlpha  [2];	// Starting alpha for interpolation
	float	mTargetAlpha [2];	// Target alpha for interpolation
	float	mStartTime	 [2];	// Timestamp of the start of the animation
	float	mAnimTime;			// Time to animate from start to target value in seconds

public:

	UIAnimatedCheckbox();

	float GetAnimationTime() const		{ return mAnimTime; }
	void  SetAnimationTime(float val)	{ mAnimTime = val;  }

	// Changes the state of the checkbox
	virtual bool SetState(uint state, bool val);

public:

	// Area creation
	static UIWidget* _CreateNew() { return new UIAnimatedCheckbox(); }

	// Area functions
	virtual bool OnUpdate (bool dimensionsChanged);
	virtual void OnFill (UIQueue* queue);

	// Serialization
	virtual bool OnSerializeFrom (const TreeNode& node);
	virtual void OnSerializeTo (TreeNode& root) const;
};