#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2010 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// Script that plays idle animations on the model instance it's attached to
//============================================================================================================

class OSPlayIdleAnimations : public Script
{
protected:

	Model*	mModel;
	ulong	mTimeToPlay;
	bool	mIdleLoop;

	Array<Animation*> mIdleAnims;

	// Use the AddScript<> template to add new scripts
	OSPlayIdleAnimations() : mModel(0), mTimeToPlay(0), mIdleLoop(false) {}

	// Immediately plays a random idle animation
	void Play();

	// Callback triggered when the idle animation finishes playing
	void OnIdleFinished (Model* model, const Animation* anim, float timeToEnd);

public:

	R5_DECLARE_INHERITED_CLASS("OSPlayIdleAnimations", OSPlayIdleAnimations, Script, Script);

	// When the script initializes, gather all idle animations
	virtual void OnInit();

	// Keeps track of when to play the next animation
	virtual void OnUpdate();
};