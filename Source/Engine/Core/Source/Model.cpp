#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Event notification for when animations are coming to an end
//============================================================================================================

struct Notification
{
	Model::AnimationEnd		mOnEnd;
	const Animation*		mAnim;
	float					mTimeToEnd;
};

//============================================================================================================
// Globally defined variables
//============================================================================================================

bool g_skinOnGPU = true;
bool g_skinToVBO = true;

//============================================================================================================
// Constructor just sets the default values
//============================================================================================================

Model::Model (const String& name) :
	Prop			(name),
	mCounter		(0),
	mAnimationSpeed	(1.0f),
	mUpdateInterval (1),
	mLastUpdate		(0),
	mAnimUpdated	(false) {}

//============================================================================================================
// Update the world matrix, advance the animation
//============================================================================================================

void Model::Update()
{
	// Don't allow updates to happen more frequently than requested
	ulong current = Time::GetMilliseconds();
	if (current - mLastUpdate < mUpdateInterval) return;

	// No point in updating models that are not referenced
	if (mCounter != 0)
	{
		// If the mesh is animated we need to update the animations
		if ( IsAnimated() )
		{
			float delta	= Float::Abs(0.001f * (current - mLastUpdate) * mAnimationSpeed);
			static Array<Notification> notifications;
			notifications.Clear();

			Lock();
			{
				const Skeleton::Bones& bones = mSkeleton->GetAllBones();

				// Reset the transform contribution values
				for (uint i = mTransforms.GetSize(); i > 0; )
				{
					BoneTransform& trans = mTransforms[--i];
					trans.mCombinedPos = 0.0f;
					trans.mCombinedRot = 0.0f;
				}

				// Run through all animation layers
				for (uint i = mAnimLayers.GetSize(); i > 0; )
				{
					// Get the list of animations playing on the current layer
					AnimationLayer& animLayer = mAnimLayers[--i];
					PlayingAnims& playingAnims = animLayer.mPlayingAnims;

					// Skip this layer if it has no animations
					if (playingAnims.IsEmpty()) continue;

					float remain = 1.0f, total = 0.0f;

					// Run through all playing animations
					for (uint b = playingAnims.GetSize(); b > 0; )
					{
						PlayingAnimation& play = playingAnims[--b];
						ActiveAnimation* aa = play.mActiveAnim;

						if (remain > 0.0f)
						{
							// Calculate the target duration
							total = Interpolation::Linear(total, aa->mPlaybackDuration, remain);
							remain -= aa->mCurrentAlpha;
						}
						else
						{
							// If the animation is completely hidden, remove it from the list
							aa->mIsActive = false;
							playingAnims.RemoveAt(b);

							if (play.mOnEnd)
							{
								// Notify the listener that this animation has been removed
								Notification& n = notifications.Expand();
								n.mOnEnd		= play.mOnEnd;
								n.mAnim			= aa->mAnimation;
								n.mTimeToEnd	= 0.0f;
							}
						}
					}

					if (total > 0.0f)
					{
						// Run through all playing animations
						for (uint b = playingAnims.GetSize(); b > 0; )
						{
							PlayingAnimation& play = playingAnims[--b];
							ActiveAnimation* aa = play.mActiveAnim;

							// Whether the animation is not yet fading out
							bool wasPlaying = (aa->mPlaybackFactor < aa->mFadeOutStart);

							// Advance the animation, transitioning the sampling
							bool done = aa->AdvanceSample(delta * (aa->mPlaybackDuration / total),
								bones, mTransforms);

							// If the animation has been deactivated, remove it from the list
							if (!aa->mIsActive)
							{
								playingAnims.RemoveAt(b);
								aa->mIsActive = false;
							}

							// Whether the animation is still actively playing
							bool isPlaying = aa->mIsActive && (aa->mPlaybackFactor < aa->mFadeOutStart);
							
							// If there is a listener and this animation is coming to an end, notify it
							if (play.mOnEnd && (!aa->mIsActive || (wasPlaying && !isPlaying)))
							{
								Notification& n = notifications.Expand();
								n.mOnEnd		= play.mOnEnd;
								n.mAnim			= aa->mAnimation;
								n.mTimeToEnd	= (isPlaying) ? (1.0f - aa->mPlaybackFactor) *
									aa->mPlaybackDuration / mAnimationSpeed : 0.0f;
							}

							// If we've reached 100%, we're done with this layer
							if (done) break;
						}
					}
				}

				// Combine all active animations, creating the final set of bone matrices
				for (uint t = 0, tmax = mTransforms.GetSize(); t < tmax; ++t)
				{
					BoneTransform& trans = mTransforms[t];

					if (trans.mParent < mTransforms.GetSize())
					{
						trans.CalculateTransformMatrix( mTransforms[trans.mParent] );
					}
					else
					{
						trans.CalculateTransformMatrix();
					}

					// Set the bone's final transformation matrix
					mMatrices[t].SetToTransform(
						trans.mInvBindPos,
						trans.mAbsolutePos,
						trans.mInvBindRot,
						trans.mAbsoluteRot );
				}
			}
			Unlock();

			// Inform the listeners
			for (uint i = notifications.GetSize(); i > 0; )
			{
				Notification& n = notifications[--i];
				n.mOnEnd(this, n.mAnim, n.mTimeToEnd);
			}

			// The animation has been updated
			mAnimUpdated = true;
		}
	}
	// Update the timestamp
	mLastUpdate = current;
}

//============================================================================================================
// Draw the object using the specified technique
//============================================================================================================

uint Model::_Draw (uint group, IGraphics* graphics, const ITechnique* tech)
{
	if ( mSkeleton == 0 || mMatrices.IsEmpty() )
	{
		// If this model has no skeleton, just render it as a static prop
		return Prop::_Draw(group, graphics, tech);
	}

	// Last rendered model
	static Model* lastModel = 0;
	uint mask = tech->GetMask();

	// If the model has changed or the animation has been updated
	bool updateSkin = (mAnimUpdated || lastModel != this);

	// Go through every limb and render it
	Lock();
	{
		// Go through all limbs
		for (Limb** start = mLimbs.GetStart(), **end = mLimbs.GetEnd(); start != end; ++start)
		{
			Limb* limb (*start);

			// Only draw the limbs that are visible with the current technique
			if (limb->IsVisibleWith(mask))
			{
				Mesh*		mesh	= limb->GetMesh();
				IMaterial*	mat		= limb->GetMaterial();

				// Skip limbs that belong to different groups
				if (group != GetUID() && group != mat->GetUID()) continue;

				IMaterial::DrawMethod* method = (mat != 0 ? mat->GetVisibleMethod(tech) : 0);

				if (mesh != 0 && method != 0)
				{
					// Set the active material, possibly updating the shader in the process
					if ( graphics->SetActiveMaterial(mat) )
					{
						if (updateSkin)
						{
							IShader* shader = method->GetShader();

							// If we're skinning on the GPU and we have a shader active
							if (g_skinOnGPU && shader != 0 && shader->GetFlag(IShader::Flag::Skinned))
							{
								Uniform uniform;
								uniform.mType		= Uniform::Type::ArrayFloat16;
								uniform.mPtr		= &mMatrices[0][0];
								uniform.mElements	= mMatrices.GetSize();

								// Update the transform matrix
								shader->SetUniform("R5_boneTransforms", uniform);

								// If we're skinning on the GPU, we don't need transformed vertices
								mesh->DiscardTransforms();
							}
							else
							{
								// If we're skinning on the CPU however, we need to apply the transforms
								mesh->ApplyTransforms( mMatrices, GetNumberOfReferences() );
							}
						}

						// Draw the mesh
						mesh->Draw(graphics);

						// Remember the current values
						mAnimUpdated	= false;
						lastModel		= this;
					}
				}
			}
		}
	}
	Unlock();
	return 1;
}

//============================================================================================================
// Draw the outline
//============================================================================================================

uint Model::_DrawOutline (IGraphics* graphics, const ITechnique* tech)
{
	if ( tech->GetColorWrite() && IsAnimated() )
	{
		Array<Vector3f> points;

		bool stencil = graphics->GetStencilTest();

		graphics->SetLighting	(false);
		graphics->SetDepthWrite	(true);
		graphics->SetDepthTest	(true);
		graphics->SetActiveTechnique(0);
		graphics->SetActiveMaterial	(0);

		const Matrix43& world = graphics->GetModelMatrix();

		// Run through all transforms and draw colored XYZ axes at each bone's position
		for (uint i = 0; i < mTransforms.GetSize(); ++i)
		{
			const BoneTransform& trans = mTransforms[i];

			if (trans.mParent < mTransforms.GetSize())
			{
				const BoneTransform& parent = mTransforms[trans.mParent];
				points.Expand() = parent.mAbsolutePos;
			}
			else
			{
				points.Expand().Set(0.0f, 0.0f, 0.0f);
			}
			points.Expand() = trans.mAbsolutePos;

			// Calculate the absolute transformation matrix
			Matrix43 mat ( trans.mAbsolutePos, trans.mAbsoluteRot, 1.0f );

			// Set the world transformation matrix
			graphics->SetModelMatrix( mat * world );

			// Draw the axes
			graphics->Draw( IGraphics::Drawable::Axis );
		}

		// Draw lines connecting the bones
		if (points.IsValid())
		{
			graphics->SetModelMatrix			( world );
			graphics->SetActiveColor			( Color4f(1.0f, 0.0f, 0.0f, 1.0f) );
			graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Normal,		0 );
			graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Color,		0 );
			graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Tangent,	0 );
			graphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord0,	0 );
			graphics->SetActiveVertexAttribute	( IGraphics::Attribute::TexCoord1,	0 );
			graphics->SetActiveVertexAttribute	( IGraphics::Attribute::BoneIndex,	0 );
			graphics->SetActiveVertexAttribute	( IGraphics::Attribute::BoneWeight,	0 );
			graphics->SetActiveVertexAttribute	( IGraphics::Attribute::Position,	points );
			graphics->DrawVertices				( IGraphics::Primitive::Line, points.GetSize() );
		}

		// Restore the active technique
		graphics->SetStencilTest	( stencil );
		graphics->SetActiveColor	( Color4f(1.0f, 1.0f, 1.0f, 1.0f) );
		graphics->SetActiveTechnique( tech );
		return 1;
	}
	return 0;
}

//============================================================================================================
// INTERNAL: Retrieves playing animations for the specified layer
//============================================================================================================

Model::PlayingAnims* Model::_GetPlayingAnims (uint layer)
{
	// Try to see if the layer already exists
	for (uint i = mAnimLayers.GetSize(); i > 0; )
	{
		AnimationLayer& animLayer = mAnimLayers[--i];

		if (animLayer.mLayer == layer)
		{
			return &animLayer.mPlayingAnims;
		}
	}

	// The layer doesn't exist just yet -- add a new entry
	AnimationLayer al;
	al.mLayer = layer;
	return &mAnimLayers[mAnimLayers.AddSorted(al)].mPlayingAnims;
}

//============================================================================================================
// INTERNAL: Stops all animations on the specified animation list
//============================================================================================================

bool Model::_StopAnimation (PlayingAnims&		 list,
							const Animation*	 anim,
							float				 duration,
							const AnimationEnd&  onAnimEnd)
{
	bool retVal = false;

	if (list.IsValid())
	{
		duration = Float::Abs(duration);

		for (uint i = list.GetSize(); i > 0; )
		{
			PlayingAnimation& pa = list[--i];
			ActiveAnimation* aa = pa.mActiveAnim;

			// Either the animation matches or we're not trying to match anything
			if (anim == 0 || aa->mAnimation == anim)
			{
				retVal = true;

				if (duration > 0.0f)
				{
					// Only start fading out if it's not already fading out
					if (aa->mOverrideDuration == 0.0f)
					{
						aa->mOverrideFactor		= 0.0f;
						aa->mOverrideDuration	= duration;
						pa.mOnEnd				= onAnimEnd;
					}
				}
				else
				{
					list.RemoveAt(i);
				}

				// If we matched this animation we don't have to continue
				if (anim != 0) break;
			}
		}
	}
	return retVal;
}

//============================================================================================================
// INTERNAL: Stops all animations covering the specified animation
//============================================================================================================

bool Model::_StopCoveringAnimations (uint layer, const Animation* anim, float duration)
{
	bool retVal = false;

	for (uint i = mAnimLayers.GetSize(); i > 0; )
	{
		AnimationLayer& animLayer = mAnimLayers[--i];

		if (animLayer.mLayer == layer)
		{
			PlayingAnims& list = animLayer.mPlayingAnims;

			if (list.IsValid())
			{
				duration = Float::Abs(duration);

				for (uint i = list.GetSize(); i > 0; )
				{
					PlayingAnimation& pa = list[--i];
					ActiveAnimation* aa = pa.mActiveAnim;

					// If we've reached the animation in question, we're done here
					if (aa->mAnimation == anim) break;

					// We're about to stop an animation
					retVal = true;

					// If a duration has been specified, adjust its fade-out factor
					if (duration > 0.0f)
					{
						// Only start fading out if it's not already fading out
						if (aa->mOverrideDuration == 0.0f)
						{
							aa->mOverrideFactor		= 0.0f;
							aa->mOverrideDuration	= duration;
						}
					}
					else
					{
						// No duration has been specified -- remove the covering animation immediately
						list.RemoveAt(i);
					}
				}
			}
		}
	}
	return retVal;
}

//============================================================================================================
// Changes the skeleton associated with the model
//============================================================================================================

void Model::_OnSkeletonChanged()
{
	mAnimLayers.Clear();
	mActiveAnims.Release();
	mTransforms.Clear();
	mMatrices.Clear();

	if (mSkeleton != 0)
	{
		Skeleton::Animations&	anims	= mSkeleton->GetAllAnimations();
		Skeleton::Bones&		bones	= mSkeleton->GetAllBones();

		// Clear all bones from the animations associated with the skeleton, forcing them to be rebuilt
		for (uint i = anims.GetSize(); i > 0; )
		{
			Animation* anim = anims[--i];
			if (anim != 0) anim->ClearBones();
		}

		// Run through all bones and copy over the parent index as well as bind pose orientation
		for (uint i = 0; i < bones.GetSize(); ++i)
		{
			const Bone* bone = bones[i];
			ASSERT(bone != 0, "Missing bone?");

			// Add a new bone transform and copy over the parent's ID as well as the bone's orientation
			// so we have all this information locally and don't have to go through bones again.
			BoneTransform& trans = mTransforms.Expand();
			trans.mParent		 = bone->GetParent();
			trans.mRelativePos	 = bone->GetPosition();
			trans.mRelativeRot	 = bone->GetRotation();

			if (trans.mParent < mTransforms.GetSize())
			{
				// Calculate the bind pose transformation matrix
				trans.CalculateTransformMatrix( mTransforms[trans.mParent] );
			}
			else
			{
				trans.CalculateTransformMatrix();
			}

			// Calculate the inverse transforms
			trans.mInvBindPos = -trans.mAbsolutePos;
			trans.mInvBindRot = -trans.mAbsoluteRot;

			// We want to have the proper number of matrices as well
			mMatrices.Expand();
		}
	}
}

//============================================================================================================
// Clears all animation data
//============================================================================================================

void Model::_OnRelease()
{
	mAnimLayers.Release();
	mActiveAnims.Release();
	mTransforms.Release();
	mMatrices.Release();
}

//============================================================================================================
// Whether the model is currently animated
//============================================================================================================

bool Model::IsAnimated() const
{
	if (mSkeleton != 0)
	{
		for (uint i = mAnimLayers.GetSize(); i > 0; )
		{
			const AnimationLayer& layer = mAnimLayers[--i];
			if (layer.mPlayingAnims.IsValid()) return true;
		}
	}
	return false;
}

//============================================================================================================
// Retrieves the calculated bone transform
//============================================================================================================

const BoneTransform* Model::GetBoneTransform (uint index) const
{
	if (mSkeleton != 0)
	{
		if (index < mTransforms.GetSize())
		{
			return &mTransforms[index];
		}
	}
	return 0;
}

//============================================================================================================
// Retrieves the calculated bone transform
//============================================================================================================

const BoneTransform* Model::GetBoneTransform (const String& name) const
{
	if (mSkeleton != 0)
	{
		uint index = mSkeleton->GetBoneIndex(name);

		if (index < mTransforms.GetSize())
		{
			return &mTransforms[index];
		}
	}
	return 0;
}

//============================================================================================================
// Retrieves the specified animation, creating it if necessary
//============================================================================================================

Animation*	Model::GetAnimation (const String& name, bool createIfMissing)
{
	return (mSkeleton != 0 ? mSkeleton->GetAnimation(name, createIfMissing) : 0);
}

//============================================================================================================
// Finds and plays the requested animation
//============================================================================================================

uint Model::PlayAnimation (const String& name, float strength, const AnimationEnd& onAnimEnd)
{
	Animation* anim = (mSkeleton != 0 ? mSkeleton->GetAnimation(name, false) : 0);
	return (anim != 0) ? PlayAnimation(anim, strength, onAnimEnd) : PlayResponse::Failed;
}

//============================================================================================================
// Plays the requested animation
//============================================================================================================

uint Model::PlayAnimation (const Animation* anim, float strength, const AnimationEnd& onAnimEnd)
{
	if (anim == 0 || mSkeleton == 0) return PlayResponse::Failed;

	Lock();
	{
		ActiveAnimation*	activeAnim		= 0;
		PlayingAnims*		playingAnims	= 0;
		PlayingAnimation*	playingAnim		= 0;
		uint				animLayer		= anim->GetLayer();

		// Check all active animations to see if the requested animation has already been played
		for (uint i = mActiveAnims.GetSize(); i > 0; )
		{
			activeAnim = mActiveAnims[--i];

			if (activeAnim->mAnimation == anim)
			{
				if (activeAnim->mIsActive)
				{
					// The animation should no longer be trying to fade out
					if (activeAnim->mOverrideDuration > 0.0f)
					{
						// Flip the duration, making the animation fade in instead
						activeAnim->mOverrideDuration = -activeAnim->mOverrideDuration;
					}

					// Update the animation's strength, just in case it has changed
					activeAnim->mStrength = strength;

					// The animation is already active -- fade out all animations that are covering it
					const Vector3f& duration = anim->GetDuration();
					float delay = anim->IsLooping() ? duration.x : duration.z;

					// Stop the animation, fading it out with an appropriate delay
					_StopCoveringAnimations(animLayer, anim, (delay > 0.0f) ? delay : 0.25f);
					Unlock();
					return PlayResponse::AlreadyPlaying;
				}

				// The requested animation is currently inactive -- make it active
				playingAnims				= _GetPlayingAnims(animLayer);
				playingAnim					= &playingAnims->Expand();
				playingAnim->mActiveAnim	= activeAnim;
				playingAnim->mOnEnd			= onAnimEnd;
				break;
			}
		}

		// If the animation was not found, we should create a new one
		if (playingAnim == 0)
		{
			ASSERT( anim == mSkeleton->GetAnimation(anim->GetID()), "Animation wasn't found!" );

			// Create a new active animation
			activeAnim = (mActiveAnims.Expand() = new ActiveAnimation());
			activeAnim->mAnimation	= const_cast<Animation*>(anim);

			// Add a new playing animation entry
			playingAnims				= _GetPlayingAnims(animLayer);
			playingAnim					= &playingAnims->Expand();
			playingAnim->mActiveAnim	= activeAnim;
			playingAnim->mOnEnd			= onAnimEnd;
		}

		const Vector3f& duration = anim->GetDuration();

		// Ensure that the specified values are positive
		float fadeInDuration	= Float::Abs(duration.x);
		float playDuration		= Float::Abs(duration.y);
		float fadeOutDuration	= Float::Abs(duration.z);

		// No need for a fade-out duration if the animation is looping
		if (anim->IsLooping()) fadeOutDuration = 0.0f;

		// Animation's combined length includes fade in and fade out times
		float fadeLength		= fadeInDuration + fadeOutDuration;
		float totalDuration		= Float::Max(playDuration, fadeLength);

		// Activate the animation
		activeAnim->Activate((totalDuration == 0.0f) ? 0.0f : (fadeInDuration	/ totalDuration),
							 (totalDuration == 0.0f) ? 0.0f : (playDuration		/ totalDuration),
							 (totalDuration == 0.0f) ? 0.0f : (fadeOutDuration	/ totalDuration),
							  totalDuration, strength);

		// Sync up looping animations
		if (playingAnims->GetSize() > 1)
		{
			ActiveAnimation* previous = playingAnims->Back(1).mActiveAnim;

			if (anim->IsLooping() && previous->mAnimation->IsLooping())
			{
				activeAnim->mSamplingOffset = previous->mSamplingFactor;
			}
		}
	}
	Unlock();
	return PlayResponse::NowPlaying;
}

//============================================================================================================
// Stops the specified animation, fading it out over the specified amount of time
//============================================================================================================

bool Model::StopAnimation (const Animation* anim, float duration, const AnimationEnd& onAnimEnd)
{
	bool retVal = false;

	if (anim != 0 && mAnimLayers.IsValid())
	{
		Lock();
		{
			for (uint i = mAnimLayers.GetSize(); i > 0; )
			{
				PlayingAnims& playingAnims = mAnimLayers[--i].mPlayingAnims;

				if (_StopAnimation(playingAnims, anim, duration, onAnimEnd))
				{
					retVal = true;
					break;
				}
			}
		}
		Unlock();
	}
	return retVal;
}

//============================================================================================================
// Stops all animations playing on the specified layer
//============================================================================================================

bool Model::StopAnimationsOnLayer (uint animationLayer, float duration, const AnimationEnd& onAnimEnd)
{
	bool retVal = false;

	if (mAnimLayers.IsValid())
	{
		Lock();
		{
			for (uint i = mAnimLayers.GetSize(); i > 0; )
			{
				AnimationLayer& layer = mAnimLayers[--i];

				if (layer.mLayer == animationLayer)
				{
					PlayingAnims& playingAnims = layer.mPlayingAnims;
					retVal = _StopAnimation(playingAnims, 0, duration, onAnimEnd);
					break;
				}
			}
		}
		Unlock();
	}
	return retVal;
}

//============================================================================================================
// Stops all active animations, fading them out over the specified amount of time
//============================================================================================================

bool Model::StopAllAnimations (float duration, const AnimationEnd& onAnimEnd)
{
	bool retVal = false;

	if (mAnimLayers.IsValid())
	{
		Lock();
		{
			for (uint i = mAnimLayers.GetSize(); i > 0; )
			{
				PlayingAnims& playingAnims = mAnimLayers[--i].mPlayingAnims;

				if (_StopAnimation(playingAnims, 0, duration, onAnimEnd))
				{
					retVal = true;
				}
			}
		}
		Unlock();
	}
	return retVal;
}

//============================================================================================================
// Returns the amount of time that's left on the specified animation's playback. 0 means the animation
// either finished, or is not active. <0 means it has no end.
//============================================================================================================

float Model::GetTimeToAnimationEnd (const Animation* anim)
{
	float time = 0.0f;

	if (anim != 0)
	{
		Lock();
		{
			for (uint i = mActiveAnims.GetSize(); i > 0; )
			{
				ActiveAnimation* aa = mActiveAnims[--i];

				if (aa != 0 && aa->mAnimation == anim)
				{
					if (aa->mIsActive)
					{
						if (mAnimationSpeed > 0.0f && !anim->IsLooping())
						{
							// Remaining time in seconds
							time = (1.0f - aa->mPlaybackFactor) * aa->mPlaybackDuration / mAnimationSpeed;
						}
						else
						{
							// The animation has no end
							time = -1.0f;
						}
					}
					break;
				}
			}
		}
		Unlock();
	}
	return time;
}

//============================================================================================================
// Special: will either enable or disable skinning on the GPU using shaders
//============================================================================================================

void Model::EnableSkinningOnGPU (bool val)
{
	g_skinOnGPU = val;
}

//============================================================================================================
// Special: will either enable or disable using VBOs for skinning
//============================================================================================================

void Model::EnableSkinningToVBO (bool val)
{
	g_skinToVBO = val;
}