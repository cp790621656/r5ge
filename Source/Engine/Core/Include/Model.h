#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2009 Michael Lyashenko. All rights reserved.
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Extended prop supporting skinned animation
//============================================================================================================

class Model : public Prop
{
public:

	typedef Array<Matrix43> Matrices;
	typedef ActiveAnimation::BoneTransforms BoneTransforms;

	// Delegate type triggered when an animation is starting to fade and when it ends
	typedef FastDelegate<void (Model* model, const Animation* anim, float timeToEnd)>	AnimationEnd;

	// Response returned by PlayAnimation() function
	struct PlayResponse
	{
		enum
		{
			Failed			= 0,
			NowPlaying		= 1,
			AlreadyPlaying	= 2,
		};
	};

protected:

	// All currently or previously active animations are stored in one list
	typedef PointerArray<ActiveAnimation> ActiveAnims;

	// Animations that are actively playing are referenced separately
	struct PlayingAnimation
	{
		ActiveAnimation*	mActiveAnim;
		AnimationEnd		mOnEnd;
	};

	typedef Array<PlayingAnimation> PlayingAnims;

	// Playing animations are separated by animation layers sorted by ID.
	// Animations always gradually replace other animations on the same layer.
	// Animations played on higher layers "cover" animations on lower layers (think photoshop layers).

	struct AnimationLayer
	{
		uint			mLayer;
		PlayingAnims	mPlayingAnims;

		bool operator < (const AnimationLayer& a) const { return mLayer < a.mLayer; }
	};

	typedef Array<AnimationLayer> AnimLayers;

protected:

	Thread::ValType	mCounter;			// Counter holding the number of references
	ActiveAnims		mActiveAnims;		// List of all animations that were played on this model
	AnimLayers		mAnimLayers;		// List referencing currently active animations spread across layers
	float			mAnimationSpeed;	// Per-model animation speed value, for haste/slow/pause effects
	BoneTransforms	mTransforms;		// Calculated final transforms, one for each bone
	Matrices		mMatrices;			// Calculated final matrices, one for each bone
	uint			mUpdateInterval;	// Interval at which to perform animation updates in milliseconds
	ulong			mLastUpdate;		// Timestamp of when the model was last updated
	bool			mAnimUpdated;		// Whether the model has been updated

public:

	Model (const String& name);

	// Object creation
	R5_DECLARE_ABSTRACT_CLASS("Model", Prop);

	// Update the world matrix, advance the animation
	void Update();

private:

	// INTERNAL: Retrieves animations currently playing on the specified layer
	PlayingAnims* _GetPlayingAnims (uint layer);

	// INTERNAL: Stops all animations on the specified animation list
	bool _StopAnimation (PlayingAnims&			list,
						 const Animation*		anim		= 0,
						 float					duration	= 0.25f,
						 const AnimationEnd&	onAnimEnd	= 0);

	// INTERNAL: Stops all animations covering the specified animation
	bool _StopCoveringAnimations (uint layer, const Animation* anim, float duration);

protected:

	// Allow model instance class to access these functions
	friend class ModelInstance;

	// Keeping track of the number of references
	void _Increment() { Thread::Increment(mCounter); }
	void _Decrement() { Thread::Decrement(mCounter); }

	// Triggered when the skeleton has changed
	virtual void _OnSkeletonChanged();

	// Clears all animation data
	virtual void _OnRelease();

	// Draw the object using the specified technique
	virtual uint _Draw (IGraphics* graphics, const ITechnique* tech);

	// Draw any special outline of the object
	virtual uint _DrawOutline (IGraphics* graphics, const ITechnique* tech);

public:

	bool	IsAnimated()			const;
	uint	GetNumberOfReferences() const	{ return (uint)mCounter; }
	float	GetAnimationSpeed()		const	{ return mAnimationSpeed; }
	uint	GetUpdateInterval()		const	{ return mUpdateInterval; }

	// Direct access to bone transforms and matrices
	const BoneTransforms&	GetAllBoneTransforms()	const	{ return mTransforms; }
	const Matrices&			GetAllBoneMatrices()	const	{ return mMatrices;	  }
	BoneTransforms&			GetAllBoneTransforms()			{ return mTransforms; }
	Matrices&				GetAllBoneMatrices()			{ return mMatrices;	  }

	// Convenience function: retrieves the index of the specified bone
	uint GetBoneIndex (const String& name) const { return (mSkeleton != 0) ? mSkeleton->GetBoneIndex(name) : 0; }

	// Retrieves the calculated bone transform
	const BoneTransform* GetBoneTransform (uint index) const;
	const BoneTransform* GetBoneTransform (const String& name) const;

	// Access to animation speed used to advance the animation
	void SetAnimationSpeed (float val) { mAnimationSpeed = val; }

	// Interval at which the model's animations will be updated. Default is 1 (updating every millisecond)
	void SetUpdateInterval (uint ms) { mUpdateInterval = ms; }

	// Retrieves the specified animation, creating it if necessary
	Animation* GetAnimation (const String& name, bool createIfMissing = false);

	// Finds and plays the requested animation
	uint PlayAnimation (const String& name, const AnimationEnd& onAnimEnd = 0);

	// Plays the requested animation
	uint PlayAnimation (const Animation* anim, const AnimationEnd& onAnimEnd = 0);

	// Stops the specified animation, fading it out over the specified amount of time
	bool StopAnimation (const Animation* anim, float duration = 0.25f, const AnimationEnd& onAnimEnd = 0);

	// Stops all animations playing on the specified layer
	bool StopAnimationsOnLayer (uint animationLayer, float duration = 0.25f, const AnimationEnd& onAnimEnd = 0);

	// Stops all active animations, fading them out over the specified amount of time
	bool StopAllAnimations (float fadeOutDuration = 0.25f, const AnimationEnd& onAnimEnd = 0);

	// Returns the amount of time that's left on the specified animation's playback
	float GetTimeToAnimationEnd (const Animation* anim);

public:

	// Special: will either enable or disable skinning on the GPU using shaders
	static void EnableSkinningOnGPU(bool val);

	// Special: will either enable or disable using VBOs for skinning
	static void EnableSkinningToVBO(bool val);
};