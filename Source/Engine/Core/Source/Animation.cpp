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
				for (uint i = 0; i < bones.GetSize(); ++i)
				{
					const Bone* bone = bones[i];
					ASSERT(bone != 0, "Missing bone?");

					const Bone::PosKeys& posKeys = bone->GetAllPosKeys();
					const Bone::RotKeys& rotKeys = bone->GetAllRotKeys();

					// Add a new animated bone entry
					AnimatedBone& animBone = mBones.Expand();
					animBone.mSmoothV = bone->IsUsingSplinesForPositions();
					animBone.mSmoothQ = bone->IsUsingSplinesForRotations();

					// Add all bone position keys to the animatable node's spline
					for (uint b = 0; b < posKeys.GetSize(); ++b)
					{
						const Bone::PosKey& key = posKeys[b];
						if (key.mFrame < start) continue;
						if (key.mFrame > end  ) continue;

						// Relative time position of this keyframe in 0 to 1 range
						float time = Float::Clamp( invLength * (key.mFrame - start), 0.0f, 1.0f );
						animBone.mSplineV.AddKey(time, key.mPos);
					}

					// Repeat the process with all rotation keys
					for (uint b = 0; b < rotKeys.GetSize(); ++b)
					{
						const Bone::RotKey& key = rotKeys[b];
						if (key.mFrame < start) continue;
						if (key.mFrame > end  ) continue;

						// Relative time position of this keyframe in 0 to 1 range
						float time = Float::Clamp( invLength * (key.mFrame - start), 0.0f, 1.0f );
						animBone.mSplineQ.AddKey(time, key.mRot);
					}
				}

				// If this is a looping animation and we have more than one frame to work with
				if (mLoop && duration > 1)
				{
					// We need to run through the list of animated bones and add
					// the first frame as the last in order to create a seamless animation.

					for (uint i = 0; i < mBones.GetSize(); ++i)
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
	return true;
}