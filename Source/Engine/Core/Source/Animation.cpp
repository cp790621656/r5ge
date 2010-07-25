#include "../Include/_All.h"
using namespace R5;

//============================================================================================================
// Sets the animation's values back to default
//============================================================================================================

void Animation::_Reset()
{
	mId			= 0;
	mFrames		= 0;
	mDuration	= 0.0f;
	mLayer		= 0;
	mLoop		= false;
	mRootIndex	= -1;
	mRootBone.Clear();
}

//============================================================================================================
// Setting the frames implies the need to re-create the splines
//============================================================================================================

void Animation::SetFrames (const Vector2i& val)
{
	if (mFrames != val)
	{
		mFrames = val;

		if (mFrames.x > mFrames.y) Swap(mFrames.x, mFrames.y);

		mBones.Lock();
		mBones.Clear();
		mBones.Unlock();
	}
}

//============================================================================================================
// Helper function that determines whether the specified bone is parented to the specified parent
//============================================================================================================

bool IsLinkedTo (uint index, uint parent, const Animation::Bones& bones)
{
	if (index == parent) return true;
	if (index == -1 || bones.GetSize() < index || bones[index] == 0) return false;
	index = bones[index]->GetParent();
	return IsLinkedTo(index, parent, bones);
}

//============================================================================================================
// Fills the local list of animated bones using the provided list of bones
//============================================================================================================

void Animation::Fill (const Bones& bones)
{
	mIsDirty		= false;
	uint start		= (uint)mFrames.x;
	uint end		= (uint)mFrames.y;
	uint duration	= end - start;

	// Looped animations need to append the first frame as the last in order to remove seams
	if (mLoop && duration > 1) duration += 1;
	float invLength = (duration > 0 ? 1.0f / duration : 0.0f);

	mBones.Lock();
	{
		mBones.Release();

		if (bones.IsValid())
		{
			bones.Lock();
			{
				mBones.ExpandTo(bones.GetSize());

				// Run through all bones and find the root bone
				if (mRootBone.IsValid())
				{
					FOREACH(i, bones)
					{
						const Bone* bone = bones[i];

						// Safety check -- if this is ever triggered then the model's bone list got corrupted
						ASSERT(bone != 0, "Missing bone?");
						if (bone == 0) continue;

						// If this is our root bone, remember its index
						if (bone->GetName() == mRootBone)
						{
							mRootIndex = i;
							break;
						}
					}
				}

				// Run through all bones and create animated bone entries
				FOREACH(i, bones)
				{
					const Bone* bone = bones[i];
					if (bone == 0) continue;

					const Bone::PosKeys& posKeys = bone->GetAllPosKeys();
					const Bone::RotKeys& rotKeys = bone->GetAllRotKeys();

					// Whether we're going to be using spline-based interpolation
					AnimatedBone& animBone = mBones[i];
					animBone.mSmoothV = bone->GetPositionInterpolation();
					animBone.mSmoothQ = bone->GetRotationInterpolation();

					// If we have a root bone for this animation and this bone is not attached to it, skip it
					if (mRootIndex != -1 && !IsLinkedTo(i, mRootIndex, bones)) continue;

					// Add all bone position keys to the animatable node's spline
					for (uint b = 0; b < posKeys.GetSize(); ++b)
					{
						const Bone::PosKey& key = posKeys[b];

						// If we have not yet reached the end of this animation, keep going
						if (key.mFrame < start) continue;

						// If we've passed the end of this animation
						if (key.mFrame > end)
						{
							if (animBone.mSplineV.IsEmpty())
							{
								// Ensure that the spline has something in it so that the bone is not ignored
								animBone.mSplineV.AddKey(0.0f, (b > 0) ? key.mPos : bone->GetPosition());
							}
							else if (!mLoop)
							{
								// If there is no key at the final timestamp, add this key instead
								float t = invLength * ((float)key.mFrame - start);
								animBone.mSplineV.AddKey(t, key.mPos);
							}
							break;
						}

						// If this is the first valid frame, and we've passed the start, add the previous frame
						if (animBone.mSplineV.IsEmpty() && (key.mFrame > start) && (b > 0))
						{
							const Bone::PosKey& prev = posKeys[b-1];
							animBone.mSplineV.AddKey(invLength * ((float)prev.mFrame - start), prev.mPos);
						}

						// Relative time position of this keyframe in 0 to 1 range
						float time = invLength * ((float)key.mFrame - start);
						animBone.mSplineV.AddKey(time, key.mPos);

						// If we've reached the end, we're done
						if (key.mFrame == end) break;
					}

					// Repeat the process with all rotation keys
					for (uint b = 0; b < rotKeys.GetSize(); ++b)
					{
						const Bone::RotKey& key = rotKeys[b];
						if (key.mFrame < start) continue;

						// If we've passed the end of this animation
						if (key.mFrame > end)
						{
							if (animBone.mSplineQ.IsEmpty())
							{
								// Ensure that the spline has something in it so that the bone is not ignored
								animBone.mSplineQ.AddKey(0.0f, (b > 0) ? key.mRot : bone->GetRotation());
							}
							else if (!mLoop)
							{
								// If there is no key at the final timestamp, add this key instead
								animBone.mSplineQ.AddKey(invLength * ((float)key.mFrame - start), key.mRot);
							}
							break;
						}

						// If this is the first valid frame, and we've passed the start, add the previous frame
						if (animBone.mSplineQ.IsEmpty() && (key.mFrame > start) && (b > 0))
						{
							const Bone::RotKey& prev = rotKeys[b-1];
							animBone.mSplineQ.AddKey(invLength * ((float)prev.mFrame - start), prev.mRot);
						}

						// Relative time position of this keyframe in 0 to 1 range
						float time = Float::Clamp( invLength * (key.mFrame - start), 0.0f, 1.0f );
						animBone.mSplineQ.AddKey(time, key.mRot);

						// If we've reached the end, we're done
						if (key.mFrame == end) break;
					}
				}

				// If this is a looping animation and we have more than one frame to work with
				if (mLoop && duration > 1)
				{
					// We need to run through the list of animated bones and add
					// the first frame as the last in order to create a seamless animation.

					FOREACH(i, mBones)
					{
						AnimatedBone& animBone = mBones[i];

						if (animBone.mSplineV.GetSize() > 1)
						{
							animBone.mSplineV.AddKey(1.0f, animBone.mSplineV.GetFirst());
							animBone.mSplineV.SetSeamless(true);
						}

						if (animBone.mSplineQ.GetSize() > 1)
						{
							animBone.mSplineQ.AddKey(1.0f, animBone.mSplineQ.GetFirst());
							animBone.mSplineQ.SetSeamless(true);
						}
					}
				}
			}
			bones.Unlock();
		}
	}
	mBones.Unlock();
}

//============================================================================================================
// Samples the splines at the specified 0-1 range time
//============================================================================================================

uint Animation::Sample (uint boneIndex, float time, Vector3f& pos, Quaternion& rot) const
{
	ASSERT( boneIndex < mBones.GetSize(), "Invalid bone index" );

	const AnimatedBone& ab = mBones[boneIndex];
	uint retVal (0);

	if (ab.mSplineV.IsValid())
	{
		retVal |= 0x1;
		pos = ab.mSplineV.Sample(time, ab.mSmoothV);
	}

	if (ab.mSplineQ.IsValid())
	{
		retVal |= 0x2;
		rot = ab.mSplineQ.Sample(time, ab.mSmoothQ);
	}
	return retVal;
}

//============================================================================================================
// Serialization - Load
//============================================================================================================

bool Animation::SerializeFrom (const TreeNode& root, bool forceUpdate)
{
	for (uint i = 0; i < root.mChildren.GetSize(); ++i)
	{
		const TreeNode& node = root.mChildren[i];

		if		(node.mTag == "Frames")			{ node.mValue >> mFrames;	}
		else if (node.mTag == "Duration")		{ node.mValue >> mDuration;	}
		else if (node.mTag == "Looping")		{ node.mValue >> mLoop;		}
		else if (node.mTag == "Layer")			{ node.mValue >> mLayer;	}
		else if (node.mTag == "Root Bone")		{ node.mValue >> mRootBone; }
	}
	return true;
}

//============================================================================================================
// Serialization - Save
//============================================================================================================

bool Animation::SerializeTo (TreeNode& root) const
{
	TreeNode& node = root.AddChild( Animation::ClassID(), mName );
	node.AddChild("Frames", mFrames);
	node.AddChild("Duration", mDuration);
	node.AddChild("Looping", mLoop);
	node.AddChild("Layer", mLayer);
	if (mRootBone.IsValid()) node.AddChild("Root Bone", mRootBone);
	return true;
}